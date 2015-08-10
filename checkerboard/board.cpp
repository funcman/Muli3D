
#include "board.h"
#include "../libappframework/include/application.h"
#include "../libappframework/include/scene.h"
#include "mycamera.h"

class CBoardVS : public IMuli3DVertexShader
{
public:
	void Execute( const shaderreg *i_pInput, vector4 &o_vPosition, shaderreg *o_pOutput )
	{
		// transform position
		o_vPosition = i_pInput[0] * matGetMatrix( m3dsc_wvpmatrix );

		// pass texcoord to pixelshader
		o_pOutput[0] = i_pInput[1];
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

class CBoardPS : public IMuli3DPixelShader
{
private:
	inline const float32 maxf( const float32 i_fValA, const float32 i_fValB ) const
	{
		return i_fValA > i_fValB ? i_fValA : i_fValB;
	}

	inline const float32 floorf( const float32 i_fVal ) const
	{
		return (float32)ftol( i_fVal );
	}

	inline const float32 fFilterWidth() const
	{
		vector4 vDdx, vDdy; GetDerivatives( 0, vDdx, vDdy );
		const float32 fChangeX = (*(vector2 *)&vDdx).length();
		const float32 fChangeY = (*(vector2 *)&vDdy).length();
		return maxf( fChangeX, fChangeY );
	}

	inline const vector2 vBumpInt( const vector2 &i_vIn ) const
	{
		const vector2 vFloorVec( floorf( i_vIn.x * 0.5f ), floorf( i_vIn.y * 0.5f ) );
		return vector2(
			vFloorVec.x + maxf( i_vIn.x - 2.0f * vFloorVec.x - 1.0f, 0.0f ),
			vFloorVec.y + maxf( i_vIn.y - 2.0f * vFloorVec.y - 1.0f, 0.0f ) );
	}

public:
	bool bMightKillPixels() { return false; }
	bool bExecute( const shaderreg *i_pInput, vector4 &io_vColor, float32 &io_fDepth )
	{
		#ifdef ANTIALIAS_BOARD
		
		const float32 fWidth = fFilterWidth();
		const vector2 vStep = vector2( 0.5f * fWidth, 0.5f * fWidth );
		const vector2 vP0 = *(vector2 *)&i_pInput[0] - vStep;
		const vector2 vP1 = *(vector2 *)&i_pInput[0] + vStep;
		const vector2 vInt = ( vBumpInt( vP1 ) - vBumpInt( vP0 ) ) / fWidth;

		const float32 fScale = vInt.x * vInt.y + ( 1.0f - vInt.x ) * ( 1.0f - vInt.y );

		#else
		const float32 fScale = ( fmodf( floorf( i_pInput[0].x ) + floorf( i_pInput[0].y ), 2.0f ) < 1.0f ) ? false : true;
		#endif

		io_vColor = vector4( fScale, fScale, fScale, 1 );
		return true;
	}
};

m3dvertexelement VertexDeclaration[] =
{
	M3DVERTEXFORMATDECL( 0, m3dvet_vector3, 0 ),
	M3DVERTEXFORMATDECL( 0, m3dvet_vector2, 1 ),
};

CBoard::CBoard( class CScene *i_pParent )
{
	m_pParent = i_pParent;

	m_pVertexFormat = 0;
	m_pVertexBuffer = 0;
	m_pVertexShader = 0;
	m_pPixelShader = 0;
}

CBoard::~CBoard()
{
	SAFE_RELEASE( m_pPixelShader );
	SAFE_RELEASE( m_pVertexShader );
	SAFE_RELEASE( m_pVertexBuffer );
	SAFE_RELEASE( m_pVertexFormat );
}

bool CBoard::bInitialize()
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

	uint32 iCheckers = 80;

	pDest->vPosition = vector3( -0.5f, 0.0f, 0.0f );
	pDest->vTexCoord0 = vector2( 0.0f, (float32)iCheckers );
	pDest++;

	pDest->vPosition = vector3( -0.5f, 0.0f, 1.0f );
	pDest->vTexCoord0 = vector2( 0.0f, 0.0f );
	pDest++;

	pDest->vPosition = vector3( 0.5f, 0.0f, 0.0f );
	pDest->vTexCoord0 = vector2( (float32)iCheckers, (float32)iCheckers );
	pDest++;

	pDest->vPosition = vector3( 0.5f, 0.0f, 1.0f );
	pDest->vTexCoord0 = vector2( (float32)iCheckers, 0.0f );
	pDest++;

	m_pVertexShader = new CBoardVS;
	m_pPixelShader = new CBoardPS;

	return true;
}

bool CBoard::bFrameMove()
{
	return false;
}

void CBoard::Render( uint32 i_iPass )
{
	switch( i_iPass )
	{
	case ePass_Default: break;
	}

	CGraphics *pGraphics = m_pParent->pGetParent()->pGetGraphics();

	CCamera *pCurCamera = pGraphics->pGetCurCamera();
	matrix44 matWorld; matMatrix44Identity( matWorld );
	pCurCamera->SetWorldMatrix( matWorld );

	m_pVertexShader->SetMatrix( m3dsc_worldmatrix, pCurCamera->matGetWorldMatrix() );
	m_pVertexShader->SetMatrix( m3dsc_viewmatrix, pCurCamera->matGetViewMatrix() );
	m_pVertexShader->SetMatrix( m3dsc_projectionmatrix, pCurCamera->matGetProjectionMatrix() );
	m_pVertexShader->SetMatrix( m3dsc_wvpmatrix, pCurCamera->matGetWorldMatrix() * pCurCamera->matGetViewMatrix() * pCurCamera->matGetProjectionMatrix() );

	pGraphics->SetVertexFormat( m_pVertexFormat );
	pGraphics->SetVertexStream( 0, m_pVertexBuffer, 0, sizeof( vertexformat ) );
	pGraphics->SetVertexShader( m_pVertexShader );
	pGraphics->SetPixelShader( m_pPixelShader );

	pGraphics->pGetM3DDevice()->DrawPrimitive( m3dpt_trianglestrip, 0, 2 );
}
