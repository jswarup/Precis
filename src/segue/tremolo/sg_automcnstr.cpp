// automcnstr.cpp ___________________________________________________________________________________________________________________

#include    "segue/tenor/sg_include.h"
#include    "segue/tremolo/sg_automcnstr.h"

using namespace Sg_RExp;
 
//_____________________________________________________________________________________________________________________________

AutomCnstr::~AutomCnstr( void)  
{

    FinalizeEpsLinks(); 
    m_Repos->m_Cnstrs[ GetId()] = NULL;
    if ( !m_State->RefCount())
        m_Repos->m_AutomRepos.Destroy( m_State->GetId());
}

//_____________________________________________________________________________________________________________________________  

void    AutomCnstr::AddEdge( const Sg_ChSet &chSet, const AutomSlot &dest) 
{ 
    m_State->AddEdge( chSet, dest->m_State);
}

//_____________________________________________________________________________________________________________________________   

void    AutomCnstr::FinalizeEpsLinks( void)
{  
    // state is frozen: It should meet its obligation to export its OutTransitions to its eps-sourcces.
    for ( auto sIt = m_EpsSourceIds.begin(); sIt != m_EpsSourceIds.end(); ++sIt)
    {
  
        AutomElem      *srcState = m_Repos->m_AutomRepos.At( *sIt);
        AutomCnstr      *srcCnstr = m_Repos->m_Cnstrs.at( *sIt);

        // export all this state transistions to the eps-source  
        for ( uint32_t i = 0; srcState && ( i < m_State->m_Dests.size()); ++i)  
            srcState->AddEdge( m_State->m_ChSets[ i], m_State->m_Dests[ i]);  

        // export all this state eps-transistions to the eps-source  
        for ( auto dIt = m_EpsDests.begin(); dIt != m_EpsDests.end(); ++dIt)
        {
            AutomCnstr      *destCnstr = *dIt;
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

bool    AutomCnstr::WriteDot( Cv_DotStream &strm)  
{

    for ( auto it = m_EpsDests.begin(); it !=  m_EpsDests.end(); ++it) 
        strm << 'R' << GetId() << " -> " << 'R' << (*it)->GetId() << " [ arrowhead=vee color=blue] ; \n"; 

    for ( auto it = m_EpsSourceIds.begin(); it !=  m_EpsSourceIds.end(); ++it)  
        strm << 'R' << GetId() << " -> " << 'R' << (*it)  << " [ arrowhead=tee color=green] ; \n"; 
    return true;
}
//_____________________________________________________________________________________________________________________________

void    AutomReposCnstr::Process( void)
{ 
    AutomSlot                       start =  ConstructCnstr();
    start->m_State->RaiseRef();
    AutomSlot                       end =  ConstructCnstr();
    end->m_State->RaiseRef();
    RExpCrate::Var  docVar = m_RexpRepos->ToVar( m_RexpRepos->m_RootId);
    docVar( [ this, start, end](  auto k) {
        Proliferate( k, start, end);
        }); 
    return;
}

//_____________________________________________________________________________________________________________________________

bool    AutomReposCnstr::WriteDot( const std::string &str)
{
    std::ofstream           rexpOStrm( str);
    Cv_DotStream			rexpDotStrm( &rexpOStrm, true);  

    m_AutomRepos.WriteDot( rexpDotStrm);
    for ( uint32_t i = 1; i < m_Cnstrs.size(); ++i)
    {
        AutomCnstr  *si = m_Cnstrs[ i];
        if (si)
            si->WriteDot( rexpDotStrm); 
    }
    return true;
}

//_____________________________________________________________________________________________________________________________

