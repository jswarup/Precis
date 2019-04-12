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
    {
        SetType( FilterCrate::TypeOf< ChSetFilter< N>>());
    }
    
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
        Sg_ChSet      chSet; 
        for ( uint32_t i = 0; i < Sg_ChSet::SzChBits; ++i)
            if ( Get( filterRepos->m_Base.Image( i)))
                chSet.Set( i, true); 
        ostr << chSet.ToString() << "\n"; 
        return true; 
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
    Id          Store(  Elem &&elm) 
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
            return Store( BitsetMapper< 8>( &m_Base).Map( chSet));  
        if ( szImg <= 16)                                      
            return Store( BitsetMapper< 16>( &m_Base).Map( chSet));  
        if ( szImg <= 32)                                      
            return Store( BitsetMapper< 32>( &m_Base).Map( chSet));  
        if ( szImg <= 64)                                      
            return Store( BitsetMapper< 64>( &m_Base).Map( chSet));  
        if ( szImg <= 128)                                     
            return Store( BitsetMapper< 128>( &m_Base).Map( chSet));  
        return Store( ChSetFilter< 256>( chSet)); 
    }

    bool            Dump( std::ostream &ostr) 
    { 
        return OperateAll( [this, &ostr]( auto k) {  return k->Dump( this, ostr); });
    }
};


//_____________________________________________________________________________________________________________________________  

template < uint32_t N>
struct      CharDistrib;
struct      CharDistribBase;
struct      DistribRepos;

typedef Cv_Crate< CharDistrib< 256>, CharDistrib< 128>, CharDistrib< 64>, CharDistrib< 32>, CharDistrib< 16>, CharDistrib< 8>, CharDistribBase>   DistribCrate; 

//_____________________________________________________________________________________________________________________________ 

struct CharDistribBase : public Cv_CrateEntry
{  
    typedef uint8_t     TypeStor;
    typedef uint16_t    IndexStor;

public:
    CharDistribBase( void) 
    {} 

    std::string		GetName( void) const { return "Filter"; } 

    int32_t         Compare( const CharDistribBase *filt) const { return 0; }
    std::string     ToString( void) const { return std::string(); }
    bool            Dump( DistribRepos *, std::ostream &ostr) { ostr << ToString() <<  "\n"; return true; }
};

//_____________________________________________________________________________________________________________________________ 

template < uint32_t Bits>
struct CharDistrib : Sg_CharPartition< Bits>, public CharDistribBase
{  
    typedef uint8_t     TypeStor;
    typedef uint16_t    IndexStor;

public:
    CharDistrib( void) 
    {} 

    std::string		GetName( void) const { return "Filter"; } 

    int32_t         Compare( const CharDistrib *filt) const { return 0; }
    std::string     ToString( void) const { return std::string(); }
    bool            Dump( DistribRepos *, std::ostream &ostr) { ostr << ToString() <<  "\n"; return true; }
};

//_____________________________________________________________________________________________________________________________ 
 
struct DistribRepos  : public Cv_CratePile< DistribCrate>           
{

    Sg_Partition                m_Base;
    
    DistribRepos( void)
    {}

    struct Discr
    {
        Id          m_DId;
        uint32_t    m_Inv;
        uint32_t    m_NxSz;
        
        Discr( Id id, uint32_t inv, uint32_t imgSz)
            :   m_DId( id), m_Inv( inv), m_NxSz( imgSz)
        { }

        
    };
  
template < uint32_t Bits>
    struct Helper
    {
        DistribRepos        *m_DRepos;

        Helper( DistribRepos *dRepos)
            : m_DRepos( dRepos)
        {}

    template < typename FilterIt>
        Discr    Map( FilterIt *filtIt)
        {
            CharDistrib< Bits>                  distrib;
            CharDistrib< Bits>::CCLImpressor    intersector( &distrib);

            while ( filtIt->IsCurValid())
            { 
                Sg_Bitset< Bits>      *bitset = static_cast< ChSetFilter< Bits> *>( filtIt->Curr().GetEntry());
                intersector.Process( *bitset); 
                filtIt->Next();
            }    
            auto            invalidCCL =  intersector.ValidCCL().Negative();
            uint32_t        invRep = invalidCCL.RepIndex();
            uint8_t         invInd = ( invRep != CV_UINT32_MAX) ? distrib.Image( invRep) : CV_UINT32_MAX; 
            return Discr( m_DRepos->Store( distrib), invInd, distrib.SzImage());
        }
    };  
  
template < typename FilterIt>
    Discr  FetchDiscr( FilterIt *filtit)
    { 
        uint32_t    szImg = m_Base.SzImage();
        if ( szImg <= 8)  
            return Helper< 8>( this).Map( filtit);                                  

        if ( szImg <= 16)  
            return Helper< 16>( this).Map( filtit);                                  
        
        if ( szImg <= 32)                                      
            return Helper< 32>( this).Map( filtit);                                  

        if ( szImg <= 64)  
            return Helper< 64>( this).Map( filtit);                                  

        if ( szImg <= 128)                                     
            return Helper< 128>( this).Map( filtit);                                  

        return Helper< 256>( this).Map( filtit);                                  
    }
}; 

//_____________________________________________________________________________________________________________________________ 

};

//_____________________________________________________________________________________________________________________________  

