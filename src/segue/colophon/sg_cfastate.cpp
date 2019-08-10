//  sg_cfastate.cpp _____________________________________________________________________________________________________________

#include    "segue/tenor/sg_include.h"
#include    "segue/colophon/sg_cfastate.h" 
#include    "segue/colophon/sg_reposatelier.h" 

using namespace Sg_RExp; 

//_____________________________________________________________________________________________________________________________

void       FsaCfaState::DescendIt::FetchFilterElems( void)
{
    FsaRepos::Var        state = m_ElemRepos->ToVar( m_SubStates[ m_StateCursor]);
    m_Filters = state( [this]( auto k) { return k->Filters(); });
    m_DestStateIds = state( [this]( auto k) { return k->Dests(); });   
}

//_____________________________________________________________________________________________________________________________

void       FsaCfaState::DescendIt::Dump( std::ostream &strm, FilterRepos *filterRepos)
{
    for ( uint32_t i = 0; i < m_SubStates.Size(); ++i)
    {
        strm << m_SubStates[i].GetId() << " ";
        if ( m_StateCursor != i)
            continue;
        strm << "[ ";
        for ( uint32_t j = 0; j  < m_Filters.Size(); ++j)
        {
            filterRepos->Dump( strm, m_Filters[ j]);
            strm << m_DestStateIds[ j].GetId() << ", "; 
        }
        strm << "] ";
    }
    strm << "\n";
    return;
}

//_____________________________________________________________________________________________________________________________

void    FsaCfaState::DoConstructTransisition( FsaId supId, FsaDfaCnstr *dfaCnstr)
{ 
    FsaDfaRepos                     *dfaRepos = dfaCnstr->m_DfaRepos;
    FsaElemRepos                    *elemRepos = dfaCnstr->m_ElemRepos;
    Cv_Seq< FsaId>                  subStates = SubStates();
    if ( !subStates.Size())
    {
        dfaRepos->Destroy( supId.GetId());
        return;
    } 
    
    DescendIt                       descIt( elemRepos, this);

    DistribRepos::DfaDistrib        dDist = dfaRepos->m_DistribRepos.ConstructDfaDistrib( &descIt); 

 
    Cv_Array< uint32_t, 256>        subTrials = dDist.m_DVar( [this, dfaCnstr]( auto distrib) { 
        Cv_Array< uint32_t, 256>    trials; 
        trials.Reset( dfaCnstr->m_BaseWts.SzFill(), 0);
        uint32_t                    mxEc = 0;
        for ( uint32_t i = 0; i < dfaCnstr->m_BaseWts.SzFill(); ++i)
        {
            uint8_t     ec = distrib->Image( i); 
            trials[ ec] += dfaCnstr->m_BaseWts[ i];
            if ( mxEc < ec)
                mxEc = ec;
        }
        trials.m_SzFill = uint32_t( mxEc) +1;
        return  trials; 
    } );  

    //dDist.Dump( std::cout, &dfaRepos->m_DistribRepos);
    descIt.DoSetup( dDist.SzDescend(), m_Level +1); 

    dfaRepos->m_DistribRepos.Classify( m_Level, dDist, &descIt);   

    Action                      *action = DetachAction();   
    
    Cv_Array< FsaId, 256>       destArr;
    for ( uint32_t k = 0; k < dDist.SzDescend(); ++k)
    {
        FsaCfaState                 *subSupState = descIt.m_SubSupStates[ k];
        subSupState->Freeze();
        if ( ! subSupState->SubStates().Size())
        {
            destArr.Append( FsaId()); 
            delete subSupState;
            continue;
        }
        subSupState->m_SzTrials = uint32_t( ( subTrials[ k] * uint64_t( m_SzTrials)) / 256); 
        FsaRuleLump                 *ruleLump =  dfaCnstr->m_RuleLumpSet.Locate( elemRepos, subSupState);
        uint32_t                    ind = ruleLump->Find( subSupState);
        if ( ind != CV_UINT32_MAX)
        { 
            destArr.Append( dfaRepos->GetId( ind));  
            delete subSupState;
            continue;
        }

        auto            subId = dfaRepos->Store( subSupState);
        subSupState->m_RuleLump = ruleLump;
        ruleLump->Register( subSupState, subId.GetId());
        ruleLump->m_ActiveRef.RaiseRef(); 
        destArr.Append( subId ); 
        dfaCnstr->m_FsaStk.push_back( subId); 
    }  
    dfaCnstr->ConstructDfaStateAt( supId.GetId(), dDist, action, destArr, m_SzTrials);   
    if ( !m_RuleLump->m_ActiveRef.LowerRef()) 
        dfaCnstr->m_RuleLumpSet.Destroy( m_RuleLump->GetId()); 
    dDist.m_DVar.Delete();
    return;
} 

//_____________________________________________________________________________________________________________________________
