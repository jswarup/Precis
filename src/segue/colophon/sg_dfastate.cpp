//  sg_dfastate.cpp _____________________________________________________________________________________________________________

#include    "segue/tenor/sg_include.h"
#include    "segue/colophon/sg_dfastate.h" 
#include    "segue/colophon/sg_reposatelier.h" 

using namespace Sg_RExp; 

//_____________________________________________________________________________________________________________________________

bool FsaDfaState::CleanupDestIds( FsaRepos *dfaRepos)
{
    Cv_Seq< FsaId>    dests = Dests(); 
    for ( uint32_t k = 0; k < dests.Size(); ++k)
        dests[ k] =  dfaRepos->GetId( dests[ k].GetId());
    return true;
}
 
//_____________________________________________________________________________________________________________________________

bool  FsaDfaState::DoSaute( FsaDfaRepos::Blossom *bRepos)
{
    bRepos->Distribs().ConvertIdToVarId( &m_DId);
    auto                states = bRepos->States();
    Cv_Seq< FsaId>      dests = Dests(); 
    for ( uint32_t i = 0; i < dests.Size(); ++i)        
        states->ConvertIdToVarId( dests.PtrAt( i));
    return true;
}


//_____________________________________________________________________________________________________________________________

bool FsaDfaXByteState::CleanupDestIds( FsaRepos *dfaRepos)
{ 
    Cv_Seq< FsaId>      dests = Dests();
    for ( uint32_t i = 0; i < DestSz(); ++i) 
        dests[ i] = dfaRepos->GetId( dests[ i].GetId()); 
    return true;
}

//_____________________________________________________________________________________________________________________________

bool  FsaDfaXByteState::DoSaute( FsaDfaRepos::Blossom *bRepos)
{
    Cv_Seq< FsaId>              dests = Dests(); 
    for ( uint32_t k = 0; k < dests.Size(); ++k) 
        bRepos->States()->ConvertIdToVarId( &dests[ k]);
    return true;
}

//_____________________________________________________________________________________________________________________________

bool        FsaDfaRepos::WriteDot( Cv_DotStream &strm)
{
    Sg_DfaReposAtelier  atelier( this);
    return atelier.WriteDot( strm);
}

//_____________________________________________________________________________________________________________________________

void    FsaDfaRepos::Blossom::SauteStates( void)
{ 
    m_States.m_SauteFLg = true;
    for ( uint32_t i = 1; i < m_States.Size(); ++i)
    {
        Var     si = m_States.VarAt( i);
        if (si)
            si( [this]( auto k) { k->DoSaute( this); });
    }
    return; 
}

//_____________________________________________________________________________________________________________________________

void    FsaDfaCnstr::SubsetConstruction( void)
{  
    FsaCfaState     *supRootState = new FsaCfaState( 0);
    FsaId           rootId = m_DfaRepos->Store( supRootState); 
    supRootState->m_SubStates.push_back( m_ElemRepos->m_RootId);
    FsaRuleLump     *ruleLump = m_RuleLumpSet.Locate( m_ElemRepos, supRootState);
    supRootState->m_RuleLump = ruleLump;
    ruleLump->Register( supRootState, rootId.GetId());
    ruleLump->m_ActiveRef.RaiseRef();
 
    m_FsaStk.push_back( rootId);
    while ( m_FsaStk.size())
    {
        FsaId           supId = m_FsaStk.back();
        FsaCfaState     *supState = m_DfaRepos->ToVar( supId).Elem< FsaCfaState>();
        m_FsaStk.pop_back(); 

        supState->DoConstructTransisition( supId, this);  
    }

    m_DfaRepos->OperateAll( [ this]( auto k, uint32_t ind) {
        return k->CleanupDestIds( m_DfaRepos);
    });
    m_DfaRepos->m_RootId =  m_DfaRepos->GetId( rootId.GetId());
    std::sort( m_HighDistribs.begin(), m_HighDistribs.end(), []( auto a, auto b) { return a.first > b.first; });
    return;
}

//_____________________________________________________________________________________________________________________________

void    FsaDfaCnstr::ConstructDfaStateAt( uint32_t index, const DistribRepos::DfaDistrib &dDistrib, Action *action, const Cv_Array< FsaId, 256> &destArr, uint32_t szTrials)
{
    uint32_t            sz = dDistrib.SzDescend();
    uint8_t             szTok = action  ? uint8_t( action->m_Values.size()) : 0;    
    bool                doneFlg = false;
    if ( !doneFlg && !szTok)
    { 
        uint32_t                    szSingles = 0;
        Cv_Array< uint16_t, 256>    byteValues;
        std::tie( szSingles, byteValues) = m_DfaRepos->m_DistribRepos.SingleChars( dDistrib.m_DVar, dDistrib.m_Inv);  
        
        Cv_Seq< uint8_t>       bytes;
        Cv_Seq< FsaId>         dests;
        if ( szSingles && ( sz == ( szSingles + ( dDistrib.m_Inv != CV_UINT16_MAX))))
        {
            if ( szSingles == 1)    
            {
                FsaDfaByteState< 1>             *dfaState = FsaDfaByteState< 1>::Construct();              
                m_DfaRepos->StoreAt( index, dfaState); 
                bytes = dfaState->Bytes();
                dests = dfaState->Dests();
                doneFlg = true;
            } else if (  szSingles == 2)    
            {
                FsaDfaByteState< 2>             *dfaState = FsaDfaByteState< 2>::Construct();              
                m_DfaRepos->StoreAt( index, dfaState); 
                bytes = dfaState->Bytes();
                dests = dfaState->Dests();
                doneFlg = true;
            } else if (  szSingles == 3)    
            {
                FsaDfaByteState< 3>             *dfaState = FsaDfaByteState< 3>::Construct();              
                m_DfaRepos->StoreAt( index, dfaState); 
                bytes = dfaState->Bytes();
                dests = dfaState->Dests();
                doneFlg = true;
            }  else if ( szSingles == 4)   
            {
                FsaDfaByteState< 4>             *dfaState = FsaDfaByteState< 4>::Construct();              
                m_DfaRepos->StoreAt( index, dfaState); 
                bytes = dfaState->Bytes();
                dests = dfaState->Dests();
                doneFlg = true;
            } else if ( szSingles == 5)    
            {
                FsaDfaByteState< 5>             *dfaState = FsaDfaByteState< 5>::Construct();              
                m_DfaRepos->StoreAt( index, dfaState); 
                bytes = dfaState->Bytes();
                dests = dfaState->Dests();
                doneFlg = true;
            }  else if ( szSingles == 6)    
            {
                FsaDfaByteState< 6>             *dfaState = FsaDfaByteState< 6>::Construct();              
                m_DfaRepos->StoreAt( index, dfaState); 
                bytes = dfaState->Bytes();
                dests = dfaState->Dests();
                doneFlg = true;
            }  else if ( szSingles == 7)    
            {
                FsaDfaByteState< 7>             *dfaState = FsaDfaByteState< 7>::Construct();              
                m_DfaRepos->StoreAt( index, dfaState); 
                bytes = dfaState->Bytes();
                dests = dfaState->Dests();
                doneFlg = true;
            }  else if ( szSingles == 8)    
            {
                FsaDfaByteState< 8>             *dfaState = FsaDfaByteState< 8>::Construct();              
                m_DfaRepos->StoreAt( index, dfaState); 
                bytes = dfaState->Bytes();
                dests = dfaState->Dests();
                doneFlg = true;
            }     else if ( szSingles < 8)    
        	{
	            FsaDfaXByteState             *dfaState = FsaDfaXByteState::Construct( szSingles);              
	            m_DfaRepos->StoreAt( index, dfaState); 
	            bytes = dfaState->Bytes();
	            dests = dfaState->Dests();
	            doneFlg = true;
	        }
        }
        for ( uint32_t i = 0, k = 0; doneFlg && ( i < sz); ++i)
        {
            if ( byteValues[ i] == CV_UINT16_MAX)
                continue;
            
            bytes[ k] = uint8_t( byteValues[ i]);
            dests[ k] = destArr[ i];
            ++k;
        }
    }
    if ( !doneFlg)
    {
        DistribRepos::Id        dId = m_DfaRepos->m_DistribRepos.StoreDistrib( dDistrib.m_DVar); 
        FsaDfaState             *dfaState = FsaDfaState::Construct( dId, dDistrib.m_Inv, dDistrib.m_MxEqClass, action, destArr);  
        if ( szTrials)
            m_HighDistribs.push_back( std::make_pair( szTrials, dId));
        m_DfaRepos->StoreAt( index, dfaState); 
    }
    return;
}

//_____________________________________________________________________________________________________________________________

bool        FsaDfaCnstr::DumpDot( const char *path)
{
    std::ofstream           fsaOStrm( path);
    Cv_DotStream			fsaDotStrm( &fsaOStrm, true); 
    return m_ElemRepos->WriteDot( fsaDotStrm) && m_DfaRepos->WriteDot( fsaDotStrm);
}

//_____________________________________________________________________________________________________________________________  

