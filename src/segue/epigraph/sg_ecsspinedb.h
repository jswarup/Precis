// sg_ecsspinedb.h ______________________________________________________________________________________________________________ 
#pragma once
 

#include    "segue/epigraph/sg_ecoptimize.h" 

//_____________________________________________________________________________________________________________________________ 
 

class Sg_ECSpineDB 
{
    typedef std::set< Sg_ECSpine *>   ECDistrbSet;

    Sg_BaseSpine                      m_FlexBase;
    std::vector< Sg_ECSpine *>        m_FlexECs;
    std::vector< Sg_ChSet>                m_CCLs;
    std::vector< uint16_t>              m_Reps;
    ECDistrbSet                         m_ECDB;
    std::vector< Sg_BaseSpine*>       m_BaseDistribs;
    
public: 
    Sg_ECSpineDB( void);
    ~Sg_ECSpineDB( void);

    void                                Setup( void);

    Sg_BaseSpine                      *FlexBase( void) { return &m_FlexBase; }
    const Sg_ChSet                        &CCL( uint32_t k) const { return m_CCLs[ k]; }
    uint16_t                            GetRep( uint32_t k) const { return m_Reps[ k]; }
    
    Sg_ECSpine                        *FetchDistrib( Sg_ECSpine *ecDistrib);
    void                                RemoveDistrib( Sg_ECSpine *ecDistrib);

    uint32_t                            SzDistrib( void) const { return uint32_t( m_BaseDistribs.size()); }
    Sg_BaseSpine                      *BaseDistrib( uint32_t k)  { return m_BaseDistribs[ k]; }
    
    const std::vector< Sg_BaseSpine*> &BaseDistribs( void)  const { return m_BaseDistribs; }

    void                                OptimizeBasis( uint32_t szBluts);
    
    std::vector< Sg_ECSpine*>         ComputeECDistribs( void);
    
    std::string                         ToString( void) const;
};

//_____________________________________________________________________________________________________________________________ 



