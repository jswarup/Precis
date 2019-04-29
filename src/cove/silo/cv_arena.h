// cv_arena.h ______________________________________________________________________________________________________________
#pragma once 

//-------------------------------------------------------------------------------------------------
 
#include	"cove/barn/cv_frager.h" 

//-------------------------------------------------------------------------------------------------

template < class Arg>
class Cv_CallBacklog : public std::vector< std::function< bool ( Arg arg)>>
{
    bool        m_ProcessingFlg;

public:
    Cv_CallBacklog( void)
        : m_ProcessingFlg( false)
    {}

    void    Process( Arg arg)
    {
        if ( m_ProcessingFlg)
            return;
        m_ProcessingFlg = true;
        std::vector< std::function< bool ( Arg arg)>>   duds;
        while ( size())
        {
            auto    curr = back();
            pop_back();
            bool    res = curr( arg);
            if ( !res)
               duds.push_back( curr); 
        }
        swap( duds);
        m_ProcessingFlg = false;
        return;
    }
};

//-------------------------------------------------------------------------------------------------

template < class Arena, class LeafBooth>
class Cv_Pin
{
protected:
     typedef typename LeafBooth::ValueType   LeafType;

    Arena                   *m_Arena;
    LeafBooth               *m_MemChunk;
    uint32_t                m_Index;

public:
    typedef LeafType                    value_type;
    typedef int                         difference_type;
    typedef LeafType                    *pointer;
    typedef LeafType                    &reference;
    typedef std::input_iterator_tag     iterator_category;

    Cv_Pin( void)
        : m_Arena( nullptr), m_Index( 0), m_MemChunk( nullptr)
    {}
    
    Cv_Pin( Arena *arena, uint32_t index)
        : Cv_Pin( m_Arena->Pin( m_Index))
    {}
    
    Cv_Pin( Arena *arena, uint32_t index, LeafBooth *memChunk)
        : m_Arena( arena), m_Index( index), m_MemChunk( memChunk)
    {
        m_MemChunk->RaiseRef();
    }
    
    Cv_Pin( const Cv_Pin &a)
        : m_Arena( a.m_Arena), m_Index( a.m_Index), m_MemChunk( a.m_MemChunk)
    {
        if ( m_MemChunk)
            m_MemChunk->RaiseRef();
    }

    Cv_Pin( Cv_Pin &&a)
        : m_Arena( a.m_Arena), m_Index( a.m_Index), m_MemChunk( a.m_MemChunk)
    {
        m_MemChunk = a.m_MemChunk;
        a.m_MemChunk = nullptr; 
        a.m_Index = 0;
    }

    ~Cv_Pin( void)
    {
        if ( m_MemChunk && !m_MemChunk->LowerRef())
        {
            m_Arena->BackLog()->push_back( [ memChunk = m_MemChunk](Arena *arena) { return  memChunk->Evict( arena); }); 
            m_Arena->BackLog()->Process( m_Arena);
        }      
    }

    Cv_Pin  &operator=( const Cv_Pin &a)
    {
        if ( this == &a)
            return *this;
        this->Cv_Pin::~Cv_Pin();
        return *( ::new (this) Cv_Pin( a));
    }
    
    operator LeafType       *( void)  { m_MemChunk->SetClean( false); return m_MemChunk->PtrAt< LeafType>( m_Index & LeafBooth::Mask); }

    operator const LeafType *( void) const { return m_MemChunk->PtrAt< LeafType>( m_Index & LeafBooth::Mask); }
    
    Cv_Pin  &operator++( void)
    {
        if ( ++m_Index & LeafBooth::Mask)
            return *this;
        *this = m_Arena->Pin( m_Index);
        return *this;
    }

    Cv_Pin  &operator+=( int k)
    {
        *this = m_Arena->Pin( m_Index + k);
        return *this;
    }

    friend Cv_Pin  operator+( const Cv_Pin  &p, int k)
    {
        return Cv_Pin( p.m_Arena->Pin( p.m_Index + k));
    }

    friend int  operator-( const Cv_Pin  &p, const Cv_Pin  &q)
    {
        return p.m_Index -q.m_Index;
    }
    friend bool  operator==( const Cv_Pin  &p, const Cv_Pin  &q)
    {
        return p.m_Index ==q.m_Index;
    }
    LeafBooth    *Snitch( void)
    {
        LeafBooth    *memChunk = m_MemChunk;
        m_MemChunk = nullptr; 
        m_Index = 0;
        return memChunk;
    }

    friend void swap(Cv_Pin& lhs, Cv_Pin& rhs);
};

//-------------------------------------------------------------------------------------------------
  
template < class Arena>  
class    Cv_MemBooth  : public Cv_Shared< Arena::MT>, public Cv_PtrIteratorImpl< Cv_MemBooth< Arena>>, public Arena::Janitor
{   
protected:
    Cv_Atomic< uint16_t, Arena::MT>        m_Info;
    Cv_Atomic< void *, Arena::MT>          m_PagePtr;
    
public:
    Cv_MemBooth( void *pgPtr)
		:  m_Info( 0), m_PagePtr( pgPtr)
	{}
    
    uint16_t    ParentOff( void) const { return Cv_Frager< 1, 15>( m_Info.Load()).Get(); }
    void        SetParentOff( uint16_t off) { m_Info.Store( Cv_Frager< 1, 15>( m_Info.Load()).Set( off)); }
    
    bool        IsClean( void) const { return Cv_Frager< 0, 1>( m_Info.Load()).Get(); }
    void        SetClean( bool t) { m_Info.Store( Cv_Frager< 0, 1>( m_Info.Load()).Set( t)); }

template < class X>
    const X     &AccAt( uint32_t i) const { return static_cast< X*>( m_PagePtr.Load())[ i]; }

template < class X>
    X           *PtrAt( uint32_t i) { return &static_cast< X*>( m_PagePtr.Load())[ i]; }
    void        *PtrAt( uint32_t i) { return &static_cast< uint8_t *>( m_PagePtr.Load())[ i]; }

template < class X>
    void        SetAt( uint32_t i, const X &x) { static_cast< X*>( m_PagePtr.load())[ i] = x; }

    bool        IsProcessed( void) const {  return IsClean() && !IsInUse(); }

    bool        IsShutdown( void) { return false; }
};

//-------------------------------------------------------------------------------------------------

template < class Arena, class Parent, typename ValueType , uint32_t SzBits>
class Cv_HeapBooth :  public Cv_Lackey< Parent>, public Cv_MemBooth< Arena>
{
public:
    typedef Parent          Parent;
    typedef ValueType       ValueType;
    typedef Cv_HeapBooth     LeafBooth;
    enum 
    { 
        SzArray         = cv_constExprLib.Pow2( SzBits),   
        PageSz          = sizeof( ValueType) * SzArray,
        SzMask          = SzBits,
        Mask            = Cv_ConstExprLib::LowMask( SzMask),
    };
    
	Cv_HeapBooth( Arena *arena, Parent *parent, uint16_t pParentlink, const ValueType &iVal = Cv_ConstExprLib::InitVal< ValueType>())
		: Cv_Minion( parent), Cv_MemBooth( arena->AllocPage< PageSz>())	
    {
        Cv_Util::Set( At< ValueType>( 0), At< ValueType>( SzArray), iVal);
        SetParentOff( pParentlink);
        if ( m_Owner)
            m_Owner->RaiseRef();
    }

    bool    Evict( Arena *arena)    
    {
        if ( IsInUse())
            return false;
        
        if ( !IsClean() && !arena->ScrubBooth( this, PageSz))
            return false;

        if ( !m_Owner->DetachChild( arena, this))
            return false;

        if ( !m_Owner->LowerRef())
            arena->BackLog()->push_back( [ parent = m_Owner](Arena *arena) { return  parent->Evict( arena); }); 
        m_Owner = nullptr;

        arena->FreePage< PageSz>( m_PagePtr.SpinGrab( nullptr));
        this->Cv_HeapBooth::~Cv_HeapBooth();
        arena->Discard< sizeof( Cv_HeapBooth) >( this);
        return true;
    }

    auto    Pin( Arena *arena, uint32_t l)  
    { 
        return  Cv_Pin< Arena, Cv_HeapBooth>( arena, l, this); 
    }
};

//-------------------------------------------------------------------------------------------------

template < class Arena, class Parent, uint8_t SzBits, class SubChunk>
class Cv_BranchBooth :  public Cv_HeapBooth< Arena, Parent, SubChunk *, SzBits>
{
public:
    typedef typename SubChunk::Parent       Derived;
    typedef typename SubChunk::LeafBooth     LeafBooth; 
    enum 
    { 
        SubChunkSz = sizeof( SubChunk), 
        SzMask = SubChunk::SzMask + SzBits,
        SubMask = Cv_ConstExprLib::LowMask( SubChunk::SzMask), 
        Mask = Cv_ConstExprLib::LowMask( SzMask), 
    };

    Cv_BranchBooth( Arena *arena, Parent *parent, uint16_t pParentlink)
		: Cv_HeapBooth( arena, parent, pParentlink, nullptr)
	{}

    auto    Pin( Arena *arena, uint32_t l)
    {
	    uint16_t        childOff = uint16_t( (l & Mask) >> SubChunk::SzMask);               // we would not be index in 64k pointers ever
        Cv_DEBUG_ASSERT( childOff < SzArray)
	    SubChunk        **childLink = PtrAt< SubChunk *>( childOff);	                    // top SzBits bits only
        bool            heapFlg = arena->IsOnHeap( *childLink);

	    if ( heapFlg)	                                                                    // The page in memory    	                                    
	        return (*childLink)->Pin( arena, l);
	  
        SubChunk                    *subChunk =  new ( arena->Allocate< SubChunkSz>()) SubChunk( arena, static_cast< Derived*>( this), childOff);

        if ( *childLink)
        {
            subChunk->PreserveRef( uint64_t( *childLink)); 
            arena->ReloadBooth( subChunk, SubChunk::PageSz);
        }	    
        *childLink = subChunk;		                                                        // change the childLink value
        auto    ptr = subChunk->Pin( arena, l);
        return ptr;
    }
    
    bool    DetachChild( Arena *arena, Cv_MemBooth< Arena> *spot)
    {
        uint16_t        childOff = spot->ParentOff();
        SubChunk        **childLink = PtrAt< SubChunk *>( childOff);
        Cv_ERROR_ASSERT( *childLink == spot)    
        *childLink = reinterpret_cast< SubChunk *>( spot->GrabRef()) ;
       
        return true;
    }
};

//-------------------------------------------------------------------------------------------------

template< class Arena, class Parent, typename LeafType, uint8_t... Rest>
class Cv_ArenaBooth;

template < class Arena, class Parent, typename LeafType, uint8_t SzBits, uint8_t... Rest>
class Cv_ArenaBooth< Arena, Parent, LeafType, SzBits, Rest...>  : public Cv_BranchBooth< Arena, Parent, SzBits, Cv_ArenaBooth< Arena, Cv_ArenaBooth< Arena, Parent, LeafType, SzBits, Rest...>, LeafType, Rest...> >
{	
public:
    Cv_ArenaBooth( Arena *arena, Parent *parent, uint16_t pParentlink)
        : Cv_BranchBooth( arena, parent, pParentlink)
    {}
};

//-------------------------------------------------------------------------------------------------

template < class Arena, class Parent, typename LeafType, uint8_t SzBits>
class Cv_ArenaBooth< Arena, Parent, LeafType, SzBits> : public  Cv_HeapBooth< Arena, Parent, LeafType, SzBits>
{	
public:
    
    Cv_ArenaBooth( Arena *arena, Parent *parent, uint16_t pParentlink)
        : Cv_HeapBooth( arena, parent, pParentlink, ValueType())
    {}
};

//-------------------------------------------------------------------------------------------------

template< class Arena, class LeafType, bool MT, uint8_t... Rest>
class Cv_BaseArena : public Cv_Shared< MT>
{
public:
    class RootBooth : public Cv_ArenaBooth< Arena, Arena, LeafType, Rest...>  
    {
    public:
        RootBooth( Arena *arena)
            : Cv_ArenaBooth( arena, arena, 0)
        {}
    };

    enum 
    {
        MT = MT,
    };
    
    constexpr static uint32_t               SzMask( void) { return  RootBooth::SzMask; }
    

protected:
    RootBooth                                           *m_TopBooth;
    Cv_CallBacklog< Arena *>                            m_FnBacklog;
    Cv_ObjectIndexor< Arena>                            m_PageSzIndexor;

public:
    Cv_BaseArena( void)
    {
        m_TopBooth = new (Allocate< sizeof( RootBooth)>()) RootBooth( static_cast< Arena*>( this));
    }

    bool    Evict( Arena *arena) { return true; }
    bool    DetachChild( Arena *arena, Cv_MemBooth< Arena> *spot) { return true; }

    bool    IsOnHeap( void *s) { return !!s; }

    auto    Pin( uint32_t k)  { return m_TopBooth->Pin( static_cast< Arena*>( this), k); }

    Cv_CallBacklog< Arena *>         *BackLog( void) { return &m_FnBacklog; }

template < uint32_t AllocSz>
    void     *Allocate( void)
    {
        void    *obj = new char[ AllocSz];
        return obj;
    }

template < uint32_t AllocSz>
    void     Discard( void *obj)
    {
        delete [] obj;
        return;
    }

template < uint32_t PageSz>
    void    *AllocPage( void)
    {
        uint32_t    m_PgIndex = m_PageSzIndexor.Index< PageSz>();
	    void    *pg = new char[ PageSz];
        return pg;
    }

template < uint32_t PageSz>
    void    FreePage( void *pg)
    {
        uint32_t    m_PgIndex = m_PageSzIndexor.Index< PageSz>();

	    delete [] pg;
    }
    class Janitor
    {
    public:
        
        void        PreserveRef( uint64_t off)     {  ; }
        uint64_t    GrabRef( void) const { return 0x0; }
    };

    void    ReloadBooth( Cv_MemBooth< Arena> *spot, uint32_t pageSz)
    {
    };

    bool    ScrubBooth( Cv_MemBooth< Arena> *spot, uint32_t sz)
    {
        return true;
    }
};

//-------------------------------------------------------------------------------------------------

template< class LeafType, bool MT, uint8_t... Rest>
class Cv_Arena : public Cv_BaseArena< Cv_Arena< LeafType, MT, Rest...>, LeafType, MT, Rest...>
{
};

//-------------------------------------------------------------------------------------------------

template< class LeafType, bool MT, uint8_t... Rest>
class Cv_FileArena : public Cv_BaseArena< Cv_FileArena< LeafType, MT, Rest...>, LeafType, MT, Rest...>
{
    FILE    *m_Fp;
  
public:
    
    Cv_FileArena( const char *nm, bool freshFlg)
    {
        m_Fp = fopen( nm, !freshFlg && Cv_Util::FileExists( nm) ? "r+b" :  "w+b");
        m_TopBooth->m_FileOffset.Store( 0);
        fseek( m_Fp, 0L, SEEK_END );
        if ( ftell( m_Fp))
            ReloadBooth( m_TopBooth, RootBooth::PageSz);
        else
            ScrubBooth( m_TopBooth, RootBooth::PageSz);
    }    

    Cv_FileArena( FILE  *f) 
        :  m_Fp( f)
    {
        m_TopBooth->m_FileOffset.Store( 0);
        fseek( m_Fp, 0L, SEEK_END );
        if ( ftell( m_Fp))
            ReloadBooth( m_TopBooth, RootBooth::PageSz);
        else
            ScrubBooth( m_TopBooth, RootBooth::PageSz);
    }    

    class Janitor
    {
    public:
        Cv_Atomic< uint32_t, MT>        m_FileOffset;
            
        Janitor( void) 
            : m_FileOffset( Cv_UINT32_MAX) {}

        void        PreserveRef( uint64_t off)     {  m_FileOffset.Store( !off ? Cv_UINT32_MAX : ( off & ~0x1)); }
        uint64_t    GrabRef( void) const { return m_FileOffset.Load() | 0x1; }
    };


    bool    IsOnHeap(  void *s) { return !!s && !( uint64_t( s) & 0x1); }

    //-------------------------------------------------------------------------------------------------

    void    ReloadBooth( Cv_MemBooth< Cv_FileArena> *spot, uint32_t pageSz)
    {
        fseek( m_Fp, spot->m_FileOffset.Load(), SEEK_SET );
        Cv_DEBUG_ASSERT( spot->m_FileOffset.Load() == ftell( m_Fp))
	    fread( spot->PtrAt( 0), 1, pageSz, m_Fp);
        return;
    }

    //-------------------------------------------------------------------------------------------------

    bool    ScrubBooth( Cv_MemBooth< Cv_FileArena> *spot, uint32_t pageSz)
    {

        if ( spot->m_FileOffset.Load() == Cv_UINT32_MAX) 
	    {
            int     res = ::fseek( m_Fp, 0L, SEEK_END );
            Cv_ERROR_ASSERT( res == 0)
            spot->m_FileOffset.Store( uint32_t( ::ftell( m_Fp)));
            fwrite(  spot->PtrAt( 0), 1, pageSz, m_Fp);
            return true;
        } 

        int         res = fseek( m_Fp, spot->m_FileOffset.Load(), SEEK_SET );
        Cv_ERROR_ASSERT( res == 0)
        fwrite( spot->PtrAt( 0), 1, pageSz,  m_Fp);
        return true;
    }
};

//-------------------------------------------------------------------------------------------------

template < class X>
class Cv_Spot
{
protected: 
    X           *m_Ptr;
    uint32_t    m_Index;
    
public:
    Cv_Spot( X *ptr, uint32_t index)
        :  m_Ptr( ptr), m_Index( index)  
    {}  

    operator X *( void) { return m_Ptr; }

    X           *Ptr( void) const { return m_Ptr; }
    uint32_t    Index( void) const { return m_Index; }
};

//-------------------------------------------------------------------------------------------------

template < class Arena>
class   Cv_ArenaStore
{

    struct  Atom : public Cv_Link< Atom> 
    {
        uint32_t    m_Index;
        
        Atom( uint32_t index)
            : m_Index( index)
        {}
    };

    Cv_LinkStack< Atom>                     m_FreeAtomStack;        // stack of free atoms
    Arena                                   *m_Arena;
    uint32_t                                m_ApprLeafSz;           // Num of leaves appropriasted

    typedef typename Arena::RootBooth::LeafBooth         LeafBooth;
    typedef typename Arena::LeafType                     LeafType;

    enum 
    {
        SzArray     = LeafBooth::SzArray,                           // Num objects in leaf
        SzBits      = Arena::RootBooth::SzMask,                     // Total bits supported..
        SzTopBits   = SzBits - LeafBooth::SzBits,                   // Bits supported by branches
        SzLeaf      = cv_constExprLib.Pow2( SzTopBits),             // Max number of leaves 
    };

    //----------------------------------------------------------------------
    
    void Appropriate( void)
    {
	    LeafBooth    *leafBooth = m_Arena->Pin( m_ApprPageSz << LeafBooth::SzBits).Snitch();
        for ( uint32_t i = 0; i < SzArray; ++i)
        {
            LeafType    *spot = leafBooth->PtrAt< LeafType>( SzArray -1 -i);
            Atom        *atom = new ( spot) Atom( ( SzArray -1 -i) | m_ApprPageSz);
            m_FreeAtomStack.Push( atom);
        }
        ++m_ApprPageSz;

    }

public:
    Cv_ArenaStore( Arena *arena)
        : m_Arena( arena), m_ApprPageSz( 0)
    {}

template< class X>
    Cv_Spot< X>     Allocate( void) 
    { 
        Atom    *atom =  m_FreeAtomStack.Top() ? m_FreeAtomStack.Pop() : ( Appropriate(), m_FreeAtomStack.Pop());
        return Cv_Spot< X>( reinterpret_cast< X *>( atom), atom->m_Index);
    }

template< class X>
	void            Discard( Cv_Spot< X> *p) 
    { 
        Atom        *atom = reinterpret_cast< Atom*>( p->m_Ptr);
        atom->m_Index = p->Index();
        m_FreeAtomStack.Push( atom); 
        *p = Cv_Spot< X>( nullptr, 0);
        return;
    }
};

//-------------------------------------------------------------------------------------------------