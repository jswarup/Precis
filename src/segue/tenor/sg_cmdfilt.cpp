// sg_cmdfilt.cpp ____________________________________________________________________________________________________________ 


#include    "segue/tenor/sg_include.h" 
#include    "cove/snip/cv_cmdexec.h"
#include    "segue/timbre/sg_chset.h" 

//_____________________________________________________________________________________________________________________________ 
 


int main3( void)
{ 
    Sg_ChSet    filt;
    filt.Set( 'a', true);
    filt.Set( 'b', true);

    char    chr = 'a';
    std::cin >> chr;
    
    bool    flg = filt.Get( chr);
    std::cout << int( flg);
    return 0;
}