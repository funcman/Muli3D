
#ifndef __RAYTRACER_H__
#define __RAYTRACER_H__

#include "common.h"
#include "../libappframework/include/entity.h"
#include "../libappframework/include/graphics.h"
#include "../libappframework/include/resmanager.h"

#define MAX_SPHERES 8
#define MAX_LIGHTS 4
#define MAX_RECURSION 2

class CRaytracer : public IEntity
{
public:
	struct vertexformat
	{
		vector3 vDirection;
	};

protected:
	CRaytracer( class CScene *i_pParent );

public:
	static IEntity *pCreate( class CScene *i_pParent ) { return new CRaytracer( i_pParent ); }

	~CRaytracer();

	bool bInitialize( float32 i_fFOVAngle );

	bool bFrameMove();
	void Render( uint32 i_iPass );

private:
	
public:
	bool bAddSphere( const vector3 &i_vOrigin, float32 i_fRadius, const vector4 &i_vColor = vector4( 0, 0, 0, 1 ), HRESOURCE i_hTexture = 0 );
	bool bAddLight( const vector3 &i_vOrigin, const vector4 &i_vColor );

private:
	class CScene *m_pParent;

	CMuli3DVertexFormat *m_pVertexFormat;
	CMuli3DVertexBuffer *m_pVertexBuffer;
	class CRaytracerVS	*m_pVertexShader;
	class CRaytracerPS	*m_pPixelShader;

	CMuli3DTexture		*m_pSphereData;
	HRESOURCE			m_hSphereTextures[MAX_SPHERES];
	uint32				m_iNumSpheres;
	CMuli3DTexture		*m_pLightData;
	uint32				m_iNumLights;
};

#endif // __RAYTRACER_H__
