//  sg_dfastate.h _______________________________________________________________________________________________________________
#pragma once

#include    "segue/colophon/sg_fsastate.h" 

//_____________________________________________________________________________________________________________________________  

namespace Sg_RExp
{ 
//_____________________________________________________________________________________________________________________________ 

struct  FsaDfaRepos  : public FsaRepos
{    
    FsaId               m_RootId;
    FsaElemRepos        *m_ElemRepos;
    DistribRepos        m_DistribRepos;   

    FsaDfaRepos( void)
        : m_ElemRepos( NULL)
    {}
 
    bool                WriteDot( Cv_DotStream &strm);  

    bool                DumpStats( std::ostream &ostr)
    {
        FsaRepos::DumpStats( ostr);
        m_DistribRepos.DumpStats( ostr);
        return true;
    }
 

    struct Cask : public Cv_MemberCask< FsaRepos, FsaId, DistribRepos>
    {  
        typedef Cv_MemberCask< FsaRepos, FsaId, DistribRepos>   BaseCask; 
        typedef typename BaseCask::ContentType                  BaseContent;

        struct  ContentType : public BaseContent
        {  
            ContentType(  const BaseContent &t2)
                : BaseContent( t2)
            {}

            auto                GetM( void) { return ((BaseCask::BaseContent *) this)->m_Value; }
        };

        static uint32_t         ContentSize( const FsaDfaRepos &obj) { return  Cv_Cask< FsaRepos>::ContentSize( obj) 
                                        +Cv_Cask< FsaId>::ContentSize( obj.m_RootId) +Cv_Cask< DistribRepos>::ContentSize( obj.m_DistribRepos); }

        static ContentType      Encase( Cv_Spritz *spritz, const FsaDfaRepos &obj)
        { 
            spritz->EnsureSize( ContentSize( obj)); 
            return BaseCask::Encase( spritz, obj, obj.m_RootId, obj.m_DistribRepos);
        }
 
    }; 

    struct Blossom  
    {
        typedef typename Cask::ContentType    ContentType; 
        typedef typename Cask::BaseContent    SubContent; 

        ContentType             *m_Root;
        FsaRepos::Blossom       m_States;   
        Blossom( void *arr)
            : m_Root( ( ContentType *) arr), m_States( ( uint8_t *) &(( ContentType *) arr)->m_Value)
        {} 
        
        FsaRepos::Blossom   *States( void) { return &m_States; }
        auto                RootId( void) { return m_Root->Base()->m_Value; }
        auto                Distribs( void) { return DistribRepos::Blossom(  &m_Root->Base()->Base()->m_Value); } 
    
        void    SauteStates( void); 
    };
};

//_____________________________________________________________________________________________________________________________ 

struct FsaSupState  : public FsaState
{   
    Cv_Slot< FsaRuleLump>        m_DfaStateMap;
    std::vector< FsaId>             m_SubStates;  
    Action                          *m_Action; 
    uint32_t                        m_Level;

    FsaSupState( uint32_t level)
        : m_Action( NULL), m_Level( level)
    {}

    ~FsaSupState( void)
    {
        if ( m_Action)
            delete m_Action;
    }

    int32_t     Compare( const FsaSupState &x2) const 
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

        ElemIt( FsaElemRepos *elemRepos,  FsaSupState *supState)
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
        FsaDfaRepos                     *m_DfaRepos;
        std::vector< FsaSupState *>     m_SubSupStates; 
 
        DescendIt( FsaElemRepos *elemRepos, FsaDfaRepos *dfaRepos, FsaSupState *supState)
            : ElemIt( elemRepos, supState), m_FilterCursor( 0), m_DfaRepos( dfaRepos)
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
                FsaSupState     *subSupState = new FsaSupState( level);
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
                    FsaSupState     *subSupState = m_SubSupStates[ images[ k]];
                    subSupState->m_SubStates.push_back( destId);  
                    subSupState->PushAction( dest( []( auto elem) { return elem->Tokens(); })); 
                }
                Next();
            } 
            return;
        } 

        void        Dump( std::ostream &strm);
    };
}; 

//_____________________________________________________________________________________________________________________________ 

struct FsaDfaState  : public FsaState
{  
private:
    Id                      m_DId;
    uint8_t                 m_MxEqClass;       
    uint8_t                 m_TokSz;   
    uint16_t                m_Inv;
    
    FsaDfaState( Id dId, uint16_t inv, uint8_t mxClass, uint8_t tokSz) 
        : m_DId( dId), m_MxEqClass( mxClass),  m_TokSz( tokSz), m_Inv( inv)
    {}

public:
    typedef FsaDfaRepos     Repos;

    ~FsaDfaState( void)
    {} 
    
    void            Delete( FsaDfaRepos *repos)
    {
        this->FsaDfaState::~FsaDfaState(); 
        delete [] ( uint8_t *) this;
    }
     
    uint16_t                DeadInd( void) { return m_Inv; }
    uint8_t                 *PastPtr( void) { return reinterpret_cast< uint8_t *>( this) +sizeof( FsaDfaState); }


    static FsaDfaState      *Construct( Id dId, uint16_t inv, uint8_t mxClass, Action *action, const Cv_Array< uint32_t, 256> &destArr)
    {
        uint32_t            sz = uint32_t( mxClass) +1;;
        uint8_t             szTok = action  ? uint8_t( action->m_Values.size()) : 0;
        auto                memSz = sizeof( FsaDfaState) + sz * sizeof( FsaId) +  szTok * sizeof( uint64_t);
        FsaDfaState         *dfaState = new ( new uint8_t[ memSz]) FsaDfaState( dId, inv, mxClass, szTok); 
        for ( uint32_t k = 0; k < sz; ++k) 
            dfaState->SetDest( k, Id( destArr[ k], 0)); 
        uint64_t            *toks = dfaState->Tokens().Ptr();
        for ( uint32_t i = 0; i < szTok; ++i)
            toks[ i] = action->m_Values[ i];
        return dfaState;
    }

    uint32_t                DestSz( void) const { return uint32_t( m_MxEqClass) +1; }
    
    DistribRepos::Id        DistribId( void) const { return m_DId; }

    Cv_Seq< FsaId>          Dests( void) { return DestSz() ? Cv_Seq< FsaId>( ( FsaId *) PastPtr(), uint32_t( DestSz())) : Cv_Seq< FsaId>(); } 
    void                    SetDest( uint8_t k, FsaId fsaId) {   Dests()[ k] = fsaId; }
    FsaId                   DestAt( uint64_t k) { return (( FsaId *) PastPtr())[ k]; } 

    uint16_t                SzTokens( void) { return m_TokSz; }
    Cv_Seq< uint64_t>       Tokens( void) { return m_TokSz ? Cv_Seq< uint64_t>( ( uint64_t *) ( PastPtr() + DestSz() * sizeof( FsaId)), uint32_t( m_TokSz)) : Cv_Seq< uint64_t>(); } 

    bool                    CleanupDestIds( FsaRepos *dfaRepos);

    bool                    WriteDot( Id id, FsaRepos *, Cv_DotStream &strm) { return false; }


template < class Atelier>
    FsaId         Eval( Atelier *atelier, uint8_t chrId)
    { 
        uint8_t             index = atelier->FetchDistib( this)( [ chrId]( auto k) { return k->Image( chrId); });
        return ( index != m_Inv) ? DestAt( index) : FsaId();
    }

template < class Atelier>
    bool                    WriteDot( Id id, Atelier *atelier, Cv_DotStream &strm)
    { 
        strm << id.GetTypeChar() << id.GetId() << " [ shape=";

        uint64_t        *toks = Tokens().Ptr();
        if ( toks)
            strm << "box color=green";
        else
            strm << "ellipse color=Red";
        strm << " label= <<FONT> " << id.GetTypeChar() << id.GetId();
        if ( m_TokSz)
            strm << "<BR />";
        for ( uint32_t i = 0; i < m_TokSz; ++i)
            strm << " T" << toks[ i];
        strm << " </FONT>>];\n "; 

        DistribCrate::Var       distrib = atelier->FetchDistib( this); 
        Cv_Seq< FsaId>          dests = Dests(); 
        for ( uint32_t k = 0; k < dests.Size(); ++k)
        {
            FsaId         regex = dests[ k];
            if ( !regex.GetId())
                continue;
            strm << id.GetTypeChar() << id.GetId() << " -> " <<  regex.GetTypeChar() << regex.GetId() << " [ arrowhead=normal color=black label=<<FONT> "; 
            strm << Cv_Aid::XmlEncode( atelier->ChSetFromImage( distrib, k).ToString());  
            strm << "</FONT>>] ; \n" ;  
        }
        return true; 
    }
 
    bool                    DoSaute( FsaDfaRepos::Blossom *bRepos);

    struct Cask : public Cv_SerializeUtils 
    {      
        typedef FsaDfaState        Type;
        typedef FsaDfaState        ContentType;  

        static uint32_t             Spread( ContentType *obj) 
        {   
            return sizeof( *obj) + obj->DestSz() * sizeof( FsaId)  + obj->m_TokSz * sizeof( uint64_t); 
        }

        static const ContentType    &Encase( Cv_Spritz *spritz, const FsaDfaState &obj) 
        {  
            FsaDfaState             &dfaState = const_cast< FsaDfaState &>( obj);
            bool                    res = spritz->Write( &dfaState, sizeof( dfaState)); 
            Cv_Seq< FsaId>         dests = dfaState.Dests();  
            res = spritz->Write( &dests[ 0], sizeof( FsaId) *  dests.Size()); 
            Cv_Seq< uint64_t>      toks =  dfaState.Tokens();   
            res = spritz->Write( &toks[ 0], sizeof( uint64_t) *  toks.Size()); 
            return obj; 
        }  

        template < typename Spritz>
        static void   SaveContent( Spritz *spritz, const FsaDfaState &obj) 
        { 
            return;
        }
    };
}; 

//_____________________________________________________________________________________________________________________________ 

template < uint32_t Sz>
struct FsaDfaByteState  : public FsaState
{ 
    uint8_t                 *PastPtr( void) { return reinterpret_cast< uint8_t *>( this) +sizeof( FsaDfaByteState< Sz>); } 
    Cv_Seq< FsaId>          Dests( void) { return Cv_Seq< FsaId>( ( FsaId *)  PastPtr(), uint32_t( Sz)); }   
    Cv_Seq< uint8_t>        Bytes( void) { return   Cv_Seq< uint8_t>( ( uint8_t *)( PastPtr() +Sz * sizeof( FsaId)), uint32_t( Sz)); } 

    static FsaDfaByteState      *Construct( void)
    { 
        auto                memSz = sizeof( FsaDfaByteState< Sz>) +  Sz * sizeof( uint8_t) + Sz * sizeof( FsaId) ;
        FsaDfaByteState     *dfaState = new ( new uint8_t[ memSz]) FsaDfaByteState();   
        return dfaState;
    } 

template < class Atelier>
    FsaId         Eval( Atelier *atelier, uint8_t chrId)
    {
        uint8_t     *bytes = Bytes().Ptr();
        FsaId       *dests = Dests().Ptr();
        for ( uint32_t i = 0; i < Sz; ++i)
            if ( chrId == bytes[ i])
                return  dests[ i];
        return Id();
    }
 
    bool                    CleanupDestIds( FsaRepos *dfaRepos)
    { 
        FsaId       *dests = Dests().Ptr();
        for ( uint32_t k = 0; k < Sz; ++k)
            dests[ k] =  dfaRepos->GetId( dests[ k].GetId());
        return true;
    }

    bool                    WriteDot( Id id, FsaRepos *, Cv_DotStream &strm) { return false; }

template < class Atelier>
    bool                    WriteDot( Id id, Atelier *atelier, Cv_DotStream &strm) 
    { 
        strm << id.GetTypeChar() << id.GetId() << " [ shape=";

        uint64_t        *toks = Tokens().Ptr(); 
        strm << "diamond  color=Orange"; 
        strm << " label= <<FONT> " << id.GetTypeChar() << id.GetId(); 
        strm << " </FONT>>];\n";  
 
        uint8_t             *bytes = Bytes().Ptr();
        FsaId               *dests = Dests().Ptr(); 
        for ( uint32_t i = 0; i < Sz; ++i)
        {
            Id       regex = dests[ i]; 
            strm << id.GetTypeChar() << id.GetId() << " -> " <<  regex.GetTypeChar() << regex.GetId() << " [ arrowhead=normal color=black label=<<FONT> ";  
            strm << Cv_Aid::XmlEncode( atelier->ChSet( bytes[ i]).ToString());  
            strm << "</FONT>>] ; \n" ;   
        }
        return true; 
    } 
 
    bool   DoSaute( FsaDfaRepos::Blossom *bRepos)
    {
        FsaId       *dests = Dests().Ptr();
        for ( uint32_t i = 0; i < Sz; ++i)
            bRepos->States()->ConvertIdToVarId( &dests[ i]);
        return true;
    }

    struct Cask : public Cv_SerializeUtils 
    {      
        typedef FsaDfaByteState        Type;
        typedef FsaDfaByteState        ContentType;  

        static uint32_t             Spread( ContentType *obj) 
        {   
            return  Sz * sizeof( uint8_t) + Sz * sizeof( FsaId);  
        }

        static const ContentType    &Encase( Cv_Spritz *spritz, const FsaDfaByteState &obj) 
        { 
            FsaDfaByteState         &dfaState = const_cast< FsaDfaByteState &>( obj); 
            bool                    res = spritz->Write( &dfaState, sizeof( dfaState)); 
            Cv_Seq< FsaId>          dests = dfaState.Dests();  
            bool                    resd = spritz->Write( &dests[ 0], sizeof( FsaId) *  Sz);  
            Cv_Seq< uint8_t>        bytes = dfaState.Bytes();  
            bool                    resb = spritz->Write( &bytes[ 0], sizeof( uint8_t) * Sz); 
            return obj; 
        }  

        template < typename Spritz>
        static void   SaveContent( Spritz *spritz, const FsaDfaByteState &obj) 
        { 
            return;
        }
    };
};

 

//_____________________________________________________________________________________________________________________________ 

struct FsaDfaXByteState  : public FsaState
{
    uint8_t                 m_MxEqClass; 
    
    FsaDfaXByteState( uint32_t mxSz)
        : m_MxEqClass( mxSz)
    {}

    uint8_t                 *PastPtr( void) { return reinterpret_cast< uint8_t *>( this) +sizeof( FsaDfaXByteState); }

    uint32_t                DestSz( void) const { return uint32_t( m_MxEqClass) +1; }
    Cv_Seq< uint8_t>        Bytes( void) { return DestSz() ? Cv_Seq< uint8_t>( ( uint8_t *) PastPtr(), uint32_t( DestSz())) : Cv_Seq< uint8_t>(); }   
    Cv_Seq< FsaId>          Dests( void) { return DestSz() ? Cv_Seq< FsaId>( ( FsaId *) ( PastPtr() +DestSz() * sizeof( uint8_t)), uint32_t( DestSz())) : Cv_Seq< FsaId>(); } 

    static FsaDfaXByteState      *Construct( uint32_t sz)
    {
        uint8_t             mxSz = uint8_t( sz -1);
        auto                memSz = sizeof( FsaDfaXByteState) + sz * sizeof( uint8_t) + sz * sizeof( FsaId) ;
        FsaDfaXByteState     *dfaState = new ( new uint8_t[ memSz]) FsaDfaXByteState( mxSz);   
        return dfaState;
    } 

    bool                    CleanupDestIds( FsaRepos *dfaRepos);

template < class Atelier>
    FsaId       Eval( Atelier *atelier, uint8_t chrId)
    { 
        Cv_Seq< uint8_t>            bytes = Bytes();
        Cv_Seq< FsaDfaRepos::Id>    dests = Dests(); 
        for ( uint32_t i = 0; i < bytes.Size(); ++i)
            if ( chrId == bytes[ i])
                return  dests[ i];
        return FsaDfaRepos::Id();
    } 

    bool                    WriteDot( Id id, FsaRepos *, Cv_DotStream &strm) { return false; }

template < class Atelier>
    bool                    WriteDot( Id id, Atelier *atelier, Cv_DotStream &strm) 
    { 
        strm << id.GetTypeChar() << id.GetId() << " [ shape=";

        uint64_t        *toks = Tokens().Ptr(); 
        strm << "diamond"; 
        strm << " color=Red label= <<FONT> " << id.GetTypeChar() << id.GetId(); 
        strm << " </FONT>>];\n"; 
 
        Cv_Seq< FsaId>              dests = Dests(); 
        Cv_Seq< uint8_t>            bytes = Bytes(); 
        for ( uint32_t k = 0; k < dests.Size(); ++k)
        {
            FsaId       regex =  dests[ k];
            strm << id.GetTypeChar() << id.GetId() << " -> " <<  regex.GetTypeChar() << regex.GetId() << " [ arrowhead=normal color=black label=<<FONT> "; 
            strm << Cv_Aid::XmlEncode( atelier->ChSet( bytes[ k]).ToString());  
            strm << "</FONT>>] ; \n" ;   
        }
        return true; 
    }

    bool                    DoSaute( FsaDfaRepos::Blossom *bRepos);

    struct Cask : public Cv_SerializeUtils 
    {      
        typedef FsaDfaXByteState        Type;
        typedef FsaDfaXByteState        ContentType;  

        static uint32_t             Spread( ContentType *obj) 
        {   
            return sizeof( *obj) + obj->DestSz() * sizeof( uint8_t) + obj->DestSz() * sizeof( FsaId);  
        }

        static const ContentType    &Encase( Cv_Spritz *spritz, const FsaDfaXByteState &obj) 
        { 
            FsaDfaXByteState         &dfaState = const_cast< FsaDfaXByteState &>( obj);
            bool                    res = spritz->Write( &dfaState, sizeof( dfaState)); 
            Cv_Seq< uint8_t>        bytes = dfaState.Bytes();  
            res = spritz->Write( &bytes[ 0], sizeof( uint8_t) *  bytes.Size()); 
            Cv_Seq< FsaId>          dests = dfaState.Dests();  
            res = spritz->Write( &dests[ 0], sizeof( FsaId) *  dests.Size());  
            return obj; 
        }  

        template < typename Spritz>
        static void   SaveContent( Spritz *spritz, const FsaDfaXByteState &obj) 
        { 
            return;
        }
    };
};

//_____________________________________________________________________________________________________________________________ 

struct FsaClip  : public FsaCrate::Var
{
    typedef FsaCrate::Var       FsaVar;
    typedef FsaRepos::Id        FsaId;
    typedef FilterCrate::Var    FiltVar;
    typedef FilterRepos::Id     FiltId;
    
    FsaClip( void)
    {}

    FsaClip( const FsaVar &v)
        : FsaVar( v)
    {} 

    Cv_Seq< uint64_t>          Tokens( void) { return SELF( [this]( auto k) { return k->Tokens(); }); }

    Cv_Seq< FsaId>             Dests( void) { return SELF( [this]( auto k) { return k->Dests(); }); }
    Cv_Seq< FiltId>            Filters( void) { return SELF( [this]( auto k) { return k->Filters(); }); } 
    Cv_Seq< FsaId>             SubStates( void) { return SELF( [this]( auto k) { return k->SubStates(); }); }
};

//_____________________________________________________________________________________________________________________________ 

struct  FsaRuleLump : public Cv_ReposEntry, public Cv_Shared
{   
    typedef std::map< FsaSupState*, uint32_t, Cv_TPtrLess< void> >     SupDfaMap;

    FsaRuleLumpSet          *m_LumpSet; 
    std::set< uint32_t>     m_Ruleset;
    SupDfaMap               m_SupDfaIdMap;

    FsaRuleLump(  const std::set< uint32_t> &ruleset)
        : m_LumpSet( NULL), m_Ruleset( ruleset)
    {}

    ~FsaRuleLump( void);

    int32_t                 Compare( const FsaRuleLump &dsMap) const
    {
        if ( m_Ruleset.size() != dsMap.m_Ruleset.size())
            return m_Ruleset.size() > dsMap.m_Ruleset.size() ? 1 : -1;
        for ( auto it1 = m_Ruleset.begin(), it2 = dsMap.m_Ruleset.begin(); it1 != m_Ruleset.end(); ++it1, ++it2)
            if ( *it1 != *it2)
                return *it1 < *it2  ? 1 : -1;
        return 0;
    }   

    void                    Register(  FsaSupState *supState, uint32_t stateId)
    {
        m_SupDfaIdMap.insert( std::make_pair( supState, stateId));
    }

    uint32_t                Find( FsaSupState *supState)
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

struct  FsaRuleLumpSet
{ 
    std::set< FsaRuleLump *, Cv_TPtrLess< void> >       m_Maps;

    FsaRuleLumpSet( FsaDfaCnstr *cnstr) 
    {}

    void    Erase( FsaRuleLump  *dsMap)
    {
        m_Maps.erase( dsMap);
    }

    Cv_Slot< FsaRuleLump>    Locate( FsaElemRepos *elemRepos, FsaSupState *supState)
    {
        FsaRuleLump     *dfaStatemap = new FsaRuleLump( supState->RuleIds( elemRepos));
        auto            it =  m_Maps.lower_bound( dfaStatemap);
        if (( it == m_Maps.end())  || m_Maps.key_comp()( dfaStatemap, *it))
        {   
            m_Maps.insert( it, dfaStatemap);
            dfaStatemap->m_LumpSet = this;         
            return dfaStatemap;
        }
        delete dfaStatemap;
        return *it;           
    } 
    bool            Dump( std::ostream &ostr) 
    { 
        for ( auto it = m_Maps.begin(); it != m_Maps.end(); ++it)
        {
            FsaRuleLump     *si = *it;  
            si->Dump( ostr);
        }
        return true;
    }
};

//_____________________________________________________________________________________________________________________________ 

struct  FsaDfaCnstr 
{ 
    typedef FsaRepos::Id                FsaId; 

    FsaElemRepos                        *m_ElemRepos; 
    FsaDfaRepos                         *m_DfaRepos;               
    std::vector< FsaId>                 m_FsaStk;
    FsaRuleLumpSet                      m_RuleLumpSet;

    FsaDfaCnstr( FsaElemRepos *elemRepos, FsaDfaRepos *dfaRepos)
        : m_ElemRepos( elemRepos), m_DfaRepos( dfaRepos), m_RuleLumpSet( this)
    {
        dfaRepos->m_ElemRepos = elemRepos;
        dfaRepos->m_DistribRepos.SetBaseDistrib( elemRepos->m_FilterRepos.BaseDistrib());
    }

    void        SubsetConstruction( void);
    bool        DumpDot( const char *path);

    void        ConstructDfaStateAt( uint32_t index, const DistribRepos::DfaDistrib &distrib, Action *action, const Cv_Array< uint32_t, 256> &destArr);
};


//_____________________________________________________________________________________________________________________________ 

};