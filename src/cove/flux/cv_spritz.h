// cv_spritz.h ______________________________________________________________________________________________________________
#pragma once

#include    <iostream>
#include    "cove/silo/cv_array.h"
#include    "cove/silo/cv_cstr.h" 

//_____________________________________________________________________________________________________________________________

template < uint32_t Sz>
class Cv_SpritzArray : public Cv_Array< uint8_t, Sz>, public std::streambuf
{
protected:

    virtual int_type overflow (int_type c) 
    {
        if ( c == EOF)
            return c;
        if ( !this->SzVoid())
            return c;
        this->Append( c);
        return c;
    }

public:
    Cv_SpritzArray( void)
    {}
};

//_____________________________________________________________________________________________________________________________

class Cv_Spritz 
{
public:
    void       FillNull( uint64_t sz) ;

    virtual    uint64_t    Size( void) const = 0; 

    virtual    uint64_t    Offset( void) const = 0;
    
    virtual    uint64_t     SetOffset( uint64_t k) = 0;

    void        SetOffsetAtEnd( void) {  SetOffset( CV_UINT64_MAX); }
    
    virtual    uint64_t     EnsureSize( uint64_t sz) = 0;
    
    virtual    bool         Read( void *buf, uint64_t sz) = 0;
    
    virtual    bool         Write( const void *buf, uint64_t sz) = 0;

    virtual    void         FlushAll( void)  = 0;
     
}; 

//_____________________________________________________________________________________________________________________________

class Cv_FileSpritz : public Cv_Spritz
{
public:
    enum Facet  
    {
        ReadOnly    =  0x0,
        WriteRead   = ( 0x1 << 0),
        WriteTrim   =  ( 0x1 << 1) | WriteRead,
    };
protected:
    FILE        *m_Fp;
    Facet       m_Facet;
    uint64_t    m_Offset;
    
public:
    Cv_FileSpritz( const Cv_CStr &fname, Facet facet);
    ~Cv_FileSpritz( void);

    uint64_t    Size( void) const;

    uint64_t    Offset( void) const { return m_Offset; }

    uint64_t    SetOffset( uint64_t k);
    
    uint64_t    EnsureSize( uint64_t sz);

    bool        Read( void *buf, uint64_t sz);

    bool        Write( const void *buf, uint64_t sz);

    void        FlushAll( void);
     
};

//_____________________________________________________________________________________________________________________________

class Cv_MemSpritz : public Cv_Spritz
{
public:
    enum Facet  
    {
        ReadOnly    =  0x0,
        WriteRead   = ( 0x1 << 0),
        WriteTrim   =  ( 0x1 << 1) | WriteRead,
    };

protected:
    std::vector< uint8_t>       m_Mem;
    Facet                       m_Facet;
    uint64_t                    m_Offset;
    
public:
    Cv_MemSpritz( const Cv_CStr &fname, Facet facet);
    ~Cv_MemSpritz( void);

    uint64_t    Size( void) const;

    uint64_t    Offset( void) const { return m_Offset; }

    uint64_t    SetOffset( uint64_t k);
    
    uint64_t    EnsureSize( uint64_t sz);

    bool        Read( void *buf, uint64_t sz);

    bool        Write( const void *buf, uint64_t sz);

    void        FlushAll( void);
     
    bool        CheckFill(  uint64_t sz);
 
};


//_____________________________________________________________________________________________________________________________

class Cv_ValidationSpritz : public Cv_Spritz
{   
    Cv_Spritz           *m_WorkSpritz;
    Cv_MemSpritz        m_TestSpritz;
      
public:
    Cv_ValidationSpritz( Cv_Spritz *work)
        :  m_WorkSpritz( work), m_TestSpritz( "test", Cv_MemSpritz::WriteRead)
    {}   
    
    ~Cv_ValidationSpritz( void);
     
    uint64_t    Size( void) const { return m_WorkSpritz->Size(); }

    uint64_t    Offset( void) const { return m_WorkSpritz->Offset(); }

    uint64_t    SetOffset( uint64_t k) { m_TestSpritz.SetOffset( k); return m_WorkSpritz->SetOffset( k); }
    
    uint64_t    EnsureSize( uint64_t sz) {  m_TestSpritz.EnsureSize( sz); return m_WorkSpritz->EnsureSize( sz);}

    bool        Read( void *buf, uint64_t sz);

    bool        Write( const void *buf, uint64_t sz); 

    void        FlushAll( void) {}
};

//_____________________________________________________________________________________________________________________________
 

//_____________________________________________________________________________________________________________________________
