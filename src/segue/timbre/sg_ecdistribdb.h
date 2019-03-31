// sg_ecdistribdb.h ______________________________________________________________________________________________________________ 
#pragma once
 

#include    "segue/timbre/sg_ecoptimize.h" 

//_____________________________________________________________________________________________________________________________ 
 

class Sg_ECDistrbDB 
{
    typedef std::set< Sg_ECDistrib *>   ECDistrbSet;

    Sg_BaseDistrib                      m_FlexBase;
    std::vector< Sg_ECDistrib *>        m_FlexECs;
    std::vector< Sg_ChSet>                m_CCLs;
    std::vector< uint16_t>              m_Reps;
    ECDistrbSet                         m_ECDB;
    std::vector< Sg_BaseDistrib*>       m_BaseDistribs;
    
public: 
    Sg_ECDistrbDB( void);
    ~Sg_ECDistrbDB( void);

    void                                Setup( void);

    Sg_BaseDistrib                      *FlexBase( void) { return &m_FlexBase; }
    const Sg_ChSet                        &CCL( uint32_t k) const { return m_CCLs[ k]; }
    uint16_t                            GetRep( uint32_t k) const { return m_Reps[ k]; }
    
    Sg_ECDistrib                        *FetchDistrib( Sg_ECDistrib *ecDistrib);
    void                                RemoveDistrib( Sg_ECDistrib *ecDistrib);

    uint32_t                            SzDistrib( void) const { return uint32_t( m_BaseDistribs.size()); }
    Sg_BaseDistrib                      *BaseDistrib( uint32_t k)  { return m_BaseDistribs[ k]; }
    
    const std::vector< Sg_BaseDistrib*> &BaseDistribs( void)  const { return m_BaseDistribs; }

    void                                OptimizeBasis( uint32_t szBluts);
    
    std::vector< Sg_ECDistrib*>         ComputeECDistribs( void);
    
    std::string                         ToString( void) const;
};

//_____________________________________________________________________________________________________________________________ 



