// cv_cmdexec.cpp _____________________________________________________________________________________________________________


#include    "cove/barn/cv_include.h"
#include    "cove/barn/cv_cmdexec.h"
#include    "cv_version.h"

//_____________________________________________________________________________________________________________________________

Cv_AppStack    Cv_AppStack::s_Instance;


///_____________________________________________________________________________________________________________________________ 

static Cv_CmdOption     s_MainOptions[] = 
{         
    { "-h",      0,  "help"},
    { "-V",      0,  "version"},
    { 0,         0,  0},
};

//_____________________________________________________________________________________________________________________________

Cv_CmdRunner::Cv_CmdRunner( const char *name, const char *doc, Cv_CmdOption *optArr)	
    : m_Name( name), m_Doc( doc ? doc : ""), m_Options( optArr)
{
    Cv_AppStack::s_Instance.Push( this);
}

//_____________________________________________________________________________________________________________________________

bool    Cv_CmdRunner::ParseOption( Cv_CmdExecutor *exe, const std::string &key, std::istream &cmdStrm)
{
    for ( Cv_CmdOption   *opt = m_Options; opt && opt->m_Opt; ++opt)
    {
        const char      *p = strchr( opt->m_Opt, '[');
        int64_t         n = p ? p -opt->m_Opt : strlen( opt->m_Opt);
        if (strncmp( opt->m_Opt, key.c_str(), n) == 0)
        {
            std::string     arg;
            if ( opt->m_ArgsDesc && (!( cmdStrm >> std::ws >> arg)))
                return false;
            return exe->ParseArg( key, arg);
        }
    }
    return false;
}

//_____________________________________________________________________________________________________________________________

void   Cv_CmdRunner::PrintDoc( std::ostream &ostr, int lev)
{
    std::string     margin;
    for ( int i = 0; i < lev; ++i)
        margin.append( "    ");

    ostr << margin << m_Name;
    if ( !m_Doc)
        ostr << "\t" << "\t;" << m_Doc;
    ostr << std::endl;
    if ( !m_Options)
        return;

    for ( Cv_CmdOption   *opt = m_Options; opt && opt->m_Opt; ++opt)
    {
        if ( opt->m_Doc && strcmp(opt->m_Doc, "HiddenOpt") == 0)
            continue;

        ostr << margin << "\t" << opt->m_Opt << "\t";
        if ( opt->m_ArgsDesc)
            ostr << opt->m_ArgsDesc;
        else
            ostr << "\t";
        ostr << "\t";
        if ( opt->m_Doc)
            ostr << opt->m_Doc;
        ostr << std::endl;
    }
    ostr << std::endl;
    return;
}

//_____________________________________________________________________________________________________________________________

int     Cv_CmdRunner::ParseRun( std::istream &cmdStrm) 
{    
    Cv_CmdExecutor      *exe = CreateExecutor();
    int                 retVal = -1;
    while (1)
    {
        int         kd = cmdStrm.peek();
        if ( kd == '-')
        {
            std::string             tok;
            if ( !(cmdStrm >> tok) || !ParseOption( exe, tok, cmdStrm))
            {
                PrintDoc( std::cout, 0);
                break;
            }
            cmdStrm >> std::ws;        // Skip white space, if any.
        }
        else
        {
            bool    rslt = exe->ProcessProgArgs( cmdStrm);
            if ( !rslt)
                PrintDoc( std::cout, 0);
            else
                retVal = exe->ParseExecute( cmdStrm);
            break;
        }
    }
    delete exe;
    return retVal;
}

//_____________________________________________________________________________________________________________________________

Cv_CmdRunner     *Cv_AppStack::FindRunner( const char *name)
{
    
    for ( Cv_CmdRunner *cndtr =  Top(); cndtr; cndtr = cndtr->GetBelow())
        if ( strcmp( cndtr->GetName(), name) == 0)
            return cndtr;
    return NULL;
}

//_____________________________________________________________________________________________________________________________

bool  Cv_MainExecutor::ParseArg( const std::string &key, const std::string &arg)
{
    if ( "-V" == key)
    { 
        std::cout << "Version: " << Cv_Version().m_GITSHA1 << '\n';
        return true;
    }
    if ( "-h" == key)
    { 
        for ( Cv_CmdRunner *cndtr =  Cv_AppStack::s_Instance.Top(); cndtr; cndtr = cndtr->GetBelow())
            cndtr->PrintDoc( std::cout, 0);
        return true;
    }

    return false;
}

//_____________________________________________________________________________________________________________________________

bool  Cv_MainExecutor::ProcessProgArgs( std::istream &cmdStrm)
{
    cmdStrm >> m_RunnerName >> std::ws;
    return !!m_RunnerName.size();
}
 
//_____________________________________________________________________________________________________________________________

int  Cv_MainExecutor::ParseExecute( std::istream &cmdLine)
{
    Cv_CmdRunner    *runner = Cv_AppStack::s_Instance.FindRunner( m_RunnerName.c_str()); 
    if ( !runner)
        return -1; 

    return runner->ParseRun( cmdLine);
}

//_____________________________________________________________________________________________________________________________

int Cv_MainExecutor::Run( std::istream &cmdStrm)
{
    Cv_CmdProcessor< Cv_MainExecutor> mainExec( "Main", "Main", s_MainOptions);
    return mainExec.ParseRun( cmdStrm);
}

//_____________________________________________________________________________________________________________________________
