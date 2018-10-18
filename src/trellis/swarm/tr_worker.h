// tr_worker.h _____________________________________________________________________________________________________________
#pragma once

#include    "trellis/tenor/tr_include.h"
#include    "trellis/stalks/tr_atomic.h"

//_____________________________________________________________________________________________________________________________

class Tr_WorkShop;

//_____________________________________________________________________________________________________________________________

class Tr_WorkRunner
{
public:
    virtual bool        DoInit( Tr_WorkShop *ws = NULL) = 0;
    virtual bool       DoRun( void) = 0;
};

//_____________________________________________________________________________________________________________________________

class Tr_WorkShop : public  Tr_WorkRunner
{
    std::vector< Tr_WorkRunner*>    m_Agents;

public:

    Tr_WorkShop( void)
    {}
        
    bool    DoInit( Tr_WorkShop *);

    bool    DoRun( void) ;  
};

//_____________________________________________________________________________________________________________________________

template < class Agent>
class Tr_WorkAgent : public Tr_WorkRunner
{
public:
    bool    IsProceed( void) 
    {
        return true;
    }

    bool    DoRun( void)  
    {
        return true;        
    }
};

//_____________________________________________________________________________________________________________________________
