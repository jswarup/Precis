//___________________________________________________ cv_ringbuf.h _________________________________________________________
#pragma once
 
#include    <cinttypes>
#include    <cstdio> 
 
//_____________________________________________________________________________________________________________________________ 

template<typename Ty> 
class   Tr_RingBuffer 
{
public:
    enum {
        Sz = 2048,
    };
    typedef Ty                  Type;
    typedef Tr_RingBuffer< Ty>  ThisClass;
    
    
private:    
    std::atomic< uint32_t>  m_WriteInd; 
    uint32_t                m_ReadableSz;
       
    Type                    m_Buffer[ Sz] alignas( CV_CACHELINE_SIZE);   
    
    std::atomic< uint32_t>  m_ReadInd alignas( CV_CACHELINE_SIZE);  
    uint32_t                m_WriteableSz; 
    
public: 
    Tr_RingBuffer()  
        : m_ReadableSz( 0), m_WriteableSz( 0)
    {
        m_WriteInd = 0;
        m_ReadInd = 0;
    }
        
    ~Tr_RingBuffer()
    {}
    
    static  Tr_RingBuffer   *Create( const char *name, uint32_t sz, uint32_t sockId, uint32_t flags) 
    {   
        void    *buffer = NULL;;
        int     rc = posix_memalign(&buffer, CV_CACHELINE_SIZE, sizeof ( Tr_RingBuffer< Type>));
        if ( rc != 0)
            return NULL; 
        return new (buffer) Tr_RingBuffer< Type>(); 
    }
    void                Discard( void) 
    { 
        delete this; 
    }
    
    uint32_t            Count( void) const 
    { 
        uint32_t    readInd = m_ReadInd.load();
        uint32_t    writeInd = m_WriteInd.load();
        return uint32_t( (Sz - readInd + writeInd) % Sz);
    }
    
    uint32_t            FreeCount( void) const 
    { 
        uint32_t    readInd = m_ReadInd.load();
        uint32_t    writeInd = m_WriteInd.load();
        return uint32_t( (Sz - 1 - writeInd + readInd) % Sz); 
    }
    
    const Type  &Get( uint32_t ind) const { return m_Buffer[ ind]; }
    Type        &Get( uint32_t ind) { return m_Buffer[ ind]; }
    
    void        Set( uint32_t ind, const Type &val) { m_Buffer[ ind] = val; }
    
    auto    ReserveRead( uint32_t dataSz)
    {
        if ( dataSz >  Sz)
            dataSz = Sz;
        uint32_t    readInd = m_ReadInd.load();
        if ( m_ReadableSz < dataSz)
        { 
            m_ReadableSz = (Sz - readInd + m_WriteInd) % Sz;
            if ( m_ReadableSz < dataSz)
                dataSz = m_ReadableSz;
        }
        m_ReadableSz -= dataSz;
        uint32_t    bytes_read1 = std::min( dataSz, Sz - readInd); 
        return std::make_tuple( readInd, bytes_read1, dataSz);
    }   
    
    void    DereserveRead(  uint32_t readInd, uint32_t dataSz)
    {
        m_ReadInd = ( readInd + dataSz) % Sz;
    }
    
    auto    ReserveWrite( uint32_t dataSz)
    {
        uint32_t    writeInd = m_WriteInd.load();
        if ( m_WriteableSz < dataSz)
        {
            m_WriteableSz = (Sz - 1 - writeInd + m_ReadInd.load()) % Sz;  
            if ( m_WriteableSz < dataSz)
                dataSz = m_WriteableSz;
        }
        
        m_WriteableSz -= dataSz;
        uint32_t    bytesWr1 = Sz - writeInd;  
        if ( bytesWr1 > dataSz)
            bytesWr1 = dataSz;
        return std::make_tuple( writeInd, bytesWr1, dataSz);
    }
    
    void    DereserveWrite(  uint32_t writeInd, uint32_t dataSz)
    {
        m_WriteInd =( writeInd + dataSz) % Sz;
    }
    
private:
    uint32_t    Read( Type  *data, uint32_t dataSz)
    {  
        //CV_PREFETCH_CACHE( &m_Buffer[ 0])
        //CV_PREFETCH_CACHE( &m_Buffer[ readInd])
        uint32_t    readInd = 0;
        uint32_t    bytes_read1 = 0;
        std::tie( readInd, bytes_read1, dataSz) = ReserveRead( dataSz);
        
        std::copy( &m_Buffer[ 0], &m_Buffer[ dataSz - bytes_read1], &data[ bytes_read1]);
        std::copy( &m_Buffer[ readInd], &m_Buffer[ readInd +bytes_read1], &data[ 0]); 
        
        DereserveRead( readInd, dataSz);
        return dataSz;
    }

    uint32_t    Write( Type   *data, uint32_t dataSz)
    {  
        uint32_t    writeInd = 0;
        uint32_t    bytesWr1 = 0;  
        std::tie( writeInd, bytesWr1, dataSz) = ReserveWrite( dataSz);
        
        std::copy( &data[ 0], &data[ bytesWr1], &m_Buffer[ writeInd]); 
        std::copy( &data[ bytesWr1], &data[ dataSz], &m_Buffer[ 0]);
        
        DereserveWrite( writeInd, dataSz);
        return dataSz;
    }     
    
};

//_____________________________________________________________________________________________________________________________

template < class RingBuf>
struct Cv_Reader
{
protected: 
    typedef typename RingBuf::Type          Type;
    
    RingBuf                                 *m_SourceRing; 
    uint32_t                                readInd;    
    uint32_t                                bytes_read1; 
    uint32_t                                m_DataSz;
    uint32_t                                m_Ind;
  
    uint32_t    RingInd( uint32_t ind) {   return ( m_Ind < bytes_read1) ? readInd +m_Ind : m_Ind - bytes_read1;   }
    
public:
    Cv_Reader( RingBuf *ringBuf = NULL)
        :   m_SourceRing( ringBuf), readInd( CV_UINT32_MAX), m_DataSz( 0), m_Ind( 0) 
    {}
    
    void        SetRing( RingBuf *ringBuf) {  m_SourceRing = ringBuf; }
    bool        IsDrained( void) const { return ( m_Ind == m_DataSz); }
    
    const Type  &GetAt( uint32_t ind) { return m_SourceRing->Get( RingInd( m_Ind));  }    
    void        SetAt( uint32_t ind, const Type &val) { m_SourceRing->Set( RingInd( ind), val); }
    
    bool        Fetch( Type *pVal, uint32_t reservedSz = 32) 
    { 
        if ( IsDrained() && !Reload( reservedSz)) 
            return false;
        *pVal = m_SourceRing->Get( RingInd( m_Ind)); 
         ++m_Ind;
        return true;   
        
    }
    
    uint32_t    Reload( uint32_t reserveSz)
    {
        Unload();
        m_Ind = 0;
        m_DataSz = reserveSz;                 
        std::tie( readInd, bytes_read1, m_DataSz) = m_SourceRing->ReserveRead( m_DataSz); 
         
        return m_DataSz;
    }
     
    
    void    Unload( void)
    {
        if ( readInd != CV_UINT32_MAX)
            m_SourceRing->DereserveRead( readInd, m_DataSz); 
    }
    
    ~Cv_Reader( void)
    { 
        Unload();         
    }
    
};
 
//_____________________________________________________________________________________________________________________________

template <  class RingBuf>
class Cv_Writer
{
protected: 
    typedef typename RingBuf::Type          Type;
    
    RingBuf                                 *m_SinkRing; 
    uint32_t                                m_WriteInd;    
    uint32_t                                bytesWr1; 
    uint32_t                                m_DataSz;
    uint32_t                                m_Ind;
    
   uint32_t     RingInd( uint32_t ind) {   return ( m_Ind < bytesWr1) ? m_WriteInd +m_Ind : m_Ind -bytesWr1;   }
   
 public:
    Cv_Writer( RingBuf *ringBuf = NULL)
        :   m_SinkRing( ringBuf), m_WriteInd( CV_UINT32_MAX), m_DataSz( 0), m_Ind( 0)
    {} 
     
    void            SetRing( RingBuf *ringBuf) {  m_SinkRing = ringBuf; }
        
    bool            IsDrained( void) const { return ( m_Ind == m_DataSz); }
    
    bool            Put( const Type &val, uint32_t reservedSz = 32) 
    { 
        if ( IsDrained() && !Reserve( reservedSz)) 
            return false;
        SetAt( m_Ind, val);
        ++m_Ind;
        return true;
    }
   
    const Type  &GetAt( uint32_t ind) { return m_SinkRing->Get( RingInd( m_Ind));  }    
    void        SetAt( uint32_t ind, const Type &val) { m_SinkRing->Set( RingInd( ind), val); } 
    
    bool        Unput(  Type *pVal = NULL) 
    { 
        if ( !m_Ind)
            return false;
        --m_Ind;
        if ( pVal) 
            *pVal = GetAt( m_Ind); 
        return true;
    }
     
    uint32_t Reserve(  uint32_t dataSz) 
    { 
        Unload();
        
        m_DataSz = dataSz;   
        m_Ind = 0;
        std::tie( m_WriteInd, bytesWr1, m_DataSz) = m_SinkRing->ReserveWrite( m_DataSz);        
        return m_DataSz;
    }
    
    void    Unload( void)
    {
        if ( m_WriteInd != CV_UINT32_MAX)
            m_SinkRing->DereserveWrite( m_WriteInd, m_DataSz); 
    }
    
    ~Cv_Writer( void)
    { 
        Unload();
    } 
};

//_____________________________________________________________________________________________________________________________

template < class RingBuf>
struct Cv_MultiReader
{
protected: 
    typedef typename RingBuf::Type          Type;
    
    std::vector< Cv_Reader< RingBuf> >      m_SourceReaders; 
    uint32_t                                m_SInd;
    
public:
    Cv_MultiReader( const std::vector< RingBuf *> &ringBufs)
        :   m_SourceReaders( ringBufs.size()), m_SInd( CV_UINT32_MAX)
    {
        for ( uint32_t i = 0; i < ringBufs.size(); ++i)
            m_SourceReaders[ i].SetRing( ringBufs[ i]);
    }
    
    bool        IsDrained( void) const { return  m_SourceReaders[ m_SInd].IsDrained(); }
    
    bool        Put( const Type &val, uint32_t reservedSz = 32) 
    { 
        if ( IsDrained() && !Reload( reservedSz)) 
            return false;
        return m_SourceReaders[ m_SInd].Put( val, reservedSz);
    }
   
    const Type  &GetAt( uint32_t ind) { return m_SourceReaders[ m_SInd].Get( ind);  }    
    void        SetAt( uint32_t ind, const Type &val) { m_SourceReaders[ m_SInd].Set( ind, val); } 
    
    bool        Unput(  Type *pVal = NULL)  {  return m_SourceReaders[ m_SInd].Unput( pVal); }
    
    uint32_t Reload(  uint32_t dataSz) 
    { 
        Unload();
                
        for ( uint32_t i = 0; i < m_SourceReaders.size(); ++i)
        {
            m_SInd = ( m_SInd +1) % m_SourceReaders.size();
            uint32_t    dSz = m_SourceReaders[ m_SInd].Reserve( dataSz);
            if ( dSz)
                return dSz;
        }        
        return 0;
    }
        
    void    Unload( void)
    {
        if ( m_SInd != CV_UINT32_MAX)
            m_SourceReaders[ m_SourceReaders].Unload(); 
    }
};

//_____________________________________________________________________________________________________________________________
