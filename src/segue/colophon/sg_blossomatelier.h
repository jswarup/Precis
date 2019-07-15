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

   // const FsaCrate::Var     &RootState( void) {  return m_Root; }

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
        return state( [ this, chrId]( auto curState) { return curState->Eval( this, chrId); } );
    }
 
    FsaDfaRepos::Id         AdvanceRoot(  const Cv_Seq< uint8_t> &chrs, uint32_t *pLen)
    { 

        FsaDfaRepos::Id   nxState; 
        switch ( m_Root.GetType())
        {
            case FsaCrate::template TypeOf< FsaDfaState>() :
            {
                FsaDfaState         *dfaState = static_cast< FsaDfaState *>( m_Root.GetEntry());
                uint16_t            deadInd = dfaState->DeadInd();
                return  m_RootDistrib( [ &chrs,  pLen, deadInd, dfaState]( auto distrib) { 
                    for ( uint32_t  &i = *pLen;  i < chrs.Size(); )
                    { 
                        uint16_t    index = distrib->Image( chrs[ i++]);
                        if ( deadInd != index) 
                            return dfaState->DestAt( index); 
                    }
                    return FsaDfaRepos::Id();
                });

            }
            default :
            {
                return m_Root( [ this, &chrs,  pLen]( auto dfaState) {
                        for ( uint32_t   &i = *pLen;  i < chrs.Size(); )
                        {
                            FsaDfaRepos::Id     nxState = dfaState->Eval( this, chrs[ i++]);
                            if ( nxState.IsValid())
                                return nxState; 
                        }
                        return FsaDfaRepos::Id();
                });
            }  
        } 
    }
};

  

//_____________________________________________________________________________________________________________________________
