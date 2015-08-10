
#ifndef __FRACTAL_H__
#define __FRACTAL_H__

#include "common.h"
#include "../libappframework/include/entity.h"
#include "../libappframework/include/graphics.h"
#include "../libappframework/include/resmanager.h"

#define MANDELBROT_ITERATIONS 32

class CFractal : public IEntity
{
public:
	struct vertexformat
	{
		vector3 vPosition;
		vector2 vTexCoord0;
	};

protected:
	CFractal( class CScene *i_pParent );

public:
	static IEntity *pCreate( class CScene *i_pParent ) { return new CFractal( i_pParent ); }

	~CFractal();

	bool bInitialize();

	bool bFrameMove();
	void Render( uint32 i_iPass );

private:

public:

private:
	class CScene *m_pParent;

	CMuli3DVertexFormat *m_pVertexFormat;
	CMuli3DVertexBuffer *m_pVertexBuffer;
	class CFractalVS	*m_pVertexShader;
	class CFractalPS	*m_pPixelShader;

	HRESOURCE m_hColormap;
};

#endif // __FRACTAL_H__
