
#ifndef __BUBBLE_H__
#define __BUBBLE_H__

#include "common.h"
#include "../libappframework/include/entity.h"
#include "../libappframework/include/graphics.h"
#include "../libappframework/include/texture.h"

// #define VISUALIZE_RATE_OF_CHANGE

class CBubble : public IEntity
{
public:
	struct vertexformat
	{
		vector3 vPosition;
		vector2 vTex;
	};

protected:
	CBubble( class CScene *i_pParent );

public:
	static IEntity *pCreate( class CScene *i_pParent ) { return new CBubble( i_pParent ); }

	~CBubble();

	bool bInitialize( float32 i_fRadius, uint32 i_iStacks, uint32 i_iSlices );

	bool bFrameMove();
	void Render( uint32 i_iPass );

private:

public:

private:
	class CScene *m_pParent;

	CMuli3DVertexFormat *m_pVertexFormat;
	CMuli3DVertexBuffer *m_pVertexBuffer;
	CMuli3DIndexBuffer	*m_pIndexBuffer;
	class CBubbleVS		*m_pVertexShader;
	class CBubblePS		*m_pPixelShader;

	uint32 m_iNumVertices, m_iNumPrimitives;

	HRESOURCE m_hRainbowFilm, m_hEnvironment;
};

#endif // __BUBBLE_H__
