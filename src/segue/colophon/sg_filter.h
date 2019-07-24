//  sg_filter.h ___________________________________________________________________________________________________________________
#pragma once

#include    "cove/barn/cv_cexpr.h"
#include 	"cove/barn/cv_aid.h"
#include    "segue/epigraph/sg_chset.h"
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
public:
    typedef void                    Copiable;

    Filter( void) 
    {} 

    std::string		GetName( void) const { return "Filter"; } 
     
    int32_t         Compare( const Filter *filt) const { return 0; }
    std::string     ToString( const Sg_Partition *base) const { return std::string(); }
    bool            Dump( const Sg_Partition *base, std::ostream &ostr) { ostr << ToString( base) <<  "\n"; return true; }
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
    
    std::string     ToString( const Sg_Partition *base) const { return std::string( &m_Char, &m_Char +1); }
    bool            Dump( const Sg_Partition *base, std::ostream &ostr) { ostr << ToString( base) << "\n"; return true; }
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

    std::string     ToString( const Sg_Partition *base) const 
    { 
        Sg_ChSet      chSet; 
        for ( uint32_t i = 0; i < Sg_ChSet::SzChBits; ++i)
            if ( this->Get( base->Image( i)))
                chSet.Set( i, true); 
        return chSet.ToString(); 
    }

    bool            Dump( const Sg_Partition *base, std::ostream &ostr) 
    {  
        ostr << ToString( base) << " "; 
        return true; 
    }
};

//_____________________________________________________________________________________________________________________________ 

struct FilterRepos  : public Cv_CratePile< FilterCrate>                                
{  
    typedef Cv_CratePile< FilterCrate>      Base;
    typedef Filter::TypeStor                TypeStor;
    typedef Filter::IndexStor               IndexStor; 

template < uint32_t N>
    struct BitsetMapper
    {
        Sg_Partition        *m_Base; 

        BitsetMapper( Sg_Partition *prtn)
            : m_Base( prtn) 
        {}

        ChSetFilter< N>     Map( const Sg_ChSet &chSet)  { return m_Base->Map< N>( chSet); }
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

private:
    std::set< Id, LessOp>       m_IdTbl; 
    FilterCrate::Var            m_TVar;
    Sg_Partition                m_Base;
    uint16_t                    m_SzImg;

public:
    FilterRepos( void) 
        : m_IdTbl( LessOp( this)), m_SzImg( 0)
    {}

    const Sg_Partition          &BaseDistrib( void) const { return m_Base; }
    void                        SetBaseDistrib( const Sg_Partition &base) 
    { 
        m_Base = base; 
        m_SzImg = m_Base.SzImage();
    } 

    FilterCrate::Var    ToVar( const Id &id)  
    {  
        if ( id.IsValid())
            return Base::ToVar( id); 
        return m_TVar;
    }
    
template < typename Elem>
    Id          Store(  Elem &&elm) 
    {
        uint8_t      type = FilterCrate::template TypeOf< Elem>() ;
        m_TVar = Var( &elm, type);
        auto    it = m_IdTbl.find( Id());
        m_TVar = Var();
        if ( it != m_IdTbl.end())
            return *it;
        Id       id = Base::Store( type, elm);
        m_IdTbl.insert( id);
        return id;
    }
    
    Id  FetchId( const Sg_ChSet &chSet)
    { 
        if ( m_SzImg <= 8)                                       
            return Store( BitsetMapper< 8>( &m_Base).Map( chSet));  
        if ( m_SzImg <= 16)                                      
            return Store( BitsetMapper< 16>( &m_Base).Map( chSet));  
        if ( m_SzImg <= 32)                                      
            return Store( BitsetMapper< 32>( &m_Base).Map( chSet));  
        if ( m_SzImg <= 64)                                      
            return Store( BitsetMapper< 64>( &m_Base).Map( chSet));  
        if ( m_SzImg <= 128)                                     
            return Store( BitsetMapper< 128>( &m_Base).Map( chSet));  
        return Store( ChSetFilter< 256>( chSet)); 
    }

    bool            Dump( std::ostream &ostr, Id fId) 
    { 
        Var     fVar = ToVar( fId);
        return fVar( [this, &ostr]( auto k) {  return k->Dump(  &m_Base, ostr); });
    }

    bool            Dump( std::ostream &ostr) 
    { 
        return OperateAll( [this, &ostr]( auto k, uint32_t ind) {  return k->Dump(  &m_Base, ostr); });
    }

    std::string     ToString( Var chVar) const 
    {
        std::string     str = chVar( [this]( auto k) { return k->ToString(  &m_Base); });
        return str;
    }
};

//_____________________________________________________________________________________________________________________________ 

};
