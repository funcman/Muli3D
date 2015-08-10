
#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#include "base.h"
#include "../../libmuli3d/include/m3d.h"
#include <stack>
#include "stateblock.h"

class CGraphics
{
protected:
	friend class IApplication;
	friend class CApplication;
	CGraphics( class IApplication *i_pParent );
	~CGraphics();

	bool bInitialize( const struct tCreationFlags &i_creationFlags );

public:

private:

public:
	inline class IApplication *pGetParent() { return m_pParent; }

	inline CMuli3D			*pGetM3D() { return m_pM3D; }
	inline CMuli3DDevice	*pGetM3DDevice() { return m_pM3DDevice; }

	inline class CCamera *pGetCurCamera() { return m_pCurCamera; }

	// Subsystems -------------------------------------------------------------
	void PushStateBlock();
	void PopStateBlock();

	// Use these functions instead of the device's set-functions! -------------
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

private:
	class IApplication *m_pParent;

	CMuli3D			*m_pM3D;
	CMuli3DDevice	*m_pM3DDevice;

	// Subsystems -------------------------------------------------------------
	stack<CStateBlock *> m_pStateBlocks;

protected:
	friend class CStateBlock;
	class CCamera *m_pCurCamera;

	friend class CCamera;
	inline void SetCurCamera( class CCamera *i_pCamera ) { if( m_pStateBlocks.empty() ) return; m_pStateBlocks.top()->SetCurCamera( i_pCamera ); }
};

#endif // __GRAPHICS_H__
