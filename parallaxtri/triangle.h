
#ifndef __TRIANGLE_H__
#define __TRIANGLE_H__

#include "common.h"
#include "../libappframework/include/entity.h"
#include "../libappframework/include/resmanager.h"
#include "../libappframework/include/texture.h"
#include "../libappframework/include/graphics.h"

class CTriangle : public IEntity
{
public:
	struct vertexformat
	{
		vector3 vPosition;
		vector3 vNormal;
		vector3 vTangent;
		vector2 vTexCoord0;
	};

protected:
	CTriangle( class CScene *i_pParent );

public:
	static IEntity *pCreate( class CScene *i_pParent ) { return new CTriangle( i_pParent ); }

	~CTriangle();

	bool bInitialize( const vertexformat *i_pVertices, string i_sTexture, string i_sNormalmap );

	bool bFrameMove();
	void Render( uint32 i_iPass );

private:

public:

private:
	class CScene *m_pParent;

	CMuli3DVertexFormat *m_pVertexFormat;
	CMuli3DVertexBuffer *m_pVertexBuffer;
	class CTriangleVS	*m_pVertexShader;
	class CTrianglePS	*m_pPixelShader;
	
	HRESOURCE m_hTexture;
	HRESOURCE m_hNormalmap;
};

#endif // __TRIANGLE_H__
