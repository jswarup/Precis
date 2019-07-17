//  sg_dfastate.cpp _____________________________________________________________________________________________________________

#include    "segue/tenor/sg_include.h"
#include    "segue/colophon/sg_dfastate.h" 
#include    "segue/colophon/sg_reposatelier.h" 

using namespace Sg_RExp; 
 
//_____________________________________________________________________________________________________________________________

void       FsaSupState::DescendIt::FetchFilterElems( void)
{
    FsaRepos::Var        state = m_ElemRepos->ToVar( m_SubStates[ m_StateCursor]);
    m_Filters = state( [this]( auto k) { return k->Filters(); });
    m_DestStateIds = state( [this]( auto k) { return k->Dests(); });   
}

//_____________________________________________________________________________________________________________________________

void       FsaSupState::DescendIt::Dump( std::ostream &strm)
{
    for ( uint32_t i = 0; i < m_SubStates.Size(); ++i)
    {
        strm << m_SubStates[i].GetId() << " ";
        if ( m_StateCursor != i)
            continue;
        strm << "[ ";
        for ( uint32_t j = 0; j  < m_Filters.Size(); ++j)
        {
            m_DfaRepos->m_ElemRepos->m_FilterRepos.Dump( strm, m_Filters[ j]);
            strm << m_DestStateIds[ j].GetId() << ", "; 
        }
        strm << "] ";
    }
    strm << "\n";
    return;
}
 //_____________________________________________________________________________________________________________________________

void    FsaSupState::DoConstructTransisition( FsaId supId, FsaDfaCnstr *dfaCnstr)
{ 
    FsaDfaRepos                     *dfaRepos = dfaCnstr->m_DfaRepos;
    FsaElemRepos                    *elemRepos = dfaCnstr->m_ElemRepos;
    Cv_Seq< FsaId>                  subStates = SubStates();
    if ( !subStates.Size())
    {
        dfaRepos->Destroy( supId.GetId());
        return;
    } 
    DescendIt                   descIt( elemRepos, dfaRepos, this);

    DistribRepos::DfaDistrib    dDist = dfaRepos->m_DistribRepos.ConstructDfaDistrib( &descIt); 

    //dDist.Dump( std::cout, &dfaRepos->m_DistribRepos);
    descIt.DoSetup( dDist.SzDescend(), m_Level +1); 
    
    dfaRepos->m_DistribRepos.Classify( m_Level, dDist, &descIt);   
    
    Action                  *action = DetachAction();  

    m_DfaStateMap->Insert( this, supId.GetId());
    Cv_Array< uint32_t, 256>        destArr;
    for ( uint32_t k = 0; k < dDist.SzDescend(); ++k)
    {
        FsaSupState                 *subSupState = descIt.m_SubSupStates[ k];
        subSupState->Freeze();
        if ( ! subSupState->SubStates().Size())
        {
            destArr.Append( 0); 
            delete subSupState;
            continue;
        }
        Cv_Slot< FsaDfaStateMap>    dfaStateMap =  dfaCnstr->m_SupDfaCltn.Locate( elemRepos, subSupState);
        uint32_t                    ind = dfaStateMap->Find( subSupState);
        if ( ind != CV_UINT32_MAX)
        { 
            destArr.Append( ind);  
            delete subSupState;
            continue;
        }

        auto            subId = dfaRepos->Store( subSupState);
        subSupState->m_DfaStateMap = dfaStateMap;
        destArr.Append( subId.GetId()); 
        dfaCnstr->m_FsaStk.push_back( subId); 
    }  
    dfaCnstr->ConstructDfaStateAt( supId.GetId(), dDist, action, destArr);   
    //m_DfaStateMap.Purge();
    dDist.m_DVar.Delete();
    return;
}


//_____________________________________________________________________________________________________________________________

FsaDfaStateMap::~FsaDfaStateMap( void)
{
    if ( m_Cltn)
    {
        m_Cltn->Erase( this);
        FsaRepos    *dfaRepos = m_Cltn->m_DfaCnstr->m_DfaRepos;
        for ( auto it = m_SupDfaMap.begin(); it != m_SupDfaMap.end(); ++it) 
            delete it->first; 
    }
}

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
    FsaSupState     *supRootState = new FsaSupState( 0);
    FsaId           rootId = m_DfaRepos->Store( supRootState); 
    supRootState->m_SubStates.push_back( m_ElemRepos->m_RootId);
    supRootState->m_DfaStateMap = m_SupDfaCltn.Locate( m_ElemRepos, supRootState);
    m_FsaStk.push_back( rootId);
    while ( m_FsaStk.size())
    {
        FsaId           supId = m_FsaStk.back();
        FsaSupState     *supState = m_DfaRepos->ToVar( supId).Elem< FsaSupState>();
        m_FsaStk.pop_back(); 

        supState->DoConstructTransisition( supId, this);  
    }

    m_DfaRepos->OperateAll( [ this]( auto k, uint32_t ind) {
        return k->CleanupDestIds( m_DfaRepos);
    });
    m_DfaRepos->m_RootId =  m_DfaRepos->GetId( rootId.GetId());
    return;
}

//_____________________________________________________________________________________________________________________________

void    FsaDfaCnstr::ConstructDfaStateAt( uint32_t index, const DistribRepos::DfaDistrib &dDistrib, Action *action, const Cv_Array< uint32_t, 256> &destArr)
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
            dests[ k] = FsaId( destArr[ i], 0);
            ++k;
        }
    }
    if ( !doneFlg)
    {
        DistribRepos::Id        dId = m_DfaRepos->m_DistribRepos.StoreDistrib( dDistrib.m_DVar); 
        FsaDfaState             *dfaState = FsaDfaState::Construct( dId, dDistrib.m_Inv, dDistrib.m_MxEqClass, action, destArr);  
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

