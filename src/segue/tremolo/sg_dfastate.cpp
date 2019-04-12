//  sg_dfastate.cpp _____________________________________________________________________________________________________________

#include    "segue/tenor/sg_include.h"
#include    "segue/tremolo/sg_dfastate.h" 

using namespace Sg_RExp; 
 
//_____________________________________________________________________________________________________________________________

void       FsaSupState::FilterIt::FetchFilters( void)
{
    FsaRepos::Var        state = m_ElemRepos->ToVar( m_SubStates[ m_StateCursor]);
    m_Filters = state( [this]( auto k) { return k->Filters(); });
}

//_____________________________________________________________________________________________________________________________

Sg_Partition  FsaSupState::RefineCharDistrib( FsaElemRepos *elemRepos)
{ 
    Sg_Partition      distrib;

    distrib.MakeUniversal();

    Sg_Partition::CCLImpressor      prtnIntersector(  &distrib);
 
    FilterIt    filtIt( elemRepos, this);
    
    while ( filtIt.IsCurValid())
    {
        ChSetFilter<256>     *chSet = filtIt.Curr();
        prtnIntersector.Process( *chSet);
        filtIt.Next();
    } 
    Sg_ChSet          validCCL = prtnIntersector.ValidCCL();

    prtnIntersector.Over();

    return distrib;
} 
//_____________________________________________________________________________________________________________________________

FsaDfaState    *FsaSupState::DoConstructTransisition( FsaDfaCnstr *dfaCnstr)
{ 
    FsaDfaRepos                     *dfaRepos = dfaCnstr->m_DfaRepos;
    FsaElemRepos                    *elemRepos = dfaCnstr->m_ElemRepos;
    Cv_CArr< FsaId>                 subStates = SubStates();
    if ( !subStates.Size())
    {
        dfaRepos->Destroy( GetId());
        return NULL;
    } 
    FilterIt                        filtIt( elemRepos, this);
    DistribRepos::Discr             discr = dfaRepos->m_DistribRepos.FetchDiscr( &filtIt);
    Cv_Array< FsaSupState *, 256>   subSupStates;
    for ( uint32_t k = 0; k < discr.m_NxSz; ++k)
    {
        FsaSupState     *subSupState = new FsaSupState();
        subSupStates.Append( subSupState);
    } 

    ElemIt    elemIt( elemRepos, this);
    while ( elemIt.IsCurValid())
    {
        FsaClip             state = elemIt.Curr(); 
        Cv_CArr< FsaId>     destStateIds = state.Dests();
        Cv_CArr< FiltId>    filters = state.Filters();
        for ( uint32_t j = 0; j < destStateIds.Size(); ++j)
        {
            FsaId               destStateId =  destStateIds[ j];
            FilterCrate::Var    chSet = elemRepos->m_FilterRepos.ToVar( filters[ j]);
            for ( uint32_t k = 0; k < discr.m_NxSz; ++k)
            { 
    //            if ( chSet->IsIntersect( domain[ k]))
                if ( false)
                {
                    subSupStates[ k]->m_SubStates.push_back( destStateId);  
                    subSupStates[ k]->PushAction( FsaClip( elemRepos->ToVar( destStateId)).Tokens());
                }
            }
        }
        elemIt.Next(); 
    }
    Action                  *action = DetachAction();
    FsaDfaState             *dfaState = FsaDfaState::Construct( discr.m_NxSz, action);

    dfaRepos->StoreAt( GetId(), dfaState); 
    m_DfaStateMap->Insert( this, dfaState);
    for ( uint32_t k = 0; k < discr.m_NxSz; ++k)
    {
        FsaSupState                 *subSupState = subSupStates[ k];
        subSupState->Freeze();

        Cv_Slot< FsaDfaStateMap>    dfaStateMap =  dfaCnstr->m_SupDfaCltn.Locate( elemRepos, subSupState);
        FsaDfaState                 *subDfaState = dfaStateMap->Find( subSupState);
        if ( subDfaState)
        {
            dfaState->SetDest( k, FsaRepos::ToId( subDfaState)); 
            delete subSupState;
            continue;
        }

        auto            subId = dfaRepos->Store( subSupState);
        subSupState->m_DfaStateMap = dfaStateMap;
        dfaState->SetDest( k, subId); 
        dfaCnstr->m_FsaStk.push_back( subSupState); 
    } 
    //m_DfaStateMap.Purge();
    return dfaState;
}

//_____________________________________________________________________________________________________________________________

bool    FsaSupState::WriteDot( FsaRepos *fsaRepos, Cv_DotStream &strm) 
{ 
    strm << 'R' << GetId() << " [ shape=oval";
    strm << " color=purple label= <<FONT> N" << GetId() << "<BR />" << "<BR />" ; 
    strm << " </FONT>>];\n "; 

    Cv_CArr< FsaId>    subStates = SubStates(); 
    for ( uint32_t k = 0; k < subStates.Size(); ++k)
    {
        FsaClip         regex = fsaRepos->ToVar( subStates[ k]);
        if ( !regex)
            continue;
        strm << 'R' << GetId() << " -> " << 'R' << regex->GetId() << " [ arrowhead=normal color=black label=<<FONT> ";   
        strm << "</FONT>>] ; \n" ;  
    }
    return false; 
}
//_____________________________________________________________________________________________________________________________

FsaDfaStateMap::~FsaDfaStateMap( void)
{
    if ( m_Cltn)
    {
        m_Cltn->Erase( this);
        FsaRepos    *dfaRepos = m_Cltn->m_DfaCnstr->m_DfaRepos;
        for ( auto it = m_SupDfaMap.begin(); it != m_SupDfaMap.end(); ++it) 
            delete it->first; 
    }
}

//_____________________________________________________________________________________________________________________________

bool    FsaDfaState::WriteDot( FsaRepos *fsaRepos, Cv_DotStream &strm) 
{ 
    strm << 'R' << GetId() << " [ shape=";

    uint64_t        *toks = Tokens().Ptr();
    if ( toks)
        strm << "box";
    else
        strm << "ellipse";
    strm << " color=Red label= <<FONT> N" << GetId() << "<BR />" <<   "<BR />" ;
    for ( uint32_t i = 0; i < m_TokSz; ++i)
        strm << " T" << toks[ i];
    strm << " </FONT>>];\n "; 

    Cv_CArr< FsaId>    dests = Dests(); 
    for ( uint32_t k = 0; k < dests.Size(); ++k)
    {
        FsaClip         regex = fsaRepos->ToVar( dests[ k]);
        if ( !regex)
            continue;
        strm << 'R' << GetId() << " -> " << 'R' << regex->GetId() << " [ arrowhead=normal color=black label=<<FONT> ";   
        strm << "</FONT>>] ; \n" ;  
    }
    return false; 
}

//_____________________________________________________________________________________________________________________________

void    FsaDfaCnstr::SubsetConstruction( void)
{  
    FsaSupState     *supRootState = m_DfaRepos->Construct< FsaSupState>(); 
    supRootState->m_SubStates.push_back( m_ElemRepos->m_RootId);
    supRootState->m_DfaStateMap = m_SupDfaCltn.Locate( m_ElemRepos, supRootState);
    m_FsaStk.push_back( supRootState);
    while ( m_FsaStk.size())
    {
        FsaSupState     *supState = m_FsaStk.back();
        m_FsaStk.pop_back(); 

        FsaDfaState             *dfaState = supState->DoConstructTransisition( this);  
    }
    return;
}

//_____________________________________________________________________________________________________________________________  

