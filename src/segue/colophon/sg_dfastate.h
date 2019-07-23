//  sg_dfastate.h _______________________________________________________________________________________________________________
#pragma once

#include    "segue/colophon/sg_cfastate.h" 
#include    "segue/colophon/sg_rulelump.h" 

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