
#ifndef __LEAF_H__
#define __LEAF_H__

#include "common.h"
#include "../libappframework/include/entity.h"
#include "../libappframework/include/graphics.h"
#include "../libappframework/include/texture.h"

class CLeaf : public IEntity
{
public:
	struct vertexformat
	{
		vector3 vPosition;
		vector2 vTex;
	};

protected:
	CLeaf( class CScene *i_pParent );

public:
	static IEntity *pCreate( class CScene *i_pParent ) { return new CLeaf( i_pParent ); }

	~CLeaf();

	bool bInitialize( float32 i_fWidth, float32 i_fHeight );

	bool bFrameMove();
	void Render( uint32 i_iPass );

private:

public:

private:
	class CScene *m_pParent;

	CMuli3DVertexFormat *m_pVertexFormat;
	CMuli3DVertexBuffer *m_pVertexBuffer;
	class CLeafVS		*m_pVertexShader;
	class CLeafPS		*m_pPixelShader;

	HRESOURCE m_hTexture;
};

#endif // __LEAF_H__
