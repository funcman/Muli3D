
#ifndef __SPHERICALLIGHT_H__
#define __SPHERICALLIGHT_H__

#include "common.h"
#include "../libappframework/include/entity.h"
#include "../libappframework/include/graphics.h"
#include "../libappframework/include/texture.h"

class CSphericalLight : public IEntity
{
public:
	struct vertexformatsphere
	{
		vector3 vPosition;
	};

	struct vertexformatflare
	{
		vector3 vPosition;
		vector2 vTex;
	};

protected:
	CSphericalLight( class CScene *i_pParent );

public:
	static IEntity *pCreate( class CScene *i_pParent ) { return new CSphericalLight( i_pParent ); }

	~CSphericalLight();

	bool bInitialize( float32 i_fRadius, uint32 i_iStacks, uint32 i_iSlices, float32 i_fFlareWidth, float32 i_fFlareHeight );

	bool bFrameMove();
	void Render( uint32 i_iPass );

private:

public:
	inline void SetColor( const vector4 &i_vColor ) { m_vColor = i_vColor; }
	inline const vector4 &vGetColor() { return m_vColor; }

private:
	class CScene *m_pParent;

	CMuli3DVertexFormat				*m_pVertexFormatSphere;
	CMuli3DVertexBuffer				*m_pVertexBufferSphere;
	class CSpherePrimitiveAssembler	*m_pPrimitiveAssemblerSphere;
	
	class CSphericalLightVS	*m_pVertexShader;
	class CSphericalLightPS	*m_pPixelShader;

	CMuli3DVertexFormat		*m_pVertexFormatFlare;
	CMuli3DVertexBuffer		*m_pVertexBufferFlare;

	vector4 m_vColor;

	uint32 m_iNumVertices, m_iNumPrimitives;

	HRESOURCE m_hFlare;

	uint32 m_iMaxVisiblePixels;
};

#endif // __SPHERICALLIGHT_H__
