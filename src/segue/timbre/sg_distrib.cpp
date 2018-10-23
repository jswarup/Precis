// sg_distrib.cpp ______________________________________________________________________________________________________________ 
 

#include    "segue/tenor/sg_include.h"
#include    "segue/timbre/sg_distrib.h" 

//_____________________________________________________________________________________________________________________________ 
// sort by decreasing weight (but pure EOS/EOP subsets last)

int32_t     Fs_SubsetDesc::Compare( const Fs_SubsetDesc &sd) const 
{ 
    if ( first < 256 && sd.first >= 256)
        return -1;
    if ( first >= 256 && sd.first < 256)
        return 1 ;
    return ( weight != sd.weight) ? ( weight > sd.weight ? -1 : 1) 
                                                         : (( first != sd.first) ? ( first < sd.first ? -1 : 1) 
                                                                                 : 0);  
}
                                                                                                                        
//_____________________________________________________________________________________________________________________________ 

Fs_Distrib::~Fs_Distrib( void)
{}

//_____________________________________________________________________________________________________________________________ 

std::string     Fs_Distrib::ToString( void) const
{
    std::stringstream       strStrm;
    strStrm << "[ ";
    std::vector< Sg_ChSet>    dom = Domain();
    for ( uint32_t k = 0; k < dom.size(); ++k)
        strStrm <<  dom[ k].ToString() << " ";
    strStrm << "]";
    return strStrm.str();
}

//_____________________________________________________________________________________________________________________________ 

Fs_CharDistrib::Fs_CharDistrib( void)
    :   m_SzEqClass( 0)
{
    memset( m_EqClassIds, 0, sizeof( m_EqClassIds));
}

//_____________________________________________________________________________________________________________________________ 

/*
Fs_CharDistrib::Fs_CharDistrib( const Fs_CharDistrib &prtn)
    :   m_SzEqClass( 0)
{
    bool     hit[ Sg_ChSet::SzChBits];
    memset( hit, 0, sizeof( hit));

    for ( uint16_t start = 0; start < Sg_ChSet::SzChBits; start++) 
    {
        if ( hit[ start]) 
            continue;
        
        hit[ start] = true;
        SetEqClassId( start, m_SzEqClass++);
        for ( uint16_t i = prtn.m_Next[ start]; !hit[i]; i = prtn.m_Next[i]) 
        {
            SetEqClassId( i, m_SzEqClass -1);
            hit[ i] = true;
        }
    }
}
*/
//_____________________________________________________________________________________________________________________________ 

std::vector< Fs_SubsetDesc>  Fs_CharDistrib::SubsetDescs( void) const
{
    std::vector< Fs_SubsetDesc>     subsets( m_SzEqClass, Fs_SubsetDesc( CV_UINT16_MAX));
    for ( uint16_t i = 0; i < Sg_ChSet::SzChBits; i++) 
    {
        Fs_SubsetDesc    *pLast = &subsets[ m_EqClassIds[ i] ];
        if (  pLast->first == CV_UINT16_MAX)
            pLast->first = i;
        ++pLast->weight;
    }
    return subsets;
}

//_____________________________________________________________________________________________________________________________ 

void    Fs_CharDistrib::MergeClass( uint16_t eq1, uint16_t eq2)
{
    CV_ERROR_ASSERT(( eq1 != eq2) && ( eq1 < eq2) && ( eq2 < m_SzEqClass))
    for ( uint16_t i = 0; i < Sg_ChSet::SzChBits; i++) 
    {
        if ( m_EqClassIds[ i] < eq2)
            continue;
        if ( m_EqClassIds[ i] == eq2)
            m_EqClassIds[ i] = eq1;
        else
            --m_EqClassIds[ i];
    }
    --m_SzEqClass;
    return;
}

//_____________________________________________________________________________________________________________________________ 
// k is Representative if not instance of same Equivalence class is encountered before.

bool    Fs_CharDistrib::IsRep( uint32_t k) const
{
    for ( uint16_t i = 0; i < k; i++) 
    {
        if ( m_EqClassIds[ i] == m_EqClassIds[ k])
            return false;
    }
    return true;
}

//_____________________________________________________________________________________________________________________________ 
// Makes record into trivial partition (single subset)

void Fs_CharDistrib::MakeUniversal( void) 
{
    //AX_TRACEPOST(())
    
    m_SzEqClass = 1;
    for ( uint16_t i = 0; i < Sg_ChSet::SzChBits; i++) 
        m_EqClassIds[ i] = 0;
    return;
}

//_____________________________________________________________________________________________________________________________ 
// Makes record into discrete partition (maximum subsets)

void Fs_CharDistrib::MakeDiscrete( void) 
{  
    m_SzEqClass = Sg_ChSet::SzChBits;
    for ( uint16_t i = 0; i < Sg_ChSet::SzChBits; i++) 
        m_EqClassIds[ i] = i;
    return;
}

//_____________________________________________________________________________________________________________________________ 

int32_t     Fs_CharDistrib::Compare( const Fs_CharDistrib &cd) const
{
    if  ( m_SzEqClass != cd.m_SzEqClass)
        return m_SzEqClass < cd.m_SzEqClass ? -1 : 1;
    return memcmp ( m_EqClassIds, cd.m_EqClassIds, sizeof( m_EqClassIds));
}

//_____________________________________________________________________________________________________________________________ 

uint16_t    Fs_CharDistrib::NumRealSubsets( void)  const
{
    return m_SzEqClass  ;
}

//_____________________________________________________________________________________________________________________________ 
// Tests if any partition subset intersects nontrivially with the given Sg_ChSet

bool    Fs_CharDistrib::IsCutByCCL( const Sg_ChSet &ccl) const
{
    std::bitset< Sg_ChSet::SzChBits>      eqClassEnc;              // whether a equivalence class was encountered
    std::bitset< Sg_ChSet::SzChBits>      eqClassInCCL;            // whether a equivalence class is in CCL
    
    for ( uint16_t i = 0; i < Sg_ChSet::SzChBits; ++i) 
    {
        uint32_t                                curEC = m_EqClassIds[ i];
        bool                                    curCCL = ccl.Get( i);
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

//_____________________________________________________________________________________________________________________________ 
// Tests if any partition subset intersects nontrivially with the given Sg_ChSet

bool    Fs_CharDistrib::IsCutByCCL( const Sg_ChSet &ccl, uint32_t first, uint32_t wt) const
{
    uint8_t             eqClassEnc[ 264];              // whether a equivalence class was encountered
    uint8_t             eqClassInCCL[ 264];            // whether a equivalence class is in CCL

    Cv_Aid::MemSetZero( ( uint64_t *) &eqClassEnc[ 0], ( uint64_t *) &eqClassEnc[ 264]);
    Cv_Aid::MemSetZero( ( uint64_t *) &eqClassInCCL[ 0], ( uint64_t *) &eqClassInCCL[ 264]);

    uint16_t                    i = 0;
    for ( ; i < first; ++i) 
        eqClassEnc[ m_EqClassIds[ i] ] = true;                          // save that the group is encountered
        
    for ( ; wt && ( i < Sg_ChSet::SzChBits); ++i) 
    {
        uint32_t                    curEC = m_EqClassIds[ i];
        bool                        curCCL = ccl.Get( i);
        if ( eqClassEnc[ curEC])
        {
            if ( curCCL != !!eqClassInCCL[ curEC])                // EqClass was encountered and We are switching.
                return true;                                    // it was in ccl and we are no longer in it
        }
        else
            eqClassEnc[ curEC] = true;                          // save that the group is encountered
        
        if ( curCCL)                                            // save that the group is in CCL
        {
            eqClassInCCL[ curEC] = true;   
            --wt;        
        }        
    }
    for ( ; i < Sg_ChSet::SzChBits; ++i) 
    {
        uint32_t                    curEC = m_EqClassIds[ i];
        if ( eqClassEnc[ curEC] && eqClassInCCL[ curEC])        // EqClass was encountered and We are switching.
            return true;                                        // it was in ccl and we are no longer in it
    }
    return false;
}

//_____________________________________________________________________________________________________________________________ 

Sg_ChSet  Fs_CharDistrib::EqClassCCL( uint16_t grId) const
{
   Sg_ChSet      ccl;
    for ( uint16_t i = 0; i < Sg_ChSet::SzChBits; ++i) 
        if ( grId ==  m_EqClassIds[ i])
            ccl.Set( i, true);
    return ccl;
}
    
//_____________________________________________________________________________________________________________________________ 
// Constructs the intersection of two partitions.  The destination may be NULL,
// in which case it will be allocated.  It may also optionally match one of the sources.

void Fs_CharDistrib::ImpressWith( const Fs_CharDistrib &q) 
{
    //AX_TRACEPOST(())
    
    struct Grid
    {
        uint16_t                m_Elem[ Sg_ChSet::SzChBits][ Sg_ChSet::SzChBits];
        Grid( void) { memset( m_Elem, -1, sizeof( m_Elem));}
        uint16_t  *operator[]( uint32_t i) { return m_Elem[ i]; }
    };
    
    static  Grid            storedGrid;
    Grid                    newGrId( storedGrid);
    uint16_t                numGr = 0;
    for ( uint16_t i = 0; i < Sg_ChSet::SzChBits; ++i) 
    {
        uint16_t                    gc = newGrId[ m_EqClassIds[ i]][ q.m_EqClassIds[ i]];
        if ( gc != CV_UINT16_MAX)       // we found a match
        {
            m_EqClassIds[ i] = gc;      // use the group code for it
        }
        else
        {
            gc = newGrId[ m_EqClassIds[ i]][ q.m_EqClassIds[ i]] = numGr++;   // register a new group and remember for future ref.
            m_EqClassIds[ i] = gc;          
        }
    }
    m_SzEqClass = numGr;
    return;
}

//_____________________________________________________________________________________________________________________________ 

Fs_CharDistrib  Fs_CharDistrib::Impression( const Fs_CharDistrib &q) const
{
    Fs_CharDistrib  cdist( SELF);
    cdist.ImpressWith( q);
    return cdist;

}
    
//_____________________________________________________________________________________________________________________________ 

bool     Fs_CharDistrib::TestFiner( const Fs_CharDistrib &q, int *sampleChar) const
{
    uint16_t            mapToQSubsets[ Sg_ChSet::SzChBits];              // whether a group was encountered
    memset( mapToQSubsets, -1, sizeof( mapToQSubsets));

    for ( uint16_t i = 0; i < Sg_ChSet::SzChBits; ++i) 
    {
        uint16_t    g1 = m_EqClassIds[ i];
        uint16_t    g2 = q.m_EqClassIds[ i];
        if ( mapToQSubsets[ g1] == CV_UINT16_MAX)
            mapToQSubsets[ g1] = g2;                // injective map from g1 to g2
        else if ( mapToQSubsets[ g1] != g2)
        {
            if ( sampleChar)
                *sampleChar = i;
            return false;
        }
    }
    return true;
}

//_____________________________________________________________________________________________________________________________ 

std::vector< Sg_ChSet>    Fs_CharDistrib::Domain( void) const
{
    std::vector< Sg_ChSet>    ccls( m_SzEqClass);
    for ( uint16_t i = 0; i < Sg_ChSet::SzChBits; ++i) 
        ccls[ m_EqClassIds[ i] ].SetChar( i);
    
    return ccls;
}

//_____________________________________________________________________________________________________________________________ 

Fs_CharDistrib::CCLImpressor::CCLImpressor( Fs_CharDistrib *distrib)
    :   m_Distrib( distrib), m_Ind( 0)
{}

//_____________________________________________________________________________________________________________________________ 

bool    Fs_CharDistrib::CCLImpressor::Process( const Sg_ChSet &ccl)
{
    m_ValidCCL.UnionWith( ccl );
    m_CCLs[ m_Ind++] = ccl;
    if ( m_Ind < 7)
        return false;
    Fs_CharDistrib::CCLImpressCntl( &m_CCLs[ 0]).ImpressWith< 7>( m_Distrib);
    m_Ind = 0;
    return true;
}
    
//_____________________________________________________________________________________________________________________________ 

void    Fs_CharDistrib::CCLImpressor::Over( void)
{
    switch ( m_Ind)
    { 
        case 1: Fs_CharDistrib::CCLImpressCntl( &m_CCLs[ 0]).ImpressWith< 1>( m_Distrib); break;
        case 2: Fs_CharDistrib::CCLImpressCntl( &m_CCLs[ 0]).ImpressWith< 2>( m_Distrib); break;
        case 3: Fs_CharDistrib::CCLImpressCntl( &m_CCLs[ 0]).ImpressWith< 3>( m_Distrib); break;
        case 4: Fs_CharDistrib::CCLImpressCntl( &m_CCLs[ 0]).ImpressWith< 4>( m_Distrib); break;
        case 5: Fs_CharDistrib::CCLImpressCntl( &m_CCLs[ 0]).ImpressWith< 5>( m_Distrib); break;
        case 6: Fs_CharDistrib::CCLImpressCntl( &m_CCLs[ 0]).ImpressWith< 6>( m_Distrib); break;
    }
    m_Ind = 0;
    return;
}

//_____________________________________________________________________________________________________________________________ 

bool  Fs_CharDistrib::CCLIdImpressor::Process( const Sg_ChSet &ccl, uint32_t id)
{ 
    if ( id == CV_UINT32_MAX)
        return CCLImpressor::Process( ccl);
            
    std::set< uint32_t>::iterator   it = m_Processed.find( id);
    if ( it != m_Processed.end())
        return true;
    
    m_Processed.insert( it, id);
    return CCLImpressor::Process( ccl);
}

//_____________________________________________________________________________________________________________________________ 

Fs_ECDistrib::Fs_ECDistrib( const Fs_BaseDistrib *distrib)
    :   m_RefCount( 0), m_BaseDistrib( distrib), m_Hashval( 0), m_LockFlg( false), m_ZeroFailFlg( false)
{
    m_Map = new uint16_t[ m_BaseDistrib->SzImage()];
    memset( m_Map, 0, sizeof( uint16_t) * m_BaseDistrib->SzImage());
}

//_____________________________________________________________________________________________________________________________ 

Fs_ECDistrib::Fs_ECDistrib( const Fs_ECDistrib &ecd)
    :   m_RefCount( ecd.m_RefCount), m_BaseDistrib( ecd.m_BaseDistrib), m_Hashval( ecd.m_Hashval), 
        m_LockFlg( ecd.m_LockFlg), m_ZeroFailFlg( ecd.m_ZeroFailFlg)
{
    m_Map = new uint16_t[ m_BaseDistrib->SzImage()];
    memcpy( m_Map, ecd.m_Map, sizeof( uint16_t) * m_BaseDistrib->SzImage());
}

//_____________________________________________________________________________________________________________________________ 

Fs_ECDistrib::~Fs_ECDistrib( void)
{
    delete [] m_Map;
}

//_____________________________________________________________________________________________________________________________ 

uint32_t    Fs_ECDistrib::SzImage( void) const
{
    uint32_t    mx = 0;
    for ( uint16_t i = 0; i < Size(); ++i) 
       if ( m_Map[ i] > mx)
           mx = m_Map[ i];
    return mx +1;
}


//_____________________________________________________________________________________________________________________________ 

uint32_t   Fs_ECDistrib::Thickness( void) const
{
    uint32_t        th = 0;
    for ( uint16_t i = 0; i < Size(); ++i) 
        if ( !m_ZeroFailFlg || m_Map[ i])
            ++th;
    return th;
}

//_____________________________________________________________________________________________________________________________ 

bool    Fs_ECDistrib::IsEqual( const Fs_ECDistrib &ecd) const
{
    if ( m_BaseDistrib != ecd.m_BaseDistrib)
        return false;
    if ( m_LockFlg != ecd.m_LockFlg)
        return false;
    if ( m_ZeroFailFlg != ecd.m_ZeroFailFlg)
        return false;
    if ( memcmp( m_Map, ecd.m_Map, sizeof( uint16_t) * m_BaseDistrib->SzImage()))
        return false;
    return true;
}

//_____________________________________________________________________________________________________________________________ 

bool Fs_ECDistrib::operator<( const Fs_ECDistrib &s2) const
{
    if ( m_BaseDistrib != s2.m_BaseDistrib)
        return m_BaseDistrib < s2.m_BaseDistrib;
    if ( m_LockFlg != s2.m_LockFlg)
        return m_LockFlg < s2.m_LockFlg;
    if ( m_ZeroFailFlg != s2.m_ZeroFailFlg)
        return m_ZeroFailFlg < s2.m_ZeroFailFlg;
    
    int     res = memcmp( m_Map, s2.m_Map, m_BaseDistrib->SzImage() * 2);
    return ( res < 0);
}

//_____________________________________________________________________________________________________________________________ 

std::vector< Sg_ChSet>    Fs_ECDistrib::Domain( void) const
{
    std::vector< Sg_ChSet>    dom = m_BaseDistrib->Domain();
    std::vector< Sg_ChSet>    ccls; 
    for ( uint16_t i = 0; i < Size(); ++i) 
    {
        uint16_t    grId = At( i);
        if ( grId >= ccls.size())
            ccls.resize( grId +1);
        ccls[ grId].UnionWith( dom[ i]);
    }
    return ccls;
}

//_____________________________________________________________________________________________________________________________ 

Fs_CharDistrib  Fs_ECDistrib::CharDistrib( void) const
{
    Fs_CharDistrib          charDistrib( *m_BaseDistrib);
    for ( uint16_t i = 0; i < Sg_ChSet::SzChBits; ++i) 
        charDistrib.SetImage( i, m_Map[ charDistrib.Image( i)]);
    charDistrib.SetSzImage( SzImage());
    return charDistrib;
}

//_____________________________________________________________________________________________________________________________ 

void    Fs_ECDistrib::SwitchBase( Fs_BaseDistrib *base)
{
    //Fs_CharDistrib      charDistrib = CharDistrib();
    uint16_t            *tMap = new uint16_t[ base->SzImage()];
    for ( uint16_t i = 0; i < Sg_ChSet::SzChBits; ++i) 
        tMap[ base->Image( i)] = Image( i);
    m_BaseDistrib = base;
    delete [] m_Map;
    m_Map = tMap;

    //Fs_CharDistrib      tCharDistrib = CharDistrib();
    //CV_ERROR_ASSERT( charDistrib.IsEqual( tCharDistrib))
    return;
}

//_____________________________________________________________________________________________________________________________ 

void    Fs_BaseDistrib::Freeze( void)
{
    m_Domain = Fs_CharDistrib::Domain();
    return;
}

//_____________________________________________________________________________________________________________________________ 

void    Fs_BaseDistrib::MergeClass( uint16_t eq1, uint16_t eq2)
{
    CV_ERROR_ASSERT(( eq1 != eq2) && ( eq1 < eq2) && ( eq2 < m_SzEqClass))
    Fs_CharDistrib::MergeClass( eq1, eq2);
    m_Domain[ eq1].UnionWith( m_Domain[ eq2]);
    m_Domain.erase( m_Domain.begin() +eq2);
    return;
}

//_____________________________________________________________________________________________________________________________ 

std::vector< Sg_ChSet>    Fs_BaseDistrib::Domain( void) const
{
    return m_Domain;
}

//_____________________________________________________________________________________________________________________________ 

