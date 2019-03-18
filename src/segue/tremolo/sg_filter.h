//  sg_filter.h ___________________________________________________________________________________________________________________
#pragma once

#include    "cove/barn/cv_cexpr.h"
#include 	"cove/barn/cv_aid.h"
#include    "segue/timbre/sg_chset.h"
#include 	"cove/silo/cv_crate.h" 

//_____________________________________________________________________________________________________________________________  

namespace Sg_RExp
{
struct      ChSetFilter;
struct      CharFilter; 
struct      Filter;

typedef Cv_Crate< ChSetFilter, CharFilter, Filter>   FilterCrate; 


//_____________________________________________________________________________________________________________________________ 
 
struct  Filter  
{ 
    typedef uint8_t     TypeStor;
    typedef uint16_t    IndexStor;

public:
    Filter( void) 
    {} 

    std::string		GetName( void) const { return "Filter"; } 
    
    bool    IsLess( Filter *filt) const { return false; }
};  

//_____________________________________________________________________________________________________________________________ 

struct     CharFilter : public Filter 
{ 
    uint8_t     m_Char;

    CharFilter( void)
        : m_Char( 0)
    {}

    std::string		GetName( void) const { return Cv_Aid::ToStr( "Ch[ ", m_Char, "]"); }

    bool    IsLess( CharFilter *filt) const { return m_Char < filt->m_Char; }
};

//_____________________________________________________________________________________________________________________________ 

struct     ChSetFilter : public Filter 
{ 
    Sg_ChSet     m_ChSet;

    ChSetFilter( void)
        : m_ChSet()
    {}

    std::string		GetName( void) const { return Cv_Aid::ToStr( "ChSet[ ", m_ChSet.ToString(), "]"); } 

    bool    IsLess( ChSetFilter *filt) const { return m_ChSet.IsLess( filt->m_ChSet); }
};

//_____________________________________________________________________________________________________________________________
 
template < typename Crate, typename=void>
struct Cv_CratePile : public Cv_CratePile< typename Crate::CrateBase>
{
    typedef Cv_CratePile< typename Crate::CrateBase>    Base;

    typedef typename Crate::Entry        Entry;
    typedef typename Crate::Elem        Elem;
    std::vector< Elem>                  m_Elems;
 
    uint32_t    Push( const Elem &elm) { m_Elems.push_back( elm); return uint32_t( m_Elems.size()) -1; }

 

    void    SetupFilterIndFns( std::function< Entry *( uint32_t ind)> *filterIndFns)
    {
        filterIndFns[ Crate::Sz -1] = [this]( uint32_t ind) { return &m_Elems[ ind]; };
        Base::SetupFilterIndFns( filterIndFns);
    }
};

template<typename Crate>
struct  Cv_CratePile< Crate, typename  Cv_TypeEngage::Same< typename Crate::Entry, typename Crate::Entry>::Note>  
{ 

    typedef typename Crate::Entry       Entry;
    typedef typename Crate::Elem        Elem;
    std::vector< Elem>                  m_Elems; 

    uint32_t    Push( const Elem &elm) { m_Elems.push_back( elm); return uint32_t( m_Elems.size()) -1; }  

    void        SetupFilterIndFns( std::function< Entry *( uint32_t ind)> *filterIndFns)
    {
        filterIndFns[ Crate::Sz -1] = [this]( uint32_t ind) { return &m_Elems[ ind]; };
    }
};

//_____________________________________________________________________________________________________________________________ 

struct FilterRepos  : public Cv_CratePile< FilterCrate>                                
{  
    typedef Cv_CratePile< FilterCrate>      Base;
    typedef Filter::TypeStor                TypeStor;
    typedef Filter::IndexStor               IndexStor;
    
    struct Id
    {   
        uint32_t        m_TypeInd;
        
        Id( uint8_t type, uint32_t ind)
            : m_TypeInd( ( type << 24) | ( 0xFFFFFF & ind))
        {} 

        uint8_t    Type( void) const { return uint8_t( m_TypeInd>> 24); }
        uint32_t   Index( void) const { return 0xFFFFFF & m_TypeInd; }
    };

    struct LessOp
    {
        FilterRepos     *m_FiltRepos;

        LessOp( FilterRepos *filtRepos)
            : m_FiltRepos( filtRepos)
        {}
        
        bool    operator()( const Id &id1, const Id &id2)  
        { 
            FilterCrate::Var    var1 = m_FiltRepos->Var( id1);
            FilterCrate::Var    var2 = m_FiltRepos->Var( id2);
            if ( var1.GetType() != var2.GetType())
                return var1.GetType() < var2.GetType();
            var1( [this]( auto e1, auto e2) {
                typedef decltype( e1)           EntType;
                return e1->IsLess( static_cast< EntType>( e2)); 
            }, var2.GetEntry());
            return id2.m_TypeInd;  
        } 
    };

    std::vector< std::set< Id, LessOp> >        m_IdTbl;
    std::function< Filter *( uint32_t ind)>     m_FilterIndFns[ FilterCrate::Sz];
    FilterCrate::Var                            m_TVar;
 
    FilterRepos( void) 
    {
        m_IdTbl.resize( FilterCrate::Sz, std::set< Id, LessOp>( LessOp( this)));
        Base::SetupFilterIndFns( m_FilterIndFns);
    }
    
    FilterCrate::Var    Var( const Id &id)  
    {  
        return id.m_TypeInd ? FilterCrate::Var( m_FilterIndFns[ id.Type() -1]( id.Index()), id.Type()) : m_TVar; 
    }

template < typename Filter>
    Id          Push( const Filter &filter) {  return Id( FilterCrate::TypeOf< Filter>(), Base::Push( filter)); }
};

};

//_____________________________________________________________________________________________________________________________  

