// cv_dlist.h ______________________________________________________________________________________________________________
#pragma once

//_____________________________________________________________________________________________________________________________

template <class DLink>
struct Cv_DLink
{ 
    DLink    *m_Prev;
    DLink    *m_Next;  
};
  
//_____________________________________________________________________________________________________________________________ 

template < typename DLinkPtr, typename DHandler, bool Circ = false>
class Cv_DList
{
protected:
    static constexpr DLinkPtr   DNIL( void) { return  DHandler::NIL(); }  
    DHandler                    *Handler( void) { return static_cast< DHandler*>( this); }
    const DHandler              *Handler( void) const { return static_cast< const DHandler*>( this); }
    
    DLinkPtr    m_Head; 
    
public:  
    Cv_DList(  DLinkPtr h = DNIL())
        : m_Head( h) 
    {}

    bool        IsEmpty( void) { return m_Head == DNIL(); }
    DLinkPtr    Head( void) const { return m_Head; }
    void        SetHead( DLinkPtr dl) { m_Head = dl; } 
    
    DLinkPtr    Tail( void) const  { return  ( m_Head == DNIL()) ? DNIL() : Handler()->Prev( m_Head); } 
    bool        IsSingle( void) const { return Head() && ( Tail() == Head()); } 
    
    DLinkPtr    Pop( void) { return Head() ? Remove( Head()) : DNIL(); }
     

    void    Append( DLinkPtr dl)
    {
        DHandler    *hndlr = Handler();
        DLinkPtr    tl = Tail();
        DLinkPtr    tlNx = tl ? hndlr->Next( tl) : ( Circ ? dl : DNIL()); 
        hndlr->SetNext( dl,  tlNx);
        hndlr->SetPrev( dl, tl);
        if ( tl != DNIL())
            hndlr->SetNext( tl, dl);
        else        
            m_Head = dl;   
        hndlr->SetPrev( m_Head, dl);               // make dl the tail 
        CV_SANITY_ASSERT( TestSanity())
        return;
    }
    
    void    Prepend( DLinkPtr dl)
    {
        DHandler    *hndlr = Handler();
        DLinkPtr    tl = Tail();
        if ( tl != DNIL())
        {
            hndlr->SetPrev( m_Head,  dl);
            hndlr->SetNext( dl, m_Head);
            hndlr->SetPrev( dl, tl);
        }
        else
        {
            CV_SANITY_ASSERT( m_Head == DNIL())
            hndlr->SetNext( dl, DNIL());
            hndlr->SetPrev( dl, dl);                // make dl the tail
        }
        m_Head = dl;
        return;
    }
     
    DLinkPtr    Remove( DLinkPtr dl)
    { 
        DHandler    *hndlr = Handler();
        if ( IsSingle())
        { 
            m_Head = DNIL();
            CV_SANITY_ASSERT( TestSanity())
            return hndlr->DoInit( dl);
        }
        
        DLinkPtr    tl = Tail();
        if (( dl != tl) && (dl != m_Head))
        {
            hndlr->SetPrev( hndlr->Next( dl), hndlr->Prev( dl));
            hndlr->SetNext( hndlr->Prev( dl),  hndlr->Next( dl));
            CV_SANITY_ASSERT( TestSanity())
            return hndlr->DoInit( dl);
        }
        if ( dl == tl)
        {
            CV_SANITY_ASSERT( TestSanity())
            hndlr->SetNext( hndlr->Prev( dl), DNIL());
            hndlr->SetPrev( m_Head, hndlr->Prev( dl));     // make dl the tail
            CV_SANITY_ASSERT( TestSanity())
        }
        else if ( dl == m_Head)
        {
            m_Head = hndlr->Next( m_Head);
            hndlr->SetPrev( m_Head, tl);
            CV_SANITY_ASSERT( TestSanity())
        }
        return hndlr->DoInit( dl);
    }

    void    InsertAfter( DLinkPtr ref, DLinkPtr dl)
    {
        DHandler    *hndlr = Handler();
        DLinkPtr    tl = Tail();
        if ( ref == tl)
        {
            Append(dl);
            return;
        }
        hndlr->SetNext( dl,  hndlr->Next( ref));
        hndlr->SetPrev( hndlr->Next( dl), dl);
        hndlr->SetPrev( dl, ref);
        hndlr->SetNext( ref, dl);
        return;
    }
 

    void    InsertBefore( DLinkPtr ref, DLinkPtr dl)
    {
        DHandler    *hndlr = Handler();
        if ( ref == m_Head)
        {
            Prepend(dl);
            return;
        }
        hndlr->SetPrev( dl, hndlr->Prev( ref));
        hndlr->SetNext( hndlr->Prev( dl),  dl);
        hndlr->SetNext( dl,  ref);
        hndlr->SetPrev( ref, dl);
        return;
    }
 

    void    Transfer( Cv_DList  *dlist)
    {
        DHandler    *hndlr = Handler();
        DLinkPtr    tl = Tail();
        DLinkPtr    dh = dlist->Head();
        DLinkPtr    dt = dlist->Tail();
        if ( tl != DNIL())
        {
            hndlr->SetNext( tl, dh);
            hndlr->SetPrev( dh,  tl);
        }
        else
        {
            //CV_SANITY_ASSERT(m_Head == DNIL())
            m_Head = dh;
        }
        hndlr->SetPrev( m_Head,  dt); // make dt the tail
        dlist->m_Head = DNIL();
        return;
    }
    
    DLinkPtr    HeadLink( DLinkPtr dl) 
    {
        DHandler    *hndlr = Handler();
        DLinkPtr    *anc = dl;
        for ( DLinkPtr pr = hndlr->Prev( dl); pr != DNIL(); pr = hndlr->Prev( pr))              // prev field s always filled
        {
            if ( hndlr->Next( pr) == DNIL())                                                        // prev of head tail which has NULL next.
                return dl;
            if ( pr == anc)                                                                             // cirular list
                return pr;
            dl = pr;
        } 
        return DNIL();
    }

    DLinkPtr    TailLink( DLinkPtr dl) 
    { 
        DHandler    *hndlr = Handler();
        while ( hndlr->Next( dl) != DNIL())
            dl =  hndlr->Next( dl);
        return dl;
    }
    
    uint32_t    Size( void) const
    {
        const DHandler      *hndlr = Handler();
        uint32_t            sz = 0;
        for ( DLinkPtr dl = Head(); dl != DNIL() ;dl =  hndlr->Next( dl) )
            ++sz;
        return sz;
    }
    
    uint32_t    SizeR( void) const
    {
        DLinkPtr            tail = Tail(); 
        if ( !tail)
            return 0;
        
        const DHandler      *hndlr = Handler();
        uint32_t            sz = 0;
        DLinkPtr            dl = tail; 
        do {
            dl =  hndlr->Prev( dl);
            ++sz;
        } while ( tail != dl);
        return sz;
    }
    
    bool    TestSanity( void)
    {
        return Size() == SizeR();
    }
}; 

//_____________________________________________________________________________________________________________________________ 

template < typename DLink, bool Circ = false>
class Cv_DLinkList : public Cv_DList< DLink *, Cv_DLinkList< DLink, Circ>, Circ >
{
    typedef Cv_DList< DLink *, Cv_DLinkList< DLink, Circ>, Circ >       Base;
    
public:  
    static constexpr DLink   *NIL( void) { return  ( DLink *) NULL; }
    
    DLink       *Prev( const DLink *dlink) const { return dlink->m_Prev; }
    DLink       *Next( const DLink *dlink) const { return dlink->m_Next; }

    void        SetPrev( DLink *dlink, DLink *d) { dlink->m_Prev = d; }
    void        SetNext( DLink *dlink, DLink *d) { dlink->m_Next = d; }
    
    DLink       *DoInit( DLink *dl)
    {
        SetPrev( dl, NIL());
        SetNext( dl, NIL());
        return dl;
    } 

    Cv_DLinkList(  DLink *h = NULL)
        : Base( h) 
    {}
  
};

//_____________________________________________________________________________________________________________________________

template < uint32_t Mx, typename Stor>
class Cv_ISeries :  public Cv_DList< Stor, Cv_ISeries< Mx, Stor> > 
{
    uint32_t                    m_Sz;
    std::tuple< Stor, Stor>     m_IndTabl[ Mx];
    
public:
    Cv_ISeries( void)
        : m_Sz( 0)
    {}
    
    static constexpr Stor    NIL( void) { return  0; }
    
    uint32_t    IndexSize( void) const { return m_Sz; }
    Stor        Prev( Stor dlink) const { return std::get< 0>( m_IndTabl[ dlink]); }
    Stor        Next( Stor dlink) const { return std::get< 1>( m_IndTabl[ dlink]); }

    void        SetPrev( Stor dlink, Stor d) { std::get< 0>( m_IndTabl[ dlink]) = d; }
    void        SetNext( Stor dlink, Stor d) { std::get< 1>( m_IndTabl[ dlink]) = d; }
    
    Stor        DoInit( Stor dl)
    {
        std::get< 0>( m_IndTabl[ dl]) = std::get< 1>( m_IndTabl[ dl]) = 0;
        return dl;
    }
    
    void        PushIndex( Stor spurt) { this->Append( spurt); ++m_Sz;    CV_SANITY_ASSERT( TestSanity())  }
    void        RemoveIndex( Stor spurt) { this->Remove( spurt); --m_Sz;   CV_SANITY_ASSERT( TestSanity())  } 
      
    void        PopHeadTo( Stor spurtId, uint32_t sz)
    {
        Stor    headId = this->Head();
        if ( spurtId == headId)
            return;
        m_Sz -= sz;
        this->SetHead( spurtId);
        if ( spurtId == NIL())
            return;
        this->SetPrev( spurtId, Prev( headId)); 
        CV_SANITY_ASSERT( TestSanity())
    }
    
};


//_____________________________________________________________________________________________________________________________


 
