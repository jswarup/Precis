//  sg_dfastate.cpp _____________________________________________________________________________________________________________

#include    "segue/tenor/sg_include.h"
#include    "segue/colophon/sg_dfastate.h" 

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

void    FsaSupState::DoConstructTransisition( FsaDfaCnstr *dfaCnstr)
{ 
    FsaDfaRepos                     *dfaRepos = dfaCnstr->m_DfaRepos;
    FsaElemRepos                    *elemRepos = dfaCnstr->m_ElemRepos;
    Cv_Seq< FsaId>                 subStates = SubStates();
    if ( !subStates.Size())
    {
        dfaRepos->Destroy( GetId());
        return;
    } 
    DescendIt               descIt( elemRepos, dfaRepos, this);

    DistribRepos::Discr     discr =  dfaRepos->m_DistribRepos.FetchDiscr( &descIt); 
    //discr.Dump( std::cout, &dfaRepos->m_DistribRepos);
    descIt.DoSetup( discr.SzDescend()); 
    
    dfaRepos->m_DistribRepos.Classify( discr, &descIt);   
    
    Action                  *action = DetachAction();  

    m_DfaStateMap->Insert( this, GetId());
    Cv_Array< uint32_t, 256>        destArr;
    for ( uint32_t k = 0; k < discr.SzDescend(); ++k)
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
            destArr.Append(ind); 
            delete subSupState;
            continue;
        }

        auto            subId = dfaRepos->Store( subSupState);
        subSupState->m_DfaStateMap = dfaStateMap;
        destArr.Append( subId.GetId()); 
        dfaCnstr->m_FsaStk.push_back( subSupState); 
    } 
    dfaCnstr->ConstructDfaStateAt( GetId(), discr, action, destArr);   
    //m_DfaStateMap.Purge();
    return;
}

//_____________________________________________________________________________________________________________________________

bool    FsaSupState::WriteDot( FsaRepos *fsaRepos, Cv_DotStream &strm) 
{ 
    strm << GetTypeChar() << GetId() << " [ shape=oval";
    strm << " color=purple label= <<FONT>" << GetTypeChar() << GetId() << "<BR />" << "<BR />" ; 
    strm << " </FONT>>];\n "; 

    FsaDfaRepos         *dfaRepos = static_cast< FsaDfaRepos *>( fsaRepos); 
    
    Cv_Seq< FsaId>     subStates = SubStates(); 
    for ( uint32_t k = 0; k < subStates.Size(); ++k)
    {
        FsaClip         regex = dfaRepos->m_ElemRepos->ToVar( subStates[ k]);
        if ( !regex)
            continue;
        strm << GetTypeChar() << GetId() << " -> " << regex->GetTypeChar() << regex->GetId() << " [ arrowhead=normal color=black label=<<FONT> ";   
        strm << "</FONT>>] ; \n" ;  
    }
    return false; 
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
    {
        FsaClip         regex = dfaRepos->ToVar( dests[ k]);
        if ( !regex)
            dests[ k] = Id();
        else
            dests[ k] = *regex.GetEntry();    
    } 
    return true;
}

//_____________________________________________________________________________________________________________________________

bool    FsaDfaState::WriteDot( FsaRepos *fsaRepos, Cv_DotStream &strm) 
{ 
    strm << GetTypeChar() << GetId() << " [ shape=";

    uint64_t        *toks = Tokens().Ptr();
    if ( toks)
        strm << "box";
    else
        strm << "ellipse";
    strm << " color=Red label= <<FONT> " << GetTypeChar() << GetId() << "<BR />" <<   "<BR />" ;
    for ( uint32_t i = 0; i < m_TokSz; ++i)
        strm << " T" << toks[ i];
    strm << " </FONT>>];\n "; 

    FsaDfaRepos                 *dfaRepos = static_cast< FsaDfaRepos *>( fsaRepos);
    std::vector< Sg_ChSet>      domain = dfaRepos->m_DistribRepos.Domain( m_DId);
    Cv_Seq< FsaId>    dests = Dests(); 
    for ( uint32_t k = 0; k < dests.Size(); ++k)
    {
        FsaClip         regex = fsaRepos->ToVar( dests[ k]);
        if ( !regex)
            continue;
        strm << GetTypeChar() << GetId() << " -> " <<  regex->GetTypeChar() << regex->GetId() << " [ arrowhead=normal color=black label=<<FONT> "; 
        strm << Cv_Aid::XmlEncode( domain[ k].ToString());  
        strm << "</FONT>>] ; \n" ;  
    }
    return true; 
}
//_____________________________________________________________________________________________________________________________

bool    FsaDfaState::DumpDot( Cv_DotStream &strm) 
{ 
    strm << GetId() << " [ shape=";

    uint64_t        *toks = Tokens().Ptr();
    if ( toks)
        strm << "box";
    else
        strm << "ellipse";
    strm << " color=Red label= <<FONT> " << GetId() << "<BR />" <<   "<BR />" ;
    for ( uint32_t i = 0; i < m_TokSz; ++i)
        strm << " T" << toks[ i];
    strm << " </FONT>>];\n ";  
    //std::vector< Sg_ChSet>      domain = dfaRepos->m_DistribRepos.Domain( m_Discr.m_DId);
    Cv_Seq< FsaId>    dests = Dests(); 
    for ( uint32_t k = 0; k < dests.Size(); ++k)
    {
   //     FsaClip         regex = fsaRepos->ToVar( dests[ k]);
        if ( !dests[ k].GetId())
            continue;
        strm << GetId() << " -> " <<  dests[ k].GetId() << " [ arrowhead=normal color=black label=<<FONT> "; 
//        strm << Cv_Aid::XmlEncode( domain[ k].ToString());  
        strm << "</FONT>>] ; \n" ;  
    }
 
    return false; 
}

//_____________________________________________________________________________________________________________________________

bool  FsaDfaState::DoSaute( FsaDfaRepos::Blossom *bRepos)
{
    bRepos->Distribs().ConvertIdToVarId( &m_DId);
    Cv_Seq< FsaId>         dests = Dests(); 
    for ( uint32_t i = 0; i < dests.Size(); ++i)
        bRepos->States().ConvertIdToVarId( dests.PtrAt( i));
    return true;
}

//_____________________________________________________________________________________________________________________________

bool FsaDfaUniXState::CleanupDestIds( FsaRepos *dfaRepos)
{ 
    FsaClip         regex = dfaRepos->ToVar( m_Dest);
    if ( !regex)
        m_Dest = Id();
    else
        m_Dest = *regex.GetEntry();    
    return true;
}

//_____________________________________________________________________________________________________________________________

bool    FsaDfaUniXState::WriteDot( FsaRepos *fsaRepos, Cv_DotStream &strm) 
{ 
    strm << GetTypeChar() << GetId() << " [ shape=";

    uint64_t        *toks = Tokens().Ptr(); 
    strm << "diamond"; 
    strm << " color=Red label= <<FONT> " << GetTypeChar() << GetId(); 
    strm << " </FONT>>];\n"; 

    FsaDfaRepos                 *dfaRepos = static_cast< FsaDfaRepos *>( fsaRepos); 
    FsaClip                     regex = fsaRepos->ToVar( m_Dest); 
    strm << GetTypeChar() << GetId() << " -> " <<  regex->GetTypeChar() << regex->GetId() << " [ arrowhead=normal color=black label=<<FONT> "; 
    strm << Cv_Aid::XmlEncode( dfaRepos->m_DistribRepos.ChSet( m_Byte).ToString());  
    strm << "</FONT>>] ; \n" ;   
    return true; 
}

//_____________________________________________________________________________________________________________________________

bool    FsaDfaUniXState::DumpDot( Cv_DotStream &strm) 
{ 
    strm <<   GetId() << " [ shape=";
 
    strm << "diamond"; 
    strm << " color=Red label= <<FONT> " <<  GetId(); 
    strm << " </FONT>>];\n"; 
  
    strm <<  GetId() << " -> " <<     m_Dest.GetId() << " [ arrowhead=normal color=black label=<<FONT> "; 
    //strm << Cv_Aid::XmlEncode( dfaRepos->m_DistribRepos.ChSet( m_Byte).ToString());  
    strm << "</FONT>>] ; \n" ;   
    return true; 
}

//_____________________________________________________________________________________________________________________________

bool  FsaDfaUniXState::DoSaute( FsaDfaRepos::Blossom *bRepos)
{
    bRepos->States().ConvertIdToVarId( &m_Dest);
    return true;
}


//_____________________________________________________________________________________________________________________________

bool FsaDfaXByteState::CleanupDestIds( FsaRepos *dfaRepos)
{ 
    Cv_Seq< FsaId>      dests = Dests();
    for ( uint32_t i = 0; i < DestSz(); ++i)
    {
        FsaClip         regex = dfaRepos->ToVar( dests[ i]);
        dests[ i] = regex ? *regex.GetEntry() : Id();    
    }
    return true;
}

//_____________________________________________________________________________________________________________________________

bool    FsaDfaXByteState::WriteDot( FsaRepos *fsaRepos, Cv_DotStream &strm) 
{ 
    strm << GetTypeChar() << GetId() << " [ shape=";

    uint64_t        *toks = Tokens().Ptr(); 
    strm << "diamond"; 
    strm << " color=Red label= <<FONT> " << GetTypeChar() << GetId(); 
    strm << " </FONT>>];\n"; 

    FsaDfaRepos                 *dfaRepos = static_cast< FsaDfaRepos *>( fsaRepos); 
    Cv_Seq< FsaId>              dests = Dests(); 
    Cv_Seq< uint8_t>            bytes = Bytes(); 
    for ( uint32_t k = 0; k < dests.Size(); ++k)
    {
        FsaClip         regex = fsaRepos->ToVar( dests[ k]);
        strm << GetTypeChar() << GetId() << " -> " <<  regex->GetTypeChar() << regex->GetId() << " [ arrowhead=normal color=black label=<<FONT> "; 
        strm << Cv_Aid::XmlEncode( dfaRepos->m_DistribRepos.ChSet( bytes[ k]).ToString());  
        strm << "</FONT>>] ; \n" ;   
    }
    return true; 
}

//_____________________________________________________________________________________________________________________________

bool    FsaDfaXByteState::DumpDot( Cv_DotStream &strm) 
{ 
    strm <<   GetId() << " [ shape=";
 
    strm << "diamond"; 
    strm << " color=Red label= <<FONT> " <<  GetId(); 
    strm << " </FONT>>];\n"; 

    Cv_Seq< uint8_t>            bytes = Bytes(); 
    Cv_Seq< FsaId>              dests = Dests(); 
    for ( uint32_t k = 0; k < dests.Size(); ++k)
    {
        strm <<  GetId() << " -> " <<     dests[ k].GetId() << " [ arrowhead=normal color=black label=<<FONT> "; 
        //strm << Cv_Aid::XmlEncode( dfaRepos->m_DistribRepos.ChSet( m_Byte).ToString());  
        strm << "</FONT>>] ; \n" ;   
    }
    return true; 
}

//_____________________________________________________________________________________________________________________________

bool  FsaDfaXByteState::DoSaute( FsaDfaRepos::Blossom *bRepos)
{
    Cv_Seq< FsaId>              dests = Dests(); 
    for ( uint32_t k = 0; k < dests.Size(); ++k) 
        bRepos->States().ConvertIdToVarId( &dests[ k]);
    return true;
}

//_____________________________________________________________________________________________________________________________

bool        FsaDfaRepos::WriteDot( Cv_DotStream &strm)
{
    for ( uint32_t i = 1; i < Size(); ++i)
    {
        Var     si = Get( i);
        if (si)
            si( [this, &strm]( auto k) { k->WriteDot( this, strm); });
    }
    return true;
}

//_____________________________________________________________________________________________________________________________

bool        FsaDfaRepos::DumpDot( const char *path)
{
    std::ofstream           fsaOStrm( path);
    Cv_DotStream			fsaDotStrm( &fsaOStrm, true); 
    return WriteDot( fsaDotStrm);
}

//_____________________________________________________________________________________________________________________________

void    FsaDfaRepos::Blossom::SauteStates( void)
{
    auto states = States();
        
    for ( uint32_t i = 1; i < states.Size(); ++i)
    {
        Var     si = states.VarAt( i);
        if (si)
            si( [this]( auto k) { k->DoSaute( this); });
    }
    return; 
}

//_____________________________________________________________________________________________________________________________

void    FsaDfaCnstr::SubsetConstruction( void)
{  
    FsaSupState     *supRootState = m_DfaRepos->Construct< FsaSupState>(); 
    uint32_t        rootId = supRootState->GetId();
    supRootState->m_SubStates.push_back( m_ElemRepos->m_RootId);
    supRootState->m_DfaStateMap = m_SupDfaCltn.Locate( m_ElemRepos, supRootState);
    m_FsaStk.push_back( supRootState);
    while ( m_FsaStk.size())
    {
        FsaSupState     *supState = m_FsaStk.back();
        m_FsaStk.pop_back(); 

        supState->DoConstructTransisition( this);  
    }

    m_DfaRepos->m_RootId = m_DfaRepos->GetId( rootId);;
    m_DfaRepos->OperateAll( [ this]( auto k) {
        return k->CleanupDestIds( m_DfaRepos);
    });
    return;
}

//_____________________________________________________________________________________________________________________________

void    FsaDfaCnstr::ConstructDfaStateAt( uint32_t index, const DistribRepos::Discr &discr, Action *action, const Cv_Array< uint32_t, 256> &destArr)
{
    uint32_t            sz = discr.SzDescend();
    uint8_t             szTok = action  ? uint8_t( action->m_Values.size()) : 0;    
    bool                doneFlg = false;
    if ( !doneFlg && !szTok)
    { 
        uint32_t                    szSingles = 0;
        Cv_Array< uint16_t, 256>    byteValues;
        std::tie( szSingles, byteValues) = m_DfaRepos->m_DistribRepos.SingleChars( discr.m_DId, discr.m_Inv);  
        
        Cv_Seq< uint8_t>       bytes;
        Cv_Seq< FsaId>         dests;
        if ( sz == ( szSingles + ( discr.m_Inv != CV_UINT16_MAX)))
        {
            if ( szSingles == 1)    
            {
                FsaDfaUniXState             *dfaState = FsaDfaUniXState::Construct();              
                m_DfaRepos->StoreAt( index, dfaState); 
                bytes = dfaState->Bytes();
                dests = dfaState->Dests();
                doneFlg = true;
            } else
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
        FsaDfaState             *dfaState = FsaDfaState::Construct( discr, action, destArr);  
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

