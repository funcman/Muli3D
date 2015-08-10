
#ifndef __STATEBLOCK_H__
#define __STATEBLOCK_H__

#include "../include/base.h"
#include "../../libmuli3d/include/m3d.h"
#include <map>

class CStateBlock
{
protected:
	friend class CGraphics;
	CStateBlock( class CGraphics *i_pParent );

public:
	~CStateBlock();

	void RestoreStates();

	result SetRenderState( m3drenderstate i_RenderState, uint32 i_iValue );
	result SetVertexFormat( CMuli3DVertexFormat *i_pVertexFormat );
	void SetPrimitiveAssembler( IMuli3DPrimitiveAssembler *i_pPrimitiveAssembler );
	result SetVertexShader( IMuli3DVertexShader *i_pVertexShader );
	void SetTriangleShader( IMuli3DTriangleShader *i_pTriangleShader );
	result SetPixelShader( IMuli3DPixelShader *i_pPixelShader );
	result SetIndexBuffer( CMuli3DIndexBuffer *i_pIndexBuffer );
	result SetVertexStream( uint32 i_iStreamNumber, CMuli3DVertexBuffer *i_pVertexBuffer, uint32 i_iOffset, uint32 i_iStride );
	result SetTexture( uint32 i_iSamplerNumber, IMuli3DBaseTexture *i_pTexture );
	result SetTextureSamplerState( uint32 i_iSamplerNumber, m3dtexturesamplerstate i_TextureSamplerState, uint32 i_iState );
	void SetRenderTarget( CMuli3DRenderTarget *i_pRenderTarget );
	result SetScissorRect( const m3drect &i_ScissorRect );

	void SetCurCamera( class CCamera *i_pCamera );

private:

public:
	inline class CGraphics *pGetParent() { return m_pParent; }

private:
	class CGraphics *m_pParent;

	// Backup variables -------------------------------------------------------
	map<m3drenderstate, uint32>	m_RenderStates;

	bool m_bChangedVertexFormat;
	CMuli3DVertexFormat *m_pVertexFormat;

	bool m_bChangedPrimitiveAssembler;
	IMuli3DPrimitiveAssembler *m_pPrimitiveAssembler;

	bool m_bChangedVertexShader;
	IMuli3DVertexShader *m_pVertexShader;

	bool m_bChangedTriangleShader;
	IMuli3DTriangleShader *m_pTriangleShader;

	bool m_bChangedPixelShader;
	IMuli3DPixelShader *m_pPixelShader;

	bool m_bChangedIndexBuffer;
	CMuli3DIndexBuffer *m_pIndexBuffer;

	struct tVertexStreamInfo
	{
		CMuli3DVertexBuffer *pVertexBuffer;
		uint32 iOffset, iStride;
	};
	map<uint32, tVertexStreamInfo> m_VertexStreams;

	map<uint32, IMuli3DBaseTexture *> m_Textures;

	map<uint32, uint32> m_TextureSamplerStates;
	
	bool m_bChangedRenderTarget;
	CMuli3DRenderTarget *m_pRenderTarget;

	bool m_bChangedScissorRect;
	m3drect m_ScissorRect;

	bool m_bChangedCamera;
	class CCamera *m_pCamera;
};

#endif // __STATEBLOCK_H__
