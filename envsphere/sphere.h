
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
	};

protected:
	CSphere( class CScene *i_pParent );

public:
	static IEntity *pCreate( class CScene *i_pParent ) { return new CSphere( i_pParent ); }

	~CSphere();

	bool bInitialize( float32 i_fRadius, uint32 i_iStacks, uint32 i_iSlices, string i_sEnvironment );

	bool bFrameMove();
	void Render( uint32 i_iPass );

private:

public:
	inline void SetColor( const vector4 &i_vColor ) { m_vColor = i_vColor; }
	inline const vector4 &vGetColor() { return m_vColor; }

private:
	class CScene *m_pParent;

	CMuli3DVertexFormat *m_pVertexFormat;
	CMuli3DVertexBuffer *m_pVertexBuffer;
	CMuli3DIndexBuffer	*m_pIndexBuffer;
	class CSphereVS		*m_pVertexShader;
	class CSpherePS		*m_pPixelShader;

	vector4 m_vColor;

	uint32 m_iNumVertices, m_iNumPrimitives;

	HRESOURCE m_hEnvironment;
};

#endif // __SPHERE_H__
