//  sg_filter.h ___________________________________________________________________________________________________________________
#pragma once

#include    "cove/barn/cv_cexpr.h"
#include 	"cove/barn/cv_aid.h"
#include    "segue/timbre/sg_chset.h"
#include 	"cove/silo/cv_crate.h" 

//_____________________________________________________________________________________________________________________________  

namespace Sg_RExp
{
struct      ChSetFilter;
struct      CharFilter; 
struct      Filter;

typedef Cv_Crate< ChSetFilter, CharFilter, Filter>   FilterCrate; 


//_____________________________________________________________________________________________________________________________ 
 
struct  Filter   : public Cv_CrateEntry 
{ 

public:
    Filter( void) 
    {} 

    std::string		GetName( void) const { return "Filter"; } 
    
};  

//_____________________________________________________________________________________________________________________________ 

struct     CharFilter : public Filter 
{ 
    uint8_t     m_Char;

    CharFilter( void)
        : m_Char( 0)
    {}

    std::string		GetName( void) const { return Cv_Aid::ToStr( "Char", GetId()); }
};

//_____________________________________________________________________________________________________________________________ 

struct     ChSetFilter : public Filter 
{ 
    Sg_ChSet     m_ChSet;

    ChSetFilter( void)
        : m_ChSet()
    {}

    std::string		GetName( void) const { return Cv_Aid::ToStr( "ChSet", GetId()); } 
};

//_____________________________________________________________________________________________________________________________ 

struct FilterRepos : public Cv_CrateRepos< FilterCrate>                                   
{ 
    Id  Locate( const Sg_ChSet &chSet);
};

};

//_____________________________________________________________________________________________________________________________  

