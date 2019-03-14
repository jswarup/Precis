// sg_autom.cpp ___________________________________________________________________________________________________________________

#include    "segue/tenor/sg_include.h"
#include    "segue/tremolo/sg_autom.h"

using namespace Sg_RExp;

static bool     skipFinalizeEpsLinks = false;
//_____________________________________________________________________________________________________________________________

AutomCnstr::~AutomCnstr( void)  
{

    m_Repos->m_Cnstrs[ GetId()] = NULL;
}

//_____________________________________________________________________________________________________________________________   

void    AutomCnstr::FinalizeEpsLinks( void)
{
    if ( skipFinalizeEpsLinks)
        return;

    //m_Repos->WriteDot( "dbg0.dot");
    // state is frozen: It should meet its obligation to export its OutTransitions to its eps-sourcces.
    for ( auto sIt = m_EpsSourceIds.begin(); sIt != m_EpsSourceIds.end(); ++sIt)
    {
        if ( (*sIt) == GetId())
            continue;  
        AutomState      *srcState = m_Repos->m_AutomRepos.At( *sIt);
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
 
    if ( !m_State->RefCount())
        m_Repos->m_AutomRepos.Destroy( m_State->GetId());
    //m_Repos->WriteDot( "dbg1.dot");
    return;
    }

//_____________________________________________________________________________________________________________________________

