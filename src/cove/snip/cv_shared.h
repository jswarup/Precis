
//--------------------- ac_shared.h ------------------------------------

#ifndef	AC_SHARED__H
#define AC_SHARED__H

//--------------------------------------------------------------------------

class Ac_Shared 
{
protected:
	//-----------------------------------------------------------------------

	Ac_Shared	&operator=( const Ac_Shared &) { m_RefCount = 0;  return *this;}
	

	//-----------------------------------------------------------------------

	uint32_t		m_RefCount;

	//-----------------------------------------------------------------------

public:
	Ac_Shared( uint32_t u = 0) : m_RefCount( u) {}
	Ac_Shared( const Ac_Shared &) : m_RefCount( 0) {}

	//-----------------------------------------------------------------------
	
    uint32_t		RaiseReference( void) {	return ++m_RefCount; }
	
	uint32_t		LowerReference( void) {	return --m_RefCount;	}

	uint32_t		RefCount( void) const { return m_RefCount;}

    void            FlipReferences( Ac_Shared *shared) { std::swap( m_RefCount, shared->m_RefCount); }
};

//--------------------------------------------------------------------------
#endif

