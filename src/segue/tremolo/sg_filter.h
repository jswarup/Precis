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
     
    int32_t         Compare( const Filter *filt) const { return 0; }
};  

//_____________________________________________________________________________________________________________________________ 

struct     CharFilter : public Filter 
{ 
    uint8_t     m_Char;

    CharFilter( void)
        : m_Char( 0)
    {}

    std::string		GetName( void) const { return Cv_Aid::ToStr( "Ch[ ", m_Char, "]"); }

    int32_t         Compare( const CharFilter *filt) const { return ( m_Char != filt->m_Char) ? (( m_Char != filt->m_Char) ? -1  : 1) : 0; }
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

    int32_t         Compare( const ChSetFilter *filt) const { return Sg_ChSet::Compare( *filt); }
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
        
        bool    operator()( const Id &id1, const Id &id2)  const
        { 
            FilterCrate::Var    var1 = m_FiltRepos->ToVar( id1);
            FilterCrate::Var    var2 = m_FiltRepos->ToVar( id2);
            if ( var1.GetType() != var2.GetType())
                return var1.GetType() < var2.GetType();
            return var1( [this]( auto e1, auto e2) {
                typedef decltype( e1)           EntType;
                return e1->Compare( static_cast< EntType>( e2)) < 0; 
            }, var2.GetEntry()); 
        } 
    };

    std::set< Id, LessOp>       m_IdTbl; 
    FilterCrate::Var            m_TVar;
 
    FilterRepos( void) 
        : m_IdTbl( LessOp( this))
    {}
      
    FilterCrate::Var    ToVar( const Id &id)  
    {  
        return id.GetType() ? Base::ToVar( id) : m_TVar; 
    }
    
template < typename Elem>
    Id          Push(  Elem &&elm) 
    {
        m_TVar = Var( &elm, FilterCrate::TypeOf< Elem>());
        auto    it = m_IdTbl.find( Id());
        if ( it != m_IdTbl.end())
            return *it;
        m_TVar = Var();
        return Base::Push( elm);
    }
};

};

//_____________________________________________________________________________________________________________________________  

