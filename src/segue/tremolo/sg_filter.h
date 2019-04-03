//  sg_filter.h ___________________________________________________________________________________________________________________
#pragma once

#include    "cove/barn/cv_cexpr.h"
#include 	"cove/barn/cv_aid.h"
#include    "segue/timbre/sg_chset.h"
#include 	"cove/silo/cv_craterepos.h" 

//_____________________________________________________________________________________________________________________________  

namespace Sg_RExp
{
struct      ChSetFilter;
struct      CharFilter; 
struct      Filter;

typedef Cv_Crate< ChSetFilter, CharFilter, Filter>   FilterCrate; 


//_____________________________________________________________________________________________________________________________ 
 
struct  Filter : public Cv_CrateEntry
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

    int32_t         Compare( CharFilter *filt) const { return ( m_Char != filt->m_Char) ? (( m_Char != filt->m_Char) ? -1  : 1) : 0; }
};

//_____________________________________________________________________________________________________________________________ 

struct     ChSetFilter : public Filter, public Sg_ChSet
{   
    ChSetFilter( void)
        : Sg_ChSet()
    {}
    
    ChSetFilter( Sg_ChSet &&chSet)
        : Sg_ChSet( chSet)
    {}

    ChSetFilter( const Sg_ChSet &chSet)
        : Sg_ChSet( chSet)
    {}

    std::string		GetName( void) const { return Cv_Aid::ToStr( "ChSet[ ", Sg_ChSet::ToString(), "]"); } 

    int32_t         Compare( ChSetFilter *filt) const { return Sg_ChSet::Compare( *filt); }
};

//_____________________________________________________________________________________________________________________________
 
template < typename Crate, typename=void>
struct Cv_CratePile : public Cv_CratePile< typename Crate::CrateBase>
{
    typedef Cv_CratePile< typename Crate::CrateBase>    Base;

    typedef typename Crate::Entry       Entry;
    typedef typename Crate::Elem        Elem;
    typedef typename Entry::Id          Id;

    std::vector< Elem>                  m_Elems;
 
    Id    Push( const Elem &elm) 
    { 
        m_Elems.push_back( elm); 
        Elem    &insrt =  m_Elems.back();
        insrt.SetId( m_Elems.size() -1);
        insrt.SetType( Crate::Sz);
        return insrt; 
    }

 

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
    typedef typename Entry::Id          Id;

    std::vector< Elem>                  m_Elems; 

    Id    Push( const Elem &elm) 
    { 
        m_Elems.push_back( elm); 
        Elem    &insrt =  m_Elems.back();
        insrt.SetId( m_Elems.size() -1);
        insrt.SetType( Crate::Sz);
        return insrt;  
    }  

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
    typedef Filter::Id                     Id;
     

    struct LessOp
    {
        FilterRepos     *m_FiltRepos;

        LessOp( FilterRepos *filtRepos)
            : m_FiltRepos( filtRepos)
        {}
        
        bool    operator()( const Id &id1, const Id &id2)  
        { 
            FilterCrate::Var    var1 = m_FiltRepos->ToVar( id1);
            FilterCrate::Var    var2 = m_FiltRepos->ToVar( id2);
            if ( var1.GetType() != var2.GetType())
                return var1.GetType() < var2.GetType();
            var1( [this]( auto e1, auto e2) {
                typedef decltype( e1)           EntType;
                return e1->IsLess( static_cast< EntType>( e2)); 
            }, var2.GetEntry());
            return id2.m_IPtr;  
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
    
    FilterCrate::Var    ToVar( const Id &id)  
    {  
        return id.GetType() ? FilterCrate::Var( m_FilterIndFns[ id.GetType() -1]( id.GetId()), id.GetType()) : m_TVar; 
    }

};

};

//_____________________________________________________________________________________________________________________________  

