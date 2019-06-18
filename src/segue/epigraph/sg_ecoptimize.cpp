// sg_ecoptimize.cpp ______________________________________________________________________________________________________________ 


#include    "segue/tenor/sg_include.h"
#include    "segue/epigraph/sg_ecoptimize.h"
  
//_____________________________________________________________________________________________________________________________ 

void Sg_ECDistribSteward::Init( Sg_ECDistrib *ecd, Sg_BaseDistrib *base)
{
    m_ECDistrib = ecd; 
    m_Base = base;
    m_ImgSize = ecd->SzImage(); 
    m_CharDistrib = ecd->CharDistrib();
    m_DreamBase = base;
    m_WasteOnDream = CV_UINT16_MAX;
    return;
}

//_____________________________________________________________________________________________________________________________ 
// this base changed, how am it affected

void    Sg_ECDistribSteward::UpdateDream( Sg_BaseDistrib *base)
{
    if (( base == m_Base) || m_ECDistrib->IsLock())
        return;

    if ( m_WasteOnDream == CV_UINT16_MAX)
    {
        m_WasteOnDream = CurrentWaste();
        if ( m_WasteOnDream == 0)                           // we are not moving anywhere 
            return;
    }

    Sg_CharDistrib          dist( m_CharDistrib.Impression( *base));

    uint32_t    wasteOnBase =  ( dist.SzImage() -m_ImgSize) * m_ECDistrib->SzRef();
    if ( wasteOnBase < m_WasteOnDream)
    {
        m_DreamBase = base;
        m_WasteOnDream = wasteOnBase;
    }
    return;
}

//_____________________________________________________________________________________________________________________________ 
// this base changed, how am it affected

void    Sg_ECDistribSteward::RecomputeJiltedBase( void)
{
    m_Base->ImpressWith( m_CharDistrib);
    m_WasteOnDream = CV_UINT16_MAX;
    return;
}

//_____________________________________________________________________________________________________________________________ 

void    Sg_ECDistribSteward::Migrate( Sg_BaseDistrib *sBase)
{
    CV_ERROR_ASSERT( m_DreamBase == sBase)
    sBase->ImpressWith( m_CharDistrib);
    m_Base = sBase;
    m_WasteOnDream = CV_UINT16_MAX;
    return;
}

//_____________________________________________________________________________________________________________________________ 

bool   Sg_ECDistribSteward::TestAndMigrateToDream( void)
{ 
    uint32_t        projectedSavings = ProjectedSavings();
    m_WasteOnDream = CV_UINT16_MAX;

    if ( projectedSavings <= 0)                                                           // dream gone bad
        return false;
    
    if ( !m_DreamBase->TestFiner( m_CharDistrib, NULL))
        return true;

    uint32_t    wasteOnBase = ( m_DreamBase->SzImage() -m_ImgSize) * m_ECDistrib->SzRef();
    uint32_t    c1 = CurrentWaste();
    if ( c1 < wasteOnBase)
        return true;
    m_Base = m_DreamBase;
 //   AX_TRACE(( "          Sidekicks[ %d]  BeginWaste[ %d] EndWaste[ %d] %s\n", GetId(), c1, CurrentWaste(), CharDistrib()->ToString().c_str()))
    return true;
}

//_____________________________________________________________________________________________________________________________ 

void   Sg_ECDistribSteward::SwitchECBase( void)
{
    m_ECDistrib->SwitchBase( m_Base);
    return;
}

//_____________________________________________________________________________________________________________________________ 

Sg_ECTableOptimize::Sg_ECTableOptimize( void)
    : m_BaseDistribs()
{
}

//_____________________________________________________________________________________________________________________________ 

Sg_ECTableOptimize::~Sg_ECTableOptimize( void)
{
    for ( uint32_t i = 0; i < m_ECStewards.size(); ++i)
        delete m_ECStewards[ i];
}

//_____________________________________________________________________________________________________________________________ 

void    Sg_ECTableOptimize::Seed( const std::vector< Sg_ECDistrib *> &flexECs)
{
    m_BaseDistribs.push_back( new Sg_BaseDistrib( *flexECs [ 0]->BaseDistrib()));

    m_ECStewards.resize( flexECs.size());
    for ( uint32_t i = 0; i < flexECs.size(); ++i)
    {
        Sg_ECDistribSteward     *ecStwd = m_ECStewards[ i] = new Sg_ECDistribSteward( i);
        ecStwd->Init( flexECs[ i], m_BaseDistribs[ 0]);
    }
    return;
}

//_____________________________________________________________________________________________________________________________ 

uint32_t    Sg_ECTableOptimize::CurrentCost( void) const
{
    uint32_t    cost = 0;
    for ( uint32_t i = 0; i < m_ECStewards.size(); ++i)
    {        
        Sg_ECDistribSteward        *ecs = m_ECStewards[ i];
        cost += ecs->CurrentCost();
    }
    return cost;
}

//_____________________________________________________________________________________________________________________________ 

uint32_t    Sg_ECTableOptimize::CurrentWaste( void) const
{
    uint32_t    waste = 0;
    for ( uint32_t i = 0; i < m_ECStewards.size(); ++i)
    {        
        Sg_ECDistribSteward        *ecs = m_ECStewards[ i];
        waste += ecs->CurrentWaste();
    }
    return waste;
}

//_____________________________________________________________________________________________________________________________ 

bool    Sg_ECTableOptimize::MigrateWinner( void)
{
    Sg_ECDistribSteward        *winner = m_ECStewards.front();
    Sg_BaseDistrib              *cBase = winner->CurrentBase();
    Sg_BaseDistrib              *dBase = winner->DreamBase();
    if (( cBase == dBase) ||( winner->ProjectedSavings() <= 0))
        return false;

    uint32_t                    c1 = winner->CurrentWaste();

//    AX_TRACEPOST(( "Winner[ %d] Base[ %d->%d] BeginWaste[ %d]", winner->GetId(), cBase->GetId(), dBase->GetId(), c1))
    
    winner->Migrate( dBase);

//    AX_TRACE(( "EndWaste[ %d] %s\n", winner->CurrentWaste(), winner->CharDistrib()->ToString().c_str()))

    for ( uint32_t i = 1; i < m_ECStewards.size(); ++i)
    {        
        Sg_ECDistribSteward        *ecs = m_ECStewards[ i];
        if ( ecs->CurrentBase() == dBase)
            continue;
        if ( ecs->DreamBase() == dBase) 
            ecs->TestAndMigrateToDream();
    }
    return true;
}

//_____________________________________________________________________________________________________________________________ 

void    Sg_ECTableOptimize::CleanJilted( void)
{
    for ( uint32_t i = 0; i < m_BaseDistribs.size(); ++i)
    {
        Sg_BaseDistrib          *bds = m_BaseDistribs[ i];
        bds->MakeUniversal();
    }
    for ( uint32_t i = 0; i < m_ECStewards.size(); ++i)
        m_ECStewards[ i]->RecomputeJiltedBase();

    for ( uint32_t i = 0; i < m_BaseDistribs.size(); ++i)
    {
        Sg_BaseDistrib          *bds = m_BaseDistribs[ i];
        bds->Freeze();
        for ( uint32_t j = 0; j < m_ECStewards.size(); ++j)
            m_ECStewards[ j]->UpdateDream( bds);
    }
    return;
}

//_____________________________________________________________________________________________________________________________ 

bool    Sg_ECTableOptimize::OptimizeOnce( uint32_t round)
{
    uint32_t    beginCost = CurrentCost();
    uint32_t    beginWaste = CurrentWaste();

//    AX_TRACEPOST(("Round[ %d] BeginCost[ %d]  BeginWaste[ %d]\n", ++round, beginCost, beginWaste))

    std::sort( m_ECStewards.begin(), m_ECStewards.end(), Sg_ECDistribSteward::PtrCmp());

    if ( !MigrateWinner())
        return false;

    CleanJilted();
    uint32_t    endCost = CurrentCost();
    uint32_t    endWaste = CurrentWaste();
//    AX_TRACE(( "EndCost[ %d],  EndWaste[ %d]\n", endCost, endWaste))
    uint32_t    reduceCost = beginCost -endCost ;
    uint32_t    reduceWaste = beginWaste -endWaste;
//    AX_TRACE(( "CostReduction[ %lf],  WasteReduction[ %lf]\n", double( reduceCost * 100)/beginCost, double( reduceWaste *100) /beginWaste))
    return true;
}

//_____________________________________________________________________________________________________________________________ 

void    Sg_ECTableOptimize::DoOptimize( uint32_t SzBlut)
{
 
    
//    AX_TRACE(( "BeginCost : %d\n", CurrentCost()))
    for ( uint32_t k = 1; k < SzBlut; ++k)
    {
        Sg_BaseDistrib      *newBase = new Sg_BaseDistrib( k);
        newBase->MakeUniversal();
        m_BaseDistribs.push_back( newBase);

        for ( uint32_t i = 0; i < m_ECStewards.size(); ++i)
        {        
            Sg_ECDistribSteward        *ecs = m_ECStewards[ i];
            ecs->UpdateDream( m_BaseDistribs[ k]);
        }
        uint32_t        round = 0;
        while ( OptimizeOnce( ++round))
        {}

    }
//    AX_TRACE(( "EndCost : %d\n", CurrentCost()))
    for ( uint32_t i = 0; i < m_ECStewards.size(); ++i)
        m_ECStewards[ i]->SwitchECBase();

    return;
}

//_____________________________________________________________________________________________________________________________ 

void    Sg_ECTableOptimize::DoOptimizeFull( uint32_t SzBlut)
{
//    AX_TRACEPOST(())
    
    if ( m_ECStewards.size() < SzBlut)
        SzBlut = uint32_t( m_ECStewards.size());

//    AX_TRACE(( "BeginCost : %d\n", CurrentCost()))

    uint32_t        round = 0;
    for ( uint32_t k = 1; k < SzBlut; ++k)
    {
        Sg_BaseDistrib      *newBase = new Sg_BaseDistrib( k);
        newBase->MakeUniversal();
        m_BaseDistribs.push_back( newBase);

        for ( uint32_t i = 0; i < m_ECStewards.size(); ++i)
        {        
            Sg_ECDistribSteward        *ecs = m_ECStewards[ i];
            ecs->UpdateDream( m_BaseDistribs[ k]);
        }
        OptimizeOnce( ++round);
    }
    
    while ( OptimizeOnce( ++round))
    {}

    
//    AX_TRACE(( "EndCost : %d\n", CurrentCost()))
    for ( uint32_t i = 0; i < m_ECStewards.size(); ++i)
        m_ECStewards[ i]->SwitchECBase();

    return;
}

//_____________________________________________________________________________________________________________________________ 

