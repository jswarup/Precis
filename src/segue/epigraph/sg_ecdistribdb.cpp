// sg_ecdistribdb.cpp ______________________________________________________________________________________________________________ 


#include    "segue/tenor/sg_include.h"
#include    "segue/epigraph/sg_ecdistribdb.h" 

//_____________________________________________________________________________________________________________________________ 

Sg_ECDistrbDB::Sg_ECDistrbDB( void)
{

}

//_____________________________________________________________________________________________________________________________ 

Sg_ECDistrbDB::~Sg_ECDistrbDB( void)
{
    for ( uint32_t k = 0; k < m_BaseDistribs.size(); ++k)
        delete m_BaseDistribs[ k];

    for ( uint32_t k = 0; k < m_FlexECs.size(); ++k)
        delete m_FlexECs[ k];
}

//_____________________________________________________________________________________________________________________________ 

void    Sg_ECDistrbDB::Setup( void)
{
    m_FlexBase.MakeUniversal();
    
    Sg_CharDistrib::CCLImpressor  prtnIntersector( &m_FlexBase);
   /*
    for ( uint32_t  i = 1; i < stateChart->Size(); ++i)
    {
        Sg_IfcDfaState              *dfaState = stateChart->GetDfaState( i);
        if ( !dfaState)
            continue;
        std::vector< Sg_TransEdge>  outEdges = dfaState->OutSwEdges( stateChart);
        for ( uint32_t j = 0; j < outEdges.size(); ++j)
            prtnIntersector.Process( outEdges[ j].Sifter().CCL());
    }
*/
    prtnIntersector.Over();
    m_FlexBase.Freeze();
     
    return;
}

//_____________________________________________________________________________________________________________________________ 

std::vector< Sg_ECDistrib*>     Sg_ECDistrbDB::ComputeECDistribs( void) 
{
    std::vector< Sg_ECDistrib*>     ecDistribs;

/*
    std::vector< Sg_ECDistrib*>     ecDistribs( stateChart->Size(), NULL);
    
    
    for ( uint32_t  i = 1; i < stateChart->Size(); ++i)
    {
        Sg_IfcDfaState              *dfaState = stateChart->GetDfaState( i);
        if ( !dfaState)
            continue;
        
        std::vector< Sg_TransEdge>  outEdges = dfaState->OutSwEdges( stateChart);
        if ( !outEdges.size())
            continue;

        std::set< uint32_t>         destSet;
        Sg_CCL                      validCCL;
        for ( uint32_t j = 0; j < outEdges.size(); ++j)
        {
            destSet.insert( outEdges[ j].Dest());
            validCCL.UnionWith( outEdges[ j].Sifter().CCL());
        }
        Sg_CCL                      invalidCCL = validCCL.Negative();                     
        int                         failChar = invalidCCL.GetChar( 0);
        bool                        hasFail = ( failChar != -1);
        std::vector< uint32_t>      destList( destSet.begin(), destSet.end());  // list of unique destinations
        std::vector< uint32_t>      destLocs( destSet.size(), 0);
        std::vector< uint32_t>      destIds;
        Sg_ECDistrib                ecDistrib( &m_FlexBase);
        uint32_t                    szBaseCCLs = 0;

        for ( uint32_t j = 0; j < ( outEdges.size() +hasFail); ++j)
        {
            const Sg_CCL                &ccl = ( j == outEdges.size()) ? invalidCCL : outEdges[ j].Sifter().CCL();
            for ( uint32_t ecId = 0; ecId < m_FlexBase.SzImage(); ++ecId)
            {
                const Sg_CCL        &baseCCL = m_FlexBase.EqClassCCL( ecId);
                if ( !baseCCL.IsIntersect( ccl))
                    continue;
                if ( j == outEdges.size())      // Set 0 to failure.
                {
                    ecDistrib.SetAt( ecId, 0);
                    continue;
                }
                ++szBaseCCLs;
                // index of dest in the unique dest list
                std::vector< uint32_t>::iterator    locDest = std::lower_bound( destList.begin(), destList.end(), outEdges[ j].Dest());
                AX_ERROR_ASSERT( locDest != destList.end())
                uint32_t                            destInd = locDest -destList.begin();
                
                // we have not encountered this class till now
                if ( !destLocs[ destInd]) 
                {
                    destIds.push_back( outEdges[ j].Dest());
                    destLocs[ destInd] = destIds.size();
                }
                ecDistrib.SetAt( ecId, destLocs[ destInd]);
            }
        }
        ecDistrib.SetZeroFail( hasFail);
        uint32_t                    th = ecDistrib.Thickness();
        if ( dfaState->StartIfc() || ( th < 3))
            ecDistrib.SetLock( true);
        Sg_ECDistrib                *ecd = FetchDistrib( &ecDistrib);
        ecDistribs[ i] = ecd; 
    }
*/
    return ecDistribs;
}

//_____________________________________________________________________________________________________________________________ 

Sg_ECDistrib    *Sg_ECDistrbDB::FetchDistrib(  Sg_ECDistrib *ecDistrib)
{
    auto    ecdIt = m_ECDB.find( ecDistrib);
    if ( ecdIt == m_ECDB.end())
    {
        Sg_ECDistrib    *ecd = new Sg_ECDistrib( *ecDistrib);
        m_ECDB.insert( ecd);
        m_FlexECs.push_back( ecd);
    }
    (*ecdIt)->RaiseRef();
    return (*ecdIt);
}

//_____________________________________________________________________________________________________________________________ 

void       Sg_ECDistrbDB::RemoveDistrib( Sg_ECDistrib *ecDistrib)
{
    if ( ecDistrib->LowerRef())
        m_ECDB.erase( ecDistrib);
}

//_____________________________________________________________________________________________________________________________ 

void        Sg_ECDistrbDB::OptimizeBasis( uint32_t szBluts)
{   
    Sg_ECTableOptimize      ecLookupTable;
    ecLookupTable.Seed( m_FlexECs);
    ecLookupTable.DoOptimize( szBluts);
    m_BaseDistribs = ecLookupTable.BaseDistribs();
/*    for ( uint32_t i = 0; i < m_BaseDistribs.size(); ++i)
        AX_TRACE(( "%s", m_BaseDistribs[ i]->ToString().c_str()))
*/
    return;
}

//_____________________________________________________________________________________________________________________________ 

std::string     Sg_ECDistrbDB::ToString( void) const
{
    std::stringstream   sstrm;
    sstrm << "BaseDistrib : " << m_FlexBase.ToString() << "\n";
    sstrm << "Size FlexECs : " << m_FlexECs.size() << "\n";
    for ( uint32_t i = 0; i < m_FlexECs.size(); ++i)
        sstrm << i << " : " << m_FlexECs[ i]->ToString() << "\n";
    return sstrm.str();
}

//_____________________________________________________________________________________________________________________________ 
