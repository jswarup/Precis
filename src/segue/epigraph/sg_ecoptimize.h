// sg_ecoptimize.h ______________________________________________________________________________________________________________ 
#pragma once
 
#include    "cove/silo/cv_repos.h"
#include    "segue/epigraph/sg_spine.h" 

//_____________________________________________________________________________________________________________________________ 

class Sg_ECDistribSteward : public Cv_ReposEntry
{
    Sg_ECSpine            *m_ECDistrib;           // utimately we will have updated it.
    Sg_BaseSpine          *m_Base;                // current base
    uint16_t                m_ImgSize;              // what is size of its image, least destinations to be encoded. Computational Intensive => save 
    Sg_CharSpine          m_CharDistrib;          // our current partition
    Sg_BaseSpine         *m_DreamBase;
    uint16_t                m_WasteOnDream;
    
public:
    Sg_ECDistribSteward( uint32_t id)
        :   Cv_ReposEntry( id),
            m_ECDistrib( NULL), 
            m_Base( NULL),
            m_ImgSize( CV_UINT16_MAX), 
            m_CharDistrib( ),
            m_DreamBase(  NULL),
            m_WasteOnDream( CV_UINT16_MAX)
    {}
    
    void    Init( Sg_ECSpine *ecd, Sg_BaseSpine *base);

    Sg_CharSpine          *CharDistrib( void) { return &m_CharDistrib; }
    Sg_BaseSpine          *CurrentBase( void) const { return m_Base; }
    Sg_BaseSpine          *DreamBase( void) const { return m_DreamBase; }

    void            UpdateDream( Sg_BaseSpine *base);
    void            RecomputeJiltedBase( void);
    uint32_t        CurrentCost( void) const { return ( m_Base->SzImage()  * m_ECDistrib->SzRef()); }
    uint32_t        BestCost( void) const { return ( m_ImgSize  * m_ECDistrib->SzRef()); }
    uint32_t        CurrentWaste( void) const { return  CurrentCost() -BestCost(); }
    int32_t         ProjectedSavings( void) const { return CurrentWaste() - m_WasteOnDream; }
    

    class PtrCmp
    {
    public:
        bool operator()( const Sg_ECDistribSteward *x, const Sg_ECDistribSteward *y) const
        {
            int32_t     xSave = x->ProjectedSavings();
            int32_t     ySave = y->ProjectedSavings();
            return ( xSave != ySave) ? ( xSave > ySave) : ( x->GetId() < y->GetId());
        }
    };

    void            Migrate( Sg_BaseSpine *sBase);
    bool            TestAndMigrateToDream( void);
    void            SwitchECBase( void);
};

//_____________________________________________________________________________________________________________________________ 

class Sg_ECTableOptimize
{
    std::vector< Sg_BaseSpine*>           m_BaseDistribs;
    std::vector< Sg_ECDistribSteward *>     m_ECStewards;

public:
    Sg_ECTableOptimize( void);
    ~Sg_ECTableOptimize( void);

    uint32_t        CurrentCost( void) const;
    uint32_t        CurrentWaste( void) const;

    bool            MigrateWinner( void);
    void            CleanJilted( void);

    bool            OptimizeOnce( uint32_t round);
    void            DoOptimize( uint32_t SzBlut);
    void            DoOptimizeFull( uint32_t SzBlut);
    void            Seed( const std::vector< Sg_ECSpine *> &flexECs);
    
    const std::vector< Sg_BaseSpine*> &BaseDistribs( void) const { return m_BaseDistribs; }

};
//_____________________________________________________________________________________________________________________________ 
