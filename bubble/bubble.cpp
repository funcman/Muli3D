
#include "bubble.h"
#include "../libappframework/include/application.h"
#include "../libappframework/include/scene.h"
#include "mycamera.h"

class CBubbleVS : public IMuli3DVertexShader
{
public:
	void Execute( const shaderreg *i_pInput, vector4 &o_vPosition, shaderreg *o_pOutput )
	{
		// const vector4 vWaveOffset( 0.0f, 0.0f, 0.0f, 0.0f );
		const vector4 vWaveSpeed( 0.6f, 0.7f, 1.2f, 1.4f );
		const vector4 vWaveDirX( 0.0f, 2.0f, 0.0f, 4.0f );
		const vector4 vWaveDirY( 2.0f, 0.0f, 4.0f, 0.0f );
		const vector4 vWaveHeight( 0.25f, 0.25f, 0.125f, 0.125f );
		const float32 fNormalWarpScale = 0.01f;

		vector4 vWaves = vWaveDirX * i_pInput[1].x + vWaveDirY * i_pInput[1].y;
		vWaves += vWaveSpeed * fGetFloat( 0 ); //+ vWaveOffset;

		// calculate fractions of warp-vector components and scale to radians.
		vWaves.x -= (int32)vWaves.x + 0.5f; vWaves.y -= (int32)vWaves.y + 0.5f;
		vWaves.z -= (int32)vWaves.z + 0.5f; vWaves.w -= (int32)vWaves.w + 0.5f;
		vWaves *= 2.0f * M3D_PI;
		
		const vector4 vSinWaves( sinf( vWaves.x ), sinf( vWaves.y ), sinf( vWaves.z ), sinf( vWaves.w ) );
		const vector4 vCosWaves( cosf( vWaves.x ), cosf( vWaves.y ), cosf( vWaves.z ), cosf( vWaves.w ) );

		// apply deformation in direction of normal using sine-waves
		vector3 vNormal = i_pInput[0]; vNormal.normalize();
		const vector4 vPosition = i_pInput[0] + vNormal * fVector4Dot( vSinWaves, vWaveHeight );
		o_vPosition = vPosition * matGetMatrix( m3dsc_wvpmatrix );

		// pass texcoord to pixel shader
		o_pOutput[0] = i_pInput[1];

		const vector3 vTangent( -vNormal.z, 0, vNormal.x ); /* vTangent.x = vNormal % vector3( 0, 0, -1 ); vTangent.y = 0; vTangent.z = vNormal % vector3( 1, 0, 0 ); */
		vector3 vBinormal; vVector3Cross( vBinormal, vNormal, vTangent );

		const float32 fNormalWarpBinormal = fVector4Dot( -vCosWaves, vWaveDirX );
		const float32 fNormalWarpTangent = fVector4Dot( -vCosWaves, vWaveDirY );
		vector3 vWarpedNormal = vBinormal * fNormalWarpBinormal + vTangent * fNormalWarpTangent;
		vWarpedNormal = vWarpedNormal * fNormalWarpScale + vNormal;
		vWarpedNormal.normalize();
		vVector3TransformNormal( vWarpedNormal, vWarpedNormal, matGetMatrix( m3dsc_worldmatrix ) );

		const vector3 vWorldPos = vPosition * matGetMatrix( m3dsc_worldmatrix );
		const vector3 vViewDir = ( (vector3)vGetVector( 0 ) - vWorldPos ).normalize();
		const float32 fViewDotNormal = fVector3Dot( vWarpedNormal, vViewDir );
	
		o_pOutput[1] = vWarpedNormal * ( 2.0f * fViewDotNormal ) - vViewDir;
		o_pOutput[2].x = fViewDotNormal;
	}

	m3dshaderregtype GetOutputRegisters( uint32 i_iRegister )
	{
		switch( i_iRegister )
		{
		case 0: return m3dsrt_vector2;	// texcoord
		case 1: return m3dsrt_vector3;	// reflection
		case 2: return m3dsrt_float32;	// n * v
		default: return m3dsrt_unused;
		}
	}
};

class CBubblePS : public IMuli3DPixelShader
{
public:
	bool bMightKillPixels() { return false; }
	bool bExecute( const shaderreg *i_pInput, vector4 &io_vColor, float32 &io_fDepth )
	{
		#ifdef VISUALIZE_RATE_OF_CHANGE
		vector4 vDdx, vDdy; GetDerivatives( 0, vDdx, vDdy );
		io_vColor.r = (*(vector2 *)&vDdx).length() * 100;
		io_vColor.g = (*(vector2 *)&vDdy).length() * 100;
		io_vColor.b = 0;
		io_vColor.a = 1;
		return true;
		#endif

		vector4 vRainbowFilm;
		SampleTexture( vRainbowFilm, 0, i_pInput[0].x, i_pInput[0].y, 0.0f );

		const float32 fFresnel = 1.0f - fabsf( i_pInput[2].x );

		vector4 vReflectionEnv;
		SampleTexture( vReflectionEnv, 1, i_pInput[1].x, i_pInput[1].y, i_pInput[1].z );

		float32 fAlpha = fSaturate( 4.0f * ( vReflectionEnv.a * vReflectionEnv.a - 0.75f ) );
		const vector4 vBaseEnvColor = ( vRainbowFilm * vReflectionEnv * 2.0f ).saturate();

		vector4 vColor;
		vVector4Lerp( vColor, vBaseEnvColor, vReflectionEnv, fAlpha );

		fAlpha += 0.6f * fFresnel + 0.1f;

		vVector4Lerp( io_vColor, io_vColor, vColor, fSaturate( fAlpha ) );
		return true;
	}
};

m3dvertexelement VertexDeclaration[] =
{
	M3DVERTEXFORMATDECL( 0, m3dvet_vector3, 0 ),
	M3DVERTEXFORMATDECL( 0, m3dvet_vector2, 1 )
};

CBubble::CBubble( class CScene *i_pParent )
{
	m_pParent = i_pParent;

	m_pVertexFormat = 0;
	m_pVertexBuffer = 0;
	m_pIndexBuffer = 0;
	m_pVertexShader = 0;
	m_pPixelShader = 0;

	m_iNumVertices = 0;
	m_iNumPrimitives = 0;

	m_hRainbowFilm = 0;
	m_hEnvironment = 0;
}

CBubble::~CBubble()
{
	m_pParent->pGetParent()->pGetResManager()->ReleaseResource( m_hEnvironment );
	m_pParent->pGetParent()->pGetResManager()->ReleaseResource( m_hRainbowFilm );

	SAFE_RELEASE( m_pPixelShader );
	SAFE_RELEASE( m_pVertexShader );
	SAFE_RELEASE( m_pIndexBuffer );
	SAFE_RELEASE( m_pVertexBuffer );
	SAFE_RELEASE( m_pVertexFormat );
}

bool CBubble::bInitialize( float32 i_fRadius, uint32 i_iStacks, uint32 i_iSlices )
{
	CGraphics *pGraphics = m_pParent->pGetParent()->pGetGraphics();
	CMuli3DDevice *pM3DDevice = pGraphics->pGetM3DDevice();

	if( FUNC_FAILED( pM3DDevice->CreateVertexFormat( &m_pVertexFormat, VertexDeclaration, sizeof( VertexDeclaration ) ) ) )
		return false;

	// Construct a sphere
	m_iNumVertices = i_iStacks * i_iSlices * 4;
	m_iNumPrimitives = i_iStacks * i_iSlices * 2;

	if( FUNC_FAILED( pM3DDevice->CreateVertexBuffer( &m_pVertexBuffer, sizeof( vertexformat ) * m_iNumVertices ) ) )
		return false;

	if( FUNC_FAILED( pM3DDevice->CreateIndexBuffer( &m_pIndexBuffer, sizeof( uint16 ) * m_iNumPrimitives * 3, m3dfmt_index16 ) ) )
		return false;

	vertexformat *pDestVertices = 0;
	if( FUNC_FAILED( m_pVertexBuffer->GetPointer( 0, (void **)&pDestVertices ) ) )
		return false;

	uint16 *pDestIndices = 0;
	if( FUNC_FAILED( m_pIndexBuffer->GetPointer( 0, (void **)&pDestIndices ) ) )
		return false;

	const float32 fStepV = 1.0f / (float32)i_iStacks;
	const float32 fStepU = 1.0f / (float32)i_iSlices;

	uint32 iCurVertex = 0;

	float32 fV = 0.0f;
	for( uint32 i = 0; i < i_iStacks; ++i, fV += fStepV )
	{
		float32 fNextV = fV + fStepV;

		float32 fU = 0.0f;
		for( uint32 j = 0; j < i_iSlices; ++j, fU += fStepU )
		{
			float32 fNextU = fU + fStepU;
			
			// create a quad
			// 0 -- 1
			// |    |
			// 2 -- 3

			float32 x[4], y[4], z[4];

			x[0] = i_fRadius * sinf( fV * M3D_PI ) * cosf( fU * 2.0f * M3D_PI );
			z[0] = i_fRadius * sinf( fV * M3D_PI ) * sinf( fU * 2.0f * M3D_PI );
			y[0] = i_fRadius * cosf( fV * M3D_PI );

			x[1] = i_fRadius * sinf( fV * M3D_PI ) * cosf( fNextU * 2.0f * M3D_PI );
			z[1] = i_fRadius * sinf( fV * M3D_PI ) * sinf( fNextU * 2.0f * M3D_PI );
			y[1] = i_fRadius * cosf( fV * M3D_PI );

			x[2] = i_fRadius * sinf( fNextV * M3D_PI ) * cosf( fU * 2.0f * M3D_PI );
			z[2] = i_fRadius * sinf( fNextV * M3D_PI ) * sinf( fU * 2.0f * M3D_PI );
			y[2] = i_fRadius * cosf( fNextV * M3D_PI );

			x[3] = i_fRadius * sinf( fNextV * M3D_PI ) * cosf( fNextU * 2.0f * M3D_PI );
			z[3] = i_fRadius * sinf( fNextV * M3D_PI ) * sinf( fNextU * 2.0f * M3D_PI );
			y[3] = i_fRadius * cosf( fNextV * M3D_PI );

			pDestVertices->vPosition = vector3( x[0], y[0], z[0] );
			pDestVertices->vTex = vector2( fU, fV );
			pDestVertices++;

			pDestVertices->vPosition = vector3( x[1], y[1], z[1] );
			pDestVertices->vTex = vector2( fNextU, fV );
			pDestVertices++;

			pDestVertices->vPosition = vector3( x[2], y[2], z[2] );
			pDestVertices->vTex = vector2( fU, fNextV );
			pDestVertices++;

			pDestVertices->vPosition = vector3( x[3], y[3], z[3] );
			pDestVertices->vTex = vector2( fNextU, fNextV );
			pDestVertices++;

			*pDestIndices++ = iCurVertex;
			*pDestIndices++ = iCurVertex + 1;
			*pDestIndices++ = iCurVertex + 2;

			*pDestIndices++ = iCurVertex + 1;
			*pDestIndices++ = iCurVertex + 3;
			*pDestIndices++ = iCurVertex + 2;

			iCurVertex += 4;
		}
	}

	m_pVertexShader = new CBubbleVS;
	m_pPixelShader = new CBubblePS;

	// Load textures ...
	CResManager *pResManager = m_pParent->pGetParent()->pGetResManager();
	m_hRainbowFilm = pResManager->hLoadResource( "rainbowfilm_smooth.png" );
	if( !m_hRainbowFilm )
		return false;

	m_hEnvironment = pResManager->hLoadResource( "room.cube" );
	if( !m_hEnvironment )
		return false;

	return true;
}

bool CBubble::bFrameMove()
{
	return false;
}

void CBubble::Render( uint32 i_iPass )
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

	CResManager *pResManager = m_pParent->pGetParent()->pGetResManager();
	CTexture *pRainbowFilm = (CTexture *)pResManager->pGetResource( m_hRainbowFilm );
	pGraphics->SetTexture( 0, pRainbowFilm->pGetTexture() );
	
	CTexture *pEnvironment = (CTexture *)pResManager->pGetResource( m_hEnvironment );
	pGraphics->SetTexture( 1, pEnvironment->pGetTexture() );

	m_pVertexShader->SetFloat( 0, pGraphics->pGetParent()->fGetElapsedTime() );
	m_pVertexShader->SetVector( 0, pCurCamera->vGetPosition() );

	pGraphics->SetVertexFormat( m_pVertexFormat );
	pGraphics->SetVertexStream( 0, m_pVertexBuffer, 0, sizeof( vertexformat ) );
	pGraphics->SetIndexBuffer( m_pIndexBuffer );
	pGraphics->SetVertexShader( m_pVertexShader );
	pGraphics->SetPixelShader( m_pPixelShader );

	pGraphics->SetRenderState( m3drs_cullmode, m3dcull_cw );
	pGraphics->pGetM3DDevice()->DrawIndexedPrimitive( m3dpt_trianglelist,
		0, 0, m_iNumVertices, 0, m_iNumPrimitives );

	pGraphics->SetRenderState( m3drs_cullmode, m3dcull_ccw );
	pGraphics->pGetM3DDevice()->DrawIndexedPrimitive( m3dpt_trianglelist,
		0, 0, m_iNumVertices, 0, m_iNumPrimitives );
}
