// cv_tuple.h  ________________________________________________________________________________________________________________
#pragma once

//_____________________________________________________________________________________________________________________________

template < typename T, typename... Rest>
class   Cv_Tuple;  

//_____________________________________________________________________________________________________________________________

template <typename T, int K>
struct Cv_TupleIndex
{
    typedef typename T::TupleBase TupleBase;

	T	*m_T;

	
	Cv_TupleIndex( T *p)
		:  m_T( p)
	{}

	auto	PVar( void)  {  return  Cv_TupleIndex< TupleBase, K -1>( static_cast< TupleBase *>( m_T)).PVar();  }
};

template <typename T>
struct Cv_TupleIndex< T, 0>
{

	T	*m_T;
	
	Cv_TupleIndex( T *p)
		:  m_T( p)
	{}

	auto	PVar( void) { return m_T->PVar(); }
};

//_____________________________________________________________________________________________________________________________

template< std::size_t I, class T >
struct Cv_TupleType;

template< std::size_t I, class Head, class... Tail >
struct Cv_TupleType< I, Cv_Tuple<Head, Tail...>> : Cv_TupleType< I - 1, Cv_Tuple< Tail...>> 
{ 
};

template< class Head, class... Tail >
struct Cv_TupleType< 0, Cv_Tuple<Head, Tail...>> 
{
    typedef Head type;
};
 
//_____________________________________________________________________________________________________________________________

template < template< class> class TypeMap, typename Tuple, typename = void>
class Cv_TypeMapTuple 
{
public:
    typedef typename Tuple::CType                       CType; 
    TypeMap<  CType>                                    m_Var;
     
    auto	PVar( void) { return &m_Var; } 
};

//_____________________________________________________________________________________________________________________________

template < template< class> class TypeMap, typename Tuple>
class Cv_TypeMapTuple< TypeMap, Tuple, typename Cv_TypeEngage::Exist< typename Tuple::TupleBase>::Note> 
                                                                            : public Cv_TypeMapTuple< TypeMap, typename Tuple::TupleBase>
{
public:
    typedef typename Tuple::TupleBase                   TupleTupleBase;
    typedef Cv_TypeMapTuple< TypeMap, TupleTupleBase>   TupleBase;
    typedef typename Tuple::CType                       CType;

    TypeMap<  CType>                                    m_Var;

    auto	PVar( void) { return &m_Var; } 
};

//_____________________________________________________________________________________________________________________________

struct  Cv_TupleTools
{   
template <std::size_t N,  typename Tuple>
    using type = typename Cv_TupleType< N, Tuple>::type;

template< typename... Types >
    static constexpr auto   Make( const Types &... args );

template< typename... Types >
    static constexpr auto   Make( Types&&... args );
	
template< typename T, typename... BT >
    static constexpr auto  	Cons( const T &t1, const Cv_Tuple< BT...> &t2);

template< typename T, typename... BT >
    static constexpr auto  	Cons( T &&t1, Cv_Tuple< BT...> &&t2);

template< typename T, typename... BT >
    static constexpr auto  	Reverse( const Cv_Tuple< T, BT...> &t);

template< typename T, typename... BT >
    static constexpr   auto Melt( const  Cv_Tuple< T, BT...> &tuple);

template< typename... Types >
    static constexpr auto 	Dump( std::ostream &ostr, const  Cv_Tuple< Types...> &tuple);
	
template< template< class> class TypeMap, typename... Types >
    static constexpr void 	PtrVectorAssign( Cv_TypeMapTuple< TypeMap, Cv_Tuple< Types...>> &ptrVecTuple, const Cv_Tuple< Types...> &tuple);

template< typename T1, typename T2>
    static constexpr   auto Fuse( const T1 &t1, const T2 &t2); 

template< typename Lambda, typename T1, typename T2>
    static constexpr   auto Binary( const Lambda &lambda, const T1 &t1, const T2 &t2);

}; 

//_____________________________________________________________________________________________________________________________

template < typename T, typename... Rest>
class   Cv_Tuple : public Cv_Tuple< Rest...>
{
public:
    T        m_Var;
   

    typedef T                     CType;
    typedef Cv_Tuple< T, Rest...> Tuple;
    typedef Cv_Tuple< Rest...>    TupleBase;

    enum {
        Sz = TupleBase::Sz +1,
    };
    
    Cv_Tuple( void)
    {}

    Cv_Tuple( const Cv_Tuple &t)
        : TupleBase( ( const TupleBase &) t), m_Var( t.m_Var)
    {}

    Cv_Tuple( const T &t, Rest... rest)
        : TupleBase( rest...), m_Var( t)
    {}

    Cv_Tuple( const  T &t, const TupleBase &base)
        : TupleBase( base), m_Var( t)
    {}


    Cv_Tuple( T &&t, TupleBase &&base)
        : TupleBase( std::move( base)), m_Var( std::move( t)) 
    {}

template <class X>
    Cv_Tuple( X x)
        : TupleBase( x), m_Var( x)
    {}
    
	auto	PVar( void) { return &m_Var; }
	

template < int K>
    auto        Ptr( void)  { return Cv_TupleIndex< Tuple, Tuple::Sz -1 -K>( this).PVar(); } 

template < int K>
    auto        Var( void) const { return *const_cast<Tuple *>( this)->Ptr< K>(); }

template < int K>
    auto        Var( void)  { return *const_cast<Tuple *>( this)->Ptr< K>(); }

template < typename Lambda>
    auto    Compose( Lambda lambda) const
    {
        return Cv_TupleTools::Cons( [=](auto... rest) { return lambda( uint32_t( Tuple::Sz -1), m_Var, rest...);},  TupleBase::Compose( lambda)); 
    };

 template < typename Lambda>
    auto    Unary( Lambda lambda) 
    {
        return  Cv_TupleTools::Cons( lambda( m_Var), TupleBase::Unary( lambda));
    };

template < typename... X>
    auto    Invoke( X... args) const
    {
        return  Cv_TupleTools::Cons( m_Var( args...), TupleBase::Invoke( args...)) ;
    }
template < typename... X>
    auto    Invoke( void) const
    {

        return  Cv_TupleTools::Cons( m_Var(), TupleBase::Invoke()) ;
    }
};

//_____________________________________________________________________________________________________________________________


template < typename T >
class   Cv_Tuple< T>
{
 public:
   T       m_Var;
    

    typedef T                       CType;
    typedef Cv_Tuple< T>            Tuple; 
    
    enum {
        Sz = 1,
    };
    
    Cv_Tuple( void)
    {}

    Cv_Tuple( const T &t)
        : m_Var( t)
    {}

    Cv_Tuple( const Cv_Tuple &t)
        : m_Var( t.m_Var) 
    {}

template <class X>
    Cv_Tuple(  X x)
        : m_Var( x) 
    {}

    auto	PVar( void) { return &m_Var; }

template < int K>
    auto    Var( void) const { return m_Var; }

template < int K>
    auto    &Var( void) const { return m_Var; }

template < typename Lambda>
    auto    Compose( Lambda lambda) const
    {
        return  Cv_TupleTools::Make( [=](auto... rest) { return lambda( uint32_t( 0), m_Var, rest...);});
    };

template < typename Lambda>
    auto    Unary( Lambda lambda) 
    {
        return  Cv_TupleTools::Make( lambda( m_Var));
    };

template < typename... X>
    auto Invoke( X... args) const
    {
        return  Cv_TupleTools::Make( m_Var( args...)) ;
    }
template < typename... X>
    auto Invoke( void) const
    {
        return  Cv_TupleTools::Make( m_Var()) ;
    }
}; 

//_____________________________________________________________________________________________________________________________

template < int K, typename Tuple>
struct Ru_Index : public Ru_Index< K -1, typename Tuple::TupleBase>
{
};

template <typename Tuple>
struct Ru_Index< 0, Tuple>
{
    typedef typename Tuple::CType  Type;
};

//_____________________________________________________________________________________________________________________________

template< typename... Types >
constexpr auto   Cv_TupleTools::Make( const Types&... args )
{
	return Cv_Tuple< Types...>( args...);
}

template< typename... Types >
constexpr auto   Cv_TupleTools::Make( Types&&... args )
{
	return Cv_Tuple< Types...>( std::forward<Types>( args)...);
}

template< typename T, typename... BT >
constexpr auto  Cv_TupleTools::Cons( T &&t1, Cv_Tuple< BT...> &&t2)
{
    typedef Cv_Tuple< T, BT...>     Tup;
	return Tup( std::forward<T>( t1), std::forward< typename Tup::TupleBase>( t2));
}

template< typename T, typename... BT >
constexpr auto  Cv_TupleTools::Cons( const T &t1, const Cv_Tuple< BT...> &t2)
{   
    typedef Cv_Tuple< T, BT...>     Tup;
	return Tup( t1, static_cast< const typename Tup::TupleBase &>( t2));
}


template< typename T, typename... BT >
constexpr auto Cv_TupleTools::Reverse( const Cv_Tuple< T, BT...> &t)
{
    return Fuse( static_cast< const Cv_Tuple< BT...> &>( t), Cv_Tuple< T>( t.m_Var));  
}

template< typename T, typename... BT >
struct Cv_TupleToolsMelter
{
    static constexpr   auto Melt( const  Cv_Tuple< T, BT...> &tuple)
    {
        return Cv_TupleTools::Fuse( tuple.m_Var, Cv_TupleToolsMelter< BT...>::Melt(  tuple)); 
    }
};
template< typename T >
struct Cv_TupleToolsMelter< T>
{
    static constexpr   auto Melt( const  Cv_Tuple< T> &tuple)
    {
        return tuple.m_Var; 
    }
};

template< typename T, typename... BT > 
constexpr   auto    Cv_TupleTools::Melt( const  Cv_Tuple< T, BT...> &tuple)
{
    return Cv_TupleToolsMelter< T, BT...>::Melt( tuple); 
}

template< typename... Types >
constexpr auto   Cv_TupleTools::Dump( std::ostream &ostr, const  Cv_Tuple< Types...> &tuple)
{
    return tuple.Compose( [&ostr]( uint32_t k, auto var) 
    { 
        if ( k == ( Cv_Tuple< Types...>::Sz -1)) 
            ostr << "[ ";
        ostr << var << (( k == 0) ? "] " : ", "); 
        return true; 
    });
}
 
template< typename T1, typename T2, typename = void>
struct Cv_TupleToolsPtrVectorAssigner
{
    static constexpr void PtrVectorAssign( T1 &ptrVecTuple, const T2 &tuple)
    {
        for (  auto it = ptrVecTuple.m_Var.begin(); it != ptrVecTuple.m_Var.end();  ++it)
            *(*it) = tuple.m_Var;       
    }
};

template< typename T1, typename T2>
struct Cv_TupleToolsPtrVectorAssigner< T1, T2, typename Cv_TypeEngage::Exist< typename T1::TupleBase>::Note>
{
    static constexpr void PtrVectorAssign( T1 &ptrVecTuple, const T2 &tuple)
    {
        for (  auto it = ptrVecTuple.m_Var.begin(); it != ptrVecTuple.m_Var.end();  ++it)
            *(*it) = tuple.m_Var;    
        Cv_TupleTools::PtrVectorAssign( static_cast< typename T1::TupleBase &>( ptrVecTuple), static_cast< const typename T2::TupleBase &>( tuple));
    }
};
 

template< template< class> class TypeMap, typename... Types >
constexpr void 	Cv_TupleTools::PtrVectorAssign( Cv_TypeMapTuple< TypeMap, Cv_Tuple< Types...>> &ptrVecTuple, const Cv_Tuple< Types...> &tuple) 
{
    Cv_TupleToolsPtrVectorAssigner< Cv_TypeMapTuple< TypeMap, Cv_Tuple< Types...>>, Cv_Tuple< Types...>>::PtrVectorAssign( ptrVecTuple, tuple);
}


template< typename T1, typename T2, typename = void>
struct Cv_TupleToolsFuser
{
    constexpr static auto Fuse( const T1 &t1, const T2 &t2)
    {
        return Cv_TupleTools::Cons( t1.m_Var, t2);
    }
};

template< typename T1, typename T2>
struct Cv_TupleToolsFuser< T1, T2, typename Cv_TypeEngage::Exist< typename T1::TupleBase>::Note>
{
    constexpr static auto Fuse( const T1 &t1, const T2 &t2)
    {
        return Cv_TupleTools::Fuse( static_cast< const typename T1::TupleBase &>( t1), Cv_TupleTools::Cons( t1.m_Var, t2));
    }
};


template< typename T1, typename T2>
constexpr auto   Cv_TupleTools::Fuse( const T1 &t1, const T2 &t2)
{
    return  Cv_TupleToolsFuser< T1, T2>::Fuse( t1, t2);
}
 

template< typename Lambda, typename T1, typename T2, typename = void>
struct Cv_TupleToolsBinaryOps
{
    constexpr static auto Binary( const Lambda &lambda, const T1 &t1, const T2 &t2 )
    {
        return Cv_TupleTools::Make( lambda( t1.m_Var, t2.m_Var));
    }
};

template< typename Lambda, typename T1, typename T2>
struct Cv_TupleToolsBinaryOps< Lambda, T1, T2, typename Cv_TypeEngage::Exist< typename T1::TupleBase>::Note>
{
    constexpr static auto Binary( const Lambda &lambda, const T1 &t1, const T2 &t2)
    {
        return Cv_TupleTools::Cons( lambda( t1.m_Var, t2.m_Var), Cv_TupleTools::Binary( lambda, static_cast< const typename T1::TupleBase &>( t1), 
                                                                                static_cast< const typename T2::TupleBase &>( t2)));
    }
};

template< typename Lambda, typename T1, typename T2>
constexpr   auto Cv_TupleTools::Binary( const Lambda &lambda, const T1 &t1, const T2 &t2) 
{
    return Cv_TupleToolsBinaryOps< Lambda, T1, T2>::Binary( lambda, t1, t2);
}


//_____________________________________________________________________________________________________________________________

template < typename X>
class Cv_Twin : public Cv_Tuple< X, X>
{
public:
    auto    First( void) const { return this->template Var< 0>(); }
    void    SetFirst( const X &x) { this->template Var< 0>() = x; }
    
    auto    Second( void) const { return this->template Var< 1>(); }
    void    SetSecond( const X &x) { this->template Var< 1>() = x; } 
};

//_____________________________________________________________________________________________________________________________

template < typename Tuple, typename = void>
class Cv_OpTuple 
{
public:
    typedef typename Tuple::CType  CType;

    std::function< bool( const CType &)>    m_Var;
     
    auto	PVar( void) { return &m_Var; }

    bool    Apply( const Tuple &tup) { return m_Var( tup.m_Var); }
};

template < typename Tuple>
class Cv_OpTuple< Tuple, typename Cv_TypeEngage::Exist< typename Tuple::TupleBase>::Note> : public Cv_OpTuple< typename Tuple::TupleBase>
{
public:
    typedef typename Tuple::TupleBase       TupleTupleBase;
    typedef Cv_OpTuple< TupleTupleBase>     TupleBase;
    typedef typename Tuple::CType           CType;

    std::function< bool( const CType &)>    m_Var;

    auto	PVar( void) { return &m_Var; }
    
    bool    Apply( const Tuple &tup)  { return m_Var( tup.m_Var) && TupleBase::Apply( static_cast< const TupleTupleBase &>( tup)); }
};

//_____________________________________________________________________________________________________________________________
