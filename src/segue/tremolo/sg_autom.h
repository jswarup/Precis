//  sg_autom.h ___________________________________________________________________________________________________________________
#pragma once

#include    "cove/barn/cv_cexpr.h"
#include 	"cove/barn/cv_aid.h"
#include 	"cove/barn/cv_ptrslot.h"
#include    "segue/timbre/sg_timbreparser.h"

//_____________________________________________________________________________________________________________________________  

namespace Sg_Timbre
{
  
struct     AutomSpurCnstr; 

//_____________________________________________________________________________________________________________________________ 

struct  AutomCnstr   : public Cv_CrateEntry 
{ 
public:
    AutomCnstr( void)  
    {}

};

//_____________________________________________________________________________________________________________________________ 

typedef Cv_Crate<  AutomSpurCnstr, AutomCnstr>      AutomCnstrCrate;  
typedef Cv_CrateRepos< AutomCnstrCrate>             AutomCnstrRepos;
typedef AutomCnstrCrate::Id                         AutomCnstrId;

//_____________________________________________________________________________________________________________________________ 

struct  AutomSpurCnstr   : public AutomCnstr 
{ 
    std::vector< Sg_ChSet>          m_ChSets;
    std::vector< AutomCnstrId>      m_Dests;
    std::set< uint32_t>             m_EpsDestIds; 
    std::set< uint32_t>             m_EpsFromSources;
public:
    AutomSpurCnstr( void)  
    {}
};

//_____________________________________________________________________________________________________________________________  
 

//_____________________________________________________________________________________________________________________________  
};

//_____________________________________________________________________________________________________________________________  

