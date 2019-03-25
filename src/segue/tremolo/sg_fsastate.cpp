//  sg_fsastate.cpp _____________________________________________________________________________________________________________

#include    "segue/tenor/sg_include.h"
#include    "segue/tremolo/sg_fsastate.h" 

using namespace Sg_RExp;

//_____________________________________________________________________________________________________________________________

bool        FsaRepos::WriteDot( Cv_DotStream &strm)
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

bool        FsaRepos::DumpDot( const char *path)
{
    std::ofstream           fsaOStrm( path);
    Cv_DotStream			fsaDotStrm( &fsaOStrm, true); 
    return WriteDot( fsaDotStrm);;
}

//_____________________________________________________________________________________________________________________________  

bool  FsaElem::WriteDot( FsaRepos *fsaRepos, Cv_DotStream &strm)  
{
    strm << 'R' << GetId() << " [ shape=";

    if ( m_Action)
        strm << "box";
    else
        strm << "ellipse";
    strm << " color=cyan label= <<FONT> N" << GetId() << "<BR />" << RefCount() << "<BR />" ;
    if ( m_Action)
        strm << 'T' << m_Action->m_Value;
    strm << " </FONT>>];\n "; 

    Cv_CArr< FsaId>    dests = Dests(); 
    for ( uint32_t k = 0; k < dests.Size(); ++k)
    {
        FsaElem      *regex = ( FsaElem *) fsaRepos->ToVar( dests[ k]);
        strm << 'R' << GetId() << " -> " << 'R' << regex->GetId() << " [ arrowhead=normal color=black label=<<FONT> ";  
        ChSetFilter     *chSet = fsaRepos->m_FilterRepos.ToVar( m_ChSets[ k]);
        strm << Cv_Aid::XmlEncode( chSet->ToString());
        strm << "</FONT>>] ; \n" ;  
    }
    return true;
}

//_____________________________________________________________________________________________________________________________

Sg_CharDistrib  FsaSupState::RefineCharDistrib( FsaRepos *fsaRepos)
{ 
    Sg_CharDistrib      distrib;

    distrib.MakeUniversal();

    Sg_CharDistrib::CCLIdImpressor      prtnIntersector(  &distrib);
 
    Cv_CArr< FsaId>                    subStates = SubStates();
    for ( uint32_t i = 0; i < subStates.Size(); ++i) 
    {
        FsaClip             state = fsaRepos->ToVar( subStates[ i]);
        Cv_CArr< FiltId>   filters = state.Filters();
        for ( uint32_t j = 0; j < filters.Size(); ++j)
        {
            ChSetFilter     *chSet = fsaRepos->m_FilterRepos.ToVar( filters[ j]);
            prtnIntersector.Process( *chSet, CV_UINT32_MAX);
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
    strm << " color=purple label= <<FONT> N" << GetId() << "<BR />" << RefCount() << "<BR />" ; 
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
    FsaRepos                        *fsaRepos = dfaCnstr->m_FsaRepos;
    Cv_CArr< FsaId>                 subStates = SubStates();
    if ( !subStates.Size())
    {
        fsaRepos->Destroy( GetId());
        return NULL;
    } 
    Sg_CharDistrib                  distrib = RefineCharDistrib( fsaRepos);
    std::vector< Sg_ChSet>          domain = distrib.Domain();
    uint32_t                        sz = uint32_t( domain.size());
    Cv_Array< FsaSupState *, 256>   subSupStates;
    FsaDfaState                     *dfaState = new FsaDfaState();
    for ( uint32_t k = 0; k < sz; ++k)
    {
        FsaSupState     *subSupState = new FsaSupState();
        subSupStates.Append( subSupState);
    } 
    for ( uint32_t i = 0; i < subStates.Size(); ++i) 
    {
        FsaId               stateId = subStates[ i];
        FsaClip             state = fsaRepos->ToVar( stateId);
        Cv_CArr< FsaId>     dests = state.Dests();
        Cv_CArr< FiltId>    filters = state.Filters();
        for ( uint32_t j = 0; j < dests.Size(); ++j)
        {
            FsaId               dest =  dests[ j];
            ChSetFilter         *chSet = fsaRepos->m_FilterRepos.ToVar( filters[ j]);
            for ( uint32_t k = 0; k < sz; ++k)
            {
                const Sg_ChSet  &ccl = domain[ k]; 
                if ( chSet->IsIntersect( ccl))
                    subSupStates[ k]->m_SubStates.push_back( dest);  
            }
        }
    } 
    for ( uint32_t k = 0; k < sz; ++k)
    {
        FsaSupState     *subSupState = subSupStates[ k];
        std::sort( subSupState->m_SubStates.begin(), subSupState->m_SubStates.end()); 
        auto            it = dfaCnstr->m_SupDfaMap.find( subSupState);
        if ( it != dfaCnstr->m_SupDfaMap.end())
        {
            dfaState->m_Dests.push_back( FsaRepos::ToId( it->second)); 
            delete subSupState;
            continue;
        }
        auto            subId = fsaRepos->Store( subSupState);
        dfaState->m_Dests.push_back( subId); 
        dfaCnstr->m_FsaStk.push_back( subSupState); 
    }
    fsaRepos->StoreAt( GetId(), dfaState);
    dfaCnstr->m_SupDfaMap.insert( std::pair( this, dfaState));
    return dfaState;
}

//_____________________________________________________________________________________________________________________________

bool    FsaDfaState::WriteDot( FsaRepos *fsaRepos, Cv_DotStream &strm) 
{ 
    strm << 'R' << GetId() << " [ shape=";

    if ( m_Action)
        strm << "box";
    else
        strm << "ellipse";
    strm << " color=Red label= <<FONT> N" << GetId() << "<BR />" << RefCount() << "<BR />" ;
    if ( m_Action)
        strm << 'T' << m_Action->m_Value;
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
    
    FsaSupState     *supRootState = m_FsaRepos->Construct< FsaSupState>(); 
    supRootState->m_SubStates.push_back( m_FsaRepos->m_RootId);
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
 