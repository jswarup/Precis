//  sg_rulelump.h _______________________________________________________________________________________________________________
#pragma once

#include    "segue/colophon/sg_cfastate.h" 

//_____________________________________________________________________________________________________________________________  

namespace Sg_RExp
{ 
//_____________________________________________________________________________________________________________________________ 

struct  FsaRuleLump : public Cv_ReposEntry 
{    
    Cv_Shared                                               m_ActiveRef;
    std::vector< uint32_t>                                  m_Ruleset;
    std::map< FsaCfaState*, uint32_t, Cv_TPtrLess< void> >  m_SupDfaIdMap;

    FsaRuleLump(  const std::set< uint32_t> &ruleset)
        : m_Ruleset( ruleset.begin(), ruleset.end())
    {}

    ~FsaRuleLump( void)
    {}

    void    Purge( void)
    {
        for ( auto it = m_SupDfaIdMap.begin(); it != m_SupDfaIdMap.end(); ++it)
            delete it->first;
    }

    int32_t                 Compare( const FsaRuleLump &dsMap) const
    {
        if ( m_Ruleset.size() != dsMap.m_Ruleset.size())
            return m_Ruleset.size() > dsMap.m_Ruleset.size() ? 1 : -1;
        for ( auto it1 = m_Ruleset.begin(), it2 = dsMap.m_Ruleset.begin(); it1 != m_Ruleset.end(); ++it1, ++it2)
            if ( *it1 != *it2)
                return *it1 < *it2  ? 1 : -1;
        return 0;
    }   

    void                    Register(  FsaCfaState *supState, uint32_t stateId)
    {
        m_SupDfaIdMap.insert( std::make_pair( supState, stateId));
    }

    uint32_t                Find( FsaCfaState *supState)
    {
        auto            it = m_SupDfaIdMap.find( supState);
        if ( it != m_SupDfaIdMap.end())
            return it->second;
        return CV_UINT32_MAX;
    }


    bool                    Dump( std::ostream &ostr) 
    {
        ostr << "[ ";  
        for ( auto it = m_Ruleset.begin(); it != m_Ruleset.end(); )
        {
            ostr << uint32_t( *it);
            if ( ++it != m_Ruleset.end())
                ostr << ", ";  
        }
        ostr << "]\n";  
        return true; 
    }
};

//_____________________________________________________________________________________________________________________________ 

struct  FsaRuleLumpSet : public Cv_Repos< FsaRuleLump>
{ 
    std::set< FsaRuleLump *, Cv_TPtrLess< void> >       m_LumpSet;

    FsaRuleLumpSet( FsaDfaCnstr *cnstr) 
    {}

    void    Erase( FsaRuleLump  *dsMap)
    {
        m_LumpSet.erase( dsMap);
    }

    FsaRuleLump     *Locate( FsaElemRepos *elemRepos, FsaCfaState *supState)
    {
        if ( IsAt( 1))
            return At( 1);
        FsaRuleLump     *ruleLump = new FsaRuleLump( supState->RuleIds( elemRepos));
        auto            it =  m_LumpSet.lower_bound( ruleLump);
        if (( it == m_LumpSet.end())  || m_LumpSet.key_comp()( ruleLump, *it))
        {   
            m_LumpSet.insert( it, ruleLump);
            Store( ruleLump);        
            return ruleLump;
        }
        delete ruleLump;
        return *it;           
    } 

    bool            Dump( std::ostream &ostr) 
    { 
        for ( auto it = m_LumpSet.begin(); it != m_LumpSet.end(); ++it)
        {
            FsaRuleLump     *si = *it;  
            si->Dump( ostr);
        }
        return true;
    }
};

//_____________________________________________________________________________________________________________________________ 
};