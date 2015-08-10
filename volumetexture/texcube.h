
#ifndef __TEXCUBE_H__
#define __TEXCUBE_H__

#include "common.h"
#include "../libappframework/include/entity.h"
#include "../libappframework/include/graphics.h"
#include "../libappframework/include/resmanager.h"

#define NUM_SHELLS	16
#define NUM_CELLS	16

// #define CUBE

class CTexCube : public IEntity
{
public:
	struct vertexformat
	{
		vector3 vPosition;
	};

protected:
	CTexCube( class CScene *i_pParent );

public:
	static IEntity *pCreate( class CScene *i_pParent ) { return new CTexCube( i_pParent ); }

	~CTexCube();

	bool bInitialize();

	bool bFrameMove();
	void Render( uint32 i_iPass );

private:

public:

private:
	class CScene *m_pParent;

	CMuli3DVertexFormat		*m_pVertexFormat;
	
	CMuli3DVertexBuffer		*m_pVertexBuffer;
	CMuli3DIndexBuffer		*m_pIndexBuffer;
	class CTexCubeVS		*m_pVertexShader;
	class CTexCubePS		*m_pPixelShader;
	CMuli3DVolumeTexture	*m_pVolumeTexture;

	CMuli3DVertexBuffer		*m_pVertexBufferWire;
	CMuli3DIndexBuffer		*m_pIndexBufferWire;
	class CTexCubeWireVS	*m_pVertexShaderWire;
	class CTexCubeWirePS	*m_pPixelShaderWire;

	float32 m_fRotY;
	uint32	m_iNumVertices, m_iNumPrimitives;
};

#endif // __TEXCUBE_H__
