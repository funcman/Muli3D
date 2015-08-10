
#include "fractal.h"
#include "../libappframework/include/application.h"
#include "../libappframework/include/scene.h"
#include "../libappframework/include/texture.h"
#include "mycamera.h"

class CFractalVS : public IMuli3DVertexShader
{
public:
	void Execute( const shaderreg *i_pInput, vector4 &o_vPosition, shaderreg *o_pOutput )
	{
		o_vPosition = i_pInput[0];

		// lerp between two views
		const vector2 vViewA = vector2( -0.5f, 0 ) + i_pInput[1] * 2.0f;
		const vector2 vViewB = i_pInput[1] * 0.01f + vector2( -0.579f, 0.65f );
		//const vector2 vViewB = i_pInput[1] * 0.001f + vector2( -0.10133f, 0.95642f );

		vVector2Lerp( *(vector2 *)&o_pOutput[0], vViewA, vViewB, ( 1.0f + cosf( 0.25f * fGetFloat( 0 ) ) ) * 0.5f );
	}

	m3dshaderregtype GetOutputRegisters( uint32 i_iRegister )
	{
		switch( i_iRegister )
		{
		case 0: return m3dsrt_vector2;
		default: return m3dsrt_unused;
		}
	}
};

class CFractalPS : public IMuli3DPixelShader
{
public:
	bool bMightKillPixels() { return false; }
	bool bExecute( const shaderreg *i_pInput, vector4 &io_vColor, float32 &io_fDepth )
	{
		const vector2 &vConst = i_pInput[0];

		vector2 vZ0 = i_pInput[0], vZ1;
		for( uint32 i = 0; i < (MANDELBROT_ITERATIONS / 2); ++i )
		{
			// ping
			vZ1.x = vZ0.x * vZ0.x - vZ0.y * vZ0.y + vConst.x;
			vZ1.y = 2.0f * vZ0.x * vZ0.y + vConst.y;
			// pong
			vZ0.x = vZ1.x * vZ1.x - vZ1.y * vZ1.y + vConst.x;
			vZ0.y = 2.0f * vZ1.x * vZ1.y + vConst.y;

			if( vZ0.lengthsq() >= 4.0f )
				break;
		}

		const float32 fColor = 1.0f - powf( 2.0f, -2.0f * vZ0.lengthsq() );
		SampleTexture( io_vColor, 0, fColor, 0 );

		return true;
	}
};

m3dvertexelement VertexDeclaration[] =
{
	M3DVERTEXFORMATDECL( 0, m3dvet_vector3, 0 ),
	M3DVERTEXFORMATDECL( 0, m3dvet_vector2, 1 ),
};

CFractal::CFractal( class CScene *i_pParent )
{
	m_pParent = i_pParent;

	m_pVertexFormat = 0;
	m_pVertexBuffer = 0;
	m_pVertexShader = 0;
	m_pPixelShader = 0;
	
	m_hColormap = 0;
}

CFractal::~CFractal()
{
	m_pParent->pGetParent()->pGetResManager()->ReleaseResource( m_hColormap );

	SAFE_RELEASE( m_pPixelShader );
	SAFE_RELEASE( m_pVertexShader );
	SAFE_RELEASE( m_pVertexBuffer );
	SAFE_RELEASE( m_pVertexFormat );
}

bool CFractal::bInitialize()
{
	CGraphics *pGraphics = m_pParent->pGetParent()->pGetGraphics();
	CMuli3DDevice *pM3DDevice = pGraphics->pGetM3DDevice();

	if( FUNC_FAILED( pM3DDevice->CreateVertexFormat( &m_pVertexFormat, VertexDeclaration, sizeof( VertexDeclaration ) ) ) )
		return false;

	if( FUNC_FAILED( pM3DDevice->CreateVertexBuffer( &m_pVertexBuffer, sizeof( vertexformat ) * 4 ) ) )
		return false;

	vertexformat *pDest = 0;
	if( FUNC_FAILED( m_pVertexBuffer->GetPointer( 0, (void **)&pDest ) ) )
		return false;

	pDest->vPosition = vector3( 1, -1, 0 );
	pDest->vTexCoord0 = vector2( 1, -1 );
	pDest++;

	pDest->vPosition = vector3( -1, -1, 0 );
	pDest->vTexCoord0 = vector2( -1, -1 );
	pDest++;

	pDest->vPosition = vector3( 1, 1, 0 );
	pDest->vTexCoord0 = vector2( 1, 1 );
	pDest++;

	pDest->vPosition = vector3( -1, 1, 0 );
	pDest->vTexCoord0 = vector2( -1, 1 );
	pDest++;

	m_pVertexShader = new CFractalVS;
	m_pPixelShader = new CFractalPS;

	// Load the colormap ...
	CResManager *pResManager = m_pParent->pGetParent()->pGetResManager();
	m_hColormap = pResManager->hLoadResource( "mandelbrot.png" );
	if( !m_hColormap )
		return false;

	return true;
}

bool CFractal::bFrameMove()
{
	return false;
}

void CFractal::Render( uint32 i_iPass )
{
	switch( i_iPass )
	{
	case ePass_Default: break;
	}

	CGraphics *pGraphics = m_pParent->pGetParent()->pGetGraphics();

	pGraphics->SetVertexFormat( m_pVertexFormat );
	pGraphics->SetVertexStream( 0, m_pVertexBuffer, 0, sizeof( vertexformat ) );
	pGraphics->SetVertexShader( m_pVertexShader );
	pGraphics->SetPixelShader( m_pPixelShader );

	CResManager *pResManager = m_pParent->pGetParent()->pGetResManager();
	CTexture *pColormap = (CTexture *)pResManager->pGetResource( m_hColormap );
	pGraphics->SetTexture( 0, pColormap->pGetTexture() );
	
	m_pVertexShader->SetFloat( 0, pGraphics->pGetParent()->fGetElapsedTime() );

	pGraphics->SetTextureSamplerState( 0, m3dtss_addressu, m3dta_clamp );

	pGraphics->pGetM3DDevice()->DrawPrimitive( m3dpt_trianglestrip, 0, 2 );
}
