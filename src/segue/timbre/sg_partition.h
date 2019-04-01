// sg_partition.h ______________________________________________________________________________________________________________ 
#pragma once

#include    "segue/timbre/sg_bitset.h" 

//_____________________________________________________________________________________________________________________________ 

template < uint32_t SzChBits>
class Sg_CharPartition  
{
protected:
    uint8_t        m_MxEqClass;
    uint8_t        m_EqClassIds[  SzChBits];         

public:
    typedef  std::bitset< SzChBits>     BitSet;

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
 
    bool            IsCutByCCL( const BitSet &ccl) const
    {
        BitSet      eqClassEnc;              // whether a equivalence class was encountered
        BitSet      eqClassInCCL;            // whether a equivalence class is in CCL

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
    Sg_Bitset< SzChBits>    EqClassCCL( uint8_t grId) const 
    {
        Sg_Bitset< SzChBits>      ccl;
        for ( uint16_t i = 0; i < SzChBits; ++i) 
            if ( grId ==  m_EqClassIds[ i])
                ccl.Set( i, true);
        return ccl;
    }

    // Extracts the partition subset containing a given character
    Sg_Bitset< SzChBits>    ClassContainingChar( uint8_t c) const { return EqClassCCL( Image( c)); }
    

    // Checks if two characters are in the same subset
    bool            CharsInSameSubset( uint16_t c1, uint16_t c2) const { return Image( c1) == Image( c2); }

    // Intersects a new BitSet with all subsets in a partition, making it finer.
    // Returns true if anything changed (equivalent to partitionCutByCCL return value).
    void            ImpressWith( const Sg_CharPartition &q)
    {          
        std::bitset< SzChBits * SzChBits>       m_MatchedFlg;
        uint8_t                                 grid[ SzChBits * SzChBits];
        uint8_t                                 numGr = 0;
        for ( uint32_t i = 0; i < SzChBits; ++i) 
        {
            uint8_t             &locValue = grid[ m_EqClassIds[ i]][ q.m_EqClassIds[ i]];
            bool                &matchedFlg = m_MatchedFlg[ m_EqClassIds[ i] * SzChBits + q.m_EqClassIds[ i]]; 
            if ( matchedFlg)    // we found a match 
                m_EqClassIds[ i] = locValue;      // use the group code for it 
            else
            {
                matchedFlg = true; ;   // register a new group and remember for future ref.
                m_EqClassIds[ i] = locValue = numGr++;          
            }
        }
        m_MxEqClass = numGr -1;
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
        BitSet      m_MatchedFlg;
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

    std::vector< BitSet>    Domain( void) const
    {
        std::vector< BitSet>    ccls( SzImage());
        for ( uint16_t i = 0; i < SzChBits; ++i) 
            ccls[ m_EqClassIds[ i] ].SetChar( i);

        return ccls;
    }

    class CCLImpressCntl
    {
        const BitSet    *m_CCLs;

    template < uint32_t N>          //  N < 7
        uint8_t         EqClassCode( uint32_t k) const;


    public:
        CCLImpressCntl( const BitSet *ccls)
            :   m_CCLs( ccls)
        {}

        template < uint32_t N> 
        void            ImpressWith( Sg_CharPartition *distrib)
        {
            std::bitset< SzChBits << N>     matchFlgs;
            uint8_t                         grMap[ SzChBits << N];                   // keep a map if the group has been encountered. 

            uint16_t    newGr = 0;
            for ( uint32_t i = 0; i < SzChBits; ++i) 
            {
                uint8_t     *pEqClassId = &distrib->m_EqClassIds[ i];
                // potentially any group group can split.  one part in ccl and other outside. get the code for new group.
                uint8_t     eqClassCode = ( *pEqClassId << N) | EqClassCode< N>( i);     
                uint8_t     *pMap = &grMap[ eqClassCode];                   // get location where we keep ids about these groups.
                bool        &matchFlg = matchFlgs[ eqClassCode];
                if ( !matchFlg) 
                {
                    matchFlg = true;
                    *pMap = newGr++;                                        // assign a new id.
                }
                *pEqClassId = *pMap;                                        // reuse id if we have encountered them before.
            }
            distrib->m_MxEqClass = uint8_t( newGr -1);
            return;
        }
    };

    //_____________________________________________________________________________________________________________________________ 

    class CCLImpressor 
    {
    protected:
        Sg_CharPartition        *m_Distrib;
        BitSet                  m_CCLs[ 7];
        uint32_t                m_Ind;
        BitSet                  m_ValidCCL; 

    public:
        CCLImpressor( Sg_CharPartition *distrib)
            :   m_Distrib( distrib), m_Ind( 0)
        {}

        const BitSet    &ValidCCL( void) const { return m_ValidCCL; }

        bool            Process( const BitSet &ccl)
        {
            m_ValidCCL.UnionWith( ccl );
            m_CCLs[ m_Ind++] = ccl;
            if ( m_Ind < 7)
                return false;
            Sg_CharDistrib::CCLImpressCntl( &m_CCLs[ 0]).ImpressWith< 7>( m_Distrib);
            m_Ind = 0;
            return true;
        }
    
        void            Over( void)
        {
            switch ( m_Ind)
            { 
                case 1: Sg_CharDistrib::CCLImpressCntl( &m_CCLs[ 0]).ImpressWith< 1>( m_Distrib); break;
                case 2: Sg_CharDistrib::CCLImpressCntl( &m_CCLs[ 0]).ImpressWith< 2>( m_Distrib); break;
                case 3: Sg_CharDistrib::CCLImpressCntl( &m_CCLs[ 0]).ImpressWith< 3>( m_Distrib); break;
                case 4: Sg_CharDistrib::CCLImpressCntl( &m_CCLs[ 0]).ImpressWith< 4>( m_Distrib); break;
                case 5: Sg_CharDistrib::CCLImpressCntl( &m_CCLs[ 0]).ImpressWith< 5>( m_Distrib); break;
                case 6: Sg_CharDistrib::CCLImpressCntl( &m_CCLs[ 0]).ImpressWith< 6>( m_Distrib); break;
            }
            m_Ind = 0;
        }
    };

    // Intersects a new BitSet with all subsets in a partition, making it finer.
    // Returns 1 if anything changed (equivalent to partitionCutByCCL return value).

    void            ImpressCCL(  const BitSet & ccl)  {  CCLImpressCntl( &ccl).ImpressWith< 1>( this); }
    
    //_____________________________________________________________________________________________________________________________ 

    class CCLIdImpressor : public CCLImpressor 
    {
        std::set< uint32_t>         m_Processed;

    public:
        CCLIdImpressor( Sg_CharPartition *distrib)
            : CCLImpressor( distrib)
        {}

        bool            Process( const BitSet &ccl, uint32_t id)
        { 
            if ( id == CV_UINT32_MAX)
                return CCLImpressor::Process( ccl);

            std::set< uint32_t>::iterator   it = m_Processed.find( id);
            if ( it != m_Processed.end())
                return true;

            m_Processed.insert( it, id);
            return CCLImpressor::Process( ccl);
        }
    };
};
 