//  cv_repos.h ___________________________________________________________________________________________________________________
#pragma once
 

template < class X>
class Cv_Repos; 

//_____________________________________________________________________________________________________________________________

class  Cv_ReposEntry 
{
public: 
    uint32_t                         m_Id;

public:
    Cv_ReposEntry( uint32_t id = CV_UINT32_MAX)
        :  m_Id( id)
    {}
 

    const char      *GetName( void) const { return ""; }

    uint32_t        GetId( void) const { return m_Id; }
    void            SetId( uint32_t k) { m_Id = k; }
};  

//_____________________________________________________________________________________________________________________________ 

template < class Entry>
class Cv_Repos 
{
public:
    std::vector< Entry *>     m_Elems;


public:
    
    Cv_Repos( void) { m_Elems.push_back( NULL); }
    
    ~Cv_Repos( void) { Cleanup(); }
    
    void Cleanup( void)
    {
        for ( uint32_t i = 0; i < Size(); ++i)
        {
            Entry     *si = m_Elems[ i];
            m_Elems[ i] = nullptr;
            if ( si && (si != ( Entry *) -1))
                delete si;
        }
        m_Elems = std::vector< Entry *>();
        m_Elems.push_back( nullptr); 
    }
 

    uint32_t    Size( void) const { return uint32_t( m_Elems.size()); }

    void        EnsurePos( uint32_t k)
    {
        if ( k >= Size())
            m_Elems.resize( k +1);
    } 

    void        Resize( uint32_t sz)
    {
        m_Elems.resize( sz);
    }
  
template < class Compare>
    Entry        *Locate( Entry *y, Compare cmp) const
    {
        typename std::vector< Entry *>::const_iterator   cIt = std::lower_bound( m_Elems.begin(), m_Elems.end(), y, cmp);
        if (( cIt != m_Elems.end()) &&  !cmp( *cIt, y))
            return *cIt;
        return  NULL;
    }
 
    void        Destroy( uint32_t k)
    {
        CV_ERROR_ASSERT( k < m_Elems.size() && m_Elems[ k])
        Entry       *x = m_Elems[ k]; 
        m_Elems[ k] = NULL;
        delete x;
        return;
    }
     
    bool        IsAt( uint32_t k) const
    {
        if (( k >= Size()) || !m_Elems[ k])
            return false;
        return true;
    }

    Entry         *At( uint32_t i) const 
    {
        return  m_Elems[ i];
    }
    
template < class Y>
    Y               *At( uint32_t i) const 
    {
        return  static_cast< Y *>( m_Elems[ i]);
    }
 
    void        SetAt( uint32_t k, Entry *x) 
    {
        EnsurePos( k); 
        if ( m_Elems[ k])
            delete m_Elems[ k];
        m_Elems[ k] = x;
        x->SetId( k);
        return;
    }
 
    

    void    Swap( Cv_Repos< Entry> *tc) {   m_Elems.swap( tc->m_Elems);} 

template < class Y>
    uint32_t    NumElem( uint32_t start = 0) const 
    {
        uint32_t    sz = 0;
        for ( uint32_t i = start; i < Size(); ++i)
            if ( At< Y>( i))
                ++sz;
        return sz;
    }
 
    // idMap hold the new destination Ids : idmap[] == 0 => They get assigned NULL

    void        Reorder( const std::vector< uint32_t> &idMap, uint32_t beg)
    {
        std::vector< Entry *>     elems( m_Elems.begin() +beg,  m_Elems.end());
        for ( uint32_t i = beg; i < m_Elems.size(); ++i)
        {
            Entry           *x = elems[ i -beg];
            uint32_t        k = idMap[ i -beg];
            if ( !x || !k)
            {
                m_Elems[ k] = NULL;
                continue;
            }
            m_Elems[ k] = x;
            x->SetId( k); 
        }
        return;
    }
    
 template < class BinOp>
    std::vector< uint32_t>    SortElems( const BinOp &binFn)
    {    
        std::vector< Entry *>          ecList;
        for ( uint32_t i = 1; i <  Size(); ++i)
        {
            Entry           *ec =  At( i);
            if ( ec)
                ecList.push_back( ec);
        }
        std::sort( ecList.begin(), ecList.end(), binFn);

        std::vector< uint32_t>          ecIdList( Size(), 0);
        for ( uint32_t i = 0; i <  ecList.size(); ++i)  
            ecIdList[  ecList[ i]->GetId()]  = i +1;
    
        Reorder( ecIdList, 0);
        Resize( ecList.size() +1);
        return ecIdList;
    }

    //_____________________________________________________________________________________________________________________________ 
    
    class                   Traversor;

template < class Anchor, typename Method>
    class TraverserStwd
    {
        Traversor           *m_Context;
        Anchor              *m_Traversor;
        Method              m_Method;

    public:
        TraverserStwd( Traversor *ctxt, Anchor *traversor, Method method)
            :   m_Context( ctxt), m_Traversor( traversor), m_Method( method)
        {}

    template < class Arg>
        bool    CallMethod( uint32_t stateId, bool entryFlg, Arg *arg)
        {
            return (m_Traversor->*m_Method)( stateId, m_Context, entryFlg, arg);
        }

        bool    CallMethod( uint32_t stateId, bool entryFlg, void *arg)
        {
            return (m_Traversor->*m_Method)( stateId, m_Context, entryFlg);
        }

    template < class Arg>
        void  DoDepthFirst( uint32_t spawnId, Arg *arg)
        {
            m_Context->Rejig();
            if ( m_Context->IsExited( spawnId))
                return;
                
            m_Context->DoQueState( spawnId);

            uint32_t    qSize = 0;
            while ( qSize = m_Context->SzQue())
            {
                uint32_t        stateId = m_Context->Peek();
                if ( m_Context->IsExited( stateId))     
                {
                    if ( qSize == m_Context->SzQue())
                        m_Context->Pop();    
                    continue;
                }

                bool            entryFlg = !m_Context->IsEntered( stateId);        // if it has not been entered then it is an entry
                if ( entryFlg)
                {
                    bool    res = CallMethod( stateId, true, arg);
                    m_Context->Rejig();
                    if ( res)                                           
                    {
                        m_Context->MarkEntered( stateId);
                    }
                    else // mark it processed
                    {
                        if ( qSize == m_Context->SzQue())
                            m_Context->Pop();
                        m_Context->MarkEntered( stateId);
                        m_Context->MarkExited( stateId);
                    }
                }
                else
                {   
                    bool        qFlg = m_Context->DoQueUnenteredDests( stateId);
                    
                    if ( qFlg)            
                        continue;   
                    bool    res = CallMethod( stateId, false, arg);
                    m_Context->Rejig();
                    if ( res)
                    {
                        if ( qSize == m_Context->SzQue())
                            m_Context->Pop();
                        m_Context->MarkExited( stateId);         // if it was an exit => processing done
                    }
                    else
                    {
                       CV_ERROR_ASSERT( m_Context->Peek() != stateId)           // else will go in infinite loop
                    }
                }
                
            }
            return;
        }
    };
    
    class Traversor
    {
        typedef typename Entry::Repos       XRepos;

    protected:
        XRepos                           *m_Repos;
        std::vector< uint32_t>          m_StateQue;
        std::vector< bool>              m_StateStatus;
        
    public:
        Traversor( XRepos *repos)
            :  m_Repos( repos), m_StateStatus( 2 * repos->Size(), false)
        {}

        XRepos      *Repos( void)  const {  return m_Repos; }

        bool        IsEntered( uint32_t stateId)  const { return  m_StateStatus[ 2 * stateId]; }

        void        MarkEntered( uint32_t stateId) { m_StateStatus[ 2 * stateId] = true; }
        
        bool        IsExited( uint32_t stateId)  const {  return m_StateStatus[ 2 * stateId +1]; }

        void        MarkExited( uint32_t stateId) { m_StateStatus[ 2 * stateId +1] = true; }
        
        void        UnmarkExited( uint32_t stateId) {  m_StateStatus[ 2 * stateId +1] = false; }
        
        void        Rejig( void)
        {
            if (!( (2 * m_Repos->Size()) < m_StateStatus.size()))
                m_StateStatus.resize( 2 * m_Repos->Size(), false);
        }

        uint32_t    SzQue( void)  const {  return m_StateQue.size(); }

        uint32_t    Root( void)  const {  return m_StateQue.front(); }
    
        uint32_t    Peek( void)  const {  return m_StateQue.back(); }
        
        uint32_t    Pop( void)  
        { 
            uint32_t        stateId = m_StateQue.back();
            m_StateQue.pop_back();
            return stateId;
        }
            
        void        DoQueState( uint32_t stateId)  
        {
            m_StateQue.push_back(stateId);
            UnmarkExited( stateId); 
        }

    template < class Iterator>
        void        DoQueStates( Iterator b, Iterator e)   { Rejig(); for ( Iterator it = b; it != e; ++it) DoQueState( *it); }
        
        bool        DoQueUnenteredDests( uint32_t stateId)                              // test if children been entered : if not push them in..
        {
            Rejig();

            uint32_t        szDest = m_Repos->SzDest( stateId);
            bool            qFlg = false;
            for ( uint32_t i = 0; i < szDest; ++i)
            {
                uint32_t    dest = m_Repos->DestAt( stateId, i);
                if (( stateId != dest) && !IsEntered( dest) && ( qFlg = true))           // child need to have entered before we process further, if not que it for processing
                    DoQueState( dest);
            }
            return qFlg;
        }

    template < class Anchor, typename Method>
        void  DoDepthTraversal( uint32_t stateId, Anchor *traversor, Method method)
        {   TraverserStwd<  Anchor, Method>( this, traversor, method).DoDepthFirst( stateId, ( void *) NULL); }
        
          
    template < class Anchor, typename Method, typename Args>
        void  DoDepthTraversal( uint32_t stateId, Anchor *traversor, Method method, Args *args)
        {   TraverserStwd<  Anchor, Method>( this, traversor, method).DoDepthFirst( stateId, args); }
    };
};

//_____________________________________________________________________________________________________________________________ 
 