// cv_compare.h ______________________________________________________________________________________________________________
#pragma once

//_____________________________________________________________________________________________________________________________

template < typename Deriv>
struct Cv_TCompare
{
template < typename X>
    int32_t  Compare( const std::vector< X> &a, const std::vector< X> &b) 
    { 
        if ( a.size() != b.size())
            return a.size() < b.size() ? -1 : 1;
        for ( uint32_t i = 0; i < a.size(); ++i)
        {
            int32_t     res = static_cast< Deriv *>( this)->Compare( a[ i], b[ i]);
            if ( res != 0)
                return res;
        }
        return 0; 
    }

template < typename X>
    int32_t  Compare( const std::vector< X *> &a, const std::vector< X *> &b) 
    { 
        if ( a.size() != b.size())
            return a.size() < b.size() ? -1 : 1;
        for ( uint32_t i = 0; i < a.size(); ++i)
        {
            if ( a[ i] == b[ i])
                continue;

            if ( !( a[ i] && b[ i]))
                return a[ i] < b[ i] ? -1 : 1;

            int32_t     res = static_cast< Deriv *>( this)->Compare( *a[ i], *b[ i]);
            if ( res != 0)
                return res;
        }
        return 0; 
    }

template < typename X>
    int32_t  Compare( const X  &a, const X &b) const
    { 
        return  static_cast< const Deriv *>( this)->CompareInst( a, b);
    }

    int32_t  Compare( int32_t a,  int32_t b) const
    { 
        return  a != b ? ( a < b ? -1: 1) : 0;
    }

    int32_t  Compare( uint32_t a,  uint32_t b) const
    { 
        return  a != b ? ( a < b ? -1: 1) : 0;
    }

    int32_t  Compare( uint64_t a,  uint64_t b) const
    { 
        return  a != b ? ( a < b ? -1: 1) : 0;
    }

template < typename X>
    int32_t  CompareInst( const X  &a, const X &b) const 
    { 
        return  a.Compare( static_cast< Deriv *>( this)->y, b);
    }
};

//_____________________________________________________________________________________________________________________________

template < typename T>
struct Cv_TLess : public Cv_TCompare< Cv_TLess< T> >
{
    T       y;

    Cv_TLess( const T &y1) : y( y1) {}

template < typename X>
    bool    operator()( const X &a, const X &b) const { return this->Compare( a, b) < 0; }
};

//_____________________________________________________________________________________________________________________________

template <>
struct Cv_TLess< void> : public Cv_TCompare< Cv_TLess< void> >
{
template < typename X>
    int32_t  CompareInst( const X  &a, const X &b)  const {  return  a.Compare( b); }

template < typename X>
    bool    operator()( const X &a, const X &b) const { return this->Compare( a, b) < 0; }
};

//_____________________________________________________________________________________________________________________________

template < typename T>
struct Cv_TEqual : public Cv_TCompare< Cv_TEqual< T> >
{
    T       y;

    Cv_TEqual( const T &y1) : y( y1) {}

template < typename X>
    bool    operator()( const X &a, const X &b) { return this->Compare( a, b) == 0; }
};

//_____________________________________________________________________________________________________________________________

template <>
struct Cv_TEqual< void> : public Cv_TCompare< Cv_TEqual< void> >
{

template < typename X>
    int32_t  CompareInst( const X  &a, const X &b) const  {  return  a.Compare( b); }

template < typename X>
    bool    operator()( const X &a, const X &b) { return this->Compare( a, b) == 0; }
};

//_____________________________________________________________________________________________________________________________

template < typename T>
struct Cv_TPtrLess : public Cv_TLess< T>
{
    Cv_TPtrLess( const T &y1) : Cv_TLess< T>( y1) {}

template < typename X>
    bool    operator()( const X *a, const X *b) const { return a == b ? false : ( a && b ? Compare( *a, *b) < 0 : a < b); }
};

//_____________________________________________________________________________________________________________________________

template <>
struct Cv_TPtrLess< void> : public Cv_TLess< void>
{
template < typename X>
    bool    operator()( const X *a, const X *b) const { return a == b ? false : ( a && b ? ( Compare( *a, *b) < 0) : a < b); }
};

//_____________________________________________________________________________________________________________________________