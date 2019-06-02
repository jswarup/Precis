//  sg_dfastate.cpp _____________________________________________________________________________________________________________

#include    "segue/tenor/sg_include.h"
#include    "segue/tremolo/sg_dfastate.h" 

using namespace Sg_RExp; 
 
//_____________________________________________________________________________________________________________________________

void       FsaSupState::DescendIt::FetchFilterElems( void)
{
    FsaRepos::Var        state = m_ElemRepos->ToVar( m_SubStates[ m_StateCursor]);
    m_Filters = state( [this]( auto k) { return k->Filters(); });
    m_DestStateIds = state( [this]( auto k) { return k->Dests(); });   
}

//_____________________________________________________________________________________________________________________________

void       FsaSupState::DescendIt::Dump( std::ostream &strm)
{
    for ( uint32_t i = 0; i < m_SubStates.Size(); ++i)
    {
        strm << m_SubStates[i].GetId() << " ";
        if ( m_StateCursor != i)
            continue;
        strm << "[ ";
        for ( uint32_t j = 0; j  < m_Filters.Size(); ++j)
        {
            m_DfaRepos->m_ElemRepos->m_FilterRepos.Dump( strm, m_Filters[ j]);
            strm << m_DestStateIds[ j].GetId() << ", "; 
        }
        strm << "] ";
    }
    strm << "\n";
    return;
}
 //_____________________________________________________________________________________________________________________________

void    FsaSupState::DoConstructTransisition( FsaDfaCnstr *dfaCnstr)
{ 
    FsaDfaRepos                     *dfaRepos = dfaCnstr->m_DfaRepos;
    FsaElemRepos                    *elemRepos = dfaCnstr->m_ElemRepos;
    Cv_CArr< FsaId>                 subStates = SubStates();
    if ( !subStates.Size())
    {
        dfaRepos->Destroy( GetId());
        return;
    } 
    DescendIt               descIt( elemRepos, dfaRepos, this);

    DistribRepos::Discr     discr =  dfaRepos->m_DistribRepos.FetchDiscr( &descIt); 
    //discr.Dump( std::cout, &dfaRepos->m_DistribRepos);
    descIt.DoSetup( discr.SzDescend()); 
    
    dfaRepos->m_DistribRepos.Classify( discr, &descIt);   
    
    Action                  *action = DetachAction();  

    m_DfaStateMap->Insert( this, GetId());
    Cv_Array< uint32_t, 256>        destArr;
    for ( uint32_t k = 0; k < discr.SzDescend(); ++k)
    {
        FsaSupState                 *subSupState = descIt.m_SubSupStates[ k];
        subSupState->Freeze();
        if ( ! subSupState->SubStates().Size())
        {
            destArr.Append( 0); 
            delete subSupState;
            continue;
        }
        Cv_Slot< FsaDfaStateMap>    dfaStateMap =  dfaCnstr->m_SupDfaCltn.Locate( elemRepos, subSupState);
        uint32_t                    ind = dfaStateMap->Find( subSupState);
        if ( ind != CV_UINT32_MAX)
        { 
            destArr.Append(ind); 
            delete subSupState;
            continue;
        }

        auto            subId = dfaRepos->Store( subSupState);
        subSupState->m_DfaStateMap = dfaStateMap;
        destArr.Append( subId.GetId()); 
        dfaCnstr->m_FsaStk.push_back( subSupState); 
    } 
    FsaDfaState             *dfaState = FsaDfaState::Construct( discr, action);
    for ( uint32_t k = 0; k < discr.SzDescend(); ++k) 
        dfaState->SetDest( k, Id( destArr[ k], 0)); 

    dfaRepos->StoreAt( GetId(), dfaState); 
    //m_DfaStateMap.Purge();
    return;
}

//_____________________________________________________________________________________________________________________________

bool    FsaSupState::WriteDot( FsaRepos *fsaRepos, Cv_DotStream &strm) 
{ 
    strm << GetTypeChar() << GetId() << " [ shape=oval";
    strm << " color=purple label= <<FONT>" << GetTypeChar() << GetId() << "<BR />" << "<BR />" ; 
    strm << " </FONT>>];\n "; 

    FsaDfaRepos         *dfaRepos = static_cast< FsaDfaRepos *>( fsaRepos); 
    
    Cv_CArr< FsaId>     subStates = SubStates(); 
    for ( uint32_t k = 0; k < subStates.Size(); ++k)
    {
        FsaClip         regex = dfaRepos->m_ElemRepos->ToVar( subStates[ k]);
        if ( !regex)
            continue;
        strm << GetTypeChar() << GetId() << " -> " << regex->GetTypeChar() << regex->GetId() << " [ arrowhead=normal color=black label=<<FONT> ";   
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

bool FsaDfaState::CleanupDestIds( FsaRepos *dfaRepos)
{
    Cv_CArr< FsaId>    dests = Dests(); 
    for ( uint32_t k = 0; k < dests.Size(); ++k)
    {
        FsaClip         regex = dfaRepos->ToVar( dests[ k]);
        if ( !regex)
            dests[ k] = Id();
        else
            dests[ k] = *regex.GetEntry();    
    }
    return true;
}

//_____________________________________________________________________________________________________________________________

bool    FsaDfaState::WriteDot( FsaRepos *fsaRepos, Cv_DotStream &strm) 
{ 
    strm << GetTypeChar() << GetId() << " [ shape=";

    uint64_t        *toks = Tokens().Ptr();
    if ( toks)
        strm << "box";
    else
        strm << "ellipse";
    strm << " color=Red label= <<FONT> " << GetTypeChar() << GetId() << "<BR />" <<   "<BR />" ;
    for ( uint32_t i = 0; i < m_TokSz; ++i)
        strm << " T" << toks[ i];
    strm << " </FONT>>];\n "; 

    FsaDfaRepos                 *dfaRepos = static_cast< FsaDfaRepos *>( fsaRepos);
    std::vector< Sg_ChSet>      domain = dfaRepos->m_DistribRepos.Domain( m_Discr.m_DId);
    Cv_CArr< FsaId>    dests = Dests(); 
    for ( uint32_t k = 0; k < dests.Size(); ++k)
    {
        FsaClip         regex = fsaRepos->ToVar( dests[ k]);
        if ( !regex)
            continue;
        strm << GetTypeChar() << GetId() << " -> " <<  regex->GetTypeChar() << regex->GetId() << " [ arrowhead=normal color=black label=<<FONT> "; 
        strm << Cv_Aid::XmlEncode( domain[ k].ToString());  
        strm << "</FONT>>] ; \n" ;  
    }
    return false; 
}
//_____________________________________________________________________________________________________________________________

bool    FsaDfaState::DumpDot( Cv_DotStream &strm) 
{ 
    strm << GetTypeChar() << GetId() << " [ shape=";

    uint64_t        *toks = Tokens().Ptr();
    if ( toks)
        strm << "box";
    else
        strm << "ellipse";
    strm << " color=Red label= <<FONT> " << GetTypeChar() << GetId() << "<BR />" <<   "<BR />" ;
    for ( uint32_t i = 0; i < m_TokSz; ++i)
        strm << " T" << toks[ i];
    strm << " </FONT>>];\n ";  
    //std::vector< Sg_ChSet>      domain = dfaRepos->m_DistribRepos.Domain( m_Discr.m_DId);
    Cv_CArr< FsaId>    dests = Dests(); 
    for ( uint32_t k = 0; k < dests.Size(); ++k)
    {
   //     FsaClip         regex = fsaRepos->ToVar( dests[ k]);
        if ( !dests[ k].GetId())
            continue;
        strm << GetTypeChar() << GetId() << " -> " <<  GetTypeChar() << dests[ k].GetId() << " [ arrowhead=normal color=black label=<<FONT> "; 
//        strm << Cv_Aid::XmlEncode( domain[ k].ToString());  
        strm << "</FONT>>] ; \n" ;  
    }
 
    return false; 
}

//_____________________________________________________________________________________________________________________________

bool        FsaDfaRepos::WriteDot( Cv_DotStream &strm)
{
    for ( uint32_t i = 1; i < Size(); ++i)
    {
        Var     si = Get( i);
        if (si)
            si( [this, &strm]( auto k) { k->WriteDot( this, strm); });
    }
    return true;
}

//_____________________________________________________________________________________________________________________________

bool        FsaDfaRepos::DumpDot( const char *path)
{
    std::ofstream           fsaOStrm( path);
    Cv_DotStream			fsaDotStrm( &fsaOStrm, true); 
    return WriteDot( fsaDotStrm);;
}
//_____________________________________________________________________________________________________________________________

void    FsaDfaCnstr::SubsetConstruction( void)
{  
    FsaSupState     *supRootState = m_DfaRepos->Construct< FsaSupState>(); 
    uint32_t        rootId = supRootState->GetId();
    supRootState->m_SubStates.push_back( m_ElemRepos->m_RootId);
    supRootState->m_DfaStateMap = m_SupDfaCltn.Locate( m_ElemRepos, supRootState);
    m_FsaStk.push_back( supRootState);
    while ( m_FsaStk.size())
    {
        FsaSupState     *supState = m_FsaStk.back();
        m_FsaStk.pop_back(); 

        supState->DoConstructTransisition( this);  
    }

    m_DfaRepos->m_RootId = m_DfaRepos->GetId( rootId);;
    m_DfaRepos->OperateAll( [ this]( auto k) {
        return k->CleanupDestIds( m_DfaRepos);
    });
    return;
}

//_____________________________________________________________________________________________________________________________

bool        FsaDfaCnstr::DumpDot( const char *path)
{
    std::ofstream           fsaOStrm( path);
    Cv_DotStream			fsaDotStrm( &fsaOStrm, true); 
    return m_ElemRepos->WriteDot( fsaDotStrm) && m_DfaRepos->WriteDot( fsaDotStrm);
}

//_____________________________________________________________________________________________________________________________  

