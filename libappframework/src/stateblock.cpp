
#include "../include/stateblock.h"
#include "../include/graphics.h"

CStateBlock::CStateBlock( CGraphics *i_pParent )
{
	m_pParent = i_pParent;

	m_bChangedVertexFormat = false;
	m_bChangedPrimitiveAssembler = false;
	m_bChangedVertexShader = false;
	m_bChangedTriangleShader = false;
	m_bChangedPixelShader = false;
	m_bChangedIndexBuffer = false;
	m_bChangedRenderTarget = false;
	m_bChangedScissorRect = false;
	m_bChangedCamera = false;
}

CStateBlock::~CStateBlock()
{
	RestoreStates();
}

result CStateBlock::SetRenderState( m3drenderstate i_RenderState, uint32 i_iValue )
{
	CMuli3DDevice *pM3DDevice = m_pParent->pGetM3DDevice();

	// Record state if not yet saved ------------------------------------------
	if( m_RenderStates.find( i_RenderState ) == m_RenderStates.end() )
	{
		uint32 iCurrentValue;
		if( !FUNC_FAILED( pM3DDevice->GetRenderState( i_RenderState, iCurrentValue ) ) )
		{
			if( iCurrentValue == i_iValue )
				return s_ok;
			m_RenderStates[i_RenderState] = iCurrentValue;
		}
	}

	return pM3DDevice->SetRenderState( i_RenderState, i_iValue );
}

result CStateBlock::SetVertexFormat( CMuli3DVertexFormat *i_pVertexFormat )
{
	CMuli3DDevice *pM3DDevice = m_pParent->pGetM3DDevice();

	// Record state if not yet saved ------------------------------------------
	if( !m_bChangedVertexFormat )
	{
		m_pVertexFormat = pM3DDevice->pGetVertexFormat();
		if( m_pVertexFormat == i_pVertexFormat )
		{
			SAFE_RELEASE( m_pVertexFormat );
			return s_ok;
		}

		m_bChangedVertexFormat = true;
	}

	return pM3DDevice->SetVertexFormat( i_pVertexFormat );
}

void CStateBlock::SetPrimitiveAssembler( IMuli3DPrimitiveAssembler *i_pPrimitiveAssembler )
{
	CMuli3DDevice *pM3DDevice = m_pParent->pGetM3DDevice();

	// Record state if not yet saved ------------------------------------------
	if( !m_bChangedPrimitiveAssembler )
	{
		m_pPrimitiveAssembler = pM3DDevice->pGetPrimitiveAssembler();
		if( i_pPrimitiveAssembler == m_pPrimitiveAssembler )
		{
			SAFE_RELEASE( m_pPrimitiveAssembler );
			return;
		}

		m_bChangedPrimitiveAssembler = true;
	}

	pM3DDevice->SetPrimitiveAssembler( i_pPrimitiveAssembler );
}

result CStateBlock::SetVertexShader( IMuli3DVertexShader *i_pVertexShader )
{
	CMuli3DDevice *pM3DDevice = m_pParent->pGetM3DDevice();

	// Record state if not yet saved ------------------------------------------
	if( !m_bChangedVertexShader )
	{
		m_pVertexShader = pM3DDevice->pGetVertexShader();
		if( i_pVertexShader == m_pVertexShader )
		{
			SAFE_RELEASE( m_pVertexShader );
			return s_ok;
		}

		m_bChangedVertexShader = true;
	}

	return pM3DDevice->SetVertexShader( i_pVertexShader );
}

void CStateBlock::SetTriangleShader( IMuli3DTriangleShader *i_pTriangleShader )
{
	CMuli3DDevice *pM3DDevice = m_pParent->pGetM3DDevice();

	// Record state if not yet saved ------------------------------------------
	if( !m_bChangedTriangleShader )
	{
		m_pTriangleShader = pM3DDevice->pGetTriangleShader();
		if( i_pTriangleShader == m_pTriangleShader )
		{
			SAFE_RELEASE( m_pTriangleShader );
			return;
		}

		m_bChangedTriangleShader = true;
	}

	pM3DDevice->SetTriangleShader( i_pTriangleShader );
}

result CStateBlock::SetPixelShader( IMuli3DPixelShader *i_pPixelShader )
{
	CMuli3DDevice *pM3DDevice = m_pParent->pGetM3DDevice();

	// Record state if not yet saved ------------------------------------------
	if( !m_bChangedPixelShader )
	{
		m_pPixelShader = pM3DDevice->pGetPixelShader();
		if( i_pPixelShader == m_pPixelShader )
		{
			SAFE_RELEASE( m_pPixelShader );
			return s_ok;
		}

		m_bChangedPixelShader = true;
	}

	return pM3DDevice->SetPixelShader( i_pPixelShader );
}

result CStateBlock::SetIndexBuffer( CMuli3DIndexBuffer *i_pIndexBuffer )
{
	CMuli3DDevice *pM3DDevice = m_pParent->pGetM3DDevice();

	// Record state if not yet saved ------------------------------------------
	if( !m_bChangedIndexBuffer )
	{
		m_pIndexBuffer = pM3DDevice->pGetIndexBuffer();
		if( i_pIndexBuffer == m_pIndexBuffer )
		{
			SAFE_RELEASE( m_pIndexBuffer );
			return s_ok;
		}

		m_bChangedIndexBuffer = true;
	}

	return pM3DDevice->SetIndexBuffer( i_pIndexBuffer );
}

result CStateBlock::SetVertexStream( uint32 i_iStreamNumber, CMuli3DVertexBuffer *i_pVertexBuffer, uint32 i_iOffset, uint32 i_iStride )
{
	CMuli3DDevice *pM3DDevice = m_pParent->pGetM3DDevice();

	// Record state if not yet saved ------------------------------------------
	if( m_VertexStreams.find( i_iStreamNumber ) == m_VertexStreams.end() )
	{
		tVertexStreamInfo streamInfo;
		if( !FUNC_FAILED( pM3DDevice->GetVertexStream( i_iStreamNumber, &streamInfo.pVertexBuffer, &streamInfo.iOffset, &streamInfo.iStride ) ) )
		{
			if( streamInfo.pVertexBuffer == i_pVertexBuffer &&
				streamInfo.iOffset == i_iOffset &&
				streamInfo.iStride == i_iStride )
			{
				SAFE_RELEASE( streamInfo.pVertexBuffer );
				return s_ok;
			}

			m_VertexStreams[i_iStreamNumber] = streamInfo;
		}
	}

	return pM3DDevice->SetVertexStream( i_iStreamNumber, i_pVertexBuffer, i_iOffset, i_iStride );
}

result CStateBlock::SetTexture( uint32 i_iSamplerNumber, IMuli3DBaseTexture *i_pTexture )
{
	CMuli3DDevice *pM3DDevice = m_pParent->pGetM3DDevice();

	// Record state if not yet saved ------------------------------------------
	if( m_Textures.find( i_iSamplerNumber ) == m_Textures.end() )
	{
		IMuli3DBaseTexture *pCurTexture;
		if( !FUNC_FAILED( pM3DDevice->GetTexture( i_iSamplerNumber, &pCurTexture ) ) )
		{
			if( pCurTexture == i_pTexture )
			{
				SAFE_RELEASE( pCurTexture );
				return s_ok;
			}

			m_Textures[i_iSamplerNumber] = pCurTexture;
		}
	}

	return pM3DDevice->SetTexture( i_iSamplerNumber, i_pTexture );
}

result CStateBlock::SetTextureSamplerState( uint32 i_iSamplerNumber, m3dtexturesamplerstate i_TextureSamplerState, uint32 i_iState )
{
	CMuli3DDevice *pM3DDevice = m_pParent->pGetM3DDevice();

	// Record state if not yet saved ------------------------------------------
	uint32 iIndex = i_iSamplerNumber * (uint32)m3dtss_numtexturesamplerstates + (uint32)i_TextureSamplerState;
	if( m_TextureSamplerStates.find( iIndex ) == m_TextureSamplerStates.end() )
	{
		uint32 iState;
		if( !FUNC_FAILED( pM3DDevice->GetTextureSamplerState( i_iSamplerNumber, i_TextureSamplerState, iState ) ) )
		{
			if( iState == i_iState )
				return s_ok;

			m_TextureSamplerStates[iIndex] = iState;
		}
	}

	return pM3DDevice->SetTextureSamplerState( i_iSamplerNumber, i_TextureSamplerState, i_iState );
}

result CStateBlock::SetScissorRect( const m3drect &i_ScissorRect )
{
	CMuli3DDevice *pM3DDevice = m_pParent->pGetM3DDevice();

	// Record state if not yet saved ------------------------------------------
	if( !m_bChangedScissorRect )
	{
		m_ScissorRect = pM3DDevice->GetScissorRect();
		m_bChangedScissorRect = true;
	}

	return pM3DDevice->SetScissorRect( i_ScissorRect );
}

void CStateBlock::SetRenderTarget( CMuli3DRenderTarget *i_pRenderTarget )
{
	CMuli3DDevice *pM3DDevice = m_pParent->pGetM3DDevice();

	// Record state if not yet saved ------------------------------------------
	if( !m_bChangedRenderTarget )
	{
		m_pRenderTarget = pM3DDevice->pGetRenderTarget();
		if( i_pRenderTarget == m_pRenderTarget )
		{
			SAFE_RELEASE( m_pRenderTarget );
			return;
		}

		m_bChangedRenderTarget = true;
	}

	pM3DDevice->SetRenderTarget( i_pRenderTarget );
}

void CStateBlock::SetCurCamera( class CCamera *i_pCamera )
{
	// Record state if not yet saved ------------------------------------------
	if( !m_bChangedCamera )
	{
		m_pCamera = m_pParent->m_pCurCamera;
		if( m_pCamera == i_pCamera )
			return;

		m_bChangedCamera = true;
	}

	m_pParent->m_pCurCamera = i_pCamera;
}

void CStateBlock::RestoreStates()
{
	CMuli3DDevice *pM3DDevice = m_pParent->pGetM3DDevice();
	
	for( map<m3drenderstate, uint32>::iterator pRenderState = m_RenderStates.begin(); pRenderState != m_RenderStates.end(); ++pRenderState )
		pM3DDevice->SetRenderState( pRenderState->first, pRenderState->second );
	m_RenderStates.clear();

	if( m_bChangedVertexFormat )
	{
		pM3DDevice->SetVertexFormat( m_pVertexFormat );
		SAFE_RELEASE( m_pVertexFormat );
		m_bChangedVertexFormat = false;
	}

	if( m_bChangedPrimitiveAssembler )
	{
		pM3DDevice->SetPrimitiveAssembler( m_pPrimitiveAssembler );
		SAFE_RELEASE( m_pPrimitiveAssembler );
		m_bChangedPrimitiveAssembler = false;
	}

	if( m_bChangedVertexShader )
	{
		pM3DDevice->SetVertexShader( m_pVertexShader );
		SAFE_RELEASE( m_pVertexShader );
		m_bChangedVertexShader = false;
	}

	if( m_bChangedTriangleShader )
	{
		pM3DDevice->SetTriangleShader( m_pTriangleShader );
		SAFE_RELEASE( m_pTriangleShader );
		m_bChangedTriangleShader = false;
	}

	if( m_bChangedPixelShader )
	{
		pM3DDevice->SetPixelShader( m_pPixelShader );
		SAFE_RELEASE( m_pPixelShader );
		m_bChangedPixelShader = false;
	}

	if( m_bChangedIndexBuffer )
	{
		pM3DDevice->SetIndexBuffer( m_pIndexBuffer );
		SAFE_RELEASE( m_pIndexBuffer );
		m_bChangedIndexBuffer = false;
	}

	for( map<uint32, tVertexStreamInfo>::iterator pVertexStream = m_VertexStreams.begin(); pVertexStream != m_VertexStreams.end(); ++pVertexStream )
	{
		pM3DDevice->SetVertexStream( pVertexStream->first, pVertexStream->second.pVertexBuffer, pVertexStream->second.iOffset, pVertexStream->second.iStride );
		SAFE_RELEASE( pVertexStream->second.pVertexBuffer );
	}
	m_VertexStreams.clear();

	for( map<uint32, IMuli3DBaseTexture *>::iterator pTexture = m_Textures.begin(); pTexture != m_Textures.end(); ++pTexture )
	{
		pM3DDevice->SetTexture( pTexture->first, pTexture->second );
		SAFE_RELEASE( pTexture->second );
	}
	m_Textures.clear();

	for( map<uint32, uint32>::iterator pTextureSamplerState = m_TextureSamplerStates.begin(); pTextureSamplerState != m_TextureSamplerStates.end(); ++pTextureSamplerState )
	{
		uint32 iSamplerNumber = pTextureSamplerState->first / (uint32)m3dtss_numtexturesamplerstates;
		uint32 iSamplerState = pTextureSamplerState->first - iSamplerNumber * (uint32)m3dtss_numtexturesamplerstates;
		pM3DDevice->SetTextureSamplerState( iSamplerNumber, (m3dtexturesamplerstate)iSamplerState, pTextureSamplerState->second );
	}
	m_TextureSamplerStates.clear();

	if( m_bChangedRenderTarget )
	{
		pM3DDevice->SetRenderTarget( m_pRenderTarget );
		SAFE_RELEASE( m_pRenderTarget );
		m_bChangedRenderTarget = false;
	}

	if( m_bChangedScissorRect )
	{
		pM3DDevice->SetScissorRect( m_ScissorRect );
		m_bChangedScissorRect = false;
	}

	if( m_bChangedCamera )
	{
		m_pParent->m_pCurCamera = m_pCamera;
		m_bChangedCamera = false;
	}
}
