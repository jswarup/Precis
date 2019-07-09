// sg_blossomatelier.h ___________________________________________________________________________________________________________________
#pragma once


#include    "segue/epigraph/sg_bitset.h"
#include    "segue/epigraph/sg_partition.h"
#include    "segue/colophon/sg_dfastate.h"

//_____________________________________________________________________________________________________________________________

using namespace Sg_RExp;
 

//_____________________________________________________________________________________________________________________________

struct Sg_DfaBaseBlossomAtelier
{   
    FsaDfaRepos::Blossom    m_DfaBlossom;
    DistribRepos::Blossom   m_Distribs;
    FsaRepos::Blossom       *m_States;

    Sg_DfaBaseBlossomAtelier( void *dfaImage)
        : m_DfaBlossom( dfaImage), m_Distribs( m_DfaBlossom.Distribs())
    {
        m_States = m_DfaBlossom.States();
         
    } 

    DistribCrate::Var       FetchDistib( FsaDfaState *dfaState) { return m_Distribs.ToVar( dfaState->DistribId()); }

    uint8_t                 ByteCode( uint8_t chr ) const  { return m_Distribs.Base()->Image( chr); }

    Sg_ChSet                ChSet( uint8_t byteCode) const { return m_Distribs.Base()->ChSet( byteCode); }

    Sg_ChSet                ChSetFromImage( const DistribCrate::Var &distrib, uint8_t imgCode) const  
    { 
        Sg_Partition    *baseDist = m_Distribs.Base();
        return distrib( [imgCode, baseDist]( auto dist) { return baseDist->XForm( dist->ChSet( imgCode)); }); 
    }

    bool        WriteDot( Cv_DotStream &strm)
    {
        for ( uint32_t i = 1; i < m_States->Size(); ++i)
        {  
            FsaCrate::Var       si = m_States->VarAt( i);
            if ( ! si.GetEntry())
                continue;
            bool    t = true;

            FsaRepos::Id        id( i, si.GetType());
            si( [this, id, &strm]( auto k)  { 
                k->WriteDot( id, this, strm); 
                });
        }
        return true;
    }
};

//_____________________________________________________________________________________________________________________________

struct Sg_DfaBlossomAtelier : public Sg_DfaBaseBlossomAtelier
{    
    FsaCrate::Var           m_Root;
    DistribCrate::Var       m_RootDistrib;

    Sg_DfaBlossomAtelier( void *dfaImage)
        : Sg_DfaBaseBlossomAtelier( dfaImage) 
    {
        m_Root = m_States->ToVar( m_DfaBlossom.RootId());
        if ( m_Root.GetType() == FsaCrate::template TypeOf< FsaDfaState>())
            m_RootDistrib = m_Distribs.ToVar(  static_cast< FsaDfaState *>( m_Root.GetEntry())->DistribId());
        m_DfaBlossom.SauteStates();
    }  

    const FsaCrate::Var     &RootState( void) {  return m_Root; }

    FsaCrate::Var           VarFromId( const FsaDfaRepos::Id &id) const { return m_States->VarId( id ); }

    DistribCrate::Var       FetchDistib( FsaDfaState *dfaState) { return  m_Distribs.VarId( dfaState->DistribId()); }

    FsaDfaRepos::Id         DfaTransition( FsaDfaState  *dfaState, const DistribCrate::Var &dVar, uint8_t chrId)
    {
        uint8_t    index = dVar( [ chrId]( auto k) { return k->Image( chrId); });
        if ( index == dfaState->DeadInd())
            return FsaDfaRepos::Id();
        return  dfaState->DestAt( index);
    }
 

    FsaDfaRepos::Id         DfaXTransition( FsaDfaXByteState  *dfaState, uint8_t chrId)
    { 
        Cv_Seq< uint8_t>            bytes = dfaState->Bytes();
        Cv_Seq< FsaDfaRepos::Id>    dests = dfaState->Dests(); 
        for ( uint32_t i = 0; i < bytes.Size(); ++i)
            if ( chrId == bytes[ i])
                return  dests[ i];
        return FsaDfaRepos::Id();
    } 

    FsaDfaRepos::Id         Advance( const FsaCrate::Var &state, uint8_t chrId)
    {
        FsaDfaRepos::Id   nxState;
        switch ( state.GetType())
        {
            case FsaCrate::template TypeOf< FsaDfaState>() :
            {
                FsaDfaState             *dfaState = static_cast< FsaDfaState *>( state.GetEntry());
                nxState =   DfaTransition( dfaState, FetchDistib( dfaState), chrId);
                break;
            }
            case FsaCrate::template TypeOf< FsaDfaByteState< 1>>() :
            {
                FsaDfaByteState< 1>     *dfaState = static_cast< FsaDfaByteState< 1> *>( state.GetEntry());
                nxState = dfaState->Eval( chrId);
                break;
            }
            case FsaCrate::template TypeOf< FsaDfaByteState< 2>>() :
            {
                FsaDfaByteState< 2>     *dfaState = static_cast< FsaDfaByteState< 2> *>( state.GetEntry());
                nxState = dfaState->Eval( chrId);
                break;
            }
            case FsaCrate::template TypeOf< FsaDfaByteState< 3>>() :
            {
                FsaDfaByteState< 3>     *dfaState = static_cast< FsaDfaByteState< 3> *>( state.GetEntry());
                nxState = dfaState->Eval( chrId);
                break;
            }
            case FsaCrate::template TypeOf< FsaDfaByteState< 4>>() :
            {
                FsaDfaByteState< 4>     *dfaState = static_cast< FsaDfaByteState< 4> *>( state.GetEntry());
                nxState = dfaState->Eval( chrId);
                break;
            }/*
			case FsaCrate::template TypeOf< FsaDfaXByteState>() :
            {
                FsaDfaXByteState     *dfaState = static_cast< FsaDfaXByteState *>( state.GetEntry());
                nxState = DfaXTransition( dfaState, chrId);
                break;
            }*/
            default :
                break;
        }
        return nxState;
    }
    
    FsaDfaRepos::Id         AdvanceRoot(  uint8_t chrId)
    {
        FsaDfaRepos::Id   nxState;
        switch ( m_Root.GetType())
        {
            case FsaCrate::template TypeOf< FsaDfaState>() :
            {
                FsaDfaState             *dfaState = static_cast< FsaDfaState *>( m_Root.GetEntry());
                nxState =   DfaTransition( dfaState, m_RootDistrib, chrId);
                break;
            }
            case FsaCrate::template TypeOf< FsaDfaByteState< 1>>() :
            {
                FsaDfaByteState< 1>     *dfaState = static_cast< FsaDfaByteState< 1> *>( m_Root.GetEntry());
                nxState = dfaState->Eval( chrId);
                break;
            }
            case FsaCrate::template TypeOf< FsaDfaByteState< 2>>() :
            {
                FsaDfaByteState< 2>     *dfaState = static_cast< FsaDfaByteState< 2> *>( m_Root.GetEntry());
                nxState = dfaState->Eval( chrId);
                break;
            }
			case FsaCrate::template TypeOf< FsaDfaByteState< 3>>() :
            {
                FsaDfaByteState< 3>     *dfaState = static_cast< FsaDfaByteState< 3> *>( m_Root.GetEntry());
                nxState = dfaState->Eval( chrId);
                break;
            }
            case FsaCrate::template TypeOf< FsaDfaByteState< 4>>() :
            {
                FsaDfaByteState< 4>     *dfaState = static_cast< FsaDfaByteState< 4> *>( m_Root.GetEntry());
                nxState = dfaState->Eval( chrId);
                break;
            }/*
			case FsaCrate::template TypeOf< FsaDfaXByteState>() :
            {
                FsaDfaXByteState     *dfaState = static_cast< FsaDfaXByteState *>( m_Root.GetEntry());
                nxState = DfaXTransition( dfaState, chrId);
                break;
            }*/
            default :
                break;
        }
        return nxState;
    }

    std::array< FsaDfaRepos::Id, 8>   Advance( const FsaCrate::Var &state, uint64_t chrOct)
    {
        uint8_t                         *chrIds = ( uint8_t *) &chrOct;
        std::array< FsaDfaRepos::Id, 8>   nxStates;
        switch ( state.GetType())
        {
            case FsaCrate::template TypeOf< FsaDfaState>() :
            {
                FsaDfaState             *dfaState = static_cast< FsaDfaState *>( state.GetEntry());
                DistribCrate::Var       dVar = FetchDistib( dfaState);

                Cv_For< 8>::RunAll( [ this, dfaState, &dVar, chrIds, &nxStates]( uint32_t ind) {
                    nxStates[ ind] =   DfaTransition( dfaState, dVar, chrIds[ ind]);
                });
                break;
            }
            case FsaCrate::template TypeOf< FsaDfaByteState< 1>>() :
            {
                FsaDfaByteState< 1>     *dfaState = static_cast< FsaDfaByteState< 1> *>( state.GetEntry());
                Cv_For< 8>::RunAll( [ this, dfaState, chrIds, &nxStates]( uint32_t ind) {
                    nxStates[ ind] = dfaState->Eval( chrIds[ ind]);
                });
                break;
            }
            case FsaCrate::template TypeOf< FsaDfaByteState< 2>>() :
            {
                FsaDfaByteState< 2>     *dfaState = static_cast< FsaDfaByteState< 2> *>( state.GetEntry());
                Cv_For< 8>::RunAll( [ this, dfaState, chrIds, &nxStates]( uint32_t ind) {
                    nxStates[ ind] = dfaState->Eval( chrIds[ ind]);
                });
                break;
            }
			case FsaCrate::template TypeOf< FsaDfaXByteState>() :
            {
                FsaDfaXByteState     *dfaState = static_cast< FsaDfaXByteState *>( state.GetEntry());
                Cv_For< 8>::RunAll( [ this, dfaState, chrIds, &nxStates]( uint32_t ind) {
                    nxStates[ ind] =   DfaXTransition(  dfaState, chrIds[ ind]);
                });
                break;
            }
            default :
                break;
        }
        return nxStates;
    }
 
    
 
};

  

//_____________________________________________________________________________________________________________________________
