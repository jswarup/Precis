// cv_ptrslot.h ____________________________________________________________________________________________________________________
#pragma once
 

//_________________________________________________________________________________________________________________________________

class Cv_Shared 
{
protected: 
    uint32_t		m_RefCount;

    Cv_Shared	    &operator=( const Cv_Shared &) { m_RefCount = 0;  return *this;} 

public:
    Cv_Shared( uint32_t u = 0) 
        : m_RefCount( u) 
    {}

    Cv_Shared( const Cv_Shared &) 
        : m_RefCount( 0) 
    {} 

    uint32_t		RaiseReference( void) {	return ++m_RefCount; }

    uint32_t		LowerReference( void) {	return --m_RefCount;	}

    uint32_t		RefCount( void) const { return m_RefCount;}

    void            FlipReferences( Cv_Shared *shared)  {  std::swap( m_RefCount, shared->m_RefCount);  }
};

//_________________________________________________________________________________________________________________________________

template < class X>
class Cv_Ptr 
{
protected:
    X   *m_Inst;

public:
    Cv_Ptr( X *u = NULL) 
        : m_Inst( u) 
    {}

    bool	operator==( const Cv_Ptr< X> &arr) const {	return ( m_Inst == arr.m_Inst); }
    bool	operator!=( const Cv_Ptr< X> &arr) const {	return ( m_Inst != arr.m_Inst); }

    bool			IsValid( void) const { return m_Inst != 0; }
    X               *Ptr( void) const { return m_Inst; }

    operator        X*( void) const { return m_Inst; }
    X               *operator->( void) const { return m_Inst; }


    template < typename T >
    int32_t         Compare( T t, const Cv_Ptr< X> &m) const
    {
        return ( m_Inst == m.m_Inst) ? 0 : (( !m_Inst || !m.m_Inst) ? ( m_Inst < m.m_Inst ? -1 : 1) : m_Inst->Compare( t, *m.m_Inst));
    }

    int32_t         Compare( const Cv_Ptr< X> &m) const
    {
        return ( m_Inst == m.m_Inst) ? 0 : (( !m_Inst || !m.m_Inst) ? ( m_Inst < m.m_Inst ? -1 : 1) : m_Inst->Compare( *m.m_Inst));
    }
};

//_____________________________________________________________________________________________________________________________

template < class Shared>
class Cv_Slot : public Cv_Ptr< Shared>
{
    typedef Cv_Ptr< Shared> 	Base;

    bool	    IsSoleRef( void) const { return Base::m_Inst && ( Base::m_Inst->RefCount() == 1); } 

public:
    Cv_Slot( Shared *u = NULL) 
        : Cv_Ptr< Shared>( u) 
    {
        if ( Base::m_Inst)
            Base::m_Inst->RaiseReference(); 
    }

    template < class XShared>
    Cv_Slot( const Cv_Slot< XShared> &arr) 
        : Cv_Ptr< Shared>( static_cast< Shared*>( arr.GetInstance()))
    {	
        if ( Base::m_Inst)
            Base::m_Inst->RaiseReference(); 
    }

    Cv_Slot( const Cv_Slot< Shared> &arr) 
        : Cv_Ptr< Shared>( arr.m_Inst)
    {	
        if ( Base::m_Inst)
            Base::m_Inst->RaiseReference(); 
    }

    ~Cv_Slot( void) 
    {	
        if ( Base::m_Inst && ( Base::m_Inst->LowerReference() == 0)) 
            delete Base::m_Inst;
    }

    Cv_Slot< Shared>	&operator=( const Cv_Slot< Shared> &arr)
    {	
        if ( arr == *this)	
            return *this;
        if ( Base::m_Inst && ( Base::m_Inst->LowerReference() == 0))
            delete Base::m_Inst;	
        Base::m_Inst = arr.m_Inst;	
        if ( Base::m_Inst)
            Base::m_Inst->RaiseReference(); 
        return *this;	
    }
};

//_____________________________________________________________________________________________________________________________
