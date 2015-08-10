
#include "../include/graphics.h"
#include "../include/application.h"
#include "../include/stateblock.h"

CGraphics::CGraphics( IApplication *i_pParent )
{
	m_pParent = i_pParent;

	m_pM3D = 0;
	m_pM3DDevice = 0;
}

CGraphics::~CGraphics()
{
	while( !m_pStateBlocks.empty() )
		PopStateBlock();

	SAFE_RELEASE( m_pM3DDevice );
	SAFE_RELEASE( m_pM3D );
}

bool CGraphics::bInitialize( const tCreationFlags &i_creationFlags )
{
	// Initialize Muli3D ------------------------------------------------------
	if( FUNC_FAILED( CreateMuli3D( &m_pM3D ) ) )
		return false;

	// Initialize Muli3D device parameters - defaulting to 32 bit colors in fullscreen mode.
	m3ddeviceparameters paramsDevice = { m_pParent->hGetWindowHandle(),
		m_pParent->bGetWindowed(), 32, m_pParent->iGetWindowWidth(),
		m_pParent->iGetWindowHeight() };

	if( FUNC_FAILED( m_pM3D->CreateDevice( &m_pM3DDevice, &paramsDevice ) ) )
	{
		SAFE_RELEASE( m_pM3D );
		return false;
	}

	// Create subsystems ------------------------------------------------------
	PushStateBlock(); // push the root stateblock

	return true;
}

void CGraphics::PushStateBlock()
{
	CStateBlock *pNewStateBlock = new CStateBlock( this );
	m_pStateBlocks.push( pNewStateBlock );
}

void CGraphics::PopStateBlock()
{
	if( !m_pStateBlocks.empty() )
	{
		SAFE_DELETE( m_pStateBlocks.top() );
		m_pStateBlocks.pop();
	}
}

result CGraphics::SetRenderState( m3drenderstate i_RenderState, uint32 i_iValue )
{
	if( m_pStateBlocks.empty() ) return e_unknown;
	return m_pStateBlocks.top()->SetRenderState( i_RenderState, i_iValue );
}

result CGraphics::SetVertexFormat( CMuli3DVertexFormat *i_pVertexFormat )
{
	if( m_pStateBlocks.empty() ) return e_unknown;
	return m_pStateBlocks.top()->SetVertexFormat( i_pVertexFormat );
}

void CGraphics::SetPrimitiveAssembler( IMuli3DPrimitiveAssembler *i_pPrimitiveAssembler )
{
	if( m_pStateBlocks.empty() ) return;
	m_pStateBlocks.top()->SetPrimitiveAssembler( i_pPrimitiveAssembler );
}

result CGraphics::SetVertexShader( IMuli3DVertexShader *i_pVertexShader )
{
	if( m_pStateBlocks.empty() ) return e_unknown;
	return m_pStateBlocks.top()->SetVertexShader( i_pVertexShader );
}

void CGraphics::SetTriangleShader( IMuli3DTriangleShader *i_pTriangleShader )
{
	if( m_pStateBlocks.empty() ) return;
	m_pStateBlocks.top()->SetTriangleShader( i_pTriangleShader );
}

result CGraphics::SetPixelShader( IMuli3DPixelShader *i_pPixelShader )
{
	if( m_pStateBlocks.empty() ) return e_unknown;
	return m_pStateBlocks.top()->SetPixelShader( i_pPixelShader );
}

result CGraphics::SetIndexBuffer( CMuli3DIndexBuffer *i_pIndexBuffer )
{
	if( m_pStateBlocks.empty() ) return e_unknown;
	return m_pStateBlocks.top()->SetIndexBuffer( i_pIndexBuffer );
}

result CGraphics::SetVertexStream( uint32 i_iStreamNumber, CMuli3DVertexBuffer *i_pVertexBuffer, uint32 i_iOffset, uint32 i_iStride )
{
	if( m_pStateBlocks.empty() ) return e_unknown;
	return m_pStateBlocks.top()->SetVertexStream( i_iStreamNumber, i_pVertexBuffer, i_iOffset, i_iStride );
}

result CGraphics::SetTexture( uint32 i_iSamplerNumber, IMuli3DBaseTexture *i_pTexture )
{
	if( m_pStateBlocks.empty() ) return e_unknown;
	return m_pStateBlocks.top()->SetTexture( i_iSamplerNumber, i_pTexture );
}

result CGraphics::SetTextureSamplerState( uint32 i_iSamplerNumber, m3dtexturesamplerstate i_TextureSamplerState, uint32 i_iState )
{
	if( m_pStateBlocks.empty() ) return e_unknown;
	return m_pStateBlocks.top()->SetTextureSamplerState( i_iSamplerNumber, i_TextureSamplerState, i_iState );
}

void CGraphics::SetRenderTarget( CMuli3DRenderTarget *i_pRenderTarget )
{
	if( m_pStateBlocks.empty() ) return;
	m_pStateBlocks.top()->SetRenderTarget( i_pRenderTarget );
}

result CGraphics::SetScissorRect( const m3drect &i_ScissorRect )
{
	if( m_pStateBlocks.empty() ) return e_unknown;
	return m_pStateBlocks.top()->SetScissorRect( i_ScissorRect );
}
