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
        m_TVar = Var( &elm, FilterCrate::template TypeOf< Elem>() );
        auto    it = m_IdTbl.find( Id());
        m_TVar = Var();
        if ( it != m_IdTbl.end())
            return *it;
        Id       id = Base::Store( FilterCrate::template TypeOf< Elem>(), elm);
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

template < uint32_t N>
struct      CharDistrib;
struct      CharDistribBase;
struct      DistribRepos;

typedef Cv_Crate< CharDistrib< 256>, CharDistrib< 128>, CharDistrib< 64>, CharDistrib< 32>, CharDistrib< 16>, CharDistrib< 8>, CharDistribBase>   DistribCrate; 

//_____________________________________________________________________________________________________________________________ 

struct CharDistribBase : public Cv_CrateEntry
{   
    typedef void                    Copiable;

    CharDistribBase( void) 
    {} 

    std::string		        GetName( void) const { return "Filter"; } 

    uint8_t                 Image( uint16_t t) const {  return 0; }

    Sg_Bitset< 256>         ChSet( uint8_t byteCode) const  { return Sg_Bitset< 256>(); }

    int32_t                 Compare( const CharDistribBase *filt) const { return 0; }
    std::string             ToString( void) const { return std::string(); }
    bool                    Dump( DistribRepos *, std::ostream &ostr) { ostr << ToString() <<  " "; return true; }
    std::vector< Sg_Bitset< 0> >   Domain( void) const { return std::vector< Sg_Bitset< 0> >(); }
};

//_____________________________________________________________________________________________________________________________ 

template < uint32_t Bits>
struct CharDistrib : public Sg_CharPartition< Bits>, public CharDistribBase
{  

    typedef void                    Copiable;

    enum  { 
        BitSz = Bits
    };    

public:
    CharDistrib( void) 
    {} 

    std::string		    GetName( void) const { return "Distrib"; } 

    uint8_t             Image( uint16_t chr) const {  return Sg_CharPartition< Bits>::Image( chr); }

    Sg_Bitset< Bits>    ChSet( uint8_t byteCode) const  { return Sg_CharPartition< Bits>::ChSet( byteCode); }

    int32_t             Compare( const CharDistrib *filt) const { return Sg_CharPartition< Bits>::Compare( *filt); }
    std::string         ToString( void) const { return std::string(); }
    bool                Dump( DistribRepos *, std::ostream &ostr) 
    { 
        Sg_CharPartition< Bits>::Dump( ostr);; 
        return true; 
    }

    auto            Domain( void) const { return Sg_CharPartition< Bits>::Domain(); }
};


//_____________________________________________________________________________________________________________________________ 

struct DistribVar : public DistribCrate::Var
{
    DistribVar( uint32_t szImg)
    {
        if ( szImg <= 8)
        {
            m_Entry = new CharDistrib< 8>();
            m_Type = DistribCrate::template TypeOf< CharDistrib< 8>>();
        }
        else if ( szImg <= 16)  
        {
            m_Entry = new CharDistrib< 16>();
            m_Type = DistribCrate::template TypeOf< CharDistrib< 16>>();
        }
        else if ( szImg <= 32)                                      
        {
            m_Entry = new CharDistrib< 32>();
            m_Type = DistribCrate::template TypeOf< CharDistrib< 32>>();
        }
        else if ( szImg <= 64)  
        {
            m_Entry = new CharDistrib< 64>();
            m_Type = DistribCrate::template TypeOf< CharDistrib< 64>>();
        }
        else if ( szImg <= 128)                                     
        {
            m_Entry = new CharDistrib< 128>();
            m_Type = DistribCrate::template TypeOf< CharDistrib< 128>>();
        }
        else 
        {
            m_Entry = new CharDistrib< 256>();
            m_Type = DistribCrate::template TypeOf< CharDistrib< 256>>(); 
        }
    }

    ~DistribVar( void)
    {
        SELF( []( auto k) { delete k; });
    }

    int32_t     Compare( const DistribVar &var)  
    { 
        if ( GetType() != var.GetType())
            return GetType() < var.GetType() ? -1 : 1;
        return SELF( [this]( auto e1, auto e2) {
                typedef decltype( e1)           EntType;
                return e1->Compare( static_cast< EntType>( e2)); 
            }, var.GetEntry()); 
    }

    std::vector< Sg_ChSet>  Domain( Sg_Partition *base)
    { 
        std::vector< Sg_ChSet>  domain = SELF( [base]( auto dist) {
            typedef decltype(dist)  Distrib;
            auto                    distDomain = dist->Domain();
            std::vector< Sg_ChSet>  dom( distDomain.size());
            for ( uint32_t  i = 0; i < distDomain.size(); ++i)
            {
                dom[ i] = base->XForm( distDomain[ i]);
            }
            return dom; } );
        return domain;
    }
};  

//_____________________________________________________________________________________________________________________________ 
 
struct DistribRepos  : public Cv_CratePile< DistribCrate>           
{
    typedef Cv_CratePile< DistribCrate>     Base;

    struct LessOp
    {
        DistribRepos     *m_DistribRepos;

        LessOp( DistribRepos *distribRepos)
            : m_DistribRepos( distribRepos)
        {}

        bool    operator()( const Id &id1, const Id &id2)  const
        { 
            DistribCrate::Var    var1 = m_DistribRepos->ToVar( id1);
            DistribCrate::Var    var2 = m_DistribRepos->ToVar( id2);
            if ( var1.GetType() != var2.GetType())
                return var1.GetType() < var2.GetType();
            return var1( [this]( auto e1, auto e2) {
                typedef decltype( e1)           EntType;
                return e1->Compare( static_cast< EntType>( e2)) < 0; 
                }, var2.GetEntry()); 
        } 
    };


    std::set< Id, LessOp>       m_IdTbl; 
    DistribCrate::Var           m_TVar;
    Sg_Partition                m_Base; 

    DistribRepos( void) 
        : m_IdTbl( LessOp( this))
    {}

    struct Discr
    {
        Id          m_DId;
        uint16_t    m_Inv;
        uint8_t     m_MxEqClass; 

        Discr( void)
            : m_Inv( 0), m_MxEqClass( uint8_t( -1))
        { }

        Discr( Id id, uint16_t inv, uint8_t mxEqClass)
            :   m_DId( id), m_Inv( inv), m_MxEqClass( mxEqClass)
        { }

        uint32_t    SzDescend( void) const { return m_MxEqClass +1; }
        
        void        Dump( std::ostream &strm, DistribRepos *dRepos)
        {
            std::vector< Sg_ChSet>  domain = dRepos->Domain(  m_DId);
            for ( uint32_t k = 0; k < SzDescend(); ++k)
                strm << Cv_Aid::XmlEncode( domain[ k].ToString()) << ' ';  
            strm << m_Inv; 
        }
    };
  
template < uint32_t Bits>
    struct DiscrHelper
    {
        DistribRepos        *m_DRepos;

        DiscrHelper( DistribRepos *dRepos)
            : m_DRepos( dRepos)
        {}

    template < typename DescendIt>
        Discr    Map( DescendIt *filtIt)
        {
            typedef typename CharDistrib< Bits>::CCLImpressor   CCLImpressor;
            
            CharDistrib< Bits>                  distrib;
            CCLImpressor        intersector( &distrib);

            while ( filtIt->IsCurValid())
            { 
                Sg_Bitset< Bits>      *bitset = static_cast< ChSetFilter< Bits> *>( filtIt->CurrFilt().GetEntry());
                intersector.Process( *bitset); 
                filtIt->Next();
            }    
            intersector.Over();
            auto            invalidCCL =  intersector.ValidCCL().Negative();
            uint32_t        invRep = invalidCCL.RepIndex();
            uint16_t        invInd = ( invRep != CV_UINT32_MAX) ? distrib.Image( invRep) : CV_UINT16_MAX; 
            Discr           discr( m_DRepos->Store( distrib), invInd, uint8_t( distrib.SzImage() -1)); 
            return discr;
        }
    };  
  
template < typename DescendIt>
    Discr  FetchDiscr( DescendIt *filtit)
    { 
        uint32_t    szImg = m_Base.SzImage();
        if ( szImg <= 8)  
            return DiscrHelper< 8>( this).Map( filtit);                                  

        if ( szImg <= 16)  
            return DiscrHelper< 16>( this).Map( filtit);                                  
        
        if ( szImg <= 32)                                      
            return DiscrHelper< 32>( this).Map( filtit);                                  

        if ( szImg <= 64)  
            return DiscrHelper< 64>( this).Map( filtit);                                  

        if ( szImg <= 128)                                     
            return DiscrHelper< 128>( this).Map( filtit);                                  

        return DiscrHelper< 256>( this).Map( filtit);                                  
    }


template < uint32_t Bits>
    struct DescendHelper
    {
        DistribRepos        *m_DRepos;

        DescendHelper( DistribRepos *dRepos)
            : m_DRepos( dRepos)
        {}

    template < typename CnstrIt>
        void    Map( Discr discr,  CnstrIt *cnstrIt)
        {
            CV_ERROR_ASSERT( discr.m_DId.GetType() == DistribCrate::TypeOf< CharDistrib< Bits>>())
            const CharDistrib< Bits>    *distrib = static_cast< const CharDistrib< Bits> *>( m_DRepos->ToVar( discr.m_DId).GetEntry());
            cnstrIt->Classify( *distrib); 
            return;
        }
    };

template < typename CnstrIt>
    void    Classify( Discr discr, CnstrIt *cnstrIt) 
    { 
        uint32_t    szImg = m_Base.SzImage();
        if ( szImg <= 8)  
            return DescendHelper< 8>( this).Map( discr, cnstrIt);                                  

        if ( szImg <= 16)  
            return DescendHelper< 16>( this).Map( discr, cnstrIt);                                  

        if ( szImg <= 32)                                      
            return DescendHelper< 32>( this).Map( discr, cnstrIt);                                  

        if ( szImg <= 64)  
            return DescendHelper< 64>( this).Map( discr, cnstrIt);                                  

        if ( szImg <= 128)                                     
            return DescendHelper< 128>( this).Map( discr, cnstrIt);                                  

        return DescendHelper< 256>( this).Map( discr, cnstrIt);                                  
    }
    

template < typename Elem>
    Id          Store(  Elem &&elm) 
    {
        m_TVar = Var( &elm, DistribCrate::TypeOf( &elm));
        auto    it = m_IdTbl.find( Id());
        m_TVar = Var();
        if ( it != m_IdTbl.end())
            return *it;
        Id       id = Base::Store( DistribCrate::TypeOf( &elm), elm);
        m_IdTbl.insert( id);
        return id;
    }

    DistribCrate::Var    ToVar( const Id &id)  
    {  
        if ( id.IsValid())
            return Base::ToVar( id); 
        return m_TVar;
    }

    std::vector< Sg_ChSet>  Domain( Id dId)
    {
        DistribCrate::Var       dVar = ToVar( dId);
        std::vector< Sg_ChSet>  domain = dVar( [this]( auto dist) {
            typedef decltype(dist)  Distrib;
            auto                    distDomain = dist->Domain();
            std::vector< Sg_ChSet>  dom( distDomain.size());
            for ( uint32_t  i = 0; i < distDomain.size(); ++i) 
                dom[ i] = m_Base.XForm( distDomain[ i]); 
            return dom; } );
        return domain;
    }
   
    auto    SingleChars( Id dId, uint16_t invInd)
    {
        DistribCrate::Var           dVar = ToVar( dId);
        return   dVar( [this, invInd]( auto dist) { 
                uint32_t                    szSingle = 0;
                auto                        distDomain = dist->Domain(); 
                Cv_Array< uint16_t, 256>    uniList;
                std::fill_n( &uniList[ 0], distDomain.size(), CV_UINT16_MAX);
                uniList.MarkFill( uint32_t( distDomain.size()));
                for ( uint16_t  i = 0; i < distDomain.size(); ++i)
                {
                    if ( invInd != i)
                    {
                        uint8_t         pcnt = distDomain[ i].PopCount();
                        if ( pcnt == 1)
                        {
                            uniList[ i] = distDomain[ i].Index( true);
                            szSingle++;
                        }
                    }
                }
                return std::make_tuple( szSingle, uniList); 
            } ); 
    } 
 
    Sg_ChSet        ChSet( uint8_t byteCode)
    {  
        return m_Base.ChSet( byteCode);                                 
    }

    bool            Dump( std::ostream &ostr) 
    { 
        return OperateAll( [this, &ostr]( auto k, uint32_t ind) {  return k->Dump(  this, ostr); });
    }

    struct Cask : public Cv_MemberCask< Sg_Partition, Cv_CratePile< DistribCrate>>
    {  
        typedef Cv_MemberCask< Sg_Partition, Cv_CratePile< DistribCrate>>     BaseCask; 
        typedef typename BaseCask::ContentType                              BaseContent;

        struct  ContentType : public BaseContent
        {  
            ContentType(  const BaseContent &t2)
                : BaseContent( t2)
            {}

            auto        GetM( void) { return ((BaseCask::BaseContent *) this)->m_Value; }
        };

        static uint32_t         ContentSize( const DistribRepos &obj) { return  sizeof( ContentType); }

        static ContentType      Encase( Cv_Spritz *spritz, const DistribRepos &obj)
        { 
            return BaseCask::Encase( spritz, obj.m_Base, obj);
        }

        static ContentType     *Bloom( uint8_t *arr)
        {
            return ( ContentType *) arr;
        }
    }; 

    struct Blossom
    {
        typedef typename Cask::ContentType                                  ContentType;
        typedef Cv_MemberCask< Sg_Partition, Cv_CratePile< DistribCrate>>   BaseCask;  
        typedef typename Cv_CratePile< DistribCrate>::Blossom               BaseBlossom;

        ContentType     *m_Root;
        
        Blossom( ContentType *arr)
            : m_Root( arr)
        {}

        Sg_Partition   *Base( void) const {   return &m_Root->m_Value; }

        auto     ToVar( const Id &id)  
        {          
            BaseCask::BaseContent     *base = m_Root; 
            BaseBlossom     pileBlsm( &base->m_Value);   
            return pileBlsm.ToVar( id);
        }
        void    ConvertIdToVarId( Id *id)
        {
            BaseCask::BaseContent       *base = m_Root; 
            BaseBlossom                 pileBlsm( &base->m_Value);   
            DistribRepos::Var           var = pileBlsm.ToVar( *id);
            uint64_t                    ptrDist = uint64_t( (( uint8_t *) var.GetEntry()) - (( uint8_t *) m_Root));
            id->SetId( ptrDist);
            DistribRepos::Var           var1 = VarId( *id);
            CV_ERROR_ASSERT( var.GetEntry() == var1.GetEntry())
            CV_ERROR_ASSERT( var.GetType() == var1.GetType())
            return;
        }
        DistribRepos::Var    VarId( const Id &id)
        {
            DistribRepos::Entry   *entry = ( DistribRepos::Entry *) ((( uint8_t *) m_Root) + id.GetId());
            return DistribRepos::Var( entry, id.GetType());
        }
    };
}; 

//_____________________________________________________________________________________________________________________________ 

};

//_____________________________________________________________________________________________________________________________  

