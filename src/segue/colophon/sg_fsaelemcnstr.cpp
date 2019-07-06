// sg_fsaelemcnstr.cpp ___________________________________________________________________________________________________________________

#include    "segue/tenor/sg_include.h"
#include    "segue/colophon/sg_fsaelemcnstr.h"

using namespace Sg_RExp;
 
//_____________________________________________________________________________________________________________________________

FsaElemCnstr::~FsaElemCnstr( void)  
{

    FinalizeEpsLinks(); 
    m_Repos->m_Cnstrs[ GetId()] = NULL;
    if ( !m_State->RefCount())
        m_Repos->m_ElemRepos->Destroy( GetId());
}

//_____________________________________________________________________________________________________________________________  

void    FsaElemCnstr::AddEdge( const Sg_ChSet &chSet, const ElemSlot &dest) 
{ 
    dest->m_State->RaiseRef();
    auto        filtId = m_Repos->m_ElemRepos->m_FilterRepos.FetchId( chSet);
    m_State->AddEdge( filtId, m_Repos->m_ElemRepos->GetId( dest->GetId()));
}

//_____________________________________________________________________________________________________________________________   

void    FsaElemCnstr::FinalizeEpsLinks( void)
{  
    // state is frozen: It should meet its obligation to export its OutTransitions to its eps-sourcces.
    for ( auto sIt = m_EpsSourceIds.begin(); sIt != m_EpsSourceIds.end(); ++sIt)
    {
  
        FsaElem       *srcState = static_cast< FsaElem *>( m_Repos->m_ElemRepos->Get( *sIt).GetEntry()) ;

        // export all this state transistions to the eps-source  
        if ( srcState)
        {
            if ( m_State->m_Action)
                srcState->m_Action = new Action( *m_State->m_Action);  
            for ( uint32_t i = 0; i < m_State->m_Dests.size(); ++i) 
                srcState->AddEdge( m_State->m_ChSets[ i], m_State->m_Dests[ i]);  
        }

        FsaElemCnstr      *srcCnstr = m_Repos->m_Cnstrs.at( *sIt);
        // export all this state eps-transistions to the eps-source  
        for ( auto dIt = m_EpsDests.begin(); dIt != m_EpsDests.end(); ++dIt)
        {
            FsaElemCnstr      *destCnstr = *dIt;
            if ( srcCnstr)
                srcCnstr->AddEpsDest( destCnstr); 
            else
                destCnstr->AddEpsSource( *sIt);
        }
        if ( srcCnstr)
            srcCnstr->m_EpsDests.erase( this);
    }   
    return;
}

//_____________________________________________________________________________________________________________________________   

bool    FsaElemCnstr::WriteDot( FsaElem::Id id, Cv_DotStream &strm)  
{

    for ( auto it = m_EpsDests.begin(); it !=  m_EpsDests.end(); ++it) 
        strm << 'R' << GetId() << " -> " << 'R' << (*it)->GetId() << " [ arrowhead=vee color=blue] ; \n"; 

    for ( auto it = m_EpsSourceIds.begin(); it !=  m_EpsSourceIds.end(); ++it)  
        strm << 'R' << GetId() << " -> " << 'R' << (*it)  << " [ arrowhead=tee color=green] ; \n"; 
    return true;
}
//_____________________________________________________________________________________________________________________________

void    FsaElemReposCnstr::Process( void)
{ 
    ElemSlot                       start =  ConstructCnstr();
    start->m_State->RaiseRef();
    ElemSlot                       end =  ConstructCnstr(); 
    RExpCrate::Var  docVar = m_RexpRepos->ToVar( m_RexpRepos->m_RootId);
    docVar( [ this, &start, &end](  auto k) {
        Proliferate( k, start, end);
    }); 
    m_ElemRepos->m_RootId = m_ElemRepos->GetId( start->GetId());
    return;
}

//_____________________________________________________________________________________________________________________________

bool    FsaElemReposCnstr::WriteDot( const std::string &str)
{
    std::ofstream           rexpOStrm( str);
    Cv_DotStream			rexpDotStrm( &rexpOStrm, true);  

    m_ElemRepos->WriteDot( rexpDotStrm);
    for ( uint32_t i = 1; i < m_Cnstrs.size(); ++i)
    {
        FsaElemCnstr  *si = m_Cnstrs[ i];
        if (si)
            si->WriteDot( Cv_CrateId( i, 0), rexpDotStrm); 
    }
    return true;
}

//_____________________________________________________________________________________________________________________________

