
#include "sphere.h"
#include "../libappframework/include/application.h"
#include "../libappframework/include/scene.h"
#include "mycamera.h"

class CSphereVS : public IMuli3DVertexShader
{
public:
	float32 fGetScale( const vector3 &i_vNormal )
	{
		vector4 vScaleA;
		SampleTexture( vScaleA, 2, i_vNormal.x, i_vNormal.y, i_vNormal.z );
		if( fGetFloat( 0 ) > 0.0f )
		{
			vector4 vScaleB;
			SampleTexture( vScaleB, 3, i_vNormal.x, i_vNormal.y, i_vNormal.z );
			return fLerp( vScaleA.r, vScaleB.r, fGetFloat( 0 ) );
		}
		else
			return vScaleA.r;
	}

	void Execute( const shaderreg *i_pInput, vector4 &o_vPosition, shaderreg *o_pOutput )
	{
		vector3 vSphereNormal = i_pInput[0]; vSphereNormal.normalize();

		const vector3 vMyPos = vSphereNormal * fGetScale( vSphereNormal );

		#ifndef USE_TRIANGLESHADER
		// calculate normal vector
		const float32 fTheta = atan2f( vSphereNormal.z, vSphereNormal.x );
		const float32 fPhi = acosf( vSphereNormal.y );
		const float32 fEpsilon = 4.0f * M3D_PI / 180.0f;

		vector3 vNextSphereNormals[4];
		vNextSphereNormals[0] = vector3( cosf( fTheta ) * sinf( fPhi - fEpsilon ), cosf( fPhi - fEpsilon ), sinf( fTheta ) * sinf( fPhi - fEpsilon ) ); // up
		vNextSphereNormals[1] = vector3( cosf( fTheta + fEpsilon ) * sinf( fPhi ), cosf( fPhi ), sinf( fTheta + fEpsilon ) * sinf( fPhi ) ); // right
		vNextSphereNormals[2] = vector3( cosf( fTheta ) * sinf( fPhi + fEpsilon ), cosf( fPhi + fEpsilon ), sinf( fTheta ) * sinf( fPhi + fEpsilon ) ); // down
		vNextSphereNormals[3] = vector3( cosf( fTheta - fEpsilon ) * sinf( fPhi ), cosf( fPhi ), sinf( fTheta - fEpsilon ) * sinf( fPhi ) ); // left

		vector3 vDeltas[4];
		for( uint32 i = 0; i < 4; ++i )
		{
			vDeltas[i] = ( vNextSphereNormals[i] * fGetScale( vNextSphereNormals[i] ) ) - vMyPos;
			vDeltas[i].normalize();
		}

		vector3 vNormal = vSphereNormal;
		for( uint32 j = 0; j < 1; ++j )
		{
			vector3 vLocalNormal;
			vVector3Cross( vLocalNormal, vDeltas[j], vDeltas[(j + 1) % 4] );
			vNormal += vLocalNormal;
		}
		vNormal /= 4;
		#endif

		// transform position and normal
		const vector4 vFinalPos = vector4( vMyPos.x, vMyPos.y, vMyPos.z, 1 );
		o_vPosition = vFinalPos * matGetMatrix( m3dsc_wvpmatrix );

		#ifndef USE_TRIANGLESHADER
		vVector3TransformNormal( *(vector3 *)&o_pOutput[0], vNormal, matGetMatrix( m3dsc_worldmatrix ) );
		#endif

		// calculate light direction
		const vector3 vWorldPosition = vFinalPos * matGetMatrix( m3dsc_worldmatrix );
		o_pOutput[1] = (vector3)vGetVector( 1 ) - vWorldPosition;

		// calculate light view direction
		o_pOutput[2] = (vector3)vGetVector( 0 ) - vWorldPosition;

		o_pOutput[3] = vWorldPosition;
	}

	m3dshaderregtype GetOutputRegisters( uint32 i_iRegister )
	{
		switch( i_iRegister )
		{
		case 0: return m3dsrt_vector3;
		case 1: return m3dsrt_vector3;
		case 2: return m3dsrt_vector3;
		case 3: return m3dsrt_vector3;
		default: return m3dsrt_unused;
		}
	}
};

#ifdef USE_TRIANGLESHADER
class CSphereTS : public IMuli3DTriangleShader
{
public:
	bool bExecute( shaderreg *io_pShaderRegs0, shaderreg *io_pShaderRegs1, shaderreg *io_pShaderRegs2 )
	{
		// calculate per triangle normal :)
		const vector3 vAB = ( io_pShaderRegs1[3] - io_pShaderRegs0[3] ).normalize();
		const vector3 vAC = ( io_pShaderRegs2[3] - io_pShaderRegs0[3] ).normalize();
		vector3 vNormal; vVector3Cross( vNormal, vAB, vAC );
		io_pShaderRegs0[0] = vNormal; io_pShaderRegs1[0] = vNormal; io_pShaderRegs2[0] = vNormal;
		return true;
	}
};
#endif

class CSpherePS : public IMuli3DPixelShader
{
public:
	bool bMightKillPixels() { return false; }
	bool bExecute( const shaderreg *i_pInput, vector4 &io_vColor, float32 &io_fDepth )
	{
		vector3 vNormal = i_pInput[0]; vNormal.normalize();
		vector3 vLightDir = i_pInput[1]; vLightDir.normalize();

		// io_vColor = vNormal * 0.5f + vector3( 0.5f, 0.5f, 0.5f );
		// return true;

		// compute fresnel term and reflection vector
		vector3 vViewDir = i_pInput[2]; vViewDir.normalize();
		const float32 fViewDotNormal = fSaturate( fVector3Dot( vNormal, vViewDir ) );
		vector3 vReflection = vNormal * (2.0f * fViewDotNormal) - vViewDir;
		vReflection.normalize();

		// compute diffuse and specular light
		float32 fDiffuse = fVector3Dot( vNormal, vLightDir );
		float32 fSpecular = 0.0f;
		if( fDiffuse >= 0.0f )
		{
			fSpecular = fVector3Dot( vLightDir, vReflection );
			if( fSpecular >= 0.0f )
				fSpecular = powf( fSpecular, 64.0f );
			else
				fSpecular = 0.0f;
		}
		else
			fDiffuse = 0.0f;

		// intersect the reflection vector with a unit-sphere for localized reflections.
		const vector3 &vPixelPos = i_pInput[3];
		float32 b = -2.0f * fVector3Dot( vReflection, vPixelPos );
		float32 c = fVector3Dot( vPixelPos, vPixelPos ) - 1.0f;
		float32 discrim = b * b - 4.0f * c;

		bool bIntersects = false;
		vector4 vReflectionColor( 1, 0, 0, 0 );
		float32 fNearT = 0.0f;
		if( discrim > 0.0f )
		{
			const float32 fEpsilon = 0.0001f;

			discrim = sqrtf( discrim );
			fNearT = -( discrim - b ) * 0.5f;
			bIntersects = fabsf( fNearT ) >= fEpsilon;
		}

		if( bIntersects )
		{
			vReflection = vReflection * fNearT - vPixelPos;
			vReflection.y = -vReflection.y;
			SampleTexture( vReflectionColor, 0, vReflection.x, vReflection.y, vReflection.z );
		}

		const float32 fKr = 1.0f;
		const float32 fKrMin = fKr * 0.1f;
		const float32 fFresExp = 2.0f;
		const float32 fFresnel = fKrMin + ( fKr - fKrMin ) * powf( 1.0f - fViewDotNormal, fFresExp );

		vector4 vAmbient; SampleTexture( vAmbient, 1, vNormal.x, vNormal.y, vNormal.z );
		const float32 fAmbientScale = 0.3f;

		const vector4 &vLightColor = vGetVector( 1 );
		io_vColor = vReflectionColor * fFresnel + vGetVector( 0 ) * ( vAmbient * fAmbientScale + vLightColor * fDiffuse ) + vLightColor * fSpecular;

		return true;
	}
};

class CCubeVS : public IMuli3DVertexShader
{
public:
	void Execute( const shaderreg *i_pInput, vector4 &o_vPosition, shaderreg *o_pOutput )
	{
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

class CCubePS : public IMuli3DPixelShader
{
public:
	bool bMightKillPixels() { return false; }
	bool bExecute( const shaderreg *i_pInput, vector4 &io_vColor, float32 &io_fDepth )
	{
		vector4 vCubeColorA; SampleTexture( vCubeColorA, 2, i_pInput[0].x, i_pInput[0].y, i_pInput[0].z );
		if( fGetFloat( 0 ) > 0.0f )
		{
			vector4 vCubeColorB;
			SampleTexture( vCubeColorB, 3, i_pInput[0].x, i_pInput[0].y, i_pInput[0].z );
			vVector4Lerp( vCubeColorA, vCubeColorA, vCubeColorB, fGetFloat( 0 ) );
		}

//		if( fGetFloat( 1 ) )
//			vVector4Lerp( io_vColor, io_vColor, vCubeColorA, 0.5f );
//		else
			io_vColor = vCubeColorA;

		return true;
	}
};

m3dvertexelement VertexDeclaration[] =
{
	M3DVERTEXFORMATDECL( 0, m3dvet_vector3, 0 )
};

m3dvertexelement VertexDeclarationCube[] =
{
	M3DVERTEXFORMATDECL( 0, m3dvet_vector3, 0 )
};

CSphere::CSphere( class CScene *i_pParent )
{
	m_pParent = i_pParent;

	m_pVertexFormat = 0;
	m_pVertexBuffer = 0;
	m_pIndexBuffer = 0;
	m_pVertexShader = 0;

	#ifdef USE_TRIANGLESHADER
	m_pTriangleShader = 0;
	#endif

	m_pPixelShader = 0;

	m_pVertexFormatCube = 0;
	m_pVertexBufferCube = 0;
	m_pIndexBufferCube = 0;
	m_pVertexShaderCube = 0;
	m_pPixelShaderCube = 0;

	SetColor( vector4( 1.0f, 0.75f, 0.25f, 1 ) );

	m_iNumVertices = 0;
	m_iNumPrimitives = 0;

	m_hEnvironment = 0;
	m_hAmbientLight = 0;
	m_hScaleMapA = 0;
	m_hScaleMapB = 0;

	m_fLerpScale = 0.0f;
}

CSphere::~CSphere()
{
	m_pParent->pGetParent()->pGetResManager()->ReleaseResource( m_hScaleMapB );
	m_pParent->pGetParent()->pGetResManager()->ReleaseResource( m_hScaleMapA );
	m_pParent->pGetParent()->pGetResManager()->ReleaseResource( m_hAmbientLight );
	m_pParent->pGetParent()->pGetResManager()->ReleaseResource( m_hEnvironment );

	SAFE_RELEASE( m_pPixelShaderCube );
	SAFE_RELEASE( m_pVertexShaderCube );
	SAFE_RELEASE( m_pIndexBufferCube );
	SAFE_RELEASE( m_pVertexBufferCube );
	SAFE_RELEASE( m_pVertexFormatCube );

	SAFE_RELEASE( m_pPixelShader );

	#ifdef USE_TRIANGLESHADER
	SAFE_RELEASE( m_pTriangleShader );
	#endif

	SAFE_RELEASE( m_pVertexShader );
	SAFE_RELEASE( m_pIndexBuffer );
	SAFE_RELEASE( m_pVertexBuffer );
	SAFE_RELEASE( m_pVertexFormat );
}

#ifdef GEOSPHERE_SUBDIVISIONS
// GEOSPHERE CONSTRUCTION -----------------------------------------------------

std::vector <CSphere::vertexformat> g_CurrentVertices;

void OutputTriangle( CSphere::vertexformat i_Vertex1, CSphere::vertexformat i_Vertex2, CSphere::vertexformat i_Vertex3 )
{
	i_Vertex1.vPosition.normalize();
	i_Vertex2.vPosition.normalize();
	i_Vertex3.vPosition.normalize();

	g_CurrentVertices.push_back( i_Vertex1 );
	g_CurrentVertices.push_back( i_Vertex2 );
	g_CurrentVertices.push_back( i_Vertex3 );
}

void Subdivide( CSphere::vertexformat i_Vertex1, CSphere::vertexformat i_Vertex2, CSphere::vertexformat i_Vertex3, uint32 i_iSubdivisions )
{
	if( !i_iSubdivisions )
	{
		OutputTriangle( i_Vertex1, i_Vertex2, i_Vertex3 );
		return;
	}
	
	CSphere::vertexformat v12, v23, v31;
	v12.vPosition = ( i_Vertex1.vPosition + i_Vertex2.vPosition ) * 0.5f;
	v23.vPosition = ( i_Vertex2.vPosition + i_Vertex3.vPosition ) * 0.5f;
	v31.vPosition = ( i_Vertex3.vPosition + i_Vertex1.vPosition ) * 0.5f;

	Subdivide( i_Vertex1, v12, v31, i_iSubdivisions - 1 );
	Subdivide( i_Vertex2, v23, v12, i_iSubdivisions - 1 );
	Subdivide( i_Vertex3, v31, v23, i_iSubdivisions - 1 );
	Subdivide( v12, v23, v31, i_iSubdivisions - 1 );
}

// ----------------------------------------------------------------------------
#endif

bool CSphere::bInitialize( uint32 i_iStacks, uint32 i_iSlices, string i_sScaleMapA, string i_sScaleMapB )
{
	CGraphics *pGraphics = m_pParent->pGetParent()->pGetGraphics();
	CMuli3DDevice *pM3DDevice = pGraphics->pGetM3DDevice();

	if( FUNC_FAILED( pM3DDevice->CreateVertexFormat( &m_pVertexFormat, VertexDeclaration, sizeof( VertexDeclaration ) ) ) )
		return false;

	uint16 *pDestIndices = 0;

#ifndef GEOSPHERE_SUBDIVISIONS
	// Construct a standard sphere
	m_iNumVertices = i_iStacks * i_iSlices * 4;
	m_iNumPrimitives = i_iStacks * i_iSlices * 2;

	if( FUNC_FAILED( pM3DDevice->CreateVertexBuffer( &m_pVertexBuffer, sizeof( vertexformat ) * m_iNumVertices ) ) )
		return false;

	if( FUNC_FAILED( pM3DDevice->CreateIndexBuffer( &m_pIndexBuffer, sizeof( uint16 ) * m_iNumPrimitives * 3, m3dfmt_index16 ) ) )
		return false;

	vertexformat *pDestVertices = 0;
	if( FUNC_FAILED( m_pVertexBuffer->GetPointer( 0, (void **)&pDestVertices ) ) )
		return false;

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

			x[0] = sinf( fV * M3D_PI ) * cosf( fU * 2.0f * M3D_PI );
			z[0] = sinf( fV * M3D_PI ) * sinf( fU * 2.0f * M3D_PI );
			y[0] = cosf( fV * M3D_PI );

			x[1] = sinf( fV * M3D_PI ) * cosf( fNextU * 2.0f * M3D_PI );
			z[1] = sinf( fV * M3D_PI ) * sinf( fNextU * 2.0f * M3D_PI );
			y[1] = cosf( fV * M3D_PI );

			x[2] = sinf( fNextV * M3D_PI ) * cosf( fU * 2.0f * M3D_PI );
			z[2] = sinf( fNextV * M3D_PI ) * sinf( fU * 2.0f * M3D_PI );
			y[2] = cosf( fNextV * M3D_PI );

			x[3] = sinf( fNextV * M3D_PI ) * cosf( fNextU * 2.0f * M3D_PI );
			z[3] = sinf( fNextV * M3D_PI ) * sinf( fNextU * 2.0f * M3D_PI );
			y[3] = cosf( fNextV * M3D_PI );

			pDestVertices->vPosition = vector3( x[0], y[0], z[0] );
			pDestVertices++;

			pDestVertices->vPosition = vector3( x[1], y[1], z[1] );
			pDestVertices++;

			pDestVertices->vPosition = vector3( x[2], y[2], z[2] );
			pDestVertices++;

			pDestVertices->vPosition = vector3( x[3], y[3], z[3] );
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
#else
	const float32 a = sqrtf( 2.0f / ( 5.0f + sqrtf( 5.0f ) ) );
	const float32 b = sqrtf( 2.0f / ( 5.0f - sqrtf( 5.0f ) ) );
	
	vertexformat Vertices[12];
	Vertices[ 0].vPosition = vector3(   -a, 0.0f,    b );
	Vertices[ 1].vPosition = vector3(    a, 0.0f,    b );
	Vertices[ 2].vPosition = vector3(   -a, 0.0f,   -b );
	Vertices[ 3].vPosition = vector3(    a, 0.0f,   -b );
	Vertices[ 4].vPosition = vector3( 0.0f,    b,    a );
	Vertices[ 5].vPosition = vector3( 0.0f,    b,   -a );
	Vertices[ 6].vPosition = vector3( 0.0f,   -b,    a );
	Vertices[ 7].vPosition = vector3( 0.0f,   -b,   -a );
	Vertices[ 8].vPosition = vector3(    b,    a, 0.0f );
	Vertices[ 9].vPosition = vector3(   -b,    a, 0.0f );
	Vertices[10].vPosition = vector3(    b,   -a, 0.0f );
	Vertices[11].vPosition = vector3(   -b,   -a, 0.0f );

	const uint16 iTriangles[20][3] =
	{
		{  1,  4,  0 }, {  4,  9,  0 }, {  4,  5,  9 }, {  8,  5,  4 },
		{  1,  8,  4 }, {  1, 10,  8 }, { 10,  3,  8 }, {  8,  3,  5 },
		{  3,  2,  5 }, {  3,  7,  2 }, {  3, 10,  7 }, { 10,  6,  7 },
		{  6, 11,  7 }, {  6,  0, 11 }, {  6,  1,  0 }, { 10,  1,  6 },
		{ 11,  0,  9 }, {  2, 11,  9 }, {  5,  2,  9 }, { 11,  2,  7 }
	};

	vertexformat *pDestVertices = 0;
	for( uint32 i = 0; i < 20; ++i )
	{
		Subdivide( Vertices[ iTriangles[i][0] ], Vertices[ iTriangles[i][1] ],
			Vertices[ iTriangles[i][2] ], GEOSPHERE_SUBDIVISIONS );

		if( !i )
		{
			m_iNumVertices = 20 * g_CurrentVertices.size();
			m_iNumPrimitives = m_iNumVertices / 3;

			if( FUNC_FAILED( pM3DDevice->CreateVertexBuffer( &m_pVertexBuffer, sizeof( vertexformat ) * m_iNumVertices ) ) )
				return false;

			if( FUNC_FAILED( m_pVertexBuffer->GetPointer( 0, (void **)&pDestVertices ) ) )
				return false;
		}

		for( vector<vertexformat>::iterator pCurVertex = g_CurrentVertices.begin(); pCurVertex != g_CurrentVertices.end(); ++pCurVertex, ++pDestVertices )
			memcpy( pDestVertices, &(*pCurVertex), sizeof( vertexformat ) );

		g_CurrentVertices.clear(); // clean up ...
	}
#endif

	m_pVertexShader = new CSphereVS;

	#ifdef USE_TRIANGLESHADER
	m_pTriangleShader = new CSphereTS;
	#endif

	m_pPixelShader = new CSpherePS;

	// Load environment texture ...
	CResManager *pResManager = m_pParent->pGetParent()->pGetResManager();
	m_hEnvironment = pResManager->hLoadResource( "majestic.cube" );
	if( !m_hEnvironment )
		return false;

	// Load ambient light texture ...
	m_hAmbientLight = pResManager->hLoadResource( "ambient.cube" );
	if( !m_hAmbientLight )
		return false;

	// Load displacement textures ...
	m_hScaleMapA = pResManager->hLoadResource( i_sScaleMapA );
	if( !m_hScaleMapA )
		return false;

	m_hScaleMapB = pResManager->hLoadResource( i_sScaleMapB );
	if( !m_hScaleMapB )
		return false;

	// Create the cube --------------------------------------------------------
	if( FUNC_FAILED( pM3DDevice->CreateVertexFormat( &m_pVertexFormatCube, VertexDeclaration, sizeof( VertexDeclarationCube ) ) ) )
		return false;

	if( FUNC_FAILED( pM3DDevice->CreateVertexBuffer( &m_pVertexBufferCube, sizeof( vertexformat ) * 8 ) ) )
		return false;

	if( FUNC_FAILED( pM3DDevice->CreateIndexBuffer( &m_pIndexBufferCube, sizeof( uint16 ) * 36, m3dfmt_index16 ) ) )
		return false;

	vertexformat_cube *pDestVerticesCube = 0;
	if( FUNC_FAILED( m_pVertexBufferCube->GetPointer( 0, (void **)&pDestVerticesCube ) ) )
		return false;

	if( FUNC_FAILED( m_pIndexBufferCube->GetPointer( 0, (void **)&pDestIndices ) ) )
		return false;

	pDestVerticesCube->vPosition = vector3( -1, -1, -1 ); ++pDestVerticesCube;
	pDestVerticesCube->vPosition = vector3( 1, -1, -1 ); ++pDestVerticesCube;
	pDestVerticesCube->vPosition = vector3( 1, 1, -1 ); ++pDestVerticesCube;
	pDestVerticesCube->vPosition = vector3( -1, 1, -1 ); ++pDestVerticesCube;
	pDestVerticesCube->vPosition = vector3( -1, -1, 1 ); ++pDestVerticesCube;
	pDestVerticesCube->vPosition = vector3( 1, -1, 1 ); ++pDestVerticesCube;
	pDestVerticesCube->vPosition = vector3( 1, 1, 1 ); ++pDestVerticesCube;
	pDestVerticesCube->vPosition = vector3( -1, 1, 1 ); ++pDestVerticesCube;

	*pDestIndices++ = 0; *pDestIndices++ = 2; *pDestIndices++ = 1;
	*pDestIndices++ = 0; *pDestIndices++ = 3; *pDestIndices++ = 2;
	*pDestIndices++ = 1; *pDestIndices++ = 6; *pDestIndices++ = 5;
	*pDestIndices++ = 1; *pDestIndices++ = 2; *pDestIndices++ = 6;
	*pDestIndices++ = 5; *pDestIndices++ = 7; *pDestIndices++ = 4;
	*pDestIndices++ = 5; *pDestIndices++ = 6; *pDestIndices++ = 7;
	*pDestIndices++ = 4; *pDestIndices++ = 3; *pDestIndices++ = 0;
	*pDestIndices++ = 4; *pDestIndices++ = 7; *pDestIndices++ = 3;
	*pDestIndices++ = 3; *pDestIndices++ = 6; *pDestIndices++ = 2;
	*pDestIndices++ = 3; *pDestIndices++ = 7; *pDestIndices++ = 6;
	*pDestIndices++ = 0; *pDestIndices++ = 1; *pDestIndices++ = 4;
	*pDestIndices++ = 1; *pDestIndices++ = 5; *pDestIndices++ = 4;

	m_pVertexShaderCube = new CCubeVS;
	m_pPixelShaderCube = new CCubePS;

	return true;
}

bool CSphere::bFrameMove()
{
	m_fLerpScale = fSaturate( cosf( m_pParent->pGetParent()->fGetElapsedTime() * 0.5f ) );
	return false;
}

void CSphere::Render( uint32 i_iPass )
{
	switch( i_iPass )
	{
	case ePass_Lighting: break;
	}

	CGraphics *pGraphics = m_pParent->pGetParent()->pGetGraphics();

	CCamera *pCurCamera = pGraphics->pGetCurCamera();
	matrix44 matWorld; matMatrix44Identity( matWorld );
	pCurCamera->SetWorldMatrix( matWorld );

	m_pVertexShader->SetMatrix( m3dsc_worldmatrix, pCurCamera->matGetWorldMatrix() );
	m_pVertexShader->SetMatrix( m3dsc_viewmatrix, pCurCamera->matGetViewMatrix() );
	m_pVertexShader->SetMatrix( m3dsc_projectionmatrix, pCurCamera->matGetProjectionMatrix() );
	m_pVertexShader->SetMatrix( m3dsc_wvpmatrix, pCurCamera->matGetWorldMatrix() * pCurCamera->matGetViewMatrix() * pCurCamera->matGetProjectionMatrix() );

	m_pPixelShader->SetVector( 0, m_vColor );

	vector3 vCamPos = pCurCamera->vGetPosition();
	m_pVertexShader->SetVector( 0, vector4( vCamPos.x, vCamPos.y, vCamPos.z, 0 ) );

	CLight *pLight = m_pParent->pGetCurrentLight();

	vector3 vLightPos = pLight->vGetPosition();
	m_pVertexShader->SetVector( 1, vector4( vLightPos.x, vLightPos.y, vLightPos.z, 0 ) );

	m_pPixelShader->SetVector( 1, pLight->vGetColor() );

	CResManager *pResManager = m_pParent->pGetParent()->pGetResManager();
	CTexture *pTexture = (CTexture *)pResManager->pGetResource( m_hEnvironment );
	pGraphics->SetTexture( 0, pTexture->pGetTexture() );

	pTexture = (CTexture *)pResManager->pGetResource( m_hAmbientLight );
	pGraphics->SetTexture( 1, pTexture->pGetTexture() );

	pTexture = (CTexture *)pResManager->pGetResource( m_hScaleMapA );
	pGraphics->SetTexture( 2, pTexture->pGetTexture() );

	pTexture = (CTexture *)pResManager->pGetResource( m_hScaleMapB );
	pGraphics->SetTexture( 3, pTexture->pGetTexture() );

	m_pVertexShader->SetFloat( 0, m_fLerpScale );

	pGraphics->SetVertexFormat( m_pVertexFormat );
	pGraphics->SetVertexStream( 0, m_pVertexBuffer, 0, sizeof( vertexformat ) );
	
	#ifndef GEOSPHERE_SUBDIVISIONS
	pGraphics->SetIndexBuffer( m_pIndexBuffer );
	#endif

	pGraphics->SetVertexShader( m_pVertexShader );

	#ifdef USE_TRIANGLESHADER
	pGraphics->SetTriangleShader( m_pTriangleShader );
	#endif

	pGraphics->SetPixelShader( m_pPixelShader );

	// pGraphics->SetRenderState( m3drs_fillmode, m3dfill_wireframe );

	#ifndef GEOSPHERE_SUBDIVISIONS
	pGraphics->pGetM3DDevice()->DrawIndexedPrimitive( m3dpt_trianglelist,
		0, 0, m_iNumVertices, 0, m_iNumPrimitives );
	#else
	pGraphics->pGetM3DDevice()->DrawPrimitive( m3dpt_trianglelist, 0, m_iNumPrimitives );
	#endif

	// Draw the cube
	m_pVertexShaderCube->SetMatrix( m3dsc_worldmatrix, pCurCamera->matGetWorldMatrix() );
	m_pVertexShaderCube->SetMatrix( m3dsc_viewmatrix, pCurCamera->matGetViewMatrix() );
	m_pVertexShaderCube->SetMatrix( m3dsc_projectionmatrix, pCurCamera->matGetProjectionMatrix() );
	m_pVertexShaderCube->SetMatrix( m3dsc_wvpmatrix, pCurCamera->matGetWorldMatrix() * pCurCamera->matGetViewMatrix() * pCurCamera->matGetProjectionMatrix() );

	m_pPixelShaderCube->SetFloat( 0, m_fLerpScale );

	pGraphics->SetVertexFormat( m_pVertexFormatCube );
	pGraphics->SetVertexStream( 0, m_pVertexBufferCube, 0, sizeof( vertexformat_cube ) );
	pGraphics->SetIndexBuffer( m_pIndexBufferCube );
	pGraphics->SetVertexShader( m_pVertexShaderCube );

	#ifdef USE_TRIANGLESHADER
	pGraphics->SetTriangleShader( 0 );
	#endif

	pGraphics->SetPixelShader( m_pPixelShaderCube );

	pGraphics->SetRenderState( m3drs_fillmode, m3dfill_solid );

//	m_pPixelShaderCube->SetFloat( 1, 0.0f );
	pGraphics->SetRenderState( m3drs_cullmode, m3dcull_cw );
	pGraphics->pGetM3DDevice()->DrawIndexedPrimitive( m3dpt_trianglelist,
		0, 0, 8, 0, 12 );

/*	m_pPixelShaderCube->SetFloat( 1, 1.0f );
	pGraphics->SetRenderState( m3drs_cullmode, m3dcull_ccw );
	pGraphics->pGetM3DDevice()->DrawIndexedPrimitive( m3dpt_trianglelist,
		0, 0, 8, 0, 12 ); */
}
