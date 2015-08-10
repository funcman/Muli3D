
#ifndef __BOARD_H__
#define __BOARD_H__

#include "common.h"
#include "../libappframework/include/entity.h"
#include "../libappframework/include/graphics.h"

#define ANTIALIAS_BOARD

class CBoard : public IEntity
{
public:
	struct vertexformat
	{
		vector3 vPosition;
		vector2 vTexCoord0;
	};

protected:
	CBoard( class CScene *i_pParent );

public:
	static IEntity *pCreate( class CScene *i_pParent ) { return new CBoard( i_pParent ); }

	~CBoard();

	bool bInitialize();

	bool bFrameMove();
	void Render( uint32 i_iPass );

private:

public:

private:
	class CScene *m_pParent;

	CMuli3DVertexFormat *m_pVertexFormat;
	CMuli3DVertexBuffer *m_pVertexBuffer;
	class CBoardVS		*m_pVertexShader;
	class CBoardPS		*m_pPixelShader;
};

#endif // __BOARD_H__
