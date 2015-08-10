
#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include "base.h"
#include "graphics.h"
#include "resmanager.h"
#include "application.h"

enum eTextureType
{
	eTextureType_Default,	// either animated or non-animated 2d texture
//	eTextureType_Volume,
	eTextureType_Cube
};

class CTexture
{
protected:
	// see CResManager
	friend void *pLoadTexture( CResManager *i_pParent, string i_sFilename );
	friend void *pLoadCubeTexture( CResManager *i_pParent, string i_sFilename );
	friend void *pLoadAnimatedTexture( CResManager *i_pParent, string i_sFilename );
	friend void UnloadTexture( CResManager *i_pParent, void *i_pResource );

	CTexture( CResManager *i_pParent, CMuli3DTexture *i_pTexture )
	{	
		// standard texture
		m_pParent = i_pParent;
		m_iNumTextures = 1;
		m_ppTextures = new CMuli3DTexture *[m_iNumTextures];
		m_ppTextures[0] = i_pTexture;
		m_pCubeTexture = 0;
		m_eTextureType = eTextureType_Default;
	}

	CTexture( CResManager *i_pParent, CMuli3DCubeTexture *i_pCubeTexture )
	{	
		// cube texture
		m_pParent = i_pParent;
		m_iNumTextures = 0;
		m_ppTextures = 0;
		m_pCubeTexture = i_pCubeTexture;
		m_eTextureType = eTextureType_Cube;
	}

	CTexture( CResManager *i_pParent, uint32 i_iNumTextures, float32 i_fFPS, CMuli3DTexture **i_ppTexture )
	{	
		// animated texture
		m_pParent = i_pParent;
		m_iNumTextures = i_iNumTextures;
		m_fFPS = i_fFPS;
		m_ppTextures = (CMuli3DTexture **)i_ppTexture;
		m_fCurTexture = 0.0f;
		m_pCubeTexture = 0;
		m_eTextureType = eTextureType_Default;
	}

	~CTexture()
	{
		SAFE_RELEASE( m_pCubeTexture );

		for( uint32 i = 0; i < m_iNumTextures; ++i )
			SAFE_RELEASE( m_ppTextures[i] );
		SAFE_DELETE_ARRAY( m_ppTextures );
	}

private:

public:
	inline CResManager *pGetParent() { return m_pParent; }

	inline IMuli3DBaseTexture *pGetTexture()
	{
		if( m_iNumTextures == 1 )
			return m_ppTextures[0];

		if( m_pCubeTexture )
			return m_pCubeTexture;

		m_fCurTexture += m_fFPS * m_pParent->pGetParent()->fGetInvFPS();

		uint32 iTexIndex = (uint32)m_fCurTexture;
		if( iTexIndex >= m_iNumTextures || iTexIndex < 0 )
		{
			m_fCurTexture = 0;
			iTexIndex = 0;
		}

		return m_ppTextures[iTexIndex];
	}

	inline eTextureType eGetTextureType() { return m_eTextureType; }

private:
	CResManager			*m_pParent;
	uint32				m_iNumTextures;
	CMuli3DTexture		**m_ppTextures;
	CMuli3DCubeTexture	*m_pCubeTexture;
	float32				m_fFPS, m_fCurTexture;
	eTextureType		m_eTextureType;
};

#endif // __TEXTURE_H__
