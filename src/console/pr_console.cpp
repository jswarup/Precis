//--------------------------------- pr_console.cpp --------------------------------------------

#include <iostream>

#include    "cove/barn/cv_include.h"
#include    "cove/snip/cv_cmdexec.h" 
#include 	"cove/barn/cv_aid.h"

//_____________________________________________________________________________________________________________________________
 
CV_CMD_DECLARE( Sg_TimbreCmdProcessor)
CV_CMD_DECLARE( Sg_XmlCmdProcessor)

//_____________________________________________________________________________________________________________________________
 
int main( int argc, char *argv[])
{
    std::string 	    progName = Cv_Aid::Basename( argv[ 0]);
    std::stringstream   cmdLine;

    for ( int i = 1; i < argc; ++i)
    {
        cmdLine <<  argv[i];
        cmdLine << " ";
    } 
    return Cv_AppStack::Main( cmdLine);
}

//_____________________________________________________________________________________________________________________________

