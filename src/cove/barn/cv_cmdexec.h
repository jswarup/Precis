// cv_cmdexec.h ______________________________________________________________________________________________________________
#pragma once

#include    "cove/silo/cv_stack.h"
#include    "cove/silo/cv_crate.h"

//_____________________________________________________________________________________________________________________________

struct Cv_CmdOption 
{
    const char      *m_Opt;
    const char      *m_ArgsDesc;
    const char      *m_Doc;

 
};

//_____________________________________________________________________________________________________________________________

class Cv_CmdExecutor
{  
public: 
    Cv_CmdExecutor( void) 
    {}    
    virtual bool            ProcessProgArgs( std::istream &cmdLine)  {  return true; }
    virtual bool            ParseArg( const std::string &key, const std::string &arg) {  return true;  } 
    virtual int             ParseExecute( std::istream &cmdLine) { return Execute(); }
    virtual int             Execute( void) { return -1; }
};


//-------------------------------------------------------------------------------------------------

class Cv_CmdRunner : public Cv_StackVar< Cv_CmdRunner> 
{
protected:
    const char                  *m_Name;
    const char                  *m_Doc;
    Cv_CmdOption				*m_Options;

    Cv_CmdRunner( void) : m_Options( NULL) {}

public:
    Cv_CmdRunner( const char *name, const char *doc, Cv_CmdOption *optArr);

    const char              *GetName(  void) const { return m_Name; }
    void                    SetName(  const char *nm) { m_Name = nm; } 
    bool                    ParseOption( Cv_CmdExecutor *exe, const std::string &key, std::istream &cmdStrm); 
    void                    PrintDoc( std::ostream &, int lev);

    virtual Cv_CmdExecutor  *CreateExecutor( void) = 0;
 
    int                     ParseRun( std::istream &cmdStrm);
};

//_____________________________________________________________________________________________________________________________
 
template < typename Executor >
struct Cv_CmdProcessor : public Cv_CmdRunner
{
    virtual Cv_CmdExecutor          *CreateExecutor( void) { return new Executor(); }	

    Cv_CmdProcessor(  const char *name, const char *doc, Cv_CmdOption *optArr)
        : Cv_CmdRunner( name, doc, optArr)
    {}   

};  

//_____________________________________________________________________________________________________________________________

class Cv_AppStack : public Cv_Stack< Cv_CmdRunner>
{
public:
    static Cv_AppStack    s_Instance;

    Cv_CmdRunner        *FindRunner( const char *name);  
};

//_____________________________________________________________________________________________________________________________

class Cv_MainExecutor :   public Cv_CmdExecutor
{ 
    std::string         m_RunnerName;

public: 
    Cv_MainExecutor( void) 
    {}

    bool            ParseArg( const std::string &key, const std::string &arg);
    bool            ProcessProgArgs( std::istream &cmdLine);
    int             ParseExecute( std::istream &cmdLine);  
    static int      Run( std::istream &cmdStrm);
};

//_____________________________________________________________________________________________________________________________

struct Cv_LinkHook
{
    void    *m_FuncPtr;
    Cv_LinkHook( void *funPtr)
        : m_FuncPtr( funPtr)
    {}
};

//_____________________________________________________________________________________________________________________________

#define CV_CMD_DECLARE( X)                                                                  \
    void            X##CMD( void);								                            \
    Cv_LinkHook     cv_##X##LinkHook( ( void *) &X##CMD);                                   \


#define CV_CMD_DEFINE( X, Name, Doc, Opts)                                                  \
                                                                                            \
    Cv_CmdProcessor< X>     s_Instance##X( Name, Doc, Opts);                                \
    void X##CMD( void)								                                        \
    {																					    \
        return ;											                                \
    }

//_____________________________________________________________________________________________________________________________

