//  sg_filter.h ___________________________________________________________________________________________________________________
#pragma once

#include    "cove/barn/cv_cexpr.h"
#include 	"cove/barn/cv_aid.h"
#include    "segue/timbre/sg_chset.h"
#include 	"cove/silo/cv_craterepos.h" 

//_____________________________________________________________________________________________________________________________  

namespace Sg_RExp
{

template < uint32_t N>
struct      ChSetFilter;
struct      CharFilter; 
struct      Filter;
struct      FilterRepos;

typedef Cv_Crate< ChSetFilter< 256>, ChSetFilter< 128>, ChSetFilter< 64>, ChSetFilter< 32>, ChSetFilter< 16>, ChSetFilter< 8>, CharFilter, Filter>   FilterCrate; 

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
    std::string     ToString( void) const { return std::string(); }
    bool            Dump( FilterRepos *, std::ostream &ostr) { ostr << ToString() <<  "\n"; return true; }
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
    
    std::string     ToString( void) const { return std::string( &m_Char, &m_Char +1); }
    bool            Dump( FilterRepos *, std::ostream &ostr) { ostr << ToString() << "\n"; return true; }
};

//_____________________________________________________________________________________________________________________________ 

template < uint32_t N>
struct     ChSetFilter : public Filter, public Sg_Bitset< N>
{   
    typedef Sg_Bitset< N>       Base;

    ChSetFilter( void)
        : Base()
    {}
    
    ChSetFilter( Base &&chSet)
        : Base( chSet)
    {}

    ChSetFilter( const Base &chSet)
        : Base( chSet)
    {}

    std::string		GetName( void) const { return Cv_Aid::ToStr( "ChSet[ ", Sg_ChSet::ToString(), "]"); } 

    int32_t         Compare( const ChSetFilter *filt) const { return Base::Compare( *filt); }

    std::string     ToString( void) const { return Base::ToString(); }

    bool            Dump( FilterRepos *filterRepos, std::ostream &ostr) 
    { 
        ostr << ToString() << "\n"; return true; 
    }
};

//_____________________________________________________________________________________________________________________________ 

struct FilterRepos  : public Cv_CratePile< FilterCrate>                                
{  
    typedef Cv_CratePile< FilterCrate>      Base;
    typedef Filter::TypeStor                TypeStor;
    typedef Filter::IndexStor               IndexStor;
    typedef Filter::Id                      Id;
     


template < uint32_t N>
    struct BitsetMapper
    {
        Sg_Partition        *m_Base;
        const Sg_ChSet      *m_ChSet; 

        BitsetMapper( Sg_Partition *prtn, const Sg_ChSet *chSet)
            : m_Base( prtn), m_ChSet( chSet)
        {}

        ChSetFilter< N>     Map() 
        { 
            return ChSetFilter< N>(); 
        }
    };

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
    Sg_Partition                m_Base;
 
    FilterRepos( void) 
        : m_IdTbl( LessOp( this))
    {}
      
    FilterCrate::Var    ToVar( const Id &id)  
    {  
        if ( id.IsValid())
            return Base::ToVar( id); 
        return m_TVar;
    }
    

template < typename Elem>
    Id          Push(  Elem &&elm) 
    {
        m_TVar = Var( &elm, FilterCrate::TypeOf< Elem>());
        auto    it = m_IdTbl.find( Id());
        m_TVar = Var();
        if ( it != m_IdTbl.end())
            return *it;
        Id       id = Base::Store( elm);
        m_IdTbl.insert( id);
        return id;
    }
    
    Id  FetchId( const Sg_ChSet &chSet)
    {
        uint32_t    szImg = m_Base.SzImage();
        if ( szImg <= 8) 
            return FilterRepos::Push( BitsetMapper< 8>( &m_Base, &chSet).Map());  
        if ( szImg <= 16) 
            return FilterRepos::Push( BitsetMapper< 8>( &m_Base, &chSet).Map());  
        if ( szImg <= 32) 
            return FilterRepos::Push( BitsetMapper< 8>( &m_Base, &chSet).Map());  
        if ( szImg <= 64) 
            return FilterRepos::Push( BitsetMapper< 8>( &m_Base, &chSet).Map());  
        if ( szImg <= 128) 
            return FilterRepos::Push( BitsetMapper< 8>( &m_Base, &chSet).Map());  
        return FilterRepos::Push( ChSetFilter< 256>( chSet)); 
    }

    bool            Dump( std::ostream &ostr) 
    { 
        return OperateAll( [this, &ostr]( auto k) {  return k->Dump( this, ostr); });
    }
};

};

//_____________________________________________________________________________________________________________________________  

