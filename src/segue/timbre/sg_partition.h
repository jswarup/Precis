// sg_partition.h ______________________________________________________________________________________________________________ 
#pragma once

#include    "segue/timbre/sg_bitset.h" 

//_____________________________________________________________________________________________________________________________ 

template < uint32_t SzChBits, uint32_t N>
struct Sg_CharPartitionHelper
{ 
    static uint8_t        GetEqClassCode( const Sg_Bitset< SzChBits>  *chSets, uint32_t k)  { return ( uint8_t( chSets->Get( k)) << ( N -1)) | 
                                            Sg_CharPartitionHelper< SzChBits, N -1>::GetEqClassCode( chSets +1, k);  }
};

template < uint32_t SzChBits>
struct Sg_CharPartitionHelper< SzChBits, 1>
{ 
    static uint8_t        GetEqClassCode( const Sg_Bitset< SzChBits>  *chSets, uint32_t k)  { return uint8_t( chSets->Get( k));  }
};

//_____________________________________________________________________________________________________________________________ 

template < uint32_t SzChBits>
class Sg_CharPartition  
{
protected:
    uint8_t        m_MxEqClass;
    uint8_t        m_EqClassIds[  SzChBits];         

public: 
    typedef Sg_Bitset< SzChBits>    Bitset;

    Sg_CharPartition( void) { MakeUniversal(); }

    uint16_t        Image( uint16_t t) const {  return m_EqClassIds[ t]; }
    void            SetImage( uint32_t k, uint16_t grId) {  m_EqClassIds[ k] = grId; }

    uint32_t        SzImage( void) const { return uint32_t( m_MxEqClass) +1; }
    void            SetSzImage( uint32_t sz) { m_MxEqClass = uint8_t( sz -1); }

    // k is Representative if not instance of same Equivalence class is encountered before.
    bool            IsRep( uint32_t k) const            
    {
        for ( uint8_t i = 0; i < k; i++)  
            if ( m_EqClassIds[ i] == m_EqClassIds[ k])
                return false; 
        return true;
    }

    int32_t         Compare( const Sg_CharPartition &cd) const
    {
        if  ( m_MxEqClass != cd.m_MxEqClass)
            return m_MxEqClass < cd.m_MxEqClass ? -1 : 1;
        return memcmp ( m_EqClassIds, cd.m_EqClassIds, sizeof( m_EqClassIds));
    }
    
    void            MergeClass( uint8_t eq1, uint8_t eq2)
    {
        CV_ERROR_ASSERT(( eq1 != eq2) && ( eq1 < eq2) && ( eq2 <= m_MxEqClass))
        for ( uint32_t i = 0; i < SzChBits; i++) 
        {
            if ( m_EqClassIds[ i] < eq2)
                continue;
            if ( m_EqClassIds[ i] == eq2)
                m_EqClassIds[ i] = eq1;
            else
                --m_EqClassIds[ i];
        }
        --m_MxEqClass;
        return;
    }

    void            MakeUniversal( void)
    {
        m_MxEqClass = 0;
        for ( uint32_t i = 0; i < SzChBits; i++) 
            m_EqClassIds[ i] = 0;
        return;
    }

    void            MakeDiscrete(  void)
    {  
        m_MxEqClass = uint8_t( SzChBits -1);
        for ( uint16_t i = 0; i < SzChBits; i++) 
            m_EqClassIds[ i] = i;
        return;
    }
 
    bool            IsCutByCCL( const Bitset &ccl) const
    {
        std::bitset< SzChBits>          eqClassEnc;              // whether a equivalence class was encountered
        std::bitset< SzChBits>          eqClassInCCL;            // whether a equivalence class is in CCL

        for ( uint16_t i = 0; i < SzChBits; ++i) 
        {
            uint32_t            curEC = m_EqClassIds[ i];
            bool                curCCL = ccl.Get( i);
            if ( eqClassEnc[ curEC])
            {
                if ( curCCL != eqClassInCCL[ curEC])            // EqClass was encountered and We are switching.
                    return true;                                // it was in ccl and we are no longer in it
            }
            else
                eqClassEnc[ curEC] = true;                      // save that the group is encountered

            if ( curCCL)                                        // save that the group is in CCL
                eqClassInCCL[ curEC] = true;             
        }
        return false;
    }
 
    // Extracts the partition subset containing a given EC
    Bitset    EqClassCCL( uint8_t grId) const 
    {
        Bitset      ccl;
        for ( uint16_t i = 0; i < SzChBits; ++i) 
            if ( grId ==  m_EqClassIds[ i])
                ccl.Set( i, true);
        return ccl;
    }

    // Extracts the partition subset containing a given character
    Bitset    ClassContainingChar( uint8_t c) const { return EqClassCCL( Image( c)); }
    

    // Checks if two characters are in the same subset
    bool            CharsInSameSubset( uint16_t c1, uint16_t c2) const { return Image( c1) == Image( c2); }

    // Intersects a new BitSet with all subsets in a partition, making it finer.
    // Returns true if anything changed (equivalent to partitionCutByCCL return value).
    void            ImpressWith( const Sg_CharPartition &q)
    {          
        std::bitset< SzChBits * SzChBits>       m_MatchedFlg;
        uint8_t                                 grid[ SzChBits * SzChBits];
        uint8_t                                 grId = -1;
        for ( uint32_t i = 0; i < SzChBits; ++i) 
        {
            bool                                &matchedFlg = m_MatchedFlg[ m_EqClassIds[ i] * SzChBits + q.m_EqClassIds[ i]]; 
            if ( !matchedFlg)    
            {
                matchedFlg = true; ;                             // register a new group and remember for future ref.
                grid[ m_EqClassIds[ i]][ q.m_EqClassIds[ i]] = ++grId;
            }
            m_EqClassIds[ i] = grId;          
        }
        m_MxEqClass = grId;
        return;
    }

    Sg_CharPartition  Impression( const Sg_CharPartition &q) const
    {
        Sg_CharPartition  cdist( SELF);
        cdist.ImpressWith( q);
        return cdist; 
    }

    // Returns true if every subset in the first partition is contained in some subset in the second
    // (including when the partitions are equal).  If not finer, a sample character C will be
    // placed at sampleChar (if not NULL), such that the p subset containing C is not contained
    // in any subset of q.
    bool            TestFiner( const Sg_CharPartition &q, int *sampleChar) const
    {
        std::bitset< SzChBits>      m_MatchedFlgs;
        uint8_t                     mapToQSubsets[ SzChBits];         
        
        for ( uint32_t i = 0; i < SzChBits; ++i) 
        {
            uint8_t     g1 = m_EqClassIds[ i];
            uint8_t     g2 = q.m_EqClassIds[ i];
            bool        &matchedFlg = m_MatchedFlg[ m_EqClassIds[ i] * SzChBits + q.m_EqClassIds[ i]];
            if ( !m_MatchedFlg[ g1])
            {
                m_MatchedFlg = true;
                mapToQSubsets[ g1] = g2;                // injective map from g1 to g2
            }
            else if ( mapToQSubsets[ g1] != g2)
            {
                if ( sampleChar)
                    *sampleChar = i;
                return false;
            }
        }
        return true;
    }

    std::vector< Bitset >    Domain( void) const
    {
        std::vector< Bitset>    ccls( SzImage());
        for ( uint16_t i = 0; i < SzChBits; ++i) 
            ccls[ m_EqClassIds[ i] ].SetChar( i);

        return ccls;
    }
    
    class CCLImpressCntl
    {
        const Bitset    *m_CCLs;

    template < uint32_t N>           
        uint8_t        EqClassCode( uint32_t k) const { return Sg_CharPartitionHelper<SzChBits, N>::GetEqClassCode( m_CCLs, k); }
 

    public:
        CCLImpressCntl( const Bitset *ccls)
            :   m_CCLs( ccls)
        {}
      
    template < uint32_t N> 
        void            ImpressWith( Sg_CharPartition *distrib)
        {
            std::bitset< SzChBits << N> matchFlgs;                    
            uint8_t                     grMap[ SzChBits << N];                   // keep a map if the group has been encountered.
            
            uint8_t     mxId = -1;
            for ( uint16_t i = 0; i < SzChBits; ++i) 
            {
                uint8_t     *pEqClassId = &distrib->m_EqClassIds[ i]; 
                uint64_t    eqClassCode = ( uint64_t( *pEqClassId) << N) | EqClassCode< N>( i);     
                auto        &matchFlg = matchFlgs[ eqClassCode]; 
                uint8_t     *pGrId = &grMap[ eqClassCode];
                if ( !matchFlg) 
                {
                    matchFlg = true;
                    *pGrId = ++mxId;                          // assign a new id.
                }
                *pEqClassId = *pGrId;     
            }
            distrib->m_MxEqClass = mxId;
            return;
        }
    }; 

    class CCLImpressor 
    {
    protected:
        Sg_CharPartition        *m_Distrib;
        Bitset                  m_CCLs[ 7];
        uint32_t                m_Ind;
        Bitset                  m_ValidCCL; 

    public:
        CCLImpressor( Sg_CharPartition *distrib)
            :   m_Distrib( distrib), m_Ind( 0)
        {}

        const Bitset    &ValidCCL( void) const { return m_ValidCCL; }

        bool            Process( const Bitset &ccl)
        {
            m_ValidCCL.UnionWith( ccl );
            m_CCLs[ m_Ind++] = ccl;
            if ( m_Ind < 7)
                return false;
            CCLImpressCntl( &m_CCLs[ 0]).template ImpressWith<7>( m_Distrib);
            m_Ind = 0;
            return true;
        }
        void            Over( void)
        {
            switch ( m_Ind)
            { 
                case 1: CCLImpressCntl( &m_CCLs[ 0]).template ImpressWith< 1>( m_Distrib); break;
                case 2: CCLImpressCntl( &m_CCLs[ 0]).template ImpressWith< 2>( m_Distrib); break;
                case 3: CCLImpressCntl( &m_CCLs[ 0]).template ImpressWith< 3>( m_Distrib); break;
                case 4: CCLImpressCntl( &m_CCLs[ 0]).template ImpressWith< 4>( m_Distrib); break;
                case 5: CCLImpressCntl( &m_CCLs[ 0]).template ImpressWith< 5>( m_Distrib); break;
                case 6: CCLImpressCntl( &m_CCLs[ 0]).template ImpressWith< 6>( m_Distrib); break;
            }
            m_Ind = 0;
            return;
        }
    };

    // Intersects a new Bitset with all subsets in a partition, making it finer.
    // Returns 1 if anything changed (equivalent to partitionCutByCCL return value).

    void                ImpressCCL(  const Bitset & ccl)  {  CCLImpressCntl( &ccl).ImpressWith< 1>( this); }
};
 
