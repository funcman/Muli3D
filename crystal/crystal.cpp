
#include "crystal.h"
#include "../libappframework/include/application.h"
#include "../libappframework/include/scene.h"
#include "../libappframework/include/model.h"
#include "mycamera.h"

class CCrystalVS : public IMuli3DVertexShader
{
public:
	void Execute( const shaderreg *i_pInput, vector4 &o_vPosition, shaderreg *o_pOutput )
	{
		// transform position
		o_vPosition = i_pInput[0] * matGetMatrix( m3dsc_wvpmatrix );

		// pass texcoord to pixelshader
		o_pOutput[0] = i_pInput[2];

		// pass view vector in world space to the pixel shader
		const vector3 vWorldPosition = i_pInput[0] * matGetMatrix( m3dsc_worldmatrix );
		o_pOutput[1] = (vector3)vGetVector( 0 ) - vWorldPosition;

		// build transformation matrix to tangent space
		vector3 vNormal; vVector3TransformNormal( vNormal, i_pInput[1], matGetMatrix( m3dsc_worldmatrix ) );
		vector3 vTangent; vVector3TransformNormal( vTangent, i_pInput[3], matGetMatrix( m3dsc_worldmatrix ) );
		vector3 vBinormal; vVector3Cross( vBinormal, vNormal, vTangent );

		// pass transformed normal, tangent and binormal vector to the pixel shader ...
		o_pOutput[2] = vTangent; o_pOutput[3] = vBinormal; o_pOutput[4] = vNormal;
	}

	m3dshaderregtype GetOutputRegisters( uint32 i_iRegister )
	{
		switch( i_iRegister )
		{
		case 0: return m3dsrt_vector2; // base texture coordinates
		case 1: return m3dsrt_vector3; // world space view vector
		case 2: return m3dsrt_vector3; // tangent
		case 3: return m3dsrt_vector3; // binormal
		case 4: return m3dsrt_vector3; // normal
		default: return m3dsrt_unused;
		}
	}
};

class CCrystalPS : public IMuli3DPixelShader
{
public:
	bool bMightKillPixels() { return false; }
	bool bExecute( const shaderreg *i_pInput, vector4 &io_vColor, float32 &io_fDepth )
	{
		// sample texture and normalmap
		vector4 vTexture; SampleTexture( vTexture, 0, i_pInput[0].x, i_pInput[0].y, 0.0f );
		vector4 vNormalmap; SampleTexture( vNormalmap, 1, i_pInput[0].x, i_pInput[0].y, 0.0f );

		// weaken bump map
		vVector4Lerp( vNormalmap, vNormalmap, vector4( 0, 0, 1, 0 ), 0.4f );

		// transform bump-normal to object space ...
		const matrix44 matTangentToWorldSpace( i_pInput[2], i_pInput[3], i_pInput[4], vector4( 0, 0, 0, 1 ) );
		const vector3 vBumpNormal = vector3( vNormalmap.x * 2 - 1, vNormalmap.y * 2 - 1, vNormalmap.z * 2 - 1 );
		vector3 vNormal; vVector3TransformNormal( vNormal, vBumpNormal, matTangentToWorldSpace );
		vNormal.normalize();

		// compute fresnel term and reflection vector
		vector3 vViewDir = i_pInput[1]; vViewDir.normalize();
		const float32 fViewDotNormal = fSaturate( fVector3Dot( vNormal, vViewDir ) );
		vector3 vReflection = vNormal * (2.0f * fViewDotNormal) - vViewDir;
		const float32 fFresnel = (1.0f - fViewDotNormal);// * (1.0f - fViewDotNormal);

		vector4 vEnvironment; SampleTexture( vEnvironment, 2, vReflection.x, vReflection.y, vReflection.z );

		const vector4 vCrystalColor = vEnvironment * fFresnel + vTexture * vGetVector( 0 );
		const float32 fAlpha = fFresnel + 0.5f;
		vVector4Lerp( io_vColor, io_vColor, vCrystalColor, fSaturate( fAlpha ) );
		return true;

	}
};

CCrystal::CCrystal( class CScene *i_pParent )
{
	m_pParent = i_pParent;

	m_pVertexShader = 0;
	m_pPixelShader = 0;

	m_hModel = 0;
	m_hTexture = 0;
	m_hNormalmap = 0;
	m_hEnvironment = 0;
}

CCrystal::~CCrystal()
{
	m_pParent->pGetParent()->pGetResManager()->ReleaseResource( m_hEnvironment );
	m_pParent->pGetParent()->pGetResManager()->ReleaseResource( m_hNormalmap );
	m_pParent->pGetParent()->pGetResManager()->ReleaseResource( m_hTexture );
	m_pParent->pGetParent()->pGetResManager()->ReleaseResource( m_hModel );

	SAFE_RELEASE( m_pPixelShader );
	SAFE_RELEASE( m_pVertexShader );
}

bool CCrystal::bInitialize( string i_sModel, string i_sTexture, string i_sNormalmap )
{
	CGraphics *pGraphics = m_pParent->pGetParent()->pGetGraphics();
	CMuli3DDevice *pM3DDevice = pGraphics->pGetM3DDevice();

	m_pVertexShader = new CCrystalVS;
	m_pPixelShader = new CCrystalPS;

	// Load the model ---------------------------------------------------------
	CResManager *pResManager = m_pParent->pGetParent()->pGetResManager();
	m_hModel = pResManager->hLoadResource( i_sModel );
	if( !m_hModel )
		return false;

	// Load textures ----------------------------------------------------------
	m_hTexture = pResManager->hLoadResource( i_sTexture );
	if( !m_hTexture )
		return false;

	m_hNormalmap = pResManager->hLoadResource( i_sNormalmap );
	if( !m_hNormalmap )
		return false;

	m_hEnvironment = pResManager->hLoadResource( "crystal.cube" );
	if( !m_hEnvironment )
		return false;

	return true;
}

bool CCrystal::bFrameMove()
{
	return false;
}

void CCrystal::Render( uint32 i_iPass )
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
	CTexture *pTexture = (CTexture *)pResManager->pGetResource( m_hTexture );
	pGraphics->SetTexture( 0, pTexture->pGetTexture() );
	
	CTexture *pNormalmap = (CTexture *)pResManager->pGetResource( m_hNormalmap );
	pGraphics->SetTexture( 1, pNormalmap->pGetTexture() );
	
	CTexture *pEnvironment = (CTexture *)pResManager->pGetResource( m_hEnvironment );
	pGraphics->SetTexture( 2, pEnvironment->pGetTexture() );

	m_pVertexShader->SetVector( 0, pCurCamera->vGetPosition() ); // camera position
	m_pPixelShader->SetVector( 0, vector4( 0.5f, 0, 0.5f, 1 ) ); // base color

	CModel *pModel = (CModel *)pResManager->pGetResource( m_hModel );
	pGraphics->SetVertexFormat( pModel->pGetVertexFormat() );
	pGraphics->SetVertexStream( 0, pModel->pGetVertexBuffer(), 0, pModel->iGetStride() );

	pGraphics->SetVertexShader( m_pVertexShader );
	pGraphics->SetPixelShader( m_pPixelShader );

	pGraphics->SetRenderState( m3drs_cullmode, m3dcull_cw );
	pGraphics->pGetM3DDevice()->DrawPrimitive( m3dpt_trianglelist, 0, pModel->iGetNumFaces() );

	pGraphics->SetRenderState( m3drs_cullmode, m3dcull_ccw );
	pGraphics->pGetM3DDevice()->DrawPrimitive( m3dpt_trianglelist, 0, pModel->iGetNumFaces() );
}
