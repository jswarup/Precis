//_______________________________________________ cv_spritz.h _______________________________________________________________
#pragma once

#include    <iostream>
#include    "cove/silo/cv_cstr.h"

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
    
    virtual   uint64_t      SaveBuffer( uint64_t fOff, const void *buf, uint64_t sz) = 0;
    virtual   void          RestoreBuffer( uint64_t fOff, void *buf, uint64_t sz) = 0;
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
    
    uint64_t    SaveBuffer( uint64_t fOff, const void *buf, uint64_t sz);

    void        RestoreBuffer( uint64_t fOff, void *buf, uint64_t sz);
};

//_____________________________________________________________________________________________________________________________

class Cv_ValidationSpritz : public Cv_Spritz
{   
    Cv_Spritz       *m_WorkSpritz;
    Cv_Spritz       *m_TestSpritz;
      
public:
    Cv_ValidationSpritz( Cv_Spritz *work, Cv_Spritz *test)
        :  m_WorkSpritz( work), m_TestSpritz( test)
    {}   
    
    ~Cv_ValidationSpritz( void);
     
    uint64_t    Size( void) const { return m_WorkSpritz->Size(); }

    uint64_t    Offset( void) const { return m_WorkSpritz->Offset(); }

    uint64_t    SetOffset( uint64_t k) { return m_WorkSpritz->SetOffset( k); }
    
    uint64_t    EnsureSize( uint64_t sz) { return m_WorkSpritz->EnsureSize( sz); }

    bool        Read( void *buf, uint64_t sz);

    bool        Write( const void *buf, uint64_t sz); 
};

//_____________________________________________________________________________________________________________________________
