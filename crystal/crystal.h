
#ifndef __CRYSTAL_H__
#define __CRYSTAL_H__

#include "common.h"
#include "../libappframework/include/entity.h"
#include "../libappframework/include/graphics.h"
#include "../libappframework/include/texture.h"

class CCrystal : public IEntity
{
public:
	struct vertexformat
	{
		vector3 vPosition;
		vector2 vTex;
	};

protected:
	CCrystal( class CScene *i_pParent );

public:
	static IEntity *pCreate( class CScene *i_pParent ) { return new CCrystal( i_pParent ); }

	~CCrystal();

	bool bInitialize( string i_sModel, string i_sTexture, string i_sNormalmap );

	bool bFrameMove();
	void Render( uint32 i_iPass );

private:

public:

private:
	class CScene *m_pParent;

	class CCrystalVS	*m_pVertexShader;
	class CCrystalPS	*m_pPixelShader;

	HRESOURCE m_hModel, m_hTexture, m_hNormalmap, m_hEnvironment;
};

#endif // __CRYSTAL_H__
