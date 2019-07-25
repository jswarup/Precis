//  sg_distrib.h ___________________________________________________________________________________________________________________
#pragma once


#include    "segue/colophon/sg_filter.h" 

//_____________________________________________________________________________________________________________________________  

namespace Sg_RExp
{
    
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
    uint32_t                SzImage( void) const {  return 0; }

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
    uint32_t            SzImage( void) const {  return Sg_CharPartition< Bits>::SzImage(); }

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

private:
    std::set< Id, LessOp>       m_IdTbl; 
    DistribCrate::Var           m_TVar;
    Sg_Partition                m_Base; 
    uint16_t                    m_SzImg;

public:
    DistribRepos( void) 
        : m_IdTbl( LessOp( this)), m_SzImg( 0)
    {}


    const Sg_Partition          &BaseDistrib( void) const { return m_Base; }

    void                        SetBaseDistrib( const Sg_Partition &base) 
    { 
        m_Base = base; 
        m_SzImg = m_Base.SzImage();
    } 
    struct DfaDistrib
    {
        Var         m_DVar;
        uint16_t    m_Inv;
        uint8_t     m_MxEqClass; 

        DfaDistrib( void)
            : m_Inv( 0), m_MxEqClass( uint8_t( -1))
        { }

        DfaDistrib( Var var, uint16_t inv, uint8_t mxEqClass)
            :   m_DVar( var), m_Inv( inv), m_MxEqClass( mxEqClass)
        { }

        uint32_t    SzDescend( void) const { return m_MxEqClass +1; }

        void        Dump( std::ostream &strm, DistribRepos *dRepos)
        {
            std::vector< Sg_ChSet>  domain = dRepos->Domain(  m_DVar);
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
        DfaDistrib    ConstructDfaDistrib( DescendIt *filtIt)
        {
            typedef typename CharDistrib< Bits>::CCLImpressor   CCLImpressor;

            CharDistrib< Bits>                  *distrib = new CharDistrib< Bits>();
            CCLImpressor        intersector( distrib);

            while ( filtIt->IsCurValid())
            { 
                Sg_Bitset< Bits>      *bitset = static_cast< ChSetFilter< Bits> *>( filtIt->CurrFilt().GetEntry());
                intersector.Process( *bitset); 
                filtIt->Next();
            }    
            intersector.Over();
            auto            invalidCCL =  intersector.ValidCCL().Negative();
            uint32_t        invRep = invalidCCL.RepIndex();
            uint16_t        invInd = ( invRep != CV_UINT32_MAX) ? distrib->Image( invRep) : CV_UINT16_MAX; 
            DfaDistrib      dfaDistrib( Var( distrib, DistribCrate::TypeOf( distrib)), invInd, uint8_t( distrib->SzImage() -1)); 
            return dfaDistrib;
        }

    };  

    template < typename DescendIt>
    DfaDistrib     ConstructDfaDistrib( DescendIt *filtit)
    {  
        if ( m_SzImg <= 8)  
            return DiscrHelper< 8>( this).ConstructDfaDistrib( filtit);                                  

        if ( m_SzImg <= 16)  
            return DiscrHelper< 16>( this).ConstructDfaDistrib( filtit);                                  

        if ( m_SzImg <= 32)                                      
            return DiscrHelper< 32>( this).ConstructDfaDistrib( filtit);                                  

        if ( m_SzImg <= 64)  
            return DiscrHelper< 64>( this).ConstructDfaDistrib( filtit);                                  

        if ( m_SzImg <= 128)                                     
            return DiscrHelper< 128>( this).ConstructDfaDistrib( filtit);                                  

        return DiscrHelper< 256>( this).ConstructDfaDistrib( filtit);                                  
    }

    Id  StoreDistrib( const Var &dVar)
    {
        return dVar( [ this]( auto distrib) {  return  Store( *distrib); });
    }


    template < typename CnstrIt> 
    void    Classify( uint32_t level, const DfaDistrib  &dDist, CnstrIt *cnstrIt) 
    { 
        uint16_t    inv = dDist.m_Inv;
        dDist.m_DVar( [ cnstrIt, level, inv]( auto distrib) {
            cnstrIt->Classify( level, *distrib, inv); 
            } );
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

    std::vector< Sg_ChSet>  Domain( Var dVar)
    { 
        std::vector< Sg_ChSet>  domain = dVar( [this]( auto dist) {
            typedef decltype(dist)  Distrib;
            auto                    distDomain = dist->Domain();
            std::vector< Sg_ChSet>  dom( distDomain.size());
            for ( uint32_t  i = 0; i < distDomain.size(); ++i) 
                dom[ i] = m_Base.XForm( distDomain[ i]); 
            return dom; } );
        return domain;
    }

    std::vector< Sg_ChSet>  Domain( Id dId) { return Domain( ToVar( dId)); }

    auto    SingleChars( Var dVar, uint16_t invInd)
    { 
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


struct DistribOptimize
{
    typedef DistribCrate::Var           DVar;
    DistribRepos                        *m_DRepos;
    std::vector< DVar>                  m_Distribs[ DistribCrate::Sz +1];
  
    DistribOptimize( DistribRepos *dRepos)
        : m_DRepos( dRepos)
    {
        m_DRepos->Iterate( [ this]( auto distrib, uint32_t i) {
            uint32_t    szImg = distrib->SzImage();
            if ( szImg <= 8)  
                m_Distribs[ 1].push_back( DVar( distrib, DistribCrate::TypeOf( distrib)));    
            else if ( szImg <= 16)  
                m_Distribs[ 2].push_back( DVar( distrib, DistribCrate::TypeOf( distrib)));    
            else if ( szImg <= 32)  
                m_Distribs[ 3].push_back( DVar( distrib, DistribCrate::TypeOf( distrib)));    
            else if ( szImg <= 64)  
                m_Distribs[ 4].push_back( DVar( distrib, DistribCrate::TypeOf( distrib)));    
            else if ( szImg <= 128)  
                m_Distribs[ 5].push_back( DVar( distrib, DistribCrate::TypeOf( distrib)));    
            else
                m_Distribs[ 6].push_back( DVar( distrib, DistribCrate::TypeOf( distrib)));                    
        });
    }  
    
    bool            Dump( std::ostream &ostr) 
    { 
        for ( uint32_t i = 0; i <= DistribCrate::Sz; ++i)
        {
            ostr << i << ": " << m_Distribs[ i].size() << '\n';
        }
        return true;
    }
};

//_____________________________________________________________________________________________________________________________  
};

//_____________________________________________________________________________________________________________________________  


//_____________________________________________________________________________________________________________________________
