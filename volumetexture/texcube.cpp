
#include "texcube.h"
#include "../libappframework/include/application.h"
#include "../libappframework/include/scene.h"
#include "../libappframework/include/texture.h"
#include "mycamera.h"

class CTexCubeVS : public IMuli3DVertexShader
{
public:
	void Execute( const shaderreg *i_pInput, vector4 &o_vPosition, shaderreg *o_pOutput )
	{
		// transform position
		o_vPosition = i_pInput[0] * matGetMatrix( m3dsc_wvpmatrix );

		// transform texture coords
		vector3 vTexCoords; vVector3TransformNormal( vTexCoords, *(vector3 *)&i_pInput[0], matGetMatrix( 4 ) );
		o_pOutput[0] = vector3( vTexCoords.x * 0.5f + 0.5f, vTexCoords.y * 0.5f + 0.5f, vTexCoords.z * 0.5f + 0.5f ).saturate();
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

class CTexCubePS : public IMuli3DPixelShader
{
public:
	bool bMightKillPixels() { return false; }
	bool bExecute( const shaderreg *i_pInput, vector4 &io_vColor, float32 &io_fDepth )
	{
		vector4 vVolumeColor;
		SampleTexture( vVolumeColor, 0, i_pInput[0].x, i_pInput[0].y, i_pInput[0].z );
		io_vColor += vVolumeColor;
		return true;
	}
};

// ----------------------------------------------------------------------------

class CTexCubeWireVS : public IMuli3DVertexShader
{
public:
	void Execute( const shaderreg *i_pInput, vector4 &o_vPosition, shaderreg *o_pOutput )
	{
		// transform position
		o_vPosition = i_pInput[0] * matGetMatrix( m3dsc_wvpmatrix );
		o_pOutput[0] = i_pInput[0];
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

class CTexCubeWirePS : public IMuli3DPixelShader
{
public:
	bool bMightKillPixels() { return true; }
	bool bExecute( const shaderreg *i_pInput, vector4 &io_vColor, float32 &io_fDepth )
	{
		const float32 fAbs[3] = { fabsf( i_pInput[0].x ),
			fabsf( i_pInput[0].y ), fabsf( i_pInput[0].z ) };

		// hide inner lines
		if( fAbs[0] < 0.98f && fAbs[1] < 0.98f ||
			fAbs[1] < 0.98f && fAbs[2] < 0.98f ||
			fAbs[2] < 0.98f && fAbs[0] < 0.98f )
		{
			return false;
		}

		io_vColor += vector4( 0.2f, 0.2f, 0.2f, 1 );
		return true;
	}
};

// ----------------------------------------------------------------------------

m3dvertexelement VertexDeclaration[] =
{
	M3DVERTEXFORMATDECL( 0, m3dvet_vector3, 0 )
};

CTexCube::CTexCube( class CScene *i_pParent )
{
	m_pParent = i_pParent;

	m_pVertexFormat = 0;

	m_pVertexBuffer = 0;
	m_pIndexBuffer = 0;
	m_pVertexShader = 0;
	m_pPixelShader = 0;
	m_pVolumeTexture = 0;

	m_pVertexBufferWire = 0;
	m_pIndexBufferWire = 0;
	m_pVertexShaderWire = 0;
	m_pPixelShaderWire = 0;

	m_iNumVertices = NUM_SHELLS * (NUM_CELLS + 1) * (NUM_CELLS + 1);
	m_iNumPrimitives = NUM_SHELLS * NUM_CELLS *  NUM_CELLS * 2;

	m_fRotY = 0.0f;
}

CTexCube::~CTexCube()
{
	SAFE_RELEASE( m_pPixelShaderWire );
	SAFE_RELEASE( m_pVertexShaderWire );
	SAFE_RELEASE( m_pIndexBufferWire );
	SAFE_RELEASE( m_pVertexBufferWire );

	SAFE_RELEASE( m_pVolumeTexture );
	SAFE_RELEASE( m_pPixelShader );
	SAFE_RELEASE( m_pVertexShader );
	SAFE_RELEASE( m_pIndexBuffer );
	SAFE_RELEASE( m_pVertexBuffer );

	SAFE_RELEASE( m_pVertexFormat );
}

bool CTexCube::bInitialize()
{
	CGraphics *pGraphics = m_pParent->pGetParent()->pGetGraphics();
	CMuli3DDevice *pM3DDevice = pGraphics->pGetM3DDevice();

	if( FUNC_FAILED( pM3DDevice->CreateVertexFormat( &m_pVertexFormat, VertexDeclaration, sizeof( VertexDeclaration ) ) ) )
		return false;

	// Create the vertexbuffer ------------------------------------------------
	if( FUNC_FAILED( pM3DDevice->CreateVertexBuffer( &m_pVertexBuffer, sizeof( vertexformat ) * m_iNumVertices ) ) )
		return false;

	vertexformat *pDest = 0;
	if( FUNC_FAILED( m_pVertexBuffer->GetPointer( 0, (void **)&pDest ) ) )
		return false;

	const float32 fDeltaXY = 2.0f / (float32)NUM_CELLS;
	const float32 fDeltaZ = 2.0f / (float32)NUM_SHELLS;

	float32 fZ = 1; uint32 iShell;
	for( iShell = 0; iShell < NUM_SHELLS; ++iShell )
	{
		// start at left of shell
		float32 fX = -1;
		for( uint32 iX  = 0; iX <= NUM_CELLS; ++iX )
		{
			// start at bottom of shell
			float32 fY = -1;
			for( uint32 iY = 0; iY <= NUM_CELLS; ++iY )
			{
				pDest->vPosition = vector3( fX, fY, fZ );
				pDest++;

				// jump across cell
				fY += fDeltaXY;
			}

			// jump across cell
			fX += fDeltaXY;
		}

		// Jump out to next shell
		fZ -= fDeltaZ;
	}

	// Create the indexbuffer -------------------------------------------------
	if( FUNC_FAILED( pM3DDevice->CreateIndexBuffer( &m_pIndexBuffer, sizeof( uint16 ) * m_iNumPrimitives * 3, m3dfmt_index16 ) ) )
		return false;

	uint16 *pIndex = 0;
	if( FUNC_FAILED( m_pIndexBuffer->GetPointer( 0, (void **)&pIndex ) ) )
		return false;
	
	for( iShell = 0; iShell < NUM_SHELLS; ++iShell )
	{
		uint32 iBaseIndex = (NUM_CELLS + 1) * (NUM_CELLS + 1) * iShell;
		for( uint32 iX = 0; iX < NUM_CELLS; ++iX )
		{
			for( uint32 iY = 0; iY < NUM_CELLS; ++iY )
			{
				*pIndex++ = ( iX + 0 ) * ( NUM_CELLS + 1 ) + ( iY + 0 ) + iBaseIndex;
				*pIndex++ = ( iX + 0 ) * ( NUM_CELLS + 1 ) + ( iY + 1 ) + iBaseIndex;
				*pIndex++ = ( iX + 1 ) * ( NUM_CELLS + 1 ) + ( iY + 1 ) + iBaseIndex;

				*pIndex++ = ( iX + 0 ) * ( NUM_CELLS + 1 ) + ( iY + 0 ) + iBaseIndex;
				*pIndex++ = ( iX + 1 ) * ( NUM_CELLS + 1 ) + ( iY + 1 ) + iBaseIndex;
				*pIndex++ = ( iX + 1 ) * ( NUM_CELLS + 1 ) + ( iY + 0 ) + iBaseIndex;
			}
		}
	}

	m_pVertexShader = new CTexCubeVS;
	m_pPixelShader = new CTexCubePS;

	// Create the volume texture ----------------------------------------------
	if( FUNC_FAILED( pM3DDevice->CreateVolumeTexture( &m_pVolumeTexture, 32, 32, 32, 1, m3dfmt_r32g32b32f ) ) )
		return false;

	vector3 *pVolume = 0;
	if( FUNC_FAILED( m_pVolumeTexture->LockBox( 0, (void **)&pVolume, 0 ) ) )
		return false;

	#ifdef CUBE
	const float32 fColorScale = 0.2f;
	for( uint32 iW = 0; iW < 32; ++iW )
	{
		const float32 fW = ( (float32)iW ) / 31.0f;
		for( uint32 iV = 0; iV < 32; ++iV )
		{
			const float32 fV = ( (float32)iV ) / 31.0f;
			for( uint32 iU = 0; iU < 32; ++iU )
			{
				const float32 fU = ( (float32)iU ) / 31.0f;
				vector3 vColor( fU, fV, fW );

				if( iU == 0 || iU == 31 || iV == 0 || iV == 31 || iW == 0 || iW == 31 )
					*pVolume++ = vector3( 0, 0, 0 );
				else
					*pVolume++ = vColor * fColorScale;
			}
		}
	}
	#else
	const float32 fColorScale = 0.4f;
	for( uint32 iW = 0; iW < 32; ++iW )
	{
		const float32 fW = ( (float32)iW - 15.5f ) / 15.5f;
		for( uint32 iV = 0; iV < 32; ++iV )
		{
			const float32 fV = ( (float32)iV - 15.5f ) / 15.5f;
			for( uint32 iU = 0; iU < 32; ++iU )
			{
				const float32 fU = ( (float32)iU - 15.5f ) / 15.5f;
				vector3 vColor( fU, fV, fW );

				if( vColor.lengthsq() > 1.0f )
					*pVolume++ = vector3( 0, 0, 0 );
				else
					*pVolume++ = vColor * ( 1.0f - vColor.length() ) * fColorScale;
			}
		}
	}
	#endif

	m_pVolumeTexture->UnlockBox( 0 );

	// Create buffers and shaders for wireframe box ---------------------------
	if( FUNC_FAILED( pM3DDevice->CreateVertexBuffer( &m_pVertexBufferWire, sizeof( vertexformat ) * 8 ) ) )
		return false;

	if( FUNC_FAILED( m_pVertexBufferWire->GetPointer( 0, (void **)&pDest ) ) )
		return false;

	pDest->vPosition = vector3( -1, -1, -1 ); ++pDest;
	pDest->vPosition = vector3( 1, -1, -1 ); ++pDest;
	pDest->vPosition = vector3( 1, 1, -1 ); ++pDest;
	pDest->vPosition = vector3( -1, 1, -1 ); ++pDest;
	pDest->vPosition = vector3( -1, -1, 1 ); ++pDest;
	pDest->vPosition = vector3( 1, -1, 1 ); ++pDest;
	pDest->vPosition = vector3( 1, 1, 1 ); ++pDest;
	pDest->vPosition = vector3( -1, 1, 1 ); ++pDest;

	if( FUNC_FAILED( pM3DDevice->CreateIndexBuffer( &m_pIndexBufferWire, sizeof( uint16 ) * 36, m3dfmt_index16 ) ) )
		return false;

	if( FUNC_FAILED( m_pIndexBufferWire->GetPointer( 0, (void **)&pIndex ) ) )
		return false;
	
	*pIndex++ = 0; *pIndex++ = 2; *pIndex++ = 1;
	*pIndex++ = 0; *pIndex++ = 3; *pIndex++ = 2;
	*pIndex++ = 1; *pIndex++ = 6; *pIndex++ = 5;
	*pIndex++ = 1; *pIndex++ = 2; *pIndex++ = 6;
	*pIndex++ = 5; *pIndex++ = 7; *pIndex++ = 4;
	*pIndex++ = 5; *pIndex++ = 6; *pIndex++ = 7;
	*pIndex++ = 4; *pIndex++ = 3; *pIndex++ = 0;
	*pIndex++ = 4; *pIndex++ = 7; *pIndex++ = 3;
	*pIndex++ = 3; *pIndex++ = 6; *pIndex++ = 2;
	*pIndex++ = 3; *pIndex++ = 7; *pIndex++ = 6;
	*pIndex++ = 0; *pIndex++ = 1; *pIndex++ = 4;
	*pIndex++ = 1; *pIndex++ = 5; *pIndex++ = 4;

	m_pVertexShaderWire = new CTexCubeWireVS;
	m_pPixelShaderWire = new CTexCubeWirePS;

	return true;
}

bool CTexCube::bFrameMove()
{
	m_fRotY += M3D_PI * 0.5f * m_pParent->pGetParent()->fGetInvFPS();
	return false;
}

void CTexCube::Render( uint32 i_iPass )
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
	
	matrix44 matTexCoords; matMatrix44RotationY( matTexCoords, m_fRotY );
	m_pVertexShader->SetMatrix( 4, matTexCoords );

	pGraphics->SetVertexFormat( m_pVertexFormat );
	pGraphics->SetVertexStream( 0, m_pVertexBuffer, 0, sizeof( vertexformat ) );
	pGraphics->SetIndexBuffer( m_pIndexBuffer );
	pGraphics->SetVertexShader( m_pVertexShader );
	pGraphics->SetPixelShader( m_pPixelShader );

	pGraphics->SetTexture( 0, m_pVolumeTexture );
	pGraphics->SetTextureSamplerState( 0, m3dtss_addressu, m3dta_clamp );
	pGraphics->SetTextureSamplerState( 0, m3dtss_addressv, m3dta_clamp );
	pGraphics->SetTextureSamplerState( 0, m3dtss_addressw, m3dta_clamp );

	pGraphics->pGetM3DDevice()->DrawIndexedPrimitive( m3dpt_trianglelist, 0, 0,
		m_iNumVertices, 0, m_iNumPrimitives );

	// Render the wireframe box -----------------------------------------------
	matMatrix44RotationY( matWorld, -m_fRotY );
	pCurCamera->SetWorldMatrix( matWorld );

	m_pVertexShaderWire->SetMatrix( m3dsc_worldmatrix, pCurCamera->matGetWorldMatrix() );
	m_pVertexShaderWire->SetMatrix( m3dsc_viewmatrix, pCurCamera->matGetViewMatrix() );
	m_pVertexShaderWire->SetMatrix( m3dsc_projectionmatrix, pCurCamera->matGetProjectionMatrix() );
	m_pVertexShaderWire->SetMatrix( m3dsc_wvpmatrix, pCurCamera->matGetWorldMatrix() * pCurCamera->matGetViewMatrix() * pCurCamera->matGetProjectionMatrix() );

	pGraphics->SetVertexStream( 0, m_pVertexBufferWire, 0, sizeof( vertexformat ) );
	pGraphics->SetIndexBuffer( m_pIndexBufferWire );
	pGraphics->SetVertexShader( m_pVertexShaderWire );
	pGraphics->SetPixelShader( m_pPixelShaderWire );

	pGraphics->SetRenderState( m3drs_fillmode, m3dfill_wireframe );
	pGraphics->SetRenderState( m3drs_cullmode, m3dcull_none );

	pGraphics->pGetM3DDevice()->DrawIndexedPrimitive( m3dpt_trianglelist, 0, 0, 8, 0, 12 );
}
