// sg_reposatelier.h ___________________________________________________________________________________________________________________
#pragma once


#include    "segue/epigraph/sg_bitset.h"
#include    "segue/epigraph/sg_partition.h"
#include    "segue/colophon/sg_dfastate.h"

//_____________________________________________________________________________________________________________________________

using namespace Sg_RExp;

//_____________________________________________________________________________________________________________________________

struct Sg_DfaReposAtelier
{
    FsaDfaRepos             *m_DfaRepos;
    FsaCrate::Var           m_Root;
    DistribCrate::Var       m_RootDistrib;

    Sg_DfaReposAtelier( FsaDfaRepos *dfaRepos = NULL)
        : m_DfaRepos( dfaRepos)
    {
        m_Root = VarFromId( m_DfaRepos->m_RootId);
        if ( m_Root.GetType() == FsaCrate::template TypeOf< FsaDfaState>())
            m_RootDistrib = FetchDistib( static_cast< FsaDfaState *>( m_Root.GetEntry()));
    }

    FsaCrate::Var           VarFromId( const FsaDfaRepos::Id &id) const { return m_DfaRepos->ToVar( id ); }
    
    uint8_t                 ByteCode( uint8_t chr ) const  { return m_DfaRepos->m_DistribRepos.BaseDistrib().Image( chr); }

    Sg_ChSet                ChSet( uint8_t byteCode) const { return m_DfaRepos->m_DistribRepos.BaseDistrib().ChSet( byteCode); }

    Sg_ChSet                ChSetFromImage( const DistribCrate::Var &distrib, uint8_t imgCode) const 
    { 
        const Sg_Partition    *baseDist = &m_DfaRepos->m_DistribRepos.BaseDistrib();
        return distrib( [imgCode, baseDist]( auto dist) { 
            return baseDist->XForm( dist->ChSet( imgCode)); 
        }); 
    }

 
    const FsaCrate::Var     &RootState( void) {  return m_Root; }


    DistribCrate::Var       FetchDistib( FsaDfaState *dfaState) { return m_DfaRepos->m_DistribRepos.ToVar( dfaState->DistribId()); }

    FsaDfaRepos::Id         DfaTransition( FsaDfaState  *dfaState, const DistribCrate::Var &dVar, uint8_t chrId)
    {
        uint8_t             img = dVar( [ chrId]( auto k) { return k->Image( chrId); });
        return dfaState->DestAt( img);
    }
 
	FsaDfaRepos::Id          DfaXTransition( FsaState *state, uint8_t chrId)
    {
        FsaDfaXByteState     *dfaState = static_cast< FsaDfaXByteState *>( state);
    
        Cv_Seq< uint8_t>            bytes = dfaState->Bytes();
        Cv_Seq< FsaDfaRepos::Id>    dests = dfaState->Dests(); 
        for ( uint32_t i = 0; i < bytes.Size(); ++i)
            if ( chrId == bytes[ i])
                return  dests[ i];
        return FsaDfaRepos::Id();
    }

    FsaDfaRepos::Id         Advance( const FsaCrate::Var &state, uint8_t chrId)
    {
        uint32_t    types[] = { FsaCrate::template TypeOf< FsaDfaState>(), 
                              FsaCrate::template TypeOf< FsaDfaByteState< 1> >(), 
                              FsaCrate::template TypeOf< FsaDfaByteState< 2> >(),
                              FsaCrate::template TypeOf< FsaDfaByteState< 3> >(),
                              FsaCrate::template TypeOf< FsaDfaByteState< 4> >() }; 

        FsaDfaRepos::Id   nxState;
        switch ( state.GetType())
        {
            case FsaCrate::template TypeOf< FsaDfaState>() :
            {
                FsaDfaState             *dfaState = static_cast< FsaDfaState *>( state.GetEntry());
                nxState =   dfaState->Eval( this, chrId);
                break;
            }
            case FsaCrate::template TypeOf< FsaDfaByteState< 1> >() :
            {

                FsaDfaByteState< 1>     *dfaState = static_cast< FsaDfaByteState< 1>  *>( state.GetEntry());
                nxState = dfaState->Eval( this, chrId);
                break;
            } 
            case FsaCrate::template TypeOf< FsaDfaByteState< 2> >() :
            {

                FsaDfaByteState< 2>     *dfaState = static_cast< FsaDfaByteState< 2>  *>( state.GetEntry());
                nxState = dfaState->Eval( this, chrId);
                break;
            }
            case FsaCrate::template TypeOf< FsaDfaByteState< 3> >() :
            {

                FsaDfaByteState< 3>     *dfaState = static_cast< FsaDfaByteState< 3>  *>( state.GetEntry());
                nxState = dfaState->Eval( this, chrId);
                break;
            }
            case FsaCrate::template TypeOf< FsaDfaByteState< 4> >() :
            {

                FsaDfaByteState< 4>     *dfaState = static_cast< FsaDfaByteState< 4>  *>( state.GetEntry());
                nxState = dfaState->Eval( this, chrId);
                break;
            }/*
            case FsaCrate::template TypeOf< FsaDfaXByteState>() :
            {
                nxState = DfaXTransition( state.GetEntry(), chrId);
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
                FsaDfaByteState< 1>     *dfaState = static_cast< FsaDfaByteState< 1>  *>( m_Root.GetEntry());
                break;
            }
            case FsaCrate::template TypeOf< FsaDfaByteState< 2>>() :
            {
                FsaDfaByteState< 2>     *dfaState = static_cast< FsaDfaByteState< 2>  *>( m_Root.GetEntry());
                break;
            }
            case FsaCrate::template TypeOf< FsaDfaByteState< 3>>() :
            {
                FsaDfaByteState< 3>     *dfaState = static_cast< FsaDfaByteState< 3>  *>( m_Root.GetEntry());
                break;
            }
            case FsaCrate::template TypeOf< FsaDfaByteState< 4>>() :
            {
                FsaDfaByteState< 4>     *dfaState = static_cast< FsaDfaByteState< 4>  *>( m_Root.GetEntry());
                break;
            }/*
			case FsaCrate::template TypeOf< FsaDfaXByteState>() :
            {
                nxState = DfaXTransition( m_Root.GetEntry(), chrId);
                break;
            }*/
            default :
                break;
        }
        return nxState; 
    }


    bool        WriteDot( Cv_DotStream &strm)
    {
        for ( uint32_t i = 1; i < m_DfaRepos->Size(); ++i)
        {
            FsaRepos::Id        id = m_DfaRepos->GetId( i);
            if ( !id.IsValid())
                continue;
            FsaCrate::Var       si = m_DfaRepos->ToVar( id);
            si( [this, id, &strm]( auto k)  { 
                    k->WriteDot( id, this, strm); 
                });
        }
        return true;
    }


};

//_____________________________________________________________________________________________________________________________
