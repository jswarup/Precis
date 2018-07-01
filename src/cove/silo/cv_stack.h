//  cv_stack.h ___________________________________________________________________________________________________________________
#pragma once

//-------------------------------------------------------------------------------------------------


template <class StackVar>
class  Cv_StackVar 
{
protected:
	StackVar            *m_Below;   // the link to next

public:
	Cv_StackVar( StackVar *b = NULL)  
        : m_Below( b) 
    {}
	
    ~Cv_StackVar( void) 
    { Initialize(); }

	void            Initialize( void) { m_Below = NULL; }
	StackVar        *GetBelow( void) const { return m_Below; }
	void            SetBelow( StackVar *b) { m_Below = b; }

};

//------------------------------------------------------------------------------------------------

template <class StackVar>
class  Cv_Stack 
{
protected:

	StackVar    *m_Top;             // the head of the linked-list

public :

	Cv_Stack( StackVar *tp = NULL) : m_Top( tp) {}
	void        Initialize( void) { m_Top = NULL; }

	StackVar    *Top( void)  const { return m_Top; }

	// fetch the last element
	StackVar    *Bottom( void) const 
	{
		StackVar        *tmp = m_Top;
		while ( tmp && tmp->GetBelow())
			tmp = tmp->GetBelow();
		return tmp;
	}

	// take out the head and put the following one in its place.
	StackVar    *Pop( void)
	{
		if ( ! m_Top)
			return NULL;
		StackVar        *tmp = m_Top;
		m_Top = m_Top->GetBelow();
		return tmp;
	}


	// put a new element at the head and make it point to old head.
	void         Push( StackVar *v) 
	{       
		if ( ! v) 
			return;
		v->SetBelow( m_Top); 
		m_Top = v; 
        return;
	}

    // put a new element at the head and make it point to old head.
	void         Append( StackVar *v) 
	{       
		if ( !m_Top)
            Push( v);
		else
        {
            StackVar    *tmp = m_Top;
            for ( ; tmp->GetBelow(); tmp = tmp->GetBelow())
                ;
            tmp->SetBelow( v);
        }
        return;
	}

	// scan the list and Remove the given element from it.
	void         Remove( StackVar *v)
	{
		if ( ! m_Top)
			return;         // list is Empty

		if ( m_Top == v)    
		{   // the element is at head
			Pop();
			return;
		}
		StackVar    *tmp = m_Top;
		do 
		{
			if ( tmp->GetBelow() == v)
				break;
			tmp = tmp->GetBelow();
		} while( tmp);
		assert( tmp && tmp->GetBelow() && ( tmp->GetBelow() == v));
		tmp->SetBelow( v->GetBelow());
		return;
	}


	// Transfer all elements from stk to this => stk becomes Empty
	int  Transfer(  Cv_Stack< StackVar>  *stk)
	{
		int             i = 0;
		for ( StackVar  *t = NULL; t = stk->Pop(); ++i)
			Push( t);
		return i;
	}

	// Reverse ourselves.
	int Reverse( void)
	{       
		Cv_Stack< StackVar>      tmp;
		int                     n = tmp.Transfer( this);
		m_Top = tmp.m_Top;
		return n;
	}

	int  SzVar(  void)
	{
		int             i = 0;
		for ( StackVar  *t = Top(); (t = t->GetBelow()) ; ++i)
			;
		return i;
	}

	bool    Find( StackVar *s) const
	{
		for ( StackVar    *v = Top(); v; v = v->GetBelow())
			if (v == s)
				return true;
		return false;
	}

	StackVar    *AboveOf( StackVar *s, StackVar *t = NULL) const
	{
        StackVar    *v = t;
        if ( !v)
            v = Top();
		for ( ; v; v = v->GetBelow())
			if (v->GetBelow() == s)
				return v;
		return NULL;
	}


    StackVar *PromoteAfter( StackVar *s, StackVar *beg, StackVar *end = NULL)
	{
        if ( !end)
            end = AboveOf( end, beg);

        assert( end);
        
        StackVar *sPrev = AboveOf( beg, s);
        if ( !sPrev)
            return NULL;
        sPrev->SetBelow( end->GetBelow());      // if end was last in chain, sPrev would now be last
        end->SetBelow( s->GetBelow());
        s->SetBelow(  beg);
        return sPrev;
    }

    
};

//-------------------------------------------------------------------------------------------------

template <class X>
class Cv_Fifo 
{
protected:
	X               *b;
	Cv_Stack<  X>    stk;

public:

	Cv_Fifo( void)  : b( NULL), stk() {}

	X           *Top( void) const { return stk.Top(); }
	X           *Bottom( void) const { return b; }
    X           *AboveOf( X *s, X *t = NULL) const { return stk.AboveOf( s, t); }
    bool        IsUniSize( void) const { return Top() && ( Top() == Bottom()); }
    bool        IsEmpty( void) const { return !Top(); }
 

	X    *PopFront( void)
	{
		if ( ! b) 
			return NULL;
		X    *x = stk.Pop();
		if ( b == x)
			b = NULL;
		return x;
	}
 
	void    PushFront( X *x)
	{
		stk.Push( x);
		if ( ! b)
			b = x;
		return;
	} 
    
    void    PushBack( X *x)
	{
		if ( ! b) 
			stk.Push( x);
		else 
			b->SetBelow( x);
		b = x;
		x->SetBelow( NULL);
		return;
	} 
    
    void    PushHind( X *x)
	{
		if ( ! b) 
			stk = Cv_Stack< X>( x);
		else 
			b->SetBelow( x);
        for ( ; x->GetBelow(); x = x->GetBelow())
            ;
		b = x;
		return;
	}
 
	
    void Transfer( Cv_Fifo< X> *list)
	{
		X    *t = NULL;
		while ( t = list->stk.Pop())
			PushFront( t);
		list->b = NULL;
		return;
	}
 

    X *PromoteAfter( X *s, X *beg, X *end = NULL)
    {
        X       *sp = stk.PromoteAfter( s, beg, end);
        if ( sp && end == b)
            b = sp;
	    return sp;
    }
};

 