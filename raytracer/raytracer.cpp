
#include "raytracer.h"
#include "../libappframework/include/application.h"
#include "../libappframework/include/scene.h"
#include "../libappframework/include/texture.h"
#include "mycamera.h"

class CRaytracerVS : public IMuli3DVertexShader
{
public:
	void Execute( const shaderreg *i_pInput, vector4 &o_vPosition, shaderreg *o_pOutput )
	{
		o_vPosition = i_pInput[0]; o_vPosition.z = 0;
		
		// rotate the direction vector to camera's world-space
		vVector3TransformNormal( *(vector3 *)&o_pOutput[0], (vector3)i_pInput[0], matGetMatrix( 0 ) );
	}

	m3dshaderregtype GetOutputRegisters( uint32 i_iRegister )
	{
		switch( i_iRegister )
		{
		case 0: return m3dsrt_vector3;
		default: return m3dsrt_unused;
		}
	}
};

class CRaytracerPS : public IMuli3DPixelShader
{
private:
	inline float32 fTrace( const vector3 &i_vRayOrigin, const vector3 &i_vRayDir, vector4 *o_pColor, uint32 i_iLevel = 0 )
	{
		float32 fCollisionDistance = FLT_MAX;
		if( i_iLevel >= MAX_RECURSION )
		{
			if( o_pColor ) *o_pColor = vector4( 0, 0, 0, 0 );
			return fCollisionDistance;
		}

		// Trace spheres ------------------------------------------------------
		int32 iCollsionSphere = -1;
		vector3 vCollisionPoint, vCollisionNormal;

		const float32 fSphereUStep = 1.0f / (float32)MAX_SPHERES;
		float32 fSphereU = fSphereUStep * 0.5f;
		const uint32 iNumSpheres = ftol( fGetFloat( 0 ) );
		for( uint32 iSphere = 0; iSphere < iNumSpheres; ++iSphere, fSphereU += fSphereUStep )
		{
			vector4 vSphereData; SampleTexture( vSphereData, 0, fSphereU, 0 );
			const vector3 vSphereOrigin( vSphereData.r, vSphereData.g, vSphereData.b );
			const float32 fSphereRadius = vSphereData.a;

			const vector3 vDiff = vSphereOrigin - i_vRayOrigin;
			const float32 fV = fVector3Dot( vDiff, i_vRayDir );

			float32 fDist = fSphereRadius * fSphereRadius + fV * fV - fVector3Dot( vDiff, vDiff );
			if( fDist < 0.0f )
				continue;

			fDist = fV - sqrtf( fDist );
			if( fDist >= 0.0f )
			{
				// collision with sphere
				if( fDist < fCollisionDistance )
				{
					fCollisionDistance = fDist;
					iCollsionSphere = iSphere;

					vCollisionPoint = i_vRayOrigin + i_vRayDir * fCollisionDistance;
					vCollisionNormal = vCollisionPoint - vSphereOrigin;
				}
			}
		}

		if( iCollsionSphere == -1 ) // no collision
		{
			if( o_pColor ) *o_pColor = vector4( 0, 0, 0, 0 );
			return fCollisionDistance;
		}
		
		if( o_pColor )
		{
			const vector3 vViewDir = -i_vRayDir;
			vCollisionNormal.normalize();

			const float32 fSphereU = fSphereUStep * ( 0.5f + (float32)iCollsionSphere );
			vector4 vSphereColor; SampleTexture( vSphereColor, 0, fSphereU, 1 );
			if( vSphereColor.a < c_iMaxTextureSamplers ) // check if a texture is associated with this sphere
			{
				const float32 fPhi = atan2f( vCollisionNormal.z, vCollisionNormal.x );
				float32 fU = -fPhi / (2.0f * M3D_PI); if( fPhi >= 0.0f ) fU += 1.0f;
				const float32 fV = asinf( vCollisionNormal.y ) / M3D_PI + 0.5f;

				const uint32 iSamplerIndex = ftol( vSphereColor.a );
				SampleTexture( vSphereColor, iSamplerIndex, 1.0f - fU, 1.0f - fV );
			}

			// Initialize output-color with reflection color ...
			const float32 fEpsilon = 0.01f; // offset
			const float32 fViewDirDotNormal = fVector3Dot( vCollisionNormal, vViewDir );
			const vector3 vReflection = ( vCollisionNormal * ( 2 * fViewDirDotNormal ) - vViewDir ).normalize();
			fTrace( vCollisionPoint + vReflection * fEpsilon, vReflection, o_pColor, i_iLevel + 1 );

			// Compute lighting ...
			const float32 fLightUStep = 1.0f / (float32)MAX_LIGHTS;
			float32 fLightU = fLightUStep * 0.5f;
			const uint32 iNumLights = ftol( fGetFloat( 1 ) );
			for( uint32 iLight = 0; iLight < iNumLights; ++iLight, fLightU += fLightUStep )
			{
				vector4 vLightColor; SampleTexture( vLightColor, 1, fLightU, 0 );
				const vector3 vLightPos = vLightColor;
				SampleTexture( vLightColor, 1, fLightU, 1 );

				vector3 vLightDir = vLightPos - vCollisionPoint;
				const float32 fDistToLight = vLightDir.length(); vLightDir.normalize();
				
				// calc phong-lighting
				*o_pColor += vSphereColor * vGetVector( 1 ); // ambient light

				const float32 fDiffuse = fVector3Dot( vCollisionNormal, vLightDir );
				if( fDiffuse > 0.0f )
				{
					// check shadow
					if( fTrace( vCollisionPoint + vLightDir * fEpsilon, vLightDir, 0 ) < fDistToLight )
						continue;

					float32 fSpecular = 0.0f;
					if( fDiffuse > 0.0f )
					{
						const float32 fLightDirDotNormal = fVector3Dot( vCollisionNormal, vLightDir );
						const vector3 vReflection = ( vCollisionNormal * ( 2 * fLightDirDotNormal ) - vLightDir ).normalize();
						fSpecular = powf( fSaturate( fVector3Dot( vReflection, vViewDir ) ), 16 );
					}

					*o_pColor += vSphereColor * vLightColor * fDiffuse + vLightColor * fSpecular;
				}
			}
		}
		
		return fCollisionDistance;
	}

public:
	bool bMightKillPixels() { return false; }
	bool bExecute( const shaderreg *i_pInput, vector4 &io_vColor, float32 &io_fDepth )
	{
		const vector3 vRayOrigin = vGetVector( 0 );
		const vector3 vRayDir = ((vector3)i_pInput[0]).normalize();
		fTrace( vRayOrigin, vRayDir, &io_vColor );
		return true;
	}
};

m3dvertexelement VertexDeclaration[] =
{
	M3DVERTEXFORMATDECL( 0, m3dvet_vector3, 0 )
};

CRaytracer::CRaytracer( class CScene *i_pParent )
{
	m_pParent = i_pParent;

	m_pVertexFormat = 0;
	m_pVertexBuffer = 0;
	m_pVertexShader = 0;
	m_pPixelShader = 0;

	m_iNumSpheres = 0;
	m_pSphereData = 0;
	memset( m_hSphereTextures, 0, sizeof( HRESOURCE ) * MAX_SPHERES );
	m_iNumLights = 0;
	m_pLightData = 0;
}

CRaytracer::~CRaytracer()
{
	SAFE_RELEASE( m_pLightData );
	for( uint32 i = 0; i < m_iNumSpheres; i++ )
		m_pParent->pGetParent()->pGetResManager()->ReleaseResource( m_hSphereTextures[i] );
	SAFE_RELEASE( m_pSphereData );

	SAFE_RELEASE( m_pPixelShader );
	SAFE_RELEASE( m_pVertexShader );
	SAFE_RELEASE( m_pVertexBuffer );
	SAFE_RELEASE( m_pVertexFormat );
}

bool CRaytracer::bInitialize( float32 i_fFOVAngle )
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

	const float32 fLocalDepth = 1.0f / tanf( i_fFOVAngle * 0.5f );
	pDest->vDirection = vector3( -1, -1, fLocalDepth ); pDest++;
	pDest->vDirection = vector3( 1, -1, fLocalDepth ); pDest++;
	pDest->vDirection = vector3( -1, 1, fLocalDepth ); pDest++;
	pDest->vDirection = vector3( 1, 1, fLocalDepth );

	m_pVertexShader = new CRaytracerVS;
	m_pPixelShader = new CRaytracerPS;

	// Create the sphere and light textures ...
	if( FUNC_FAILED( pM3DDevice->CreateTexture( &m_pSphereData, MAX_SPHERES, 2, 1, m3dfmt_r32g32b32a32f ) ) )
		return false;
	if( FUNC_FAILED( pM3DDevice->CreateTexture( &m_pLightData, MAX_LIGHTS, 2, 1, m3dfmt_r32g32b32a32f ) ) )
		return false;

	return true;
}

bool CRaytracer::bAddSphere( const vector3 &i_vOrigin, float32 i_fRadius, const vector4 &i_vColor, HRESOURCE i_hTexture )
{
	if( m_iNumSpheres >= MAX_SPHERES )
		return false;

	m_hSphereTextures[m_iNumSpheres] = i_hTexture;

	uint32 iSamplerIndex = 2; // 2 is first available texture stage for spheres
	if( i_hTexture )
	{
		for( uint32 i = 0; i < m_iNumSpheres; ++i )
		{
			if( m_hSphereTextures[i] )
				++iSamplerIndex;
		}

		if( iSamplerIndex > c_iMaxTextureSamplers )
			iSamplerIndex = c_iMaxTextureSamplers;
	}
	else
		iSamplerIndex = c_iMaxTextureSamplers;

	vector4 *pData = 0;
	if( FUNC_FAILED( m_pSphereData->LockRect( 0, (void **)&pData, 0 ) ) )
		return false;

	pData[m_iNumSpheres] = vector4( i_vOrigin.x, i_vOrigin.y, i_vOrigin.z, i_fRadius );
	pData[m_iNumSpheres + MAX_SPHERES] = vector4( i_vColor.r, i_vColor.g, i_vColor.b, (float32)iSamplerIndex ); // encode sampler index in alpha component of sphere color.

	m_pSphereData->UnlockRect( 0 );

	m_iNumSpheres++;

	return true;
}

bool CRaytracer::bAddLight( const vector3 &i_vOrigin, const vector4 &i_vColor )
{
	if( m_iNumLights >= MAX_LIGHTS )
		return false;

	vector4 *pData = 0;
	if( FUNC_FAILED( m_pLightData->LockRect( 0, (void **)&pData, 0 ) ) )
		return false;

	pData[m_iNumLights] = i_vOrigin;
	pData[m_iNumLights + MAX_LIGHTS] = i_vColor;

	m_pLightData->UnlockRect( 0 );

	m_iNumLights++;

	return true;
}

bool CRaytracer::bFrameMove()
{
	return false;
}

void CRaytracer::Render( uint32 i_iPass )
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

	// set spheres
	m_pPixelShader->SetFloat( 0, (float32)m_iNumSpheres );
	pGraphics->SetTexture( 0, m_pSphereData );
	pGraphics->SetTextureSamplerState( 0, m3dtss_minfilter, m3dtf_point );
	pGraphics->SetTextureSamplerState( 0, m3dtss_magfilter, m3dtf_point );
	pGraphics->SetTextureSamplerState( 0, m3dtss_addressu, m3dta_clamp );
	pGraphics->SetTextureSamplerState( 0, m3dtss_addressv, m3dta_clamp );

	// set lights
	m_pPixelShader->SetFloat( 1, (float32)m_iNumLights );
	pGraphics->SetTexture( 1, m_pLightData );
	pGraphics->SetTextureSamplerState( 1, m3dtss_minfilter, m3dtf_point );
	pGraphics->SetTextureSamplerState( 1, m3dtss_magfilter, m3dtf_point );
	pGraphics->SetTextureSamplerState( 1, m3dtss_addressu, m3dta_clamp );
	pGraphics->SetTextureSamplerState( 1, m3dtss_addressv, m3dta_clamp );

	// set sphere-textures
	uint32 iSamplerIndex = 2;
	CResManager *pResManager = m_pParent->pGetParent()->pGetResManager();
	for( uint32 i = 0; i < m_iNumSpheres && iSamplerIndex < c_iMaxTextureSamplers; ++i )
	{
		if( !m_hSphereTextures[i] )
			continue;

		CTexture *pTexture = (CTexture *)pResManager->pGetResource( m_hSphereTextures[i] );
		pGraphics->SetTexture( iSamplerIndex, pTexture->pGetTexture() );
		pGraphics->SetTextureSamplerState( iSamplerIndex, m3dtss_minfilter, m3dtf_linear );
		pGraphics->SetTextureSamplerState( iSamplerIndex, m3dtss_magfilter, m3dtf_linear );
		pGraphics->SetTextureSamplerState( iSamplerIndex, m3dtss_addressu, m3dta_wrap );
		pGraphics->SetTextureSamplerState( iSamplerIndex, m3dtss_addressv, m3dta_wrap );
		++iSamplerIndex;
	}

	CCamera *pCurCamera = pGraphics->pGetCurCamera();
	matrix44 matInvView; matMatrix44Transpose( matInvView, pCurCamera->matGetViewMatrix() );
	m_pVertexShader->SetMatrix( 0, matInvView ); // inverse view matrix
	m_pPixelShader->SetVector( 0, pCurCamera->vGetPosition() ); // camera position
	m_pPixelShader->SetVector( 1, vector4( 0.15f, 0.15f, 0.15f, 1 ) ); // ambient light

	pGraphics->SetRenderState( m3drs_cullmode, m3dcull_none );
	pGraphics->pGetM3DDevice()->DrawPrimitive( m3dpt_trianglestrip, 0, 2 );
}
