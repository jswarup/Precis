//  sg_dfastate.cpp _____________________________________________________________________________________________________________

#include    "segue/tenor/sg_include.h"
#include    "segue/tremolo/sg_dfastate.h" 

using namespace Sg_RExp; 
 
//_____________________________________________________________________________________________________________________________

Sg_Partition  FsaSupState::RefineCharDistrib( FsaRepos *elemRepos)
{ 
    Sg_Partition      distrib;

    distrib.MakeUniversal();

    Sg_Partition::CCLImpressor      prtnIntersector(  &distrib);

    Cv_CArr< FsaId>                    subStates = SubStates();
    for ( uint32_t i = 0; i < subStates.Size(); ++i) 
    {
        FsaClip             state = elemRepos->ToVar( subStates[ i]);
        Cv_CArr< FiltId>   filters = state.Filters();
        for ( uint32_t j = 0; j < filters.Size(); ++j)
        {
            ChSetFilter<256>     *chSet = elemRepos->m_FilterRepos.ToVar( filters[ j]);
            prtnIntersector.Process( *chSet);
        }

    }
    Sg_ChSet          validCCL = prtnIntersector.ValidCCL();

    prtnIntersector.Over();

    return distrib;
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

FsaDfaState    *FsaSupState::DoConstructTransisition( FsaDfaCnstr *dfaCnstr)
{ 
    FsaRepos                        *dfaRepos = dfaCnstr->m_DfaRepos;
    FsaElemRepos                    *elemRepos = dfaCnstr->m_ElemRepos;
    Cv_CArr< FsaId>                 subStates = SubStates();
    if ( !subStates.Size())
    {
        dfaRepos->Destroy( GetId());
        return NULL;
    } 
    Sg_Partition                    distrib = RefineCharDistrib( elemRepos);
    auto                            domain = distrib.Domain();
    uint32_t                        sz = uint32_t( domain.size());
    Cv_Array< FsaSupState *, 256>   subSupStates;
    for ( uint32_t k = 0; k < sz; ++k)
    {
        FsaSupState     *subSupState = new FsaSupState();
        subSupStates.Append( subSupState);
    } 
    for ( uint32_t i = 0; i < subStates.Size(); ++i) 
    {
        FsaId               stateId = subStates[ i];
        FsaClip             state = elemRepos->ToVar( stateId);


        Cv_CArr< FsaId>     destStateIds = state.Dests();
        Cv_CArr< FiltId>    filters = state.Filters();
        for ( uint32_t j = 0; j < destStateIds.Size(); ++j)
        {
            FsaId               destStateId =  destStateIds[ j];
            ChSetFilter<256>    *chSet = elemRepos->m_FilterRepos.ToVar( filters[ j]);
            for ( uint32_t k = 0; k < sz; ++k)
            {
                auto    ccl = domain[ k]; 
                if ( chSet->IsIntersect( ccl))
                {
                    subSupStates[ k]->m_SubStates.push_back( destStateId);  
                    subSupStates[ k]->PushAction( FsaClip( elemRepos->ToVar( destStateId)).Tokens());
                }
            }
        }
    }  
    Action                  *action = DetachAction();
    FsaDfaState             *dfaState = FsaDfaState::Construct( sz, action);

    dfaRepos->StoreAt( GetId(), dfaState); 
    m_DfaStateMap->Insert( this, dfaState);
    for ( uint32_t k = 0; k < sz; ++k)
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
    m_DfaStateMap = Cv_Slot< FsaDfaStateMap>();
    return dfaState;
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

