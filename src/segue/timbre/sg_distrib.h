// sg_distrib.h ______________________________________________________________________________________________________________ 
#pragma once
 
#include    "segue/timbre/sg_chset.h" 

//_____________________________________________________________________________________________________________________________ 

class Fs_BaseDistrib;

//_____________________________________________________________________________________________________________________________ 

struct Fs_SubsetDesc 
{
    uint16_t  first;
    uint16_t  weight;

public:
    Fs_SubsetDesc( uint16_t f = 0)
        :   first( f), weight( 0)
    {}

    int32_t             Compare( const Fs_SubsetDesc &sd) const;
};

//_____________________________________________________________________________________________________________________________ 

class Fs_Distrib
{
public:
    virtual ~Fs_Distrib( void);

    virtual uint16_t                Image( uint16_t t) const = 0;
    virtual uint32_t                SzImage( void) const = 0;

    virtual std::vector< Sg_ChSet>    Domain( void) const = 0;
    std::string                     ToString( void) const; 

};

//_____________________________________________________________________________________________________________________________ 

class Fs_CharDistrib : public Fs_Distrib
{
protected:
    uint16_t        m_SzEqClass;
    uint16_t        m_EqClassIds[ Sg_ChSet::SzChBits];        // list the equivalence class the entry belongs : [0, m_SzEqClass)
    
public:
    Fs_CharDistrib( void);
    
    
    uint16_t        Image( uint16_t t) const {  return m_EqClassIds[ t]; }
    void            SetImage( uint32_t k, uint16_t grId) {  m_EqClassIds[ k] = grId; }

    uint32_t        SzImage( void) const { return m_SzEqClass; }
    void            SetSzImage( uint32_t sz) { m_SzEqClass = sz; }

    bool            IsRep( uint32_t k) const;
    
    
    int32_t         Compare( const Fs_CharDistrib &cd) const;
    
    std::vector< Fs_SubsetDesc>     SubsetDescs( void) const;

    void            MergeClass( uint16_t eq1, uint16_t eq2);

    // Makes record into trivial partition (single subset)
    void            MakeUniversal( void);

    // Makes record into discrete partition (maximum subsets)
    void            MakeDiscrete(  void);

    
    // Number of subsets with at least one real character (<256)
    uint16_t        NumRealSubsets( void)  const;

    bool            IsCutByCCL( const Sg_ChSet &ccl) const;
    bool            IsCutByCCL( const Sg_ChSet &ccl, uint32_t first, uint32_t wt) const;
     // Extracts the partition subset containing a given character
    Sg_ChSet          ClassContainingChar( uint16_t c) const { return EqClassCCL( Image( c)); }
    Sg_ChSet          EqClassCCL( uint16_t grId) const;
    
    // Checks if two characters are in the same subset
    bool            CharsInSameSubset( uint16_t c1, uint16_t c2) const { return Image( c1) == Image( c2); }

    // Intersects a new Sg_ChSet with all subsets in a partition, making it finer.
    // Returns true if anything changed (equivalent to partitionCutByCCL return value).
    Fs_CharDistrib  Impression( const Fs_CharDistrib &q) const;
    void            ImpressWith( const Fs_CharDistrib &q);

    // Returns true if every subset in the first partition is contained in some subset in the second
    // (including when the partitions are equal).  If not finer, a sample character C will be
    // placed at sampleChar (if not NULL), such that the p subset containing C is not contained
    // in any subset of q.
    bool            TestFiner( const Fs_CharDistrib &q, int *sampleChar) const;
   
    std::vector< Sg_ChSet>    Domain( void) const;

    
    class CCLImpressCntl
    {
        const Sg_ChSet    *m_CCLs;

template < uint32_t N>          //  N < 7
        uint16_t        EqClassCode( uint32_t k) const;


    public:
        CCLImpressCntl( const Sg_ChSet *ccls)
            :   m_CCLs( ccls)
        {}

template < uint32_t N> 
        void            ImpressWith( Fs_CharDistrib *distrib)
        {
            uint16_t    grMap[ Sg_ChSet::SzChBits << N];                   // keep a map if the group has been encountered.
            memset( grMap, -1, sizeof( grMap));
    
            uint16_t    newGr = 0;
            for ( uint16_t i = 0; i < Sg_ChSet::SzChBits; ++i) 
            {
                uint16_t    *pEqClassId = &distrib->m_EqClassIds[ i];
                // potentially any group group can split.  one part in ccl and other outside. get the code for new group.
                uint16_t    eqClassCode = ( *pEqClassId << N) | EqClassCode< N>( i);     
                uint16_t    *pMap = &grMap[ eqClassCode];                   // get location where we keep ids about these groups.
                if ( *pMap == CV_UINT16_MAX) 
                    *pMap = newGr++;                                        // assign a new id.
                *pEqClassId = *pMap;                                        // reuse id if we have encountered them before.
            }
            distrib->m_SzEqClass = newGr;
            return;
        }
    };

    //_____________________________________________________________________________________________________________________________ 

    class CCLImpressor 
    {
    protected:
        Fs_CharDistrib          *m_Distrib;
        Sg_ChSet                  m_CCLs[ 7];
        uint32_t                m_Ind;
        Sg_ChSet                  m_ValidCCL; 

    public:
        CCLImpressor( Fs_CharDistrib *distrib);

        const Sg_ChSet    &ValidCCL( void) const { return m_ValidCCL; }
    
        bool            Process( const Sg_ChSet &ccl);
        void            Over( void);
    };

    // Intersects a new Sg_ChSet with all subsets in a partition, making it finer.
    // Returns 1 if anything changed (equivalent to partitionCutByCCL return value).

    void            ImpressCCL(  const Sg_ChSet & ccl)  {  CCLImpressCntl( &ccl).ImpressWith< 1>( this); }

     //_____________________________________________________________________________________________________________________________ 

    class CCLIdImpressor : public CCLImpressor 
    {
        std::set< uint32_t>         m_Processed;

    public:
        CCLIdImpressor( Fs_CharDistrib *distrib)
            : CCLImpressor( distrib)
        {}

        bool            Process( const Sg_ChSet &ccl, uint32_t id);
    };
};

template < uint32_t N>          //  N < 7
inline uint16_t        Fs_CharDistrib::CCLImpressCntl::EqClassCode( uint32_t k) const { return ( uint16_t( m_CCLs->Get( k)) << ( N -1)) | CCLImpressCntl( m_CCLs +1).EqClassCode< N -1>( k);  }

template <>          
inline uint16_t        Fs_CharDistrib::CCLImpressCntl::EqClassCode< 1>( uint32_t k) const { return uint16_t( m_CCLs->Get( k));  }


//_____________________________________________________________________________________________________________________________ 

class Fs_BaseDistrib :  public Fs_CharDistrib
{
protected:
    std::vector< Sg_ChSet>    m_Domain;
    
public: 
    Fs_BaseDistrib( uint32_t id = 0) 
    {}

    void                    Freeze( void);
    void                    MergeClass( uint16_t eq1, uint16_t eq2);

    std::vector< Sg_ChSet>    Domain( void) const;

    const Sg_ChSet            &EqClassCCL( uint16_t grId) const { return m_Domain[ grId]; }
};

//_____________________________________________________________________________________________________________________________ 

class Fs_ECDistrib : public Fs_Distrib
{
protected:
    uint32_t                m_RefCount;         // NumTotalRefs
    const Fs_BaseDistrib    *m_BaseDistrib;     
    uint16_t                *m_Map;             // map over the equivalence class ids.
    uint32_t                m_Hashval;    
    bool                    m_LockFlg;          // Blut Migration is inhibited.
    bool                    m_ZeroFailFlg;

public:
    Fs_ECDistrib( const Fs_BaseDistrib *distrib);

    Fs_ECDistrib( const Fs_ECDistrib &ecd);

    ~Fs_ECDistrib( void);

    uint32_t        SzRef( void) const { return m_RefCount; }
    void            RaiseRef( void) {  ++m_RefCount; }
    bool            LowerRef( void) {  return --m_RefCount == 0; }
    
    uint32_t        GetHash( void) const { return m_Hashval; }
    void            SetHash( uint32_t h) { m_Hashval = h; }
    
    bool            IsLock( void) const { return m_LockFlg; }
    void            SetLock( bool t) { m_LockFlg = t; }

    bool            IsZeroFail( void) const { return m_ZeroFailFlg; }
    void            SetZeroFail( bool t) { m_ZeroFailFlg = t; }

    const Fs_BaseDistrib  *BaseDistrib( void) const { return m_BaseDistrib; }

    uint16_t        Image( uint16_t t) const {  return m_Map[ m_BaseDistrib->Image( t)]; }
    uint32_t        SzImage( void) const;
    
    std::vector< Sg_ChSet> Domain( void) const;

    uint32_t        Size( void) const { return  m_BaseDistrib->SzImage(); }
	void            SetAt( uint32_t k, uint16_t	t) { m_Hashval += t -m_Map[ k]; m_Map[ k] = t; }
	const uint16_t  &At( uint32_t k)  const	{  return  m_Map[ k]; } 


    uint32_t        Thickness( void) const;      //  in-use ECs spanned in BaseDistrib
    
    bool            IsEqual( const Fs_ECDistrib &ecd) const;
    bool            operator<( const Fs_ECDistrib &s) const;

    Fs_CharDistrib  CharDistrib( void) const;

    void            SwitchBase( Fs_BaseDistrib *base);

};

//_____________________________________________________________________________________________________________________________  
