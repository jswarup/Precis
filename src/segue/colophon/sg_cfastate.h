//  sg_cfastate.h _______________________________________________________________________________________________________________
#pragma once

#include    "segue/colophon/sg_fsastate.h" 

//_____________________________________________________________________________________________________________________________  

namespace Sg_RExp
{

//_____________________________________________________________________________________________________________________________ 

struct FsaCfaState  : public FsaState
{   
    std::vector< FsaId>             m_SubStates;  
    Action                          *m_Action; 
    uint32_t                        m_Level;
    FsaRuleLump                     *m_RuleLump;

    FsaCfaState( uint32_t level)
        : m_Action( NULL), m_Level( level), m_RuleLump( NULL)
    {}

    ~FsaCfaState( void)
    {
        if ( m_Action)
            delete m_Action;
    }

    int32_t     Compare( const FsaCfaState &x2) const 
    {
        if ( m_SubStates.size() != x2.m_SubStates.size())
            return m_SubStates.size() < x2.m_SubStates.size() ? 1 : -1;
        const FsaId       *arr1 = &m_SubStates[ 0];
        const FsaId       *arr2 = &x2.m_SubStates[ 0];
        for ( uint32_t i = 0; i < m_SubStates.size(); ++i)
            if ( arr1[ i] != arr2[ i])
                return arr1[ i] < arr2[ i] ? 1 : -1;
        if ( !m_Action != !x2.m_Action)
            return !m_Action > !x2.m_Action ? -1 : 1;
        if ( m_Action == x2.m_Action) 
            return 0;         
        return m_Action->Compare( *x2.m_Action);
    } 

    Cv_Seq< FsaId>         SubStates( void) { return m_SubStates.size() ? Cv_Seq< FsaId>( &m_SubStates[ 0], uint32_t( m_SubStates.size())) : Cv_Seq< FsaId>(); }

    std::set< uint32_t>     RuleIds( FsaElemRepos *fsaElemRepos) const
    {  
        std::set< uint32_t>    ruleIds; 
        for ( uint32_t i = 0; i < m_SubStates.size(); ++i)
            ruleIds.insert( fsaElemRepos->RuleIdFromState( m_SubStates[ i].GetId()));
        return ruleIds;
    }

    void                    PushAction( const Cv_Seq< uint64_t>  &tokens)
    { 
        if ( !tokens.Size())
            return;
        if ( !m_Action)
            m_Action = new Action(); 
        m_Action->Push( tokens);
        return;
    }

    void                    Freeze( void)
    {
        std::sort( m_SubStates.begin(), m_SubStates.end());
        auto    it = std::unique( m_SubStates.begin(), m_SubStates.end());
        m_SubStates.resize( std::distance( m_SubStates.begin(), it));
        if ( !m_Action)
            return;
        std::sort( m_Action->m_Values.begin(), m_Action->m_Values.end()); 
        auto    itAct = std::unique( m_Action->m_Values.begin(), m_Action->m_Values.end());
        m_Action->m_Values.resize( std::distance( m_Action->m_Values.begin(), itAct));
        return;
    }

    Action                  *DetachAction( void) 
    { 
        Action  *act = m_Action; 
        m_Action = NULL;
        return act;
    } 

    void                    DoConstructTransisition( FsaId supId, FsaDfaCnstr *dfaCnstr); 

    bool                    WriteDot( Id id, void *fsaRepos, Cv_DotStream &strm) 
    { 
        strm << id.GetTypeChar() << id.GetId() << " [ shape=oval";
        strm << " color=purple label= <<FONT>" << id.GetTypeChar() << id.GetId(); 
        strm << " </FONT>>];\n "; 

        Cv_Seq< FsaId>     subStates = SubStates(); 
        for ( uint32_t k = 0; k < subStates.Size(); ++k)
        {
            FsaId           regex = subStates[ k];
            if ( !regex.GetId())
                continue;
            strm << id.GetTypeChar() << id.GetId() << " -> " << regex.GetTypeChar() << regex.GetId() << " [ arrowhead=normal color=black label=<<FONT> ";   
            strm << "</FONT>>] ; \n" ;  
        }
        return false; 
    }

    struct ElemIt
    {
        FsaElemRepos        *m_ElemRepos;
        Cv_Seq< FsaId>     m_SubStates;
        uint32_t            m_StateCursor;

        ElemIt( FsaElemRepos *elemRepos,  FsaCfaState *supState)
            :  m_ElemRepos( elemRepos), m_SubStates( supState->SubStates()), m_StateCursor( 0)
        {}

        void                Reset( void) { m_StateCursor = 0; }

        bool                IsCurValid( void) { return m_StateCursor < m_SubStates.Size(); }
        FsaElemRepos::Var   Curr( void) { return m_ElemRepos->ToVar( m_SubStates[ m_StateCursor]); }
        bool                Next( void) 
        { 
            if ( ++m_StateCursor < m_SubStates.Size())
                return true;
            return false; 
        } 
    };

    struct DescendIt : public ElemIt
    { 
        Cv_Seq< FiltId>                m_Filters;
        Cv_Seq< FsaId>                 m_DestStateIds;
        uint32_t                        m_FilterCursor;  
        std::vector< FsaCfaState *>     m_SubSupStates; 

        DescendIt( FsaElemRepos *elemRepos,  FsaCfaState *supState)
            : ElemIt( elemRepos, supState), m_FilterCursor( 0)
        {
            if ( ElemIt::IsCurValid())
                FetchFilterElems();
        }

        void                Reset( void) 
        { 
            ElemIt::Reset(); 
            m_FilterCursor = 0; 
            if ( ElemIt::IsCurValid()) 
                FetchFilterElems(); 
        }

        void                DoSetup( uint32_t szDescend, uint32_t level)
        { 
            Reset();
            for ( uint32_t k = 0; k < szDescend; ++k)
            {
                FsaCfaState     *subSupState = new FsaCfaState( level);
                m_SubSupStates.push_back( subSupState);
            }
        } 

        bool                IsCurValid( void) { return ElemIt::IsCurValid() && ( m_FilterCursor < m_Filters.Size()); }
        FilterRepos::Var    CurrFilt( void) { return m_ElemRepos->m_FilterRepos.ToVar( m_Filters[ m_FilterCursor]); } 

        bool                Next( void) 
        { 
            if ( ++m_FilterCursor < m_Filters.Size())
                return true;
            m_FilterCursor = 0;
            if ( ElemIt::Next())
            {
                FetchFilterElems(); 
                return true;
            }              
            return false; 
        } 

        void        FetchFilterElems( void);

        void        Classify( uint32_t lev, const CharDistribBase &distrib, uint16_t inv) {} 

        template < uint32_t BitSz> 
        void        Classify( uint32_t lev, const CharDistrib< BitSz> &distrib, uint16_t inv)
        {   
            while ( IsCurValid())
            { 
                FilterCrate::Var            chSet = CurrFilt();
                Sg_Bitset< BitSz>           *bitset = static_cast< ChSetFilter< BitSz> *>( chSet.GetEntry());
                Cv_Array< uint8_t, BitSz>   images = distrib.CCLImages( *bitset); 
                FsaId                       destId = m_DestStateIds[ m_FilterCursor];
                FsaCrate::Var               dest =  m_ElemRepos->ToVar( destId);
                for ( uint32_t k = 0; k < images.SzFill(); ++k)
                { 
                    FsaCfaState     *subSupState = m_SubSupStates[ images[ k]];
                    subSupState->m_SubStates.push_back( destId);  
                    subSupState->PushAction( dest( []( auto elem) { return elem->Tokens(); })); 
                }
                Next();
            } 
            return;
        } 

        void        Dump( std::ostream &strm, FilterRepos *filterRepos);
    };
}; 


//_____________________________________________________________________________________________________________________________ 
};