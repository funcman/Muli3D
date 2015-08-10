
#ifndef __SPHERE_H__
#define __SPHERE_H__

#include "common.h"
#include "../libappframework/include/entity.h"
#include "../libappframework/include/graphics.h"
#include "../libappframework/include/texture.h"

class CSphere : public IEntity
{
public:
	struct vertexformat
	{
		vector3 vPosition;
		vector2 vTex;
	};

protected:
	CSphere( class CScene *i_pParent );

public:
	static IEntity *pCreate( class CScene *i_pParent ) { return new CSphere( i_pParent ); }

	~CSphere();

	bool bInitialize( float32 i_fRadius, uint32 i_iStacks, uint32 i_iSlices, string i_sTexture );

	bool bFrameMove();
	void Render( uint32 i_iPass );

private:

public:

private:
	class CScene *m_pParent;

	CMuli3DVertexFormat *m_pVertexFormat;
	CMuli3DVertexBuffer *m_pVertexBuffer;
	class CSpherePrimitiveAssembler *m_pPrimitiveAssembler;
	class CSphereVS		*m_pVertexShader;
	class CSpherePS		*m_pPixelShader;

	uint32 m_iNumVertices, m_iNumPrimitives;

	HRESOURCE m_hTexture;
};

#endif // __SPHERE_H__
