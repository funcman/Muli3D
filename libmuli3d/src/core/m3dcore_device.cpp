/*
	Muli3D - a software rendering library
	Copyright (C) 2004, 2005 Stephan Reiter <streiter@aon.at>

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "../../include/core/m3dcore_device.h"
#include "../../include/core/m3dcore.h"
#include "../../include/core/m3dcore_basetexture.h"
#include "../../include/core/m3dcore_cubetexture.h"
#include "../../include/core/m3dcore_indexbuffer.h"
#include "../../include/core/m3dcore_presenttarget.h"
#include "../../include/core/m3dcore_rendertarget.h"
#include "../../include/core/m3dcore_shaders.h"
#include "../../include/core/m3dcore_surface.h"
#include "../../include/core/m3dcore_texture.h"
#include "../../include/core/m3dcore_primitiveassembler.h"
#include "../../include/core/m3dcore_vertexbuffer.h"
#include "../../include/core/m3dcore_vertexformat.h"
#include "../../include/core/m3dcore_volume.h"
#include "../../include/core/m3dcore_volumetexture.h"

CMuli3DDevice::CMuli3DDevice( CMuli3D *i_pParent, const m3ddeviceparameters *i_pDeviceParameters )
	: m_pParent( i_pParent ), m_pPresentTarget( 0 ), m_pVertexFormat( 0 ), m_pPrimitiveAssembler( 0 ),
	  m_pVertexShader( 0 ), m_pTriangleShader( 0 ), m_pPixelShader( 0 ), m_pIndexBuffer( 0 ),
	  m_pRenderTarget( 0 )
{
	m_pParent->AddRef();

	memcpy( &m_DeviceParameters, i_pDeviceParameters, sizeof( m3ddeviceparameters ) );

	memset( m_VertexStreams, 0, sizeof( m_VertexStreams ) );
	memset( m_TextureSamplers, 0, sizeof( m_TextureSamplers ) );
	memset( &m_ScissorRect, 0, sizeof( m_ScissorRect ) );
	memset( &m_RenderInfo, 0, sizeof( m_RenderInfo ) );
	memset( &m_TriangleInfo, 0, sizeof( m_TriangleInfo ) );

	memset( &m_VertexCache, 0, sizeof( m_VertexCache ) );

	memset( &m_ClipVertices, 0, sizeof( m_ClipVertices ) );
	memset( &m_pClipVertices, 0, sizeof( m_pClipVertices ) );

	SetDefaultRenderStates();
	SetDefaultTextureSamplerStates();
	SetDefaultClippingPlanes();
}

CMuli3DDevice::~CMuli3DDevice()
{
	SAFE_RELEASE( m_pPresentTarget );

	SAFE_RELEASE( m_pParent );
}

result CMuli3DDevice::Create()
{
	// Create the present-target ----------------------------------------------

	// NOTE: add support for other platforms here
	#ifdef WIN32
	m_pPresentTarget = new CMuli3DPresentTargetWin32( this );
	#endif
	
	#ifdef LINUX_X11
	m_pPresentTarget = new CMuli3DPresentTargetLinuxX11( this );
	#endif

	#ifdef __amigaos4__
	m_pPresentTarget = new CMuli3DPresentTargetAmigaOS4( this );
	#endif

	if( !m_pPresentTarget )
	{
		FUNC_FAILING( "CMuli3DDevice::Create: out of memory, cannot create present-target.\n" );
		return e_outofmemory;
	}

	return m_pPresentTarget->Create();
}

void CMuli3DDevice::SetDefaultRenderStates()
{
	SetRenderState( m3drs_zenable, true );
	SetRenderState( m3drs_zwriteenable, true );
	SetRenderState( m3drs_zfunc, m3dcmp_less );

	SetRenderState( m3drs_colorwriteenable, true );
	SetRenderState( m3drs_fillmode, m3dfill_solid );

	SetRenderState( m3drs_cullmode, m3dcull_ccw );

	SetRenderState( m3drs_subdivisionmode, m3dsubdiv_none );
	SetRenderState( m3drs_subdivisionlevels, 1 );
	SetRenderState( m3drs_subdivisionpositionregister, 0 );
	SetRenderState( m3drs_subdivisionnormalregister, 1 );
	const float32 fDefaultSubdividingMaxScreenArea = 1.0f;
	SetRenderState( m3drs_subdivisionmaxscreenarea, *(uint32 *)&fDefaultSubdividingMaxScreenArea );
	SetRenderState( m3drs_subdivisionmaxinnerlevels, 1 );

	SetRenderState( m3drs_scissortestenable, false );

	SetRenderState( m3drs_linethickness, 1 );
}

void CMuli3DDevice::SetDefaultTextureSamplerStates()
{
	const float32 fMipLODBias = 0.0f, fMaxMipLevel = 16.0f;
	for( uint32 iTextureSampler = 0; iTextureSampler < c_iMaxTextureSamplers; ++iTextureSampler )
	{
		SetTextureSamplerState( iTextureSampler, m3dtss_addressu, m3dta_wrap );
		SetTextureSamplerState( iTextureSampler, m3dtss_addressv, m3dta_wrap );
		SetTextureSamplerState( iTextureSampler, m3dtss_addressw, m3dta_wrap );
		SetTextureSamplerState( iTextureSampler, m3dtss_minfilter, m3dtf_linear );
		SetTextureSamplerState( iTextureSampler, m3dtss_magfilter, m3dtf_linear );
		SetTextureSamplerState( iTextureSampler, m3dtss_mipfilter, m3dtf_point );
		SetTextureSamplerState( iTextureSampler, m3dtss_miplodbias, *(uint32 *)&fMipLODBias );
		SetTextureSamplerState( iTextureSampler, m3dtss_maxmiplevel, *(uint32 *)&fMaxMipLevel );
	}
}

void CMuli3DDevice::SetDefaultClippingPlanes()
{
	m_RenderInfo.ClippingPlanes[m3dcp_left] = plane( 1, 0, 0, 1 );
	m_RenderInfo.ClippingPlanes[m3dcp_right] = plane( -1, 0, 0, 1 );
	m_RenderInfo.ClippingPlanes[m3dcp_top] = plane( 0, -1, 0, 1 );
	m_RenderInfo.ClippingPlanes[m3dcp_bottom] = plane( 0, 1, 0, 1 );
	m_RenderInfo.ClippingPlanes[m3dcp_near] = plane( 0, 0, 1, 0 );
	m_RenderInfo.ClippingPlanes[m3dcp_far] = plane( 0, 0, -1, 1 );

	// Enable the default clipping planes ...
	for( uint32 iPlane = m3dcp_left; iPlane <= m3dcp_far; ++iPlane )
		m_RenderInfo.bClippingPlaneEnabled[iPlane] = true;
}

result CMuli3DDevice::SetRenderState( m3drenderstate i_RenderState, uint32 i_iValue )
{
	if( i_RenderState >= m3drs_numrenderstates )
	{
		FUNC_FAILING( "CMuli3DDevice::SetRenderState: invalid renderstate.\n" );
		return e_invalidparameters;
	}

	m_iRenderStates[i_RenderState] = i_iValue;
	return s_ok;
}

result CMuli3DDevice::GetRenderState( m3drenderstate i_RenderState, uint32 &o_iValue )
{
	if( i_RenderState >= m3drs_numrenderstates )
	{
		o_iValue = 0;
		FUNC_FAILING( "CMuli3DDevice::GetRenderState: invalid renderstate.\n" );
		return e_invalidparameters;
	}

	o_iValue = m_iRenderStates[i_RenderState];
	return s_ok;
}

result CMuli3DDevice::SetVertexFormat( CMuli3DVertexFormat *i_pVertexFormat )
{
	m_pVertexFormat = i_pVertexFormat;
	if( !i_pVertexFormat )
		return s_ok;

	/// Initialize part of internal render-info structure.
	for( uint32 iReg = 0; iReg < c_iVertexShaderRegisters; ++iReg )
		m_RenderInfo.VSInputs[iReg] = m3dsrt_unused;

	const m3dvertexelement *pCurVertexElement = m_pVertexFormat->pGetElements();
	uint32 iElement = m_pVertexFormat->iGetNumVertexElements();
	while( iElement-- )
	{
		switch( pCurVertexElement->Type )
		{
		case m3dvet_float32: m_RenderInfo.VSInputs[pCurVertexElement->iRegister] = m3dsrt_float32; break;
		case m3dvet_vector2: m_RenderInfo.VSInputs[pCurVertexElement->iRegister] = m3dsrt_vector2; break;
		case m3dvet_vector3: m_RenderInfo.VSInputs[pCurVertexElement->iRegister] = m3dsrt_vector3; break;
		case m3dvet_vector4: m_RenderInfo.VSInputs[pCurVertexElement->iRegister] = m3dsrt_vector4; break;
		default: /* cannot happen */ FUNC_FAILING( "CMuli3DDevice::SetVertexFormat: invalid vertex element type.\n" ); return e_invalidparameters;
		}
		++pCurVertexElement;
	}

	return s_ok;
}

CMuli3DVertexFormat *CMuli3DDevice::pGetVertexFormat()
{
	if( m_pVertexFormat )
		m_pVertexFormat->AddRef();

	return m_pVertexFormat;
}

void CMuli3DDevice::SetPrimitiveAssembler( IMuli3DPrimitiveAssembler *i_pPrimitiveAssembler )
{
	m_pPrimitiveAssembler = i_pPrimitiveAssembler;
}

IMuli3DPrimitiveAssembler *CMuli3DDevice::pGetPrimitiveAssembler()
{
	if( m_pPrimitiveAssembler )
		m_pPrimitiveAssembler->AddRef();

	return m_pPrimitiveAssembler;
}

result CMuli3DDevice::SetVertexShader( IMuli3DVertexShader *i_pVertexShader )
{
	if( !i_pVertexShader )
	{
		m_pVertexShader = i_pVertexShader;
		return s_ok;
	}

	// Validate vertex shader register output types *phew*
	for( uint32 iReg = 0; iReg < c_iPixelShaderRegisters; ++iReg )
	{
		uint32 iType = i_pVertexShader->GetOutputRegisters( iReg );
		if( iType < m3dsrt_unused || iType > m3dsrt_vector4 )
		{
			FUNC_FAILING( "CMuli3DDevice::SetVertexShader: type of vertexshader output register is invalid.\n" );
			return e_invalidstate;
		}
	}

	m_pVertexShader = i_pVertexShader;

	return s_ok;
}

IMuli3DVertexShader *CMuli3DDevice::pGetVertexShader()
{
	if( m_pVertexShader )
		m_pVertexShader->AddRef();

	return m_pVertexShader;
}

void CMuli3DDevice::SetTriangleShader( IMuli3DTriangleShader *i_pTriangleShader )
{
	m_pTriangleShader = i_pTriangleShader;
}

IMuli3DTriangleShader *CMuli3DDevice::pGetTriangleShader()
{
	if( m_pTriangleShader )
		m_pTriangleShader->AddRef();

	return m_pTriangleShader;
}

result CMuli3DDevice::SetPixelShader( IMuli3DPixelShader *i_pPixelShader )
{
	if( !i_pPixelShader )
	{
		m_pPixelShader = i_pPixelShader;
		return s_ok;
	}

	m_pPixelShader = i_pPixelShader;
	return s_ok;
}

IMuli3DPixelShader *CMuli3DDevice::pGetPixelShader()
{
	if( m_pPixelShader )
		m_pPixelShader->AddRef();

	return m_pPixelShader;
}

result CMuli3DDevice::SetIndexBuffer( CMuli3DIndexBuffer *i_pIndexBuffer )
{
	m_pIndexBuffer = i_pIndexBuffer;
	return s_ok;
}

CMuli3DIndexBuffer *CMuli3DDevice::pGetIndexBuffer()
{
	if( m_pIndexBuffer )
		m_pIndexBuffer->AddRef();

	return m_pIndexBuffer;
}

result CMuli3DDevice::SetVertexStream( uint32 i_iStreamNumber, CMuli3DVertexBuffer *i_pVertexBuffer, uint32 i_iOffset, uint32 i_iStride )
{
	if( i_iStreamNumber >= c_iMaxVertexStreams )
	{
		FUNC_FAILING( "CMuli3DDevice::SetVertexStream: i_iStreamNumber exceeds number of available vertex streams.\n" );
		return e_invalidparameters;
	}

	if( !i_iStride )
	{
		FUNC_FAILING( "CMuli3DDevice::SetVertexStream: stride is 0.\n" );
		return e_invalidparameters;
	}

	m_VertexStreams[i_iStreamNumber].pVertexBuffer = i_pVertexBuffer;
	m_VertexStreams[i_iStreamNumber].iOffset = i_iOffset;
	m_VertexStreams[i_iStreamNumber].iStride = i_iStride;

	return s_ok;
}

result CMuli3DDevice::GetVertexStream( uint32 i_iStreamNumber, CMuli3DVertexBuffer **o_ppVertexBuffer, uint32 *o_pOffset, uint32 *o_pStride )
{
	if( i_iStreamNumber >= c_iMaxVertexStreams )
	{
		if( o_ppVertexBuffer ) *o_ppVertexBuffer = 0;
		if( o_pOffset ) *o_pOffset = 0;
		if( o_pStride ) *o_pStride = 0;
		FUNC_FAILING( "CMuli3DDevice::GetVertexStream: i_iStreamNumber exceeds number of available vertex streams.\n" );
		return e_invalidparameters;
	}

	if( o_ppVertexBuffer )
	{
		*o_ppVertexBuffer = m_VertexStreams[i_iStreamNumber].pVertexBuffer;
		if( m_VertexStreams[i_iStreamNumber].pVertexBuffer ) m_VertexStreams[i_iStreamNumber].pVertexBuffer->AddRef();
	}

	if( o_pOffset )
		*o_pOffset = m_VertexStreams[i_iStreamNumber].iOffset;

	if( o_pStride )
		*o_pStride = m_VertexStreams[i_iStreamNumber].iStride;

	return s_ok;
}

result CMuli3DDevice::SetTexture( uint32 i_iSamplerNumber, IMuli3DBaseTexture *i_pTexture )
{
	if( i_iSamplerNumber >= c_iMaxTextureSamplers )
	{
		FUNC_FAILING( "CMuli3DDevice::SetTexture: i_iSamplerNumber exceeds number of available texture samplers.\n" );
		return e_invalidparameters;
	}

	m_TextureSamplers[i_iSamplerNumber].pTexture = i_pTexture;
	if( i_pTexture )
		m_TextureSamplers[i_iSamplerNumber].TextureSampleInput = i_pTexture->eGetTexSampleInput();

	return s_ok;
}

result CMuli3DDevice::GetTexture( uint32 i_iSamplerNumber, IMuli3DBaseTexture **o_ppTexture )
{
	if( i_iSamplerNumber >= c_iMaxTextureSamplers )
	{
		*o_ppTexture = 0;
		FUNC_FAILING( "CMuli3DDevice::GetTexture: i_iSamplerNumber exceeds number of available texture samplers.\n" );
		return e_invalidparameters;
	}

	if( !o_ppTexture )
	{
		FUNC_FAILING( "CMuli3DDevice::GetTexture: parameter o_ppTexture points to null.\n" );
		return e_invalidparameters;
	}

	*o_ppTexture = m_TextureSamplers[i_iSamplerNumber].pTexture;
	if( m_TextureSamplers[i_iSamplerNumber].pTexture ) m_TextureSamplers[i_iSamplerNumber].pTexture->AddRef();
	return s_ok;
}

result CMuli3DDevice::SetTextureSamplerState( uint32 i_iSamplerNumber, m3dtexturesamplerstate i_TextureSamplerState, uint32 i_iState )
{
	if( i_iSamplerNumber >= c_iMaxTextureSamplers  )
	{
		FUNC_FAILING( "CMuli3DDevice::SetTextureSamplerState: i_iSamplerNumber exceeds number of available texture samplers.\n" );
		return e_invalidparameters;
	}

	if( i_TextureSamplerState >= m3dtss_numtexturesamplerstates )
	{
		FUNC_FAILING( "CMuli3DDevice::SetTextureSamplerState: invalid texture sampler state.\n" );
		return e_invalidparameters;
	}

	m_TextureSamplers[i_iSamplerNumber].iTextureSamplerStates[i_TextureSamplerState] = i_iState;
	return s_ok;
}

result CMuli3DDevice::GetTextureSamplerState( uint32 i_iSamplerNumber, m3dtexturesamplerstate i_TextureSamplerState, uint32 &o_iState )
{
	if( i_iSamplerNumber >= c_iMaxTextureSamplers )
	{
		o_iState = 0;
		FUNC_FAILING( "CMuli3DDevice::GetTextureSamplerState: i_iSamplerNumber exceeds number of available texture samplers.\n" );
		return e_invalidparameters;
	}

	if( i_TextureSamplerState >= m3dtss_numtexturesamplerstates )
	{
		FUNC_FAILING( "CMuli3DDevice::GetTextureSamplerState: invalid texture sampler state.\n" );
		return e_invalidparameters;
	}

	o_iState = m_TextureSamplers[i_iSamplerNumber].iTextureSamplerStates[i_TextureSamplerState];
	return s_ok;
}

result CMuli3DDevice::SampleTexture( vector4 &o_vColor, uint32 i_iSamplerNumber, float32 i_fU, float32 i_fV, float32 i_fW, const vector4 *i_pXGradient, const vector4 *i_pYGradient )
{
	if( i_iSamplerNumber >= c_iMaxTextureSamplers )
	{
		o_vColor = vector4( 0, 0, 0, 0 );
		FUNC_FAILING( "CMuli3DDevice::SampleTexture: i_iSamplerNumber exceeds number of available texture samplers.\n" );
		return e_invalidparameters;
	}

	const texturesampler &TextureSampler = m_TextureSamplers[i_iSamplerNumber];

	IMuli3DBaseTexture *pTexture = TextureSampler.pTexture;
	if( !pTexture )
	{
		o_vColor = vector4( 0, 0, 0, 0 );
		return s_ok;
	}

	switch( TextureSampler.TextureSampleInput )
	{
	case m3dtsi_vector:
		if( i_fU == 0.0f && i_fV == 0.0f && i_fW == 0.0f )
		{
			o_vColor = vector4( 0, 0, 0, 0 );
			FUNC_FAILING( "CMuli3DDevice::SampleTexture: sampling vector [u,v,w] = [0,0,0].\n" );
			return e_invalidparameters;
		}
		break;

	case m3dtsi_3coords:
		switch( TextureSampler.iTextureSamplerStates[m3dtss_addressw] )
		{
		case m3dta_wrap: i_fW -= ftol( i_fW );
		case m3dta_clamp: i_fW = fSaturate( i_fW ); break;
		default: o_vColor = vector4( 0, 0, 0, 0 ); FUNC_FAILING( "CMuli3DDevice::SampleTexture: value of texture sampler state m3dtss_addressw is invalid.\n" ); return e_invalidstate;
		}

	case m3dtsi_2coords:
		switch( TextureSampler.iTextureSamplerStates[m3dtss_addressu] )
		{
		case m3dta_wrap: i_fU -= ftol( i_fU );
		case m3dta_clamp: i_fU = fSaturate( i_fU ); break;
		default: o_vColor = vector4( 0, 0, 0, 0 ); FUNC_FAILING( "CMuli3DDevice::SampleTexture: value of texture sampler state m3dtss_addressu is invalid.\n" ); return e_invalidstate;
		}

		switch( TextureSampler.iTextureSamplerStates[m3dtss_addressv] )
		{
		case m3dta_wrap: i_fV -= ftol( i_fV );
		case m3dta_clamp: i_fV = fSaturate( i_fV ); break;
		default: o_vColor = vector4( 0, 0, 0, 0 ); FUNC_FAILING( "CMuli3DDevice::SampleTexture: value of texture sampler state m3dtss_addressv is invalid.\n" ); return e_invalidstate;
		}
		break;

	default:
		o_vColor = vector4( 0, 0, 0, 0 );
		FUNC_FAILING( "CMuli3DDevice::SampleTexture: invalid texture-sampling input!\n" );
		return e_invalidstate;
	}

	return pTexture->SampleTexture( o_vColor, i_fU, i_fV, i_fW,
		i_pXGradient, i_pYGradient, TextureSampler.iTextureSamplerStates );
}

void CMuli3DDevice::SetRenderTarget( CMuli3DRenderTarget *i_pRenderTarget )
{
	m_pRenderTarget = i_pRenderTarget;
}

CMuli3DRenderTarget *CMuli3DDevice::pGetRenderTarget()
{
	if( m_pRenderTarget )
		m_pRenderTarget->AddRef();

	return m_pRenderTarget;
}

result CMuli3DDevice::SetScissorRect( const m3drect &i_ScissorRect )
{
	if( i_ScissorRect.iLeft >= i_ScissorRect.iRight ||
		i_ScissorRect.iTop >= i_ScissorRect.iBottom )
	{
		FUNC_FAILING( "CMuli3DDevice::SetScissorRect: invalid scissor rect!\n" );
		return e_invalidparameters;
	}

	m_ScissorRect = i_ScissorRect;

	// Construct planes for clipping to the scissor rect.
	m_RenderInfo.ScissorPlanes[0] = plane(  1,  0,  0, -(float32)m_ScissorRect.iLeft );
	m_RenderInfo.ScissorPlanes[1] = plane( -1,  0,  0, (float32)m_ScissorRect.iRight );
	m_RenderInfo.ScissorPlanes[2] = plane(  0,  1,  0, -(float32)m_ScissorRect.iTop );
	m_RenderInfo.ScissorPlanes[3] = plane(  0, -1,  0, (float32)m_ScissorRect.iBottom );

	return s_ok;
}

m3drect CMuli3DDevice::GetScissorRect()
{
	return m_ScissorRect;
}

result CMuli3DDevice::SetDepthBounds( float32 i_fMinZ, float32 i_fMaxZ )
{
	if( i_fMinZ < 0 || i_fMinZ > 1 ||
		i_fMaxZ < 0 || i_fMaxZ > 1 ||
		i_fMinZ >= i_fMaxZ )
	{
		FUNC_FAILING( "CMuli3DDevice::SetDepthBounds: invalid depth bounds!\n" );
		return e_invalidparameters;
	}

	m_RenderInfo.ClippingPlanes[m3dcp_near].d = i_fMinZ;
	m_RenderInfo.ClippingPlanes[m3dcp_far].d = i_fMaxZ;
	return s_ok;
}

void CMuli3DDevice::GetDepthBounds( float32 &o_fMinZ, float32 &o_fMaxZ )
{
	o_fMinZ = m_RenderInfo.ClippingPlanes[m3dcp_near].d;
	o_fMaxZ = m_RenderInfo.ClippingPlanes[m3dcp_far].d;
}

result CMuli3DDevice::SetClippingPlane( m3dclippingplanes i_eIndex, const plane *i_pPlane )
{
	if( i_eIndex < m3dcp_user0 || i_eIndex >= m3dcp_numplanes )
		return e_invalidparameters;

	if( i_pPlane != 0 )
	{
		m_RenderInfo.ClippingPlanes[i_eIndex] = *i_pPlane;
		m_RenderInfo.bClippingPlaneEnabled[i_eIndex] = true;
	}
	else
		m_RenderInfo.bClippingPlaneEnabled[i_eIndex] = false;

	return s_ok;
}

result CMuli3DDevice::GetClippingPlane( m3dclippingplanes i_eIndex, plane &o_plane )
{
	if( i_eIndex < m3dcp_user0 || i_eIndex >= m3dcp_numplanes )
		return e_invalidparameters;

	if( !m_RenderInfo.bClippingPlaneEnabled[i_eIndex] )
		return e_invalidstate;

	o_plane = m_RenderInfo.ClippingPlanes[i_eIndex];
	return s_ok;
}

uint32 CMuli3DDevice::iGetRenderedPixels()
{
	return m_RenderInfo.iRenderedPixels;
}

result CMuli3DDevice::CreateVertexFormat( CMuli3DVertexFormat **o_ppVertexFormat, const m3dvertexelement *i_pVertexDeclaration, uint32 i_iVertexDeclSize )
{
	if( !o_ppVertexFormat )
	{
		FUNC_FAILING( "CMuli3DDevice::CreateVertexFormat: parameter o_ppVertexFormat points to null.\n" );
		return e_invalidparameters;
	}

	*o_ppVertexFormat = new CMuli3DVertexFormat( this );
	if( !(*o_ppVertexFormat) )
	{
		FUNC_FAILING( "CMuli3DDevice::CreateVertexFormat: out of memory, cannot create vertex format.\n" );
		return e_outofmemory;
	}

	result resCreate = (*o_ppVertexFormat)->Create( i_pVertexDeclaration, i_iVertexDeclSize );
	if( FUNC_FAILED( resCreate ) )
	{
		SAFE_RELEASE( *o_ppVertexFormat );
		return resCreate;
	}

	return s_ok;
}

result CMuli3DDevice::CreateIndexBuffer( CMuli3DIndexBuffer **o_ppIndexBuffer, uint32 i_iLength, m3dformat i_fmtFormat )
{
	if( !o_ppIndexBuffer )
	{
		FUNC_FAILING( "CMuli3DDevice::CreateIndexBuffer: parameter o_ppIndexBuffer points to null.\n" );
		return e_invalidparameters;
	}

	*o_ppIndexBuffer = new CMuli3DIndexBuffer( this );
	if( !(*o_ppIndexBuffer) )
	{
		FUNC_FAILING( "CMuli3DDevice::CreateIndexBuffer: out of memory, cannot create indexbuffer.\n" );
		return e_outofmemory;
	}

	result resCreate = (*o_ppIndexBuffer)->Create( i_iLength, i_fmtFormat );
	if( FUNC_FAILED( resCreate ) )
	{
		SAFE_RELEASE( *o_ppIndexBuffer );
		return resCreate;
	}

	return s_ok;
}

result CMuli3DDevice::CreateVertexBuffer( CMuli3DVertexBuffer **o_ppVertexBuffer, uint32 i_iLength )
{
	if( !o_ppVertexBuffer )
	{
		FUNC_FAILING( "CMuli3DDevice::CreateVertexBuffer: parameter o_ppVertexBuffer points to null.\n" );
		return e_invalidparameters;
	}

	*o_ppVertexBuffer = new CMuli3DVertexBuffer( this );
	if( !(*o_ppVertexBuffer) )
	{
		FUNC_FAILING( "CMuli3DDevice::CreateVertexBuffer: out of memory, cannot create vertexbuffer.\n" );
		return e_outofmemory;
	}

	result resCreate = (*o_ppVertexBuffer)->Create( i_iLength );
	if( FUNC_FAILED( resCreate ) )
	{
		SAFE_RELEASE( *o_ppVertexBuffer );
		return resCreate;
	}

	return s_ok;
}

result CMuli3DDevice::CreateSurface( CMuli3DSurface **o_ppSurface, uint32 i_iWidth, uint32 i_iHeight, m3dformat i_fmtFormat )
{
	if( !o_ppSurface )
	{
		FUNC_FAILING( "CMuli3DDevice::CreateSurface: parameter o_ppSurface points to null.\n" );
		return e_invalidparameters;
	}

	*o_ppSurface = new CMuli3DSurface( this );
	if( !(*o_ppSurface) )
	{
		FUNC_FAILING( "CMuli3DDevice::CreateSurface: out of memory, cannot create surface.\n" );
		return e_outofmemory;
	}

	result resCreate = (*o_ppSurface)->Create( i_iWidth, i_iHeight, i_fmtFormat );
	if( FUNC_FAILED( resCreate ) )
	{
		SAFE_RELEASE( *o_ppSurface );
		return resCreate;
	}

	return s_ok;
}

result CMuli3DDevice::CreateTexture( CMuli3DTexture **o_ppTexture, uint32 i_iWidth, uint32 i_iHeight, uint32 i_iMipLevels, m3dformat i_fmtFormat )
{
	if( !o_ppTexture )
	{
		FUNC_FAILING( "CMuli3DDevice::CreateTexture: parameter o_ppTexture points to null.\n" );
		return e_invalidparameters;
	}

	*o_ppTexture = new CMuli3DTexture( this );
	if( !(*o_ppTexture) )
	{
		FUNC_FAILING( "CMuli3DDevice::CreateTexture: out of memory, cannot create texture.\n" );
		return e_outofmemory;
	}

	result resCreate = (*o_ppTexture)->Create( i_iWidth, i_iHeight, i_iMipLevels, i_fmtFormat );
	if( FUNC_FAILED( resCreate ) )
	{
		SAFE_RELEASE( *o_ppTexture );
		return resCreate;
	}

	return s_ok;
}

result CMuli3DDevice::CreateCubeTexture( CMuli3DCubeTexture **o_ppCubeTexture, uint32 i_iEdgeLength, uint32 i_iMipLevels, m3dformat i_fmtFormat )
{
	if( !o_ppCubeTexture )
	{
		FUNC_FAILING( "CMuli3DDevice::CreateCubeTexture: parameter o_ppCubeTexture points to null.\n" );
		return e_invalidparameters;
	}

	*o_ppCubeTexture = new CMuli3DCubeTexture( this );
	if( !(*o_ppCubeTexture) )
	{
		FUNC_FAILING( "CMuli3DDevice::CreateCubeTexture: out of memory, cannot create cube-texture.\n" );
		return e_outofmemory;
	}

	result resCreate = (*o_ppCubeTexture)->Create( i_iEdgeLength, i_iMipLevels, i_fmtFormat );
	if( FUNC_FAILED( resCreate ) )
	{
		SAFE_RELEASE( *o_ppCubeTexture );
		return resCreate;
	}

	return s_ok;
}

result CMuli3DDevice::CreateVolume( CMuli3DVolume **o_ppSurface, uint32 i_iWidth, uint32 i_iHeight, uint32 i_iDepth, m3dformat i_fmtFormat )
{
	if( !o_ppSurface )
	{
		FUNC_FAILING( "CMuli3DDevice::CreateVolume: parameter o_ppSurface points to null.\n" );
		return e_invalidparameters;
	}

	*o_ppSurface = new CMuli3DVolume( this );
	if( !(*o_ppSurface) )
	{
		FUNC_FAILING( "CMuli3DDevice::CreateVolume: out of memory, cannot create surface.\n" );
		return e_outofmemory;
	}

	result resCreate = (*o_ppSurface)->Create( i_iWidth, i_iHeight, i_iDepth, i_fmtFormat );
	if( FUNC_FAILED( resCreate ) )
	{
		SAFE_RELEASE( *o_ppSurface );
		return resCreate;
	}

	return s_ok;
}

result CMuli3DDevice::CreateVolumeTexture( CMuli3DVolumeTexture **o_ppVolumeTexture, uint32 i_iWidth, uint32 i_iHeight, uint32 i_iDepth, uint32 i_iMipLevels, m3dformat i_fmtFormat )
{
	if( !o_ppVolumeTexture )
	{
		FUNC_FAILING( "CMuli3DDevice::CreateVolumeTexture: parameter o_ppVolumeTexture points to null.\n" );
		return e_invalidparameters;
	}

	*o_ppVolumeTexture = new CMuli3DVolumeTexture( this );
	if( !(*o_ppVolumeTexture) )
	{
		FUNC_FAILING( "CMuli3DDevice::CreateVolumeTexture: out of memory, cannot create cube-texture.\n" );
		return e_outofmemory;
	}

	result resCreate = (*o_ppVolumeTexture)->Create( i_iWidth, i_iHeight, i_iDepth, i_iMipLevels, i_fmtFormat );
	if( FUNC_FAILED( resCreate ) )
	{
		SAFE_RELEASE( *o_ppVolumeTexture );
		return resCreate;
	}

	return s_ok;
}

result CMuli3DDevice::CreateRenderTarget( CMuli3DRenderTarget **o_ppRenderTarget )
{
	if( !o_ppRenderTarget )
	{
		FUNC_FAILING( "CMuli3DDevice::CreateRenderTarget: parameter o_ppRenderTarget points to null.\n" );
		return e_invalidparameters;
	}

	*o_ppRenderTarget = new CMuli3DRenderTarget( this );
	if( !(*o_ppRenderTarget) )
	{
		FUNC_FAILING( "CMuli3DDevice::CreateRenderTarget: out of memory, cannot create rendertarget.\n" );
		return e_outofmemory;
	}

	return s_ok;
}

CMuli3D *CMuli3DDevice::pGetMuli3D()
{
	if( m_pParent )
		m_pParent->AddRef();
	return m_pParent;
}

const m3ddeviceparameters &CMuli3DDevice::GetDeviceParameters()
{
	return m_DeviceParameters;
}

result CMuli3DDevice::Present( CMuli3DRenderTarget *i_pRenderTarget )
{
	if( !i_pRenderTarget )
	{
		FUNC_FAILING( "CMuli3DDevice::Present: parameter i_pRenderTarget points to null.\n" );
		return e_invalidparameters;
	}

	// Get pointer to the colorbuffer of the rendertarget ---------------------
	CMuli3DSurface *pColorBuffer = i_pRenderTarget->pGetColorBuffer();
	if( !pColorBuffer )
	{
		FUNC_FAILING( "CMuli3DDevice::Present: rendertarget doesn't have a colorbuffer attached\n" );
		return e_invalidstate;
	}

	if( pColorBuffer->iGetWidth() != m_DeviceParameters.iBackbufferWidth ||
		pColorBuffer->iGetHeight() != m_DeviceParameters.iBackbufferHeight )
	{
		SAFE_RELEASE( pColorBuffer );
		FUNC_FAILING( "CMuli3DDevice::Present: colorbuffer's dimensions don't match backbuffer\n" );
		return e_invalidstate;
	}

	const uint32 iFloats = pColorBuffer->iGetFormatFloats();
	if( iFloats < 3 )
	{
		SAFE_RELEASE( pColorBuffer );
		FUNC_FAILING( "CMuli3DDevice::Present: invalid colorbuffer format - only m3dfmt_r32g32b32f and m3dfmt_r32g32b32a32f are supported!\n" );
		return e_invalidformat;
	}

	const float32 *pSource;
	if( FUNC_FAILED( pColorBuffer->LockRect( (void **)&pSource, 0 ) ) )
	{
		SAFE_RELEASE( pColorBuffer );
		FUNC_FAILING( "CMuli3DDevice::Present: couldn't access colorbuffer.\n" );
		return e_unknown;
	}

	result resPresent = m_pPresentTarget->Present( pSource, iFloats );

	pColorBuffer->UnlockRect();

	SAFE_RELEASE( pColorBuffer );

	return resPresent;
}

result CMuli3DDevice::PreRender()
{
	if( !m_pVertexFormat )
	{
		FUNC_FAILING( "CMuli3DDevice::PreRender: no vertex format has been set.\n" );
		return e_invalidstate;
	}

	if( !m_pVertexShader )
	{
		FUNC_FAILING( "CMuli3DDevice::PreRender: no vertex shader has been set.\n" );
		return e_invalidstate;
	}

	if( !m_pPixelShader )
	{
		FUNC_FAILING( "CMuli3DDevice::PreRender: no pixel shader has been set.\n" );
		return e_invalidstate;
	}

	if( !m_pRenderTarget )
	{
		FUNC_FAILING( "CMuli3DDevice::PreRender: no rendertarget has been set.\n" );
		return e_invalidstate;
	}

	const matrix44 &matViewportMatrix = m_pRenderTarget->matGetViewportMatrix();
	m_RenderInfo.ViewportRect.iLeft = (uint32)(matViewportMatrix._41 - matViewportMatrix._11);
	m_RenderInfo.ViewportRect.iRight = (uint32)(matViewportMatrix._41 + matViewportMatrix._11);
	m_RenderInfo.ViewportRect.iTop = (uint32)(matViewportMatrix._42 + matViewportMatrix._22);
	m_RenderInfo.ViewportRect.iBottom = (uint32)(matViewportMatrix._42 - matViewportMatrix._22);

	if( m_RenderInfo.ViewportRect.iLeft >= m_RenderInfo.ViewportRect.iRight ||
		m_RenderInfo.ViewportRect.iTop >= m_RenderInfo.ViewportRect.iBottom )
	{
		FUNC_FAILING( "CMuli3DDevice::PreRender: invalid viewport matrix.\n" );
		return e_invalidstate;
	}

	CMuli3DSurface *pColorBuffer = m_pRenderTarget->pGetColorBuffer();
	CMuli3DSurface *pDepthBuffer = m_pRenderTarget->pGetDepthBuffer();
	if( !pColorBuffer && !pDepthBuffer )
	{
		FUNC_FAILING( "CMuli3DDevice::PreRender: rendertarget has no associated frame- and depthbuffer.\n" );
		return e_invalidstate;
	}

	if( pColorBuffer && ( pColorBuffer->iGetWidth() < m_RenderInfo.ViewportRect.iRight ||
		pColorBuffer->iGetHeight() < m_RenderInfo.ViewportRect.iBottom ) )
	{
		FUNC_FAILING( "CMuli3DDevice::PreRender: colorbuffer's dimensions are smaller than set viewport.\n" );
		SAFE_RELEASE( pColorBuffer );
		SAFE_RELEASE( pDepthBuffer );
		return e_invalidstate;
	}

	if( pDepthBuffer && ( pDepthBuffer->iGetWidth() < m_RenderInfo.ViewportRect.iRight ||
		pDepthBuffer->iGetHeight() < m_RenderInfo.ViewportRect.iBottom ) )
	{
		FUNC_FAILING( "CMuli3DDevice::PreRender: depthbuffers's dimensions are smaller than set viewport.\n" );
		SAFE_RELEASE( pColorBuffer );
		SAFE_RELEASE( pDepthBuffer );
		return e_invalidstate;
	}

	SAFE_RELEASE( pColorBuffer );
	SAFE_RELEASE( pDepthBuffer );

	const vertexstream *pCurVertexStream = m_VertexStreams;
	for( uint32 iStream = 0; iStream <= m_pVertexFormat->iGetHighestStream(); ++iStream, ++pCurVertexStream )
	{
		if( !pCurVertexStream->pVertexBuffer )
		{
			FUNC_FAILING( "CMuli3DDevice::PreRender: vertex format references an empty vertex stream.\n" );
			return e_invalidstate;
		}
	}

	// Check status of scissor-testing ----------------------------------------
	if( m_iRenderStates[m3drs_scissortestenable] )
	{
		if( !( m_ScissorRect.iLeft >= m_RenderInfo.ViewportRect.iLeft && m_ScissorRect.iLeft < m_RenderInfo.ViewportRect.iRight ) ||
			!( m_ScissorRect.iRight > m_RenderInfo.ViewportRect.iLeft && m_ScissorRect.iRight <= m_RenderInfo.ViewportRect.iRight ) ||
			!( m_ScissorRect.iTop >= m_RenderInfo.ViewportRect.iTop && m_ScissorRect.iTop < m_RenderInfo.ViewportRect.iBottom ) ||
			!( m_ScissorRect.iBottom > m_RenderInfo.ViewportRect.iTop && m_ScissorRect.iBottom <= m_RenderInfo.ViewportRect.iBottom ) )
		{
			FUNC_FAILING( "CMuli3DDevice::PreRender: scissor rect exceeds viewport's dimensions.\n" );
			return e_invalidstate;
		}
	}

	// Check line-thickness ---------------------------------------------------
	if( m_iRenderStates[m3drs_linethickness] == 0 )
	{
		FUNC_FAILING( "CMuli3DDevice::PreRender: line-thickness is invalid.\n" );
		return e_invalidstate;
	}


	// Check if renderstates for subdivision-mode are valid -------------------
	switch( m_iRenderStates[m3drs_subdivisionmode] )
	{
	case m3dsubdiv_none: break;
	case m3dsubdiv_simple:
		if( !m_iRenderStates[m3drs_subdivisionlevels] )
		{
			FUNC_FAILING( "CMuli3DDevice::PreRender: subdivisionlevels for simple-subdivision are 0.\n" );
			return e_invalidstate;
		}
		else
			break;

	case m3dsubdiv_smooth:
		if( !m_iRenderStates[m3drs_subdivisionlevels] )
		{
			FUNC_FAILING( "CMuli3DDevice::PreRender: subdivisionlevels for smooth-subdivision are 0.\n" );
			return e_invalidstate;
		}
		else if( m_iRenderStates[m3drs_subdivisionpositionregister] >= c_iVertexShaderRegisters )
		{
			FUNC_FAILING( "CMuli3DDevice::PreRender: position register for smooth-subdivision exceeds number of avilable vertex shader input registers.\n" );
			return e_invalidstate;
		}
		else if( m_iRenderStates[m3drs_subdivisionnormalregister] >= c_iVertexShaderRegisters )
		{
			FUNC_FAILING( "CMuli3DDevice::PreRender: position register for smooth-subdivision exceeds number of avilable vertex shader input registers.\n" );
			return e_invalidstate;
		}
		else
			break;

	case m3dsubdiv_adaptive:
		if( !m_iRenderStates[m3drs_subdivisionlevels] && !m_iRenderStates[m3drs_subdivisionmaxinnerlevels] )
		{
			FUNC_FAILING( "CMuli3DDevice::PreRender: both subdivisionlevels for adaptive-subdivision are 0.\n" );
			return e_invalidstate;
		}
		else if( *(float32 *)&m_iRenderStates[m3drs_subdivisionmaxscreenarea] <= 0.0f )
		{
			FUNC_FAILING( "CMuli3DDevice::PreRender: max screenarea for smooth-subdivision is <= 0.0f.\n" );
			return e_invalidstate;
		}
		else
			break;

	default: FUNC_FAILING( "CMuli3DDevice::PreRender: value of renderstate m3drs_subdivisionmode is invalid.\n" ); return e_invalidstate;
	}

	// Check for valid device-states, which won't produce any output ----------
	if( m_iRenderStates[m3drs_zenable] && m_iRenderStates[m3drs_zfunc] == m3dcmp_never )
	{
		FUNC_NOTIFY( "CMuli3DDevice::PreRender: nothing will be rendered - depthbuffering has been enabled and the compare-function has been set to m3dcmp_never - nothing will be rendered to the screen.\n" );
	}

	if( m_iRenderStates[m3drs_scissortestenable] )
	{
		if( m_ScissorRect.iLeft == m_ScissorRect.iRight ||
			m_ScissorRect.iTop == m_ScissorRect.iBottom )
		{
			FUNC_NOTIFY( "CMuli3DDevice::PreRender: nothing will be rendered - scissor testing is enabled and scissor rect spans across an area of 0 pixels.\n" );
		}
	}

	if( !m_iRenderStates[m3drs_colorwriteenable] && ( !m_iRenderStates[m3drs_zenable] || !m_iRenderStates[m3drs_zwriteenable] ) )
	{
		FUNC_NOTIFY( "CMuli3DDevice::PreRender: nothing will be rendered - writing to the colorbuffer and the depthbuffer has been disabled.\n" );
	}

	// TODO? add more checks

	// Initialize internal render-info structure ------------------------------

	// note: m_RenderInfo.ShaderInputRegisterType is initialized when a vertex format is set

	// Store output types in the internal render-info structure.
	for( uint32 iReg = 0; iReg < c_iPixelShaderRegisters; ++iReg )
		m_RenderInfo.VSOutputs[iReg] = m_pVertexShader->GetOutputRegisters( iReg );

	// Get colorbuffer-related states -----------------------------------------
	pColorBuffer = m_pRenderTarget->pGetColorBuffer();
	if( pColorBuffer )
	{
		result resBuffer = pColorBuffer->LockRect( (void **)&m_RenderInfo.pFrameData, 0 );
		if( FUNC_FAILED( resBuffer ) )
		{
			FUNC_NOTIFY( "CMuli3DDevice::PreRender: couldn't access colorbuffer.\n" );
			SAFE_RELEASE( pColorBuffer );
			return resBuffer;
		}

		m_RenderInfo.iColorFloats = pColorBuffer->iGetFormatFloats();
		if( !m_RenderInfo.iColorFloats )
		{
			pColorBuffer->UnlockRect();
			SAFE_RELEASE( pColorBuffer );
			return e_unknown;
		}

		m_RenderInfo.iColorBufferPitch = pColorBuffer->iGetWidth() * m_RenderInfo.iColorFloats;
		m_RenderInfo.bColorWrite = m_iRenderStates[m3drs_colorwriteenable] ? true : false;
	}
	else
	{
		m_RenderInfo.pFrameData = 0;
		m_RenderInfo.iColorFloats = 0;
		m_RenderInfo.iColorBufferPitch = 0;
		m_RenderInfo.bColorWrite = false;
	}

	// Get depthbuffer-related states -----------------------------------------
	pDepthBuffer = m_iRenderStates[m3drs_zenable] ? m_pRenderTarget->pGetDepthBuffer() : 0;
	if( pDepthBuffer )
	{
		result resBuffer = pDepthBuffer->LockRect( (void **)&m_RenderInfo.pDepthData, 0 );
		if( FUNC_FAILED( resBuffer ) )
		{
			FUNC_NOTIFY( "CMuli3DDevice::PreRender: couldn't access depthbuffer.\n" );
			if( m_RenderInfo.pFrameData ) pColorBuffer->UnlockRect();
			SAFE_RELEASE( pColorBuffer );
			SAFE_RELEASE( pDepthBuffer );
			return resBuffer;
		}

		m_RenderInfo.iDepthBufferPitch = pDepthBuffer->iGetWidth();
		m_RenderInfo.DepthCompare = (m3dcmpfunc)m_iRenderStates[m3drs_zfunc];
		m_RenderInfo.bDepthWrite = m_iRenderStates[m3drs_zwriteenable] ? true : false;
	}
	else
	{
		m_RenderInfo.pDepthData = 0;
		m_RenderInfo.iDepthBufferPitch = 0;
		m_RenderInfo.DepthCompare = m3dcmp_always;
		m_RenderInfo.bDepthWrite = false;
	}

	SAFE_RELEASE( pColorBuffer );
	SAFE_RELEASE( pDepthBuffer );

	// reset pixel-counter to 0
	m_RenderInfo.iRenderedPixels = 0;

	// Depending on m_pPixelShader->GetShaderOutput() chose the appropriate
	// RasterizeScanline-function and assign it to the function pointer
	switch( m_pPixelShader->GetShaderOutput() )
	{
	case m3dpso_coloronly:
		m_RenderInfo.fpRasterizeScanline = m_pPixelShader->bMightKillPixels() ? &CMuli3DDevice::RasterizeScanline_ColorOnly_MightKillPixels : &CMuli3DDevice::RasterizeScanline_ColorOnly;
		m_RenderInfo.fpDrawPixel = &CMuli3DDevice::DrawPixel_ColorOnly;
		break;
	case m3dpso_colordepth:
		m_RenderInfo.fpRasterizeScanline = &CMuli3DDevice::RasterizeScanline_ColorDepth;
		m_RenderInfo.fpDrawPixel = &CMuli3DDevice::DrawPixel_ColorDepth;
		break;
	default: FUNC_FAILING( "CMuli3DDevice::PreRender: type of pixelshader is invalid.\n" ); return e_invalidstate;
	}

	// Initialize shaders' pointer to the rendering device --------------------
	// have to do this right before drawing and not at set-time, because a shader
	// may be used with different devices ...
	m_pVertexShader->SetDevice( this );
	if( m_pTriangleShader ) m_pTriangleShader->SetDevice( this );
	m_pPixelShader->SetDevice( this );

	// Initialize pixel shader's pointers to info structures ------------------
	m_pPixelShader->SetInfo( m_RenderInfo.VSOutputs, &m_TriangleInfo );

	// Initialize vertex cache ------------------------------------------------
	m_iNumValidCacheEntries = 0;
	m_iFetchedVertices = 0;

	fpuTruncate(); // ftol() returns expected integer values
	return s_ok;
}

void CMuli3DDevice::PostRender()
{
	fpuReset(); // reset FPU to (default)rounding mode

	if( m_RenderInfo.pFrameData )
	{
		CMuli3DSurface *pColorBuffer = m_pRenderTarget->pGetColorBuffer();

		if( pColorBuffer )
			pColorBuffer->UnlockRect();

		SAFE_RELEASE( pColorBuffer );
	}

	if( m_RenderInfo.pDepthData )
	{
		CMuli3DSurface *pDepthBuffer = m_pRenderTarget->pGetDepthBuffer();

		if( pDepthBuffer )
			pDepthBuffer->UnlockRect();

		SAFE_RELEASE( pDepthBuffer );
	}
}

result CMuli3DDevice::DecodeVertexStream( m3dvsinput &o_VertexShaderInput, uint32 i_iVertex )
{
	const byte *pVertex[c_iMaxVertexStreams];
	const vertexstream *pCurVertexStream = m_VertexStreams;
	for( uint32 iStream = 0; iStream <= m_pVertexFormat->iGetHighestStream(); ++iStream, ++pCurVertexStream )
	{
		const uint32 iOffset = pCurVertexStream->iOffset + i_iVertex * pCurVertexStream->iStride;
		if( iOffset >= pCurVertexStream->pVertexBuffer->iGetLength() )
		{
			FUNC_FAILING( "CMuli3DDevice::DecodeVertexStream: vertex stream offset exceeds vertex buffer length.\n" );
			return e_unknown;
		}

		result resVB = pCurVertexStream->pVertexBuffer->GetPointer( iOffset, (void **)&pVertex[iStream] );
		if( FUNC_FAILED( resVB ) )
			return resVB;
	}

	// Fill vertex-info structure, which can be passed to the vertex shader,
	// with data from the vertex-streams, depending on the current vertexformat
	const m3dvertexelement *pCurVertexElement = m_pVertexFormat->pGetElements();
	uint32 iElement = m_pVertexFormat->iGetNumVertexElements();
	while( iElement-- )
	{
		shaderreg &Register = o_VertexShaderInput.ShaderInputs[pCurVertexElement->iRegister];
		const float32 *pData = (float32 *)pVertex[pCurVertexElement->iStream];
		switch( pCurVertexElement->Type )
		{
		case m3dvet_float32:
			pVertex[pCurVertexElement->iStream] += sizeof( float32 );
			Register = shaderreg( pData[0], 0, 0, 1 );
			break;
		case m3dvet_vector2:
			pVertex[pCurVertexElement->iStream] += 2 * sizeof( float32 );
			Register = shaderreg( pData[0], pData[1], 0, 1 );
			break;
		case m3dvet_vector3:
			pVertex[pCurVertexElement->iStream] += 3 * sizeof( float32 );
			Register = shaderreg( pData[0], pData[1], pData[2], 1 );
			break;
		case m3dvet_vector4:
			pVertex[pCurVertexElement->iStream] += 4 * sizeof( float32 );
			Register = shaderreg( pData[0], pData[1], pData[2], pData[3] );
			break;
		default: /* cannot happen */ break;
		}

		++pCurVertexElement;
	}

	return s_ok;
}

result CMuli3DDevice::FetchVertex( m3dvertexcacheentry **io_ppVertex, uint32 i_iVertex )
{
	// Check if the incoming point already points to the desired vertex.
	if( *io_ppVertex && (*io_ppVertex)->iVertexIndex == i_iVertex )
	{
		(*io_ppVertex)->iFetchTime = m_iFetchedVertices++;
		return s_ok;
	}

	const bool bCanAppend = ( m_iNumValidCacheEntries < c_iVertexCacheSize );

	// Find vertex in cache
	m3dvertexcacheentry *pCacheEntry = &m_VertexCache[0], *pDestEntry = pCacheEntry;
	for( uint32 iCurEntry = 0; iCurEntry < m_iNumValidCacheEntries; ++iCurEntry, ++pCacheEntry )
	{
		if( pCacheEntry->iVertexIndex == i_iVertex )
		{
			// Vertex is already in cache, return it.
			pCacheEntry->iFetchTime = m_iFetchedVertices++;
			*io_ppVertex = pCacheEntry;
			return s_ok;
		}

		// if( !bCanAppend ) // Profiling turned out, that leaving this away is faster because most of the time we can't append anyway.
		{
			// The vertex cache is full, so we have to look for the oldest entry and replace it in case we cannot find the desired vertex in the cache.
			if( pCacheEntry->iFetchTime < pDestEntry->iFetchTime )
				pDestEntry = pCacheEntry;
		}
	}

	if( bCanAppend ) // The cache is not full yet, so we can append a new vertex.
		pDestEntry = &m_VertexCache[m_iNumValidCacheEntries++];

	// Update the destination cache entry and return it -----------------------
	pDestEntry->iVertexIndex = i_iVertex;
	pDestEntry->iFetchTime = m_iFetchedVertices++;

	result resDecode = DecodeVertexStream( pDestEntry->VertexOutput.SourceInput, i_iVertex );
	if( FUNC_FAILED( resDecode ) )
		return resDecode;

	m_pVertexShader->Execute( pDestEntry->VertexOutput.SourceInput.ShaderInputs,
		pDestEntry->VertexOutput.vPosition, pDestEntry->VertexOutput.ShaderOutputs );

	*io_ppVertex = pDestEntry;

	return s_ok;
}

inline void CMuli3DDevice::ProcessTriangle( const m3dvsoutput *i_pVSOutput0, const m3dvsoutput *i_pVSOutput1, const m3dvsoutput *i_pVSOutput2 )
{
	switch( m_iRenderStates[m3drs_subdivisionmode] )
	{
	case m3dsubdiv_none: DrawTriangle( i_pVSOutput0, i_pVSOutput1, i_pVSOutput2 ); break;
	case m3dsubdiv_simple: SubdivideTriangle_Simple( 0, i_pVSOutput0, i_pVSOutput1, i_pVSOutput2 ); break;
	case m3dsubdiv_smooth: SubdivideTriangle_Smooth( 0, i_pVSOutput0, i_pVSOutput1, i_pVSOutput2 ); break;
	case m3dsubdiv_adaptive: SubdivideTriangle_Adaptive( i_pVSOutput0, i_pVSOutput1, i_pVSOutput2 ); break;
	default: /* cannot happen */ break;
	}
}

result CMuli3DDevice::DrawPrimitive( m3dprimitivetype i_PrimitiveType, uint32 i_iStartVertex, uint32 i_iPrimitiveCount )
{
	if( !i_iPrimitiveCount )
	{
		FUNC_FAILING( "CMuli3DDevice::DrawPrimitive: primitive count is 0.\n" );
		return e_invalidparameters;
	}

	// Process incoming vertices ----------------------------------------------
	uint32 iNumVertices;
	switch( i_PrimitiveType )
	{
	case m3dpt_trianglefan:	iNumVertices = i_iPrimitiveCount + 2; break;
	case m3dpt_trianglestrip: iNumVertices = i_iPrimitiveCount + 2; break;
	case m3dpt_trianglelist: iNumVertices = i_iPrimitiveCount * 3; break;
	default: FUNC_FAILING( "CMuli3DDevice::DrawPrimitive: invalid primitive type specified.\n" ); return e_invalidparameters;
	}

	result resCheck = PreRender();
	if( FUNC_FAILED( resCheck ) )
		return resCheck;

	uint32 iVertexIndices[3] = { i_iStartVertex, i_iStartVertex + 1, i_iStartVertex + 2 };
	bool bFlip = false; // used when drawing tristrips
	while( i_iPrimitiveCount-- )
	{
		m3dvertexcacheentry *pVertices[3] = { 0, 0, 0 };
		for( uint32 iVertex = 0; iVertex < 3; ++iVertex )
		{
			result resFetch = FetchVertex( &pVertices[iVertex], iVertexIndices[iVertex] );
			if( FUNC_FAILED( resFetch ) )
			{
				FUNC_FAILING( "CMuli3DDevice::DrawPrimitive: couldn't fetch vertex from streams.\n" );
				PostRender();
				return resFetch;
			}
		}

		if( bFlip )
			ProcessTriangle( &pVertices[0]->VertexOutput, &pVertices[2]->VertexOutput, &pVertices[1]->VertexOutput );
		else
			ProcessTriangle( &pVertices[0]->VertexOutput, &pVertices[1]->VertexOutput, &pVertices[2]->VertexOutput );

		// Prepare vertex-indices for the next triangle ...
		switch( i_PrimitiveType )
		{
		case m3dpt_trianglefan:
			iVertexIndices[1] = iVertexIndices[2];
			++iVertexIndices[2];
			break;

		case m3dpt_trianglestrip:
			bFlip = !bFlip;
			iVertexIndices[0] = iVertexIndices[1];
			iVertexIndices[1] = iVertexIndices[2];
			++iVertexIndices[2];
			break;

		case m3dpt_trianglelist:
			iVertexIndices[0] += 3; iVertexIndices[1] += 3; iVertexIndices[2] += 3;
			break;

		default: /* cannot happen */ break;
		}
	}

	PostRender();

	return s_ok;
}

result CMuli3DDevice::DrawIndexedPrimitive( m3dprimitivetype i_PrimitiveType, int32 i_iBaseVertexIndex, uint32 i_iMinIndex, uint32 i_iNumVertices, uint32 i_iStartIndex, uint32 i_iPrimitiveCount )
{
	if( !i_iPrimitiveCount )
	{
		FUNC_FAILING( "CMuli3DDevice::DrawIndexedPrimitive primitive count is 0.\n" );
		return e_invalidparameters;
	}

	if( !i_iNumVertices )
	{
		FUNC_FAILING( "CMuli3DDevice::DrawIndexedPrimitive: number of vertices is 0.\n" );
		return e_invalidparameters;
	}

	if( !( i_PrimitiveType == m3dpt_trianglefan || i_PrimitiveType == m3dpt_trianglestrip || i_PrimitiveType == m3dpt_trianglelist ) )
	{
		FUNC_FAILING( "CMuli3DDevice::DrawIndexedPrimitive: invalid primitive type specified.\n" );
		return e_invalidparameters;
	}

	if( !m_pIndexBuffer )
	{
		FUNC_FAILING( "CMuli3DDevice::DrawIndexedPrimitive: no indexbuffer has been set!\n" );
		return e_invalidstate;
	}

	result resCheck = PreRender();
	if( FUNC_FAILED( resCheck ) )
		return resCheck;

	uint32 iIndexIndices[3] = { i_iStartIndex, i_iStartIndex + 1, i_iStartIndex + 2 };
	bool bFlip = false; // used when drawing tristrips
	while( i_iPrimitiveCount-- )
	{
		m3dvertexcacheentry *pVertices[3] = { 0, 0, 0 };
		for( uint32 iVertex = 0; iVertex < 3; ++iVertex )
		{
			uint32 iVertexIndex;

			result resGetVertexIndex = m_pIndexBuffer->GetVertexIndex( iIndexIndices[iVertex], iVertexIndex );
			if( FUNC_FAILED( resGetVertexIndex ) )
			{
				FUNC_FAILING( "CMuli3DDevice::DrawIndexedPrimitive: couldn't read vertex index from indexbuffer.\n" );
				PostRender();
				return resGetVertexIndex;
			}

			result resFetch = FetchVertex( &pVertices[iVertex], iVertexIndex + i_iBaseVertexIndex );
			if( FUNC_FAILED( resFetch ) )
			{
				FUNC_FAILING( "CMuli3DDevice::DrawIndexedPrimitive: couldn't fetch vertex from streams.\n" );
				PostRender();
				return resFetch;
			}
		}

		if( bFlip )
			ProcessTriangle( &pVertices[0]->VertexOutput, &pVertices[2]->VertexOutput, &pVertices[1]->VertexOutput );
		else
			ProcessTriangle( &pVertices[0]->VertexOutput, &pVertices[1]->VertexOutput, &pVertices[2]->VertexOutput );

		// Prepare vertex-indices for the next triangle ...
		switch( i_PrimitiveType )
		{
		case m3dpt_trianglefan:
			iIndexIndices[1] = iIndexIndices[2];
			++iIndexIndices[2];
			break;

		case m3dpt_trianglestrip:
			bFlip = !bFlip;
			iIndexIndices[0] = iIndexIndices[1];
			iIndexIndices[1] = iIndexIndices[2];
			++iIndexIndices[2];
			break;

		case m3dpt_trianglelist:
			iIndexIndices[0] += 3; iIndexIndices[1] += 3; iIndexIndices[2] += 3;
			break;

		default: /* cannot happen */ break;
		}
	}

	PostRender();

	return s_ok;
}

result CMuli3DDevice::DrawDynamicPrimitive( uint32 i_iStartVertex, uint32 i_iNumVertices )
{
	if( !i_iNumVertices )
	{
		FUNC_FAILING( "CMuli3DDevice::DrawDynamicPrimitive: number of vertices is 0.\n" );
		return e_invalidparameters;
	}

	if( !m_pPrimitiveAssembler )
	{
		FUNC_FAILING( "CMuli3DDevice::DrawDynamicPrimitive: no primitive assembler has been set.\n" );
		return e_invalidstate;
	}

	result resCheck = PreRender();
	if( FUNC_FAILED( resCheck ) )
		return resCheck;

	std::vector<uint32> VertexIndices;
	m3dprimitivetype PrimitiveType = m_pPrimitiveAssembler->Execute( VertexIndices, i_iNumVertices );

	uint32 iPrimitiveCount;
	switch( PrimitiveType )
	{
	case m3dpt_trianglefan:	iPrimitiveCount = (uint32)VertexIndices.size() - 2; break;
	case m3dpt_trianglestrip: iPrimitiveCount = (uint32)VertexIndices.size() - 2; break;
	case m3dpt_trianglelist: iPrimitiveCount = (uint32)VertexIndices.size() / 3; break;
	default: FUNC_FAILING( "CMuli3DDevice::DrawPrimitive: invalid primitive type specified.\n" ); return e_invalidparameters;
	}

	if( !iPrimitiveCount )
		return s_ok;

	std::vector<uint32>::iterator pVertexIndexIterator = VertexIndices.begin();

	uint32 iVertexIndices[3] = { *pVertexIndexIterator++, *pVertexIndexIterator++, *pVertexIndexIterator++ };
	bool bFlip = false; // used when drawing tristrips
	while( iPrimitiveCount-- )
	{
		m3dvertexcacheentry *pVertices[3] = { 0, 0, 0 };
		for( uint32 iVertex = 0; iVertex < 3; ++iVertex )
		{
			result resFetch = FetchVertex( &pVertices[iVertex], iVertexIndices[iVertex] );
			if( FUNC_FAILED( resFetch ) )
			{
				FUNC_FAILING( "CMuli3DDevice::DrawIndexedPrimitive: couldn't fetch vertex from streams.\n" );
				PostRender();
				return resFetch;
			}
		}

		if( bFlip )
			ProcessTriangle( &pVertices[0]->VertexOutput, &pVertices[2]->VertexOutput, &pVertices[1]->VertexOutput );
		else
			ProcessTriangle( &pVertices[0]->VertexOutput, &pVertices[1]->VertexOutput, &pVertices[2]->VertexOutput );

		// Prepare vertex-indices for the next triangle ...
		switch( PrimitiveType )
		{
		case m3dpt_trianglefan:
			iVertexIndices[1] = iVertexIndices[2];
			iVertexIndices[2] = *pVertexIndexIterator++;
			break;

		case m3dpt_trianglestrip:
			bFlip = !bFlip;
			iVertexIndices[0] = iVertexIndices[1];
			iVertexIndices[1] = iVertexIndices[2];
			iVertexIndices[2] = *pVertexIndexIterator++;
			break;

		case m3dpt_trianglelist:
			iVertexIndices[0] = *pVertexIndexIterator++;
			iVertexIndices[1] = *pVertexIndexIterator++;
			iVertexIndices[2] = *pVertexIndexIterator++;
			break;

		default: /* cannot happen */ break;
		}
	}

	PostRender();

	return s_ok;
}

void CMuli3DDevice::InterpolateVertexShaderOutput( m3dvsoutput *o_pVSOutput, const m3dvsoutput *i_pVSOutputA, const m3dvsoutput *i_pVSOutputB, float32 i_fInterpolation )
{
	// interpolate vertex position
	vVector4Lerp( o_pVSOutput->vPosition, i_pVSOutputA->vPosition, i_pVSOutputB->vPosition, i_fInterpolation );

	// interpolate registers
	shaderreg *pO = o_pVSOutput->ShaderOutputs;
	const shaderreg *pA = i_pVSOutputA->ShaderOutputs;
	const shaderreg *pB = i_pVSOutputB->ShaderOutputs;
	for( uint32 iReg = 0; iReg < c_iPixelShaderRegisters; ++iReg, ++pO, ++pA, ++pB )
	{
		switch( m_RenderInfo.VSOutputs[iReg] )
		{
		case m3dsrt_vector4:
			pO->w = fLerp( pA->w, pB->w, i_fInterpolation );
		case m3dsrt_vector3:
			pO->z = fLerp( pA->z, pB->z, i_fInterpolation );
		case m3dsrt_vector2:
			pO->y = fLerp( pA->y, pB->y, i_fInterpolation );
		case m3dsrt_float32:
			pO->x = fLerp( pA->x, pB->x, i_fInterpolation );
		case m3dsrt_unused:
		default: // cannot happen
			break;
		}
	}
}

inline void CMuli3DDevice::MultiplyVertexShaderOutputRegisters( m3dvsoutput *o_pDest, const m3dvsoutput *i_pSrc, float32 i_fVal )
{
	shaderreg *pDest = o_pDest->ShaderOutputs;
	const shaderreg *pSrc = i_pSrc->ShaderOutputs;
	for( uint32 iReg = 0; iReg < c_iPixelShaderRegisters; ++iReg, ++pDest, ++pSrc )
	{
		switch( m_RenderInfo.VSOutputs[iReg] )
		{
		case m3dsrt_vector4:
			pDest->w = pSrc->w * i_fVal;
		case m3dsrt_vector3:
			pDest->z = pSrc->z * i_fVal;
		case m3dsrt_vector2:
			pDest->y = pSrc->y * i_fVal;
		case m3dsrt_float32:
			pDest->x = pSrc->x * i_fVal;
		case m3dsrt_unused:
		default: // cannot happen
			break;
		}
	}
}

void CMuli3DDevice::InterpolateVertexShaderInput( m3dvsinput *o_pVSInput, const m3dvsinput *i_pVSInputA, const m3dvsinput *i_pVSInputB, float32 i_fInterpolation )
{
	// interpolate registers
	shaderreg *pO = o_pVSInput->ShaderInputs;
	const shaderreg *pA = i_pVSInputA->ShaderInputs;
	const shaderreg *pB = i_pVSInputB->ShaderInputs;

	for( uint32 iReg = 0; iReg < c_iVertexShaderRegisters; ++iReg, ++pO, ++pA, ++pB )
	{
		switch( m_RenderInfo.VSInputs[iReg] )
		{
		case m3dsrt_float32:
			pO->x = fLerp( pA->x, pB->x, i_fInterpolation );
			pO->y = 0.0f; pO->z = 0.0f; pO->w = 1.0f;
			break;
		case m3dsrt_vector2:
			pO->x = fLerp( pA->x, pB->x, i_fInterpolation );
			pO->y = fLerp( pA->y, pB->y, i_fInterpolation );
			pO->z = 0.0f; pO->w = 1.0f;
			break;
		case m3dsrt_vector3:
			pO->x = fLerp( pA->x, pB->x, i_fInterpolation );
			pO->y = fLerp( pA->y, pB->y, i_fInterpolation );
			pO->z = fLerp( pA->z, pB->z, i_fInterpolation );
			pO->w = 1.0f;
			break;
		case m3dsrt_vector4:
			pO->x = fLerp( pA->x, pB->x, i_fInterpolation );
			pO->y = fLerp( pA->y, pB->y, i_fInterpolation );
			pO->z = fLerp( pA->z, pB->z, i_fInterpolation );
			pO->w = fLerp( pA->w, pB->w, i_fInterpolation );
			break;
		case m3dsrt_unused:
		default: // cannot happen
			break;
		}
	}
}

inline void CMuli3DDevice::ProjectVertex( m3dvsoutput *io_pVSOutput )
{
	if( io_pVSOutput->vPosition.w < FLT_EPSILON )
		return;

	const float32 fInvW = 1.0f / io_pVSOutput->vPosition.w;
	io_pVSOutput->vPosition.x *= fInvW;
	io_pVSOutput->vPosition.y *= fInvW;
	io_pVSOutput->vPosition.z *= fInvW;
	io_pVSOutput->vPosition.w = 1.0f;

	io_pVSOutput->vPosition *= m_pRenderTarget->matGetViewportMatrix();

	// divide shader output registers by w; this way we can interpolate them linearly while rasterizing ...
	io_pVSOutput->vPosition.w = fInvW;
	MultiplyVertexShaderOutputRegisters( io_pVSOutput, io_pVSOutput, io_pVSOutput->vPosition.w );
}

// TRIANGLES ------------------------------------------------------------------

void CMuli3DDevice::SubdivideTriangle_Simple( uint32 i_iSubdivisionLevel, const m3dvsoutput *i_pVSOutput0, const m3dvsoutput *i_pVSOutput1, const m3dvsoutput *i_pVSOutput2 )
{
	// In case the triangle has been subdivided to the requested level, draw it ...
	if( i_iSubdivisionLevel >= m_iRenderStates[m3drs_subdivisionlevels] )
	{
		DrawTriangle( i_pVSOutput0, i_pVSOutput1, i_pVSOutput2 );
		return;
	}

	++i_iSubdivisionLevel;

	// Generate three new vertices: in the middle of each edge
	// Interpolate inputs for the new vertices (we're splitting the triangle's edges)
	m3dvsoutput NewVSOutputs[3];
	InterpolateVertexShaderInput( &NewVSOutputs[0].SourceInput, &i_pVSOutput0->SourceInput, &i_pVSOutput1->SourceInput, 0.5f ); // Edge between v0 and v1
	InterpolateVertexShaderInput( &NewVSOutputs[1].SourceInput, &i_pVSOutput1->SourceInput, &i_pVSOutput2->SourceInput, 0.5f ); // Edge between v0 and v1
	InterpolateVertexShaderInput( &NewVSOutputs[2].SourceInput, &i_pVSOutput2->SourceInput, &i_pVSOutput0->SourceInput, 0.5f ); // Edge between v0 and v1

	// Calculate new vertex shader outputs ------------------------------------
	m3dvsoutput *pCurVSOutput = NewVSOutputs;
	for( uint32 i = 0; i < 3; ++i, ++pCurVSOutput )
		m_pVertexShader->Execute( pCurVSOutput->SourceInput.ShaderInputs, pCurVSOutput->vPosition, pCurVSOutput->ShaderOutputs );

	SubdivideTriangle_Simple( i_iSubdivisionLevel, i_pVSOutput0, &NewVSOutputs[0], &NewVSOutputs[2] );
	SubdivideTriangle_Simple( i_iSubdivisionLevel, i_pVSOutput1, &NewVSOutputs[1], &NewVSOutputs[0] );
	SubdivideTriangle_Simple( i_iSubdivisionLevel, i_pVSOutput2, &NewVSOutputs[2], &NewVSOutputs[1] );
	SubdivideTriangle_Simple( i_iSubdivisionLevel, &NewVSOutputs[0], &NewVSOutputs[1], &NewVSOutputs[2] );
}

void CMuli3DDevice::SubdivideTriangle_Smooth( uint32 i_iSubdivisionLevel, const m3dvsoutput *i_pVSOutput0, const m3dvsoutput *i_pVSOutput1, const m3dvsoutput *i_pVSOutput2 )
{
	static const float32 c_fMultDivideBySix = 1.0f / 6.0f;

	// In case the triangle has been subdivided to the requested level, draw it ...
	if( i_iSubdivisionLevel >= m_iRenderStates[m3drs_subdivisionlevels] )
	{
		DrawTriangle( i_pVSOutput0, i_pVSOutput1, i_pVSOutput2 );
		return;
	}

	++i_iSubdivisionLevel;

	// Generate three new vertices: in the middle of each edge
	// Interpolate inputs for the new vertices (we're splitting the triangle's edges)
	m3dvsoutput NewVSOutputs[3];
	InterpolateVertexShaderInput( &NewVSOutputs[0].SourceInput, &i_pVSOutput0->SourceInput, &i_pVSOutput1->SourceInput, 0.5f ); // Edge between v0 and v1
	InterpolateVertexShaderInput( &NewVSOutputs[1].SourceInput, &i_pVSOutput1->SourceInput, &i_pVSOutput2->SourceInput, 0.5f ); // Edge between v0 and v1
	InterpolateVertexShaderInput( &NewVSOutputs[2].SourceInput, &i_pVSOutput2->SourceInput, &i_pVSOutput0->SourceInput, 0.5f ); // Edge between v0 and v1

	// Offset positions using normals as a base ...
	const uint32 iPos = m_iRenderStates[m3drs_subdivisionpositionregister];
	const uint32 iNormal = m_iRenderStates[m3drs_subdivisionnormalregister];

	// Normal-vectors should be re-normalized (they're not unit-length anymore due
	// to linear-interpolation) for best results, but because the error is very small
	// this step is skipped.

	const shaderreg *pShaderInputs[3] = { i_pVSOutput0->SourceInput.ShaderInputs,
		i_pVSOutput1->SourceInput.ShaderInputs, i_pVSOutput2->SourceInput.ShaderInputs };

	// offset middle of edge between v0 and v1
	{
		const vector3 vNormalA = pShaderInputs[0][iNormal] * fVector3Dot( (vector3)pShaderInputs[1][iPos] - (vector3)pShaderInputs[0][iPos], pShaderInputs[0][iNormal] );
		const vector3 vNormalB = pShaderInputs[1][iNormal] * fVector3Dot( (vector3)pShaderInputs[0][iPos] - (vector3)pShaderInputs[1][iPos], pShaderInputs[1][iNormal] );
		vector4 &vPos = NewVSOutputs[0].SourceInput.ShaderInputs[iPos];
		vPos -= (vNormalA + vNormalB) * c_fMultDivideBySix;
	}

	// offset middle of edge between v1 and v2
	{
		const vector3 vNormalA = pShaderInputs[1][iNormal] * fVector3Dot( (vector3)pShaderInputs[2][iPos] - (vector3)pShaderInputs[1][iPos], pShaderInputs[1][iNormal] );
		const vector3 vNormalB = pShaderInputs[2][iNormal] * fVector3Dot( (vector3)pShaderInputs[1][iPos] - (vector3)pShaderInputs[2][iPos], pShaderInputs[2][iNormal] );
		vector4 &vPos = NewVSOutputs[1].SourceInput.ShaderInputs[iPos];
		vPos -= (vNormalA + vNormalB) * c_fMultDivideBySix;
	}

	// offset middle of edge between v2 and v0
	{
		const vector3 vNormalA = pShaderInputs[2][iNormal] * fVector3Dot( (vector3)pShaderInputs[0][iPos] - (vector3)pShaderInputs[2][iPos], pShaderInputs[2][iNormal] );
		const vector3 vNormalB = pShaderInputs[0][iNormal] * fVector3Dot( (vector3)pShaderInputs[2][iPos] - (vector3)pShaderInputs[0][iPos], pShaderInputs[0][iNormal] );
		vector4 &vPos = NewVSOutputs[2].SourceInput.ShaderInputs[iPos];
		vPos -= (vNormalA + vNormalB) * c_fMultDivideBySix;
	}

	// Calculate new vertex shader outputs ------------------------------------
	m3dvsoutput *pCurVSOutput = NewVSOutputs;
	for( uint32 i = 0; i < 3; ++i, ++pCurVSOutput )
		m_pVertexShader->Execute( pCurVSOutput->SourceInput.ShaderInputs, pCurVSOutput->vPosition, pCurVSOutput->ShaderOutputs );

	SubdivideTriangle_Smooth( i_iSubdivisionLevel, i_pVSOutput0, &NewVSOutputs[0], &NewVSOutputs[2] );
	SubdivideTriangle_Smooth( i_iSubdivisionLevel, i_pVSOutput1, &NewVSOutputs[1], &NewVSOutputs[0] );
	SubdivideTriangle_Smooth( i_iSubdivisionLevel, i_pVSOutput2, &NewVSOutputs[2], &NewVSOutputs[1] );
	SubdivideTriangle_Smooth( i_iSubdivisionLevel, &NewVSOutputs[0], &NewVSOutputs[1], &NewVSOutputs[2] );
}

void CMuli3DDevice::SubdivideTriangle_Adaptive_SubdivideInnerPart( uint32 i_iSubdivisionLevel, const m3dvsoutput *i_pVSOutput0, const m3dvsoutput *i_pVSOutput1, const m3dvsoutput *i_pVSOutput2 )
{
	static const float32 c_fMultDivideByThree = 1.0f / 3.0f;

	// Info about i_iSubdivisionLevel: here we are counting the maximum inner subdivisions
	if( i_iSubdivisionLevel >= m_iRenderStates[m3drs_subdivisionmaxinnerlevels] )
	{
		DrawTriangle( i_pVSOutput0, i_pVSOutput1, i_pVSOutput2 );
		return;
	}

	// check area of triangle in screen space
	{
		vector4 vPos[3] = { i_pVSOutput0->vPosition, i_pVSOutput1->vPosition, i_pVSOutput2->vPosition };

		for( uint32 iVertex = 0; iVertex < 3; ++iVertex )
		{
			// TODO: should actually be clipped to view frustum

			// project vertex position + scale to rendertarget's viewport
			vPos[iVertex].homogenize();
			vPos[iVertex] *= m_pRenderTarget->matGetViewportMatrix();
		}

		const vector3 v0To1 = (vector3)vPos[1] - (vector3)vPos[0];
		const vector3 v0To2 = (vector3)vPos[2] - (vector3)vPos[0];
		vector3 vNormal; vVector3Cross( vNormal, v0To1, v0To2 );
		const float32 fArea = 0.5f * vNormal.length();

		if( fArea < *(float32 *)&m_iRenderStates[m3drs_subdivisionmaxscreenarea] )
		{
			DrawTriangle( i_pVSOutput0, i_pVSOutput1, i_pVSOutput2 );
			return;
		}
	}

	// Continue splitting: find center vertex and call SubdivideInnerPart for the three new vertices ...
	++i_iSubdivisionLevel;

	// Average inputs for the center vertex
	const vector4 *pShaderInputs[3] = { i_pVSOutput0->SourceInput.ShaderInputs,
		i_pVSOutput1->SourceInput.ShaderInputs, i_pVSOutput2->SourceInput.ShaderInputs };

	m3dvsoutput VSOutputCenter;
	for( uint32 i = 0; i < c_iVertexShaderRegisters; ++i )
		VSOutputCenter.SourceInput.ShaderInputs[i] = ( pShaderInputs[0][i] + pShaderInputs[1][i] + pShaderInputs[2][i] ) * c_fMultDivideByThree;

	// call vertex shader
	m_pVertexShader->Execute( VSOutputCenter.SourceInput.ShaderInputs, VSOutputCenter.vPosition, VSOutputCenter.ShaderOutputs );

	// split outer triangle-edges
	SubdivideTriangle_Adaptive_SubdivideInnerPart( i_iSubdivisionLevel, i_pVSOutput0, i_pVSOutput1, &VSOutputCenter );
	SubdivideTriangle_Adaptive_SubdivideInnerPart( i_iSubdivisionLevel, i_pVSOutput1, i_pVSOutput2, &VSOutputCenter );
	SubdivideTriangle_Adaptive_SubdivideInnerPart( i_iSubdivisionLevel, i_pVSOutput2, i_pVSOutput0, &VSOutputCenter );
}

void CMuli3DDevice::SubdivideTriangle_Adaptive_SubdivideEdges( uint32 i_iSubdivisionLevel, const m3dvsoutput *i_pVSOutputEdge0, const m3dvsoutput *i_pVSOutputEdge1, const m3dvsoutput *i_pVSOutputCenter )
{
	// In case the triangle-edges have been subdivided to the requested level, begin adaptive-subdivision of inner part
	if( i_iSubdivisionLevel >= m_iRenderStates[m3drs_subdivisionlevels] )
	{
		SubdivideTriangle_Adaptive_SubdivideInnerPart( 0, i_pVSOutputEdge0, i_pVSOutputEdge1, i_pVSOutputCenter );
		return;
	}

	++i_iSubdivisionLevel;

	// split edge and call subdivideedges recursively
	m3dvsoutput VSOutputMiddleEdge;
	InterpolateVertexShaderInput( &VSOutputMiddleEdge.SourceInput, &i_pVSOutputEdge0->SourceInput, &i_pVSOutputEdge1->SourceInput, 0.5f ); // Edge between v0 and v1

	// call vertex shader
	m_pVertexShader->Execute( VSOutputMiddleEdge.SourceInput.ShaderInputs, VSOutputMiddleEdge.vPosition, VSOutputMiddleEdge.ShaderOutputs );

	SubdivideTriangle_Adaptive_SubdivideEdges( i_iSubdivisionLevel, i_pVSOutputEdge0, &VSOutputMiddleEdge, i_pVSOutputCenter );
	SubdivideTriangle_Adaptive_SubdivideEdges( i_iSubdivisionLevel, &VSOutputMiddleEdge, i_pVSOutputEdge1, i_pVSOutputCenter );
}

void CMuli3DDevice::SubdivideTriangle_Adaptive( const m3dvsoutput *i_pVSOutput0, const m3dvsoutput *i_pVSOutput1, const m3dvsoutput *i_pVSOutput2 )
{
	static const float32 c_fMultDivideByThree = 1.0f / 3.0f;

	// Average inputs for the center vertex
	const shaderreg *pShaderInputs[3] = { i_pVSOutput0->SourceInput.ShaderInputs,
		i_pVSOutput1->SourceInput.ShaderInputs, i_pVSOutput2->SourceInput.ShaderInputs };

	m3dvsoutput VSOutputCenter;
	for( uint32 i = 0; i < c_iVertexShaderRegisters; ++i )
		VSOutputCenter.SourceInput.ShaderInputs[i] = ( pShaderInputs[0][i] + pShaderInputs[1][i] + pShaderInputs[2][i] ) * c_fMultDivideByThree;

	// call vertex shader
	m_pVertexShader->Execute( VSOutputCenter.SourceInput.ShaderInputs, VSOutputCenter.vPosition, VSOutputCenter.ShaderOutputs );

	// Split outer triangle-edges
	SubdivideTriangle_Adaptive_SubdivideEdges( 0, i_pVSOutput0, i_pVSOutput1, &VSOutputCenter );
	SubdivideTriangle_Adaptive_SubdivideEdges( 0, i_pVSOutput1, i_pVSOutput2, &VSOutputCenter );
	SubdivideTriangle_Adaptive_SubdivideEdges( 0, i_pVSOutput2, i_pVSOutput0, &VSOutputCenter );
}

inline bool CMuli3DDevice::bCullTriangle( const m3dvsoutput *i_pVSOutput0, const m3dvsoutput *i_pVSOutput1, const m3dvsoutput *i_pVSOutput2 )
{
	// Do backface-culling ----------------------------------------------------
	if( m_iRenderStates[m3drs_cullmode] == m3dcull_none )
		return false;

	/* vector3 vAB = vScreenSpacePos[1] - vScreenSpacePos[0];
	vector3 vAC = vScreenSpacePos[2] - vScreenSpacePos[0];
	vector3 vFaceNormal; vVector3Cross( vFaceNormal, vAB, vAC );
	float32 fDirTest = fVector3Dot( vFaceNormal, vector3( 0, 0, 1 ) ); */
	const float32 fDirTest = ( i_pVSOutput1->vPosition.x - i_pVSOutput0->vPosition.x ) * ( i_pVSOutput2->vPosition.y - i_pVSOutput0->vPosition.y ) - ( i_pVSOutput1->vPosition.y - i_pVSOutput0->vPosition.y ) * ( i_pVSOutput2->vPosition.x - i_pVSOutput0->vPosition.x );
	if( m_iRenderStates[m3drs_cullmode] == m3dcull_ccw )
	{
		if( fDirTest <= 0.0f )
			return true;
	}
	else	// m3dcull_cw
	{
		if( fDirTest >= 0.0f )
			return true;
	}

	return false;
}

uint32 CMuli3DDevice::iClipToPlane( uint32 i_iNumVertices, uint32 i_iSrcIndex, const plane &i_plane, bool i_bHomogenous )
{
	m3dvsoutput **ppSrcVertices = m_pClipVertices[i_iSrcIndex];
	m3dvsoutput **ppDestVertices = m_pClipVertices[(i_iSrcIndex + 1) & 1];

	uint32 iNumClippedVertices = 0;
	for( uint32 i = 0, j = 1; i < i_iNumVertices; ++i, ++j )
	{
		if( j == i_iNumVertices ) // wrap over
			j = 0;

		float32 di, dj; // Distances of current and next vertex to clipping plane.
		if( i_bHomogenous )
		{
			di = i_plane * ppSrcVertices[i]->vPosition;
			dj = i_plane * ppSrcVertices[j]->vPosition;
		}
		else
		{
			di = i_plane * ( *(vector3 *)&ppSrcVertices[i]->vPosition );
			dj = i_plane * ( *(vector3 *)&ppSrcVertices[j]->vPosition );
		}

		if( di >= 0.0f )
		{
			ppDestVertices[iNumClippedVertices++] = ppSrcVertices[i];
			if( dj < 0.0f )
			{
				InterpolateVertexShaderOutput( &m_ClipVertices[m_iNextFreeClipVertex], ppSrcVertices[i], ppSrcVertices[j], di / (di - dj) );
				ppDestVertices[iNumClippedVertices++] = &m_ClipVertices[m_iNextFreeClipVertex];
				m_iNextFreeClipVertex = ( m_iNextFreeClipVertex + 1 ) % 20; // TODO: 20 enough?

				#ifdef _DEBUG
				if( m_iNextFreeClipVertex == 0 )
					FUNC_FAILING( "wrap over: vertex creation during clipping! array too small ...?\n" );
				#endif
			}
		}
		else
		{
			if( dj >= 0.0f )
			{
				InterpolateVertexShaderOutput( &m_ClipVertices[m_iNextFreeClipVertex], ppSrcVertices[j], ppSrcVertices[i], dj / (dj - di) );
				ppDestVertices[iNumClippedVertices++] = &m_ClipVertices[m_iNextFreeClipVertex];
				m_iNextFreeClipVertex = ( m_iNextFreeClipVertex + 1 ) % 20; // TODO: 20 enough?

				#ifdef _DEBUG
				if( m_iNextFreeClipVertex == 0 )
					FUNC_FAILING( "wrap over: vertex creation during clipping! array too small ...?\n" );
				#endif
			}
		}
	}

	return iNumClippedVertices;
}

void CMuli3DDevice::DrawTriangle( const m3dvsoutput *i_pVSOutput0, const m3dvsoutput *i_pVSOutput1, const m3dvsoutput *i_pVSOutput2 )
{
	// Prepare triangle for homogenous clipping -------------------------------
	uint32 iNumVertices = 3;
	memcpy( &m_ClipVertices[0], i_pVSOutput0, sizeof( m3dvsoutput ) );
	memcpy( &m_ClipVertices[1], i_pVSOutput1, sizeof( m3dvsoutput ) );
	memcpy( &m_ClipVertices[2], i_pVSOutput2, sizeof( m3dvsoutput ) );
	m_iNextFreeClipVertex = 3;

	uint32 iStage = 0;
	m_pClipVertices[iStage][0] = &m_ClipVertices[0];
	m_pClipVertices[iStage][1] = &m_ClipVertices[1];
	m_pClipVertices[iStage][2] = &m_ClipVertices[2];

	// Call the triangle shader -----------------------------------------------
	if( m_pTriangleShader )
	{
		if( !m_pTriangleShader->bExecute( m_pClipVertices[0][0]->ShaderOutputs,
			m_pClipVertices[0][1]->ShaderOutputs, m_pClipVertices[0][2]->ShaderOutputs ) )
		{
			return; // Triangle got rejected.
		}
	}

	// Perform clipping to the frustum planes ---------------------------------
	for( uint32 iPlane = 0; iPlane < m3dcp_numplanes; ++iPlane )
	{
		if( !m_RenderInfo.bClippingPlaneEnabled[iPlane] )
			continue;

		iNumVertices = iClipToPlane( iNumVertices, iStage, m_RenderInfo.ClippingPlanes[iPlane], true );
		if( iNumVertices < 3 )
			return;

		iStage = ( iStage + 1 ) & 1;
	}

	// Project and rasterize the clipped triangle -----------------------------
	uint32 iVertex;

	// Project the first three vertices for culling
	m3dvsoutput **ppSrc = m_pClipVertices[iStage];
	for( iVertex = 0; iVertex < 3; ++iVertex )
		ProjectVertex( ppSrc[iVertex] );

	// We do not have to check for culling for each sub-polygon of the triangle, as they
	// are all in the same plane. If the first polygon is culled then all other polygons
	// would be culled, too.
	if( bCullTriangle( ppSrc[0], ppSrc[1], ppSrc[2] ) )
		return;

	// Project the remaining vertices
	for( iVertex = 3; iVertex < iNumVertices; ++iVertex )
		ProjectVertex( ppSrc[iVertex] );

	// Perform clipping (in screenspace) to the scissor rectangle if enabled --
	if( m_iRenderStates[m3drs_scissortestenable] )
	{
		for( uint32 iPlane = 0; iPlane < 4; ++iPlane, iStage = ( iStage + 1 ) & 1 )
		{
			iNumVertices = iClipToPlane( iNumVertices, iStage, m_RenderInfo.ScissorPlanes[iPlane], false );
			if( iNumVertices < 3 )
				return;
		}

		// New source for rasterization after scissoring ...
		ppSrc = m_pClipVertices[iStage];
	}

	for( iVertex = 1; iVertex < iNumVertices - 1; ++iVertex )
		RasterizeTriangle( ppSrc[0], ppSrc[iVertex], ppSrc[iVertex + 1] );
}

void CMuli3DDevice::CalculateTriangleGradients( const m3dvsoutput *i_pVSOutput0, const m3dvsoutput *i_pVSOutput1, const m3dvsoutput *i_pVSOutput2 )
{
	const float32 fDeltaX[2] = { i_pVSOutput1->vPosition.x - i_pVSOutput0->vPosition.x, i_pVSOutput2->vPosition.x - i_pVSOutput0->vPosition.x };
	const float32 fDeltaY[2] = { i_pVSOutput1->vPosition.y - i_pVSOutput0->vPosition.y, i_pVSOutput2->vPosition.y - i_pVSOutput0->vPosition.y };
	m_TriangleInfo.fCommonGradient = 1.0f / ( fDeltaX[0] * fDeltaY[1] - fDeltaX[1] * fDeltaY[0] );
	m_TriangleInfo.pBaseVertex = i_pVSOutput0;

	// The derivatives with respect to the y-coordinate are negated, because in screen-space the y-axis is reversed.

	const float32 fDeltaZ[2] = { i_pVSOutput1->vPosition.z - i_pVSOutput0->vPosition.z, i_pVSOutput2->vPosition.z - i_pVSOutput0->vPosition.z };
	m_TriangleInfo.fZDdx = ( fDeltaZ[0] * fDeltaY[1] - fDeltaZ[1] * fDeltaY[0] ) * m_TriangleInfo.fCommonGradient;
	m_TriangleInfo.fZDdy = -( fDeltaZ[0] * fDeltaX[1] - fDeltaZ[1] * fDeltaX[0] ) * m_TriangleInfo.fCommonGradient;

	const float32 fDeltaW[2] = { i_pVSOutput1->vPosition.w - i_pVSOutput0->vPosition.w, i_pVSOutput2->vPosition.w - i_pVSOutput0->vPosition.w };
	m_TriangleInfo.fWDdx = ( fDeltaW[0] * fDeltaY[1] - fDeltaW[1] * fDeltaY[0] ) * m_TriangleInfo.fCommonGradient;
	m_TriangleInfo.fWDdy = -( fDeltaW[0] * fDeltaX[1] - fDeltaW[1] * fDeltaX[0] ) * m_TriangleInfo.fCommonGradient;

	shaderreg *pDestDdx = m_TriangleInfo.ShaderOutputsDdx;
	shaderreg *pDestDdy = m_TriangleInfo.ShaderOutputsDdy;
	for( uint32 iReg = 0; iReg < c_iPixelShaderRegisters; ++iReg, ++pDestDdx, ++pDestDdy )
	{
		switch( m_RenderInfo.VSOutputs[iReg] )
		{
		case m3dsrt_vector4:
			{
				const float32 fDeltaRegVal[2] = { i_pVSOutput1->ShaderOutputs[iReg].w - i_pVSOutput0->ShaderOutputs[iReg].w,
					i_pVSOutput2->ShaderOutputs[iReg].w - i_pVSOutput0->ShaderOutputs[iReg].w };
				pDestDdx->w = ( fDeltaRegVal[0] * fDeltaY[1] - fDeltaRegVal[1] * fDeltaY[0] ) * m_TriangleInfo.fCommonGradient;
				pDestDdy->w = -( fDeltaRegVal[0] * fDeltaX[1] - fDeltaRegVal[1] * fDeltaX[0] ) * m_TriangleInfo.fCommonGradient;
			}
		case m3dsrt_vector3:
			{
				const float32 fDeltaRegVal[2] = { i_pVSOutput1->ShaderOutputs[iReg].z - i_pVSOutput0->ShaderOutputs[iReg].z,
					i_pVSOutput2->ShaderOutputs[iReg].z - i_pVSOutput0->ShaderOutputs[iReg].z };
				pDestDdx->z = ( fDeltaRegVal[0] * fDeltaY[1] - fDeltaRegVal[1] * fDeltaY[0] ) * m_TriangleInfo.fCommonGradient;
				pDestDdy->z = -( fDeltaRegVal[0] * fDeltaX[1] - fDeltaRegVal[1] * fDeltaX[0] ) * m_TriangleInfo.fCommonGradient;
			}
		case m3dsrt_vector2:
			{
				const float32 fDeltaRegVal[2] = { i_pVSOutput1->ShaderOutputs[iReg].y - i_pVSOutput0->ShaderOutputs[iReg].y,
					i_pVSOutput2->ShaderOutputs[iReg].y - i_pVSOutput0->ShaderOutputs[iReg].y };
				pDestDdx->y = ( fDeltaRegVal[0] * fDeltaY[1] - fDeltaRegVal[1] * fDeltaY[0] ) * m_TriangleInfo.fCommonGradient;
				pDestDdy->y = -( fDeltaRegVal[0] * fDeltaX[1] - fDeltaRegVal[1] * fDeltaX[0] ) * m_TriangleInfo.fCommonGradient;
			}
		case m3dsrt_float32:
			{
				const float32 fDeltaRegVal[2] = { i_pVSOutput1->ShaderOutputs[iReg].x - i_pVSOutput0->ShaderOutputs[iReg].x,
					i_pVSOutput2->ShaderOutputs[iReg].x - i_pVSOutput0->ShaderOutputs[iReg].x };
				pDestDdx->x = ( fDeltaRegVal[0] * fDeltaY[1] - fDeltaRegVal[1] * fDeltaY[0] ) * m_TriangleInfo.fCommonGradient;
				pDestDdy->x = -( fDeltaRegVal[0] * fDeltaX[1] - fDeltaRegVal[1] * fDeltaX[0] ) * m_TriangleInfo.fCommonGradient;
			}
		case m3dsrt_unused:
		default: // cannot happen
			break;
		}
	}
}

void CMuli3DDevice::SetVSOutputFromGradient( m3dvsoutput *o_pVSOutput, float32 i_fX, float32 i_fY )
{
	const float32 fOffsetX = ( i_fX - m_TriangleInfo.pBaseVertex->vPosition.x );
	const float32 fOffsetY = ( i_fY - m_TriangleInfo.pBaseVertex->vPosition.y );

	o_pVSOutput->vPosition.z = m_TriangleInfo.pBaseVertex->vPosition.z +
		m_TriangleInfo.fZDdx * fOffsetX + m_TriangleInfo.fZDdy * fOffsetY;
	o_pVSOutput->vPosition.w = m_TriangleInfo.pBaseVertex->vPosition.w +
		m_TriangleInfo.fWDdx * fOffsetX + m_TriangleInfo.fWDdy * fOffsetY;

	shaderreg *pDest = o_pVSOutput->ShaderOutputs;
	const shaderreg *pBase = m_TriangleInfo.pBaseVertex->ShaderOutputs;
	const shaderreg *pDdx = m_TriangleInfo.ShaderOutputsDdx;
	const shaderreg *pDdy = m_TriangleInfo.ShaderOutputsDdy;
	for( uint32 iReg = 0; iReg < c_iPixelShaderRegisters; ++iReg, ++pDest, ++pBase, ++pDdx, ++pDdy )
	{
		// The following assignments to pDest automatically zero out unused components.
		switch( m_RenderInfo.VSOutputs[iReg] )
		{
		case m3dsrt_float32:
			*pDest = *(float32 *)pBase + *(float32 *)pDdx * fOffsetX + *(float32 *)pDdy * fOffsetY;
			break;
		case m3dsrt_vector2:
			*pDest = *(vector2 *)pBase + *(vector2 *)pDdx * fOffsetX + *(vector2 *)pDdy * fOffsetY;
			break;
		case m3dsrt_vector3:
			*pDest = *(vector3 *)pBase + *(vector3 *)pDdx * fOffsetX + *(vector3 *)pDdy * fOffsetY;
			break;
		case m3dsrt_vector4:
			*pDest = *(vector4 *)pBase + *(vector4 *)pDdx * fOffsetX + *(vector4 *)pDdy * fOffsetY;
			break;
		case m3dsrt_unused:
		default: // cannot happen
			break;
		}
	}
}

inline void CMuli3DDevice::StepXVSOutputFromGradient( m3dvsoutput *io_pVSOutput )
{
	io_pVSOutput->vPosition.z += m_TriangleInfo.fZDdx;
	io_pVSOutput->vPosition.w += m_TriangleInfo.fWDdx;

	shaderreg *pDest = io_pVSOutput->ShaderOutputs;
	const shaderreg *pDdx = m_TriangleInfo.ShaderOutputsDdx;
	for( uint32 iReg = 0; iReg < c_iPixelShaderRegisters; ++iReg, ++pDest, ++pDdx )
	{
		switch( m_RenderInfo.VSOutputs[iReg] )
		{
		case m3dsrt_vector4:
			pDest->w += pDdx->w;
		case m3dsrt_vector3:
			pDest->z += pDdx->z;
		case m3dsrt_vector2:
			pDest->y += pDdx->y;
		case m3dsrt_float32:
			pDest->x += pDdx->x;
		case m3dsrt_unused:
		default: // cannot happen
			break;
		}
	}
}

void CMuli3DDevice::RasterizeTriangle( const m3dvsoutput *i_pVSOutput0, const m3dvsoutput *i_pVSOutput1, const m3dvsoutput *i_pVSOutput2 )
{
	CalculateTriangleGradients( i_pVSOutput0, i_pVSOutput1, i_pVSOutput2 );

	// If in wireframe mode draw triangle edges as lines.
	if( m_iRenderStates[m3drs_fillmode] == m3dfill_wireframe )
	{
		RasterizeLine( i_pVSOutput0, i_pVSOutput1 );
		RasterizeLine( i_pVSOutput1, i_pVSOutput2 );
		RasterizeLine( i_pVSOutput2, i_pVSOutput0 );
		return;
	}

	// Sort vertices by y-coordinate ------------------------------------------
	const m3dvsoutput *pVertices[3] = { i_pVSOutput0, i_pVSOutput1, i_pVSOutput2 };
	if( i_pVSOutput1->vPosition.y < pVertices[0]->vPosition.y ) { pVertices[1] = pVertices[0]; pVertices[0] = i_pVSOutput1; }
	if( i_pVSOutput2->vPosition.y < pVertices[0]->vPosition.y ) { pVertices[2] = pVertices[1]; pVertices[1] = pVertices[0]; pVertices[0] = i_pVSOutput2; }
	else if( i_pVSOutput2->vPosition.y < pVertices[1]->vPosition.y ) { pVertices[2] = pVertices[1]; pVertices[1] = i_pVSOutput2; }

	// Screenspace-position references ----------------------------------------
	const vector4 &vA = pVertices[0]->vPosition;
	const vector4 &vB = pVertices[1]->vPosition;
	const vector4 &vC = pVertices[2]->vPosition;

	// Calculate slopes for stepping ------------------------------------------
	const float32 fStepX[3] = {
		( vB.y - vA.y > 0.0f ) ? ( vB.x - vA.x ) / ( vB.y - vA.y ) : 0.0f,
		( vC.y - vA.y > 0.0f ) ? ( vC.x - vA.x ) / ( vC.y - vA.y ) : 0.0f,
		( vC.y - vB.y > 0.0f ) ? ( vC.x - vB.x ) / ( vC.y - vB.y ) : 0.0f };

	// Begin rasterization ----------------------------------------------------
	float32 fX[2] = { vA.x, vA.x };
	for( uint32 iPart = 0; iPart < 2; ++iPart )
	{
		uint32 iY[2];
		float32 fDeltaX[2];

		switch( iPart )
		{
		case 0: // Draw upper triangle-part
			{
				iY[0] = ftol( ceilf( vA.y ) );
				iY[1] = ftol( ceilf( vB.y ) );

				if( fStepX[0] > fStepX[1] ) // left <-> right ?
				{
					fDeltaX[0] = fStepX[1];
					fDeltaX[1] = fStepX[0];
				}
				else
				{
					fDeltaX[0] = fStepX[0];
					fDeltaX[1] = fStepX[1];
				}

				const float32 fPreStepY = (float32)iY[0] - vA.y;
				fX[0] += fDeltaX[0] * fPreStepY;
				fX[1] += fDeltaX[1] * fPreStepY;
			}
			break;

		case 1: // Draw lower triangle-part
			{
				// iY[0] = iY[1];
				iY[1] = ftol( ceilf( vC.y ) );

				const float32 fPreStepY = (float32)iY[0] - vB.y;
				if( fStepX[1] > fStepX[2] ) // left <-> right ?
				{
					fDeltaX[0] = fStepX[1];
					fDeltaX[1] = fStepX[2];
					fX[1] = vB.x + fDeltaX[1] * fPreStepY;
				}
				else
				{
					fDeltaX[0] = fStepX[2];
					fDeltaX[1] = fStepX[1];
					fX[0] = vB.x + fDeltaX[0] * fPreStepY;
				}
			}
			break;
		}

		for( ; iY[0] < iY[1]; ++iY[0], fX[0] += fDeltaX[0], fX[1] += fDeltaX[1] )
		{
			const int32 iX[2] = { ftol( ceilf( fX[0] ) ), ftol( ceilf( fX[1] ) ) };
			// const float32 fPreStepX = (float32)iX[0] - fX[0];

			m3dvsoutput VSOutput;
			SetVSOutputFromGradient( &VSOutput, (float32)iX[0], (float32)iY[0] );
			m_TriangleInfo.iCurPixelY = iY[0];
			(*this.*m_RenderInfo.fpRasterizeScanline)( iY[0], iX[0], iX[1], &VSOutput );
		}
	}
}

void CMuli3DDevice::RasterizeScanline_ColorOnly( uint32 i_iY, uint32 i_iX, uint32 i_iX2, m3dvsoutput *io_pVSOutput )
{
	float32 *pFrameData = m_RenderInfo.pFrameData + (i_iY * m_RenderInfo.iColorBufferPitch + i_iX * m_RenderInfo.iColorFloats);
	float32 *pDepthData = m_RenderInfo.pDepthData + (i_iY * m_RenderInfo.iDepthBufferPitch + i_iX);
	//float32 *pFrameData = m_RenderInfo.pFrameData ? &m_RenderInfo.pFrameData[i_iY * m_RenderInfo.iColorBufferPitch + i_iX * m_RenderInfo.iColorFloats ] : 0;
	//float32 *pDepthData = m_RenderInfo.pDepthData ? &m_RenderInfo.pDepthData[i_iY * m_RenderInfo.iDepthBufferPitch + i_iX ] : 0;

	for( ; i_iX < i_iX2; ++i_iX,
		pFrameData += m_RenderInfo.iColorFloats, ++pDepthData,
		StepXVSOutputFromGradient( io_pVSOutput ) )
	{
		// Get depth of current pixel
		float32 fDepth = io_pVSOutput->vPosition.z;

		// Perform depth-test
		switch( m_RenderInfo.DepthCompare )
		{
		case m3dcmp_never: return;
		case m3dcmp_equal: if( fabsf( fDepth - *pDepthData ) < FLT_EPSILON ) break; else continue;
		case m3dcmp_notequal: if( fabsf( fDepth - *pDepthData ) >= FLT_EPSILON ) break; else continue;
		case m3dcmp_less: if( fDepth < *pDepthData ) break; else continue;
		case m3dcmp_lessequal: if( fDepth <= *pDepthData ) break; else continue;
		case m3dcmp_greaterequal: if( fDepth >= *pDepthData ) break; else continue;
		case m3dcmp_greater: if( fDepth > *pDepthData ) break; else continue;
		case m3dcmp_always: break;
		}

		// passed depth test - update depthbuffer!
		if( m_RenderInfo.bDepthWrite )
			*pDepthData = fDepth;

		if( m_RenderInfo.bColorWrite )
		{
			m3dvsoutput PSInput;
			m_TriangleInfo.fCurPixelInvW = 1.0f / io_pVSOutput->vPosition.w;
			MultiplyVertexShaderOutputRegisters( &PSInput, io_pVSOutput, m_TriangleInfo.fCurPixelInvW );
			// note: PSInput now only contains valid register data, position etc. are not initialized!

			// Read in current pixel's color in the colorbuffer
			vector4 vPixelColor( 0, 0, 0, 1 );
			switch( m_RenderInfo.iColorFloats )
			{
			case 4: vPixelColor.a = pFrameData[3];
			case 3: vPixelColor.b = pFrameData[2];
			case 2: vPixelColor.g = pFrameData[1];
			case 1: vPixelColor.r = pFrameData[0];
			}

			// Execute the pixel shader
			m_TriangleInfo.iCurPixelX = i_iX;
			m_pPixelShader->bExecute( PSInput.ShaderOutputs, vPixelColor, fDepth );

			// Write the new color to the colorbuffer
			switch( m_RenderInfo.iColorFloats )
			{
			case 4: pFrameData[3] = vPixelColor.a;
			case 3: pFrameData[2] = vPixelColor.b;
			case 2: pFrameData[1] = vPixelColor.g;
			case 1: pFrameData[0] = vPixelColor.r;
			}
		}

		++m_RenderInfo.iRenderedPixels;
	}
}

void CMuli3DDevice::RasterizeScanline_ColorOnly_MightKillPixels( uint32 i_iY, uint32 i_iX, uint32 i_iX2, m3dvsoutput *io_pVSOutput )
{
	float32 *pFrameData = m_RenderInfo.pFrameData + (i_iY * m_RenderInfo.iColorBufferPitch + i_iX * m_RenderInfo.iColorFloats);
	float32 *pDepthData = m_RenderInfo.pDepthData + (i_iY * m_RenderInfo.iDepthBufferPitch + i_iX);
	//float32 *pFrameData = m_RenderInfo.pFrameData ? &m_RenderInfo.pFrameData[i_iY * m_RenderInfo.iColorBufferPitch + i_iX * m_RenderInfo.iColorFloats ] : 0;
	//float32 *pDepthData = m_RenderInfo.pDepthData ? &m_RenderInfo.pDepthData[i_iY * m_RenderInfo.iDepthBufferPitch + i_iX ] : 0;

	for( ; i_iX < i_iX2; ++i_iX,
		pFrameData += m_RenderInfo.iColorFloats, ++pDepthData,
		StepXVSOutputFromGradient( io_pVSOutput ) )
	{
		// Get depth of current pixel
		float32 fDepth = io_pVSOutput->vPosition.z;

		// Perform depth-test
		switch( m_RenderInfo.DepthCompare )
		{
		case m3dcmp_never: return;
		case m3dcmp_equal: if( fabsf( fDepth - *pDepthData ) < FLT_EPSILON ) break; else continue;
		case m3dcmp_notequal: if( fabsf( fDepth - *pDepthData ) >= FLT_EPSILON ) break; else continue;
		case m3dcmp_less: if( fDepth < *pDepthData ) break; else continue;
		case m3dcmp_lessequal: if( fDepth <= *pDepthData ) break; else continue;
		case m3dcmp_greaterequal: if( fDepth >= *pDepthData ) break; else continue;
		case m3dcmp_greater: if( fDepth > *pDepthData ) break; else continue;
		case m3dcmp_always: break;
		}

		if( m_RenderInfo.bColorWrite || m_RenderInfo.bDepthWrite )
		{
			m3dvsoutput PSInput;
			m_TriangleInfo.fCurPixelInvW = 1.0f / io_pVSOutput->vPosition.w;
			MultiplyVertexShaderOutputRegisters( &PSInput, io_pVSOutput, m_TriangleInfo.fCurPixelInvW );
			// note: PSInput now only contains valid register data, position etc. are not initialized!

			// Read in current pixel's color in the colorbuffer
			vector4 vPixelColor( 0, 0, 0, 1 );
			switch( m_RenderInfo.iColorFloats )
			{
			case 4: vPixelColor.a = pFrameData[3];
			case 3: vPixelColor.b = pFrameData[2];
			case 2: vPixelColor.g = pFrameData[1];
			case 1: vPixelColor.r = pFrameData[0];
			}

			// Execute the pixel shader
			m_TriangleInfo.iCurPixelX = i_iX;
			if( !m_pPixelShader->bExecute( PSInput.ShaderOutputs, vPixelColor, fDepth ) )
				continue; // pixel got killed

			// Passed depth-test and pixel was not killed, so update depthbuffer
			if( m_RenderInfo.bDepthWrite )
				*pDepthData = fDepth;

			// Write the new color to the colorbuffer
			if( m_RenderInfo.bColorWrite )
			{
				switch( m_RenderInfo.iColorFloats )
				{
				case 4: pFrameData[3] = vPixelColor.a;
				case 3: pFrameData[2] = vPixelColor.b;
				case 2: pFrameData[1] = vPixelColor.g;
				case 1: pFrameData[0] = vPixelColor.r;
				}
			}
		}

		++m_RenderInfo.iRenderedPixels;
	}
}

void CMuli3DDevice::RasterizeScanline_ColorDepth( uint32 i_iY, uint32 i_iX, uint32 i_iX2, m3dvsoutput *io_pVSOutput )
{
	float32 *pFrameData = m_RenderInfo.pFrameData + (i_iY * m_RenderInfo.iColorBufferPitch + i_iX * m_RenderInfo.iColorFloats);
	float32 *pDepthData = m_RenderInfo.pDepthData + (i_iY * m_RenderInfo.iDepthBufferPitch + i_iX);
	//float32 *pFrameData = m_RenderInfo.pFrameData ? &m_RenderInfo.pFrameData[i_iY * m_RenderInfo.iColorBufferPitch + i_iX] : 0;
	//float32 *pDepthData = m_RenderInfo.pDepthData ? &m_RenderInfo.pDepthData[i_iY * m_RenderInfo.iDepthBufferPitch + i_iX] : 0;

	for( ; i_iX < i_iX2; ++i_iX,
		pFrameData += m_RenderInfo.iColorFloats, ++pDepthData,
		StepXVSOutputFromGradient( io_pVSOutput ) )
	{
		m3dvsoutput PSInput;
		m_TriangleInfo.fCurPixelInvW = 1.0f / io_pVSOutput->vPosition.w;
		MultiplyVertexShaderOutputRegisters( &PSInput, io_pVSOutput, m_TriangleInfo.fCurPixelInvW );
		// note: PSInput now only contains valid register data, position etc. are not initialized!

		// Read in current colorbuffer-color
		vector4 vPixelColor( 0, 0, 0, 1 );
		switch( m_RenderInfo.iColorFloats )
		{
		case 4: vPixelColor.a = pFrameData[3];
		case 3: vPixelColor.b = pFrameData[2];
		case 2: vPixelColor.g = pFrameData[1];
		case 1: vPixelColor.r = pFrameData[0];
		}

		// Get depth of current pixel
		float32 fDepth = io_pVSOutput->vPosition.z;

		// Execute pixel shader
		m_TriangleInfo.iCurPixelX = i_iX;
		if( !m_pPixelShader->bExecute( PSInput.ShaderOutputs, vPixelColor, fDepth ) )
			continue; // pixel got killed

		// Perform depth-test
		switch( m_RenderInfo.DepthCompare )
		{
		case m3dcmp_never: return;
		case m3dcmp_equal: if( fabsf( fDepth - *pDepthData ) < FLT_EPSILON ) break; else continue;
		case m3dcmp_notequal: if( fabsf( fDepth - *pDepthData ) >= FLT_EPSILON ) break; else continue;
		case m3dcmp_less: if( fDepth < *pDepthData ) break; else continue;
		case m3dcmp_lessequal: if( fDepth <= *pDepthData ) break; else continue;
		case m3dcmp_greaterequal: if( fDepth >= *pDepthData ) break; else continue;
		case m3dcmp_greater: if( fDepth > *pDepthData ) break; else continue;
		case m3dcmp_always: break;
		}

		// Passed depth-test, so update depthbuffer
		if( m_RenderInfo.bDepthWrite )
			*pDepthData = fDepth;

		// Write new color to colorbuffer
		if( m_RenderInfo.bColorWrite )
		{
			switch( m_RenderInfo.iColorFloats )
			{
			case 4: pFrameData[3] = vPixelColor.a;
			case 3: pFrameData[2] = vPixelColor.b;
			case 2: pFrameData[1] = vPixelColor.g;
			case 1: pFrameData[0] = vPixelColor.r;
			}
		}

		++m_RenderInfo.iRenderedPixels;
	}
}

// LINES & POINTS -------------------------------------------------------------

void CMuli3DDevice::RasterizeLine( const m3dvsoutput *i_pVSOutput0, const m3dvsoutput *i_pVSOutput1 )
{
	const vector4 &vA = i_pVSOutput0->vPosition;
	const vector4 &vB = i_pVSOutput1->vPosition;

	const uint32 iIntCoordsA[2] = { ftol( vA.x ), ftol( vA.y ) };
	const float32 fDeltaX = vB.x - vA.x, fDeltaY = vB.y - vA.y;

	const int32 iLineThicknessHalf = -((int32)(m_iRenderStates[m3drs_linethickness] / 2));
	const int32 iPosOffset = ( m_iRenderStates[m3drs_linethickness] & 1 ) ? 0 : 1;

	if( fabsf( fDeltaX ) > fabsf( fDeltaY ) )
	{
		// drawing a line which is more horizontal than vertical
		const int32 iPixelsX = ftol( fDeltaX );
		if( !iPixelsX )
			return;

		const int32 iSignX = iPixelsX > 0 ? 1 : -1;
		const float32 fSlope = fDeltaY / fDeltaX;

		float32 fInterpolation = iSignX > 0 ? 0.0f : 1.0f;
		const float32 fInterpolationStep = (float32)iSignX / (float32)(abs( iPixelsX ) - 1);

		for( int32 i = 0; i != iPixelsX; i += iSignX, fInterpolation += fInterpolationStep )
		{
			const uint32 iPixelX = iIntCoordsA[0] + i;
			const uint32 iPixelY = iIntCoordsA[1] + ftol( fSlope * i );

			m3dvsoutput PSInput;
			SetVSOutputFromGradient( &PSInput, (float32)iPixelX, (float32)iPixelY );
			m_TriangleInfo.fCurPixelInvW = 1.0f / PSInput.vPosition.w;
			MultiplyVertexShaderOutputRegisters( &PSInput, &PSInput, m_TriangleInfo.fCurPixelInvW );

			if( !iLineThicknessHalf )
				(*this.*m_RenderInfo.fpDrawPixel)( iPixelX, iPixelY, &PSInput );
			else
			{
				for( int32 j = iLineThicknessHalf + iPosOffset; j <= -iLineThicknessHalf; ++j )
				{
					const int32 iNewPixelY = iPixelY + j;
					if( iNewPixelY < (int32)m_RenderInfo.ViewportRect.iTop ||
						iNewPixelY >= (int32)m_RenderInfo.ViewportRect.iBottom )
					{
						continue;
					}

					(*this.*m_RenderInfo.fpDrawPixel)( iPixelX, iNewPixelY, &PSInput );
				}
			}
		}
	}
	else
	{
		const int32 iPixelsY = ftol( fDeltaY );
		if( !iPixelsY )
			return;

		const int32 iSignY = iPixelsY > 0 ? 1 : -1;
		const float32 fSlope = fDeltaX / fDeltaY;

		float32 fInterpolation = iSignY > 0 ? 0.0f : 1.0f;
		const float32 fInterpolationStep = (float32)iSignY / (float32)(abs( iPixelsY ) - 1);

		for( int32 i = 0; i != iPixelsY; i += iSignY, fInterpolation += fInterpolationStep )
		{
			const uint32 iPixelX = iIntCoordsA[0] + ftol( fSlope * i );
			const uint32 iPixelY = iIntCoordsA[1] + i;

			m3dvsoutput PSInput;
			SetVSOutputFromGradient( &PSInput, (float32)iPixelX, (float32)iPixelY );
			m_TriangleInfo.fCurPixelInvW = 1.0f / PSInput.vPosition.w;
			MultiplyVertexShaderOutputRegisters( &PSInput, &PSInput, m_TriangleInfo.fCurPixelInvW );

			if( !iLineThicknessHalf )
				(*this.*m_RenderInfo.fpDrawPixel)( iPixelX, iPixelY, &PSInput );
			else
			{
				for( int32 j = iLineThicknessHalf + iPosOffset; j <= -iLineThicknessHalf; ++j )
				{
					const int32 iNewPixelX = iPixelX + j;
					if( iNewPixelX < (int32)m_RenderInfo.ViewportRect.iLeft ||
						iNewPixelX >= (int32)m_RenderInfo.ViewportRect.iRight )
					{
						continue;
					}

					(*this.*m_RenderInfo.fpDrawPixel)( iNewPixelX, iPixelY, &PSInput );
				}
			}
		}
	}
}

void CMuli3DDevice::DrawPixel_ColorOnly( uint32 i_iX, uint32 i_iY, const m3dvsoutput *i_pVSOutput )
{
	float32 *pFrameData = m_RenderInfo.pFrameData + (i_iY * m_RenderInfo.iColorBufferPitch + i_iX * m_RenderInfo.iColorFloats);
	float32 *pDepthData = m_RenderInfo.pDepthData + (i_iY * m_RenderInfo.iDepthBufferPitch + i_iX);
	//float32 *pFrameData = m_RenderInfo.pFrameData ? &m_RenderInfo.pFrameData[i_iY * m_RenderInfo.iColorBufferPitch + i_iX * m_RenderInfo.iColorFloats ] : 0;
	//float32 *pDepthData = m_RenderInfo.pDepthData ? &m_RenderInfo.pDepthData[i_iY * m_RenderInfo.iDepthBufferPitch + i_iX ] : 0;

	// Perform depth-test
	switch( m_RenderInfo.DepthCompare )
	{
	case m3dcmp_never: return;
	case m3dcmp_equal: if( fabsf( i_pVSOutput->vPosition.z - *pDepthData ) < FLT_EPSILON ) break; else return;
	case m3dcmp_notequal: if( fabsf( i_pVSOutput->vPosition.z - *pDepthData ) >= FLT_EPSILON ) break; else return;
	case m3dcmp_less: if( i_pVSOutput->vPosition.z < *pDepthData ) break; else return;
	case m3dcmp_lessequal: if( i_pVSOutput->vPosition.z <= *pDepthData ) break; else return;
	case m3dcmp_greaterequal: if( i_pVSOutput->vPosition.z >= *pDepthData ) break; else return;
	case m3dcmp_greater: if( i_pVSOutput->vPosition.z > *pDepthData ) break; else return;
	case m3dcmp_always: break;
	}

	if( m_RenderInfo.bColorWrite || m_RenderInfo.bDepthWrite )
	{
		// Read in current pixel's color in the colorbuffer
		vector4 vPixelColor( 0, 0, 0, 1 );
		switch( m_RenderInfo.iColorFloats )
		{
		case 4: vPixelColor.a = pFrameData[3];
		case 3: vPixelColor.b = pFrameData[2];
		case 2: vPixelColor.g = pFrameData[1];
		case 1: vPixelColor.r = pFrameData[0];
		}

		// Execute the pixel shader
		float32 fPSDepth = i_pVSOutput->vPosition.z; // if we passed i_pVSOutput->vPosition.z directly to the pixel shader, it might modify it, which is not allowed in this function
		m_TriangleInfo.iCurPixelX = i_iX;
		m_TriangleInfo.iCurPixelY = i_iY;

		if( !m_pPixelShader->bExecute( i_pVSOutput->ShaderOutputs, vPixelColor, fPSDepth ) )
			return; // pixel got killed

		// Passed depth-test and pixel was not killed, so update depthbuffer
		if( m_RenderInfo.bDepthWrite )
			*pDepthData = i_pVSOutput->vPosition.z;

		// Write the new color to the colorbuffer
		if( m_RenderInfo.bColorWrite )
		{
			switch( m_RenderInfo.iColorFloats )
			{
			case 4: pFrameData[3] = vPixelColor.a;
			case 3: pFrameData[2] = vPixelColor.b;
			case 2: pFrameData[1] = vPixelColor.g;
			case 1: pFrameData[0] = vPixelColor.r;
			}
		}
	}

	++m_RenderInfo.iRenderedPixels;
}

void CMuli3DDevice::DrawPixel_ColorDepth( uint32 i_iX, uint32 i_iY, const m3dvsoutput *i_pVSOutput )
{
	float32 *pFrameData = m_RenderInfo.pFrameData + (i_iY * m_RenderInfo.iColorBufferPitch + i_iX * m_RenderInfo.iColorFloats);
	float32 *pDepthData = m_RenderInfo.pDepthData + (i_iY * m_RenderInfo.iDepthBufferPitch + i_iX);
	//float32 *pFrameData = m_RenderInfo.pFrameData ? &m_RenderInfo.pFrameData[i_iY * m_RenderInfo.iColorBufferPitch + i_iX * m_RenderInfo.iColorFloats ] : 0;
	//float32 *pDepthData = m_RenderInfo.pDepthData ? &m_RenderInfo.pDepthData[i_iY * m_RenderInfo.iDepthBufferPitch + i_iX ] : 0;

	// Read in current pixel's color in the colorbuffer
	vector4 vPixelColor( 0, 0, 0, 1 );
	switch( m_RenderInfo.iColorFloats )
	{
	case 4: vPixelColor.a = pFrameData[3];
	case 3: vPixelColor.b = pFrameData[2];
	case 2: vPixelColor.g = pFrameData[1];
	case 1: vPixelColor.r = pFrameData[0];
	}

	// Execute the pixel shader
	float32 fPSDepth = i_pVSOutput->vPosition.z;
	m_TriangleInfo.iCurPixelX = i_iX;
	m_TriangleInfo.iCurPixelY = i_iY;

	if( !m_pPixelShader->bExecute( i_pVSOutput->ShaderOutputs, vPixelColor, fPSDepth ) )
		return; // pixel got killed

	// Perform depth-test
	switch( m_RenderInfo.DepthCompare )
	{
	case m3dcmp_never: return;
	case m3dcmp_equal: if( fabsf( fPSDepth - *pDepthData ) < FLT_EPSILON ) break; else return;
	case m3dcmp_notequal: if( fabsf( fPSDepth - *pDepthData ) >= FLT_EPSILON ) break; else return;
	case m3dcmp_less: if( fPSDepth < *pDepthData ) break; else return;
	case m3dcmp_lessequal: if( fPSDepth <= *pDepthData ) break; else return;
	case m3dcmp_greaterequal: if( fPSDepth >= *pDepthData ) break; else return;
	case m3dcmp_greater: if( fPSDepth > *pDepthData ) break; else return;
	case m3dcmp_always: break;
	}

	// Passed depth-test and pixel was not killed, so update depthbuffer
	if( m_RenderInfo.bDepthWrite )
		*pDepthData = fPSDepth;

	// Write the new color to the colorbuffer
	if( m_RenderInfo.bColorWrite )
	{
		switch( m_RenderInfo.iColorFloats )
		{
		case 4: pFrameData[3] = vPixelColor.a;
		case 3: pFrameData[2] = vPixelColor.b;
		case 2: pFrameData[1] = vPixelColor.g;
		case 1: pFrameData[0] = vPixelColor.r;
		}
	}

	++m_RenderInfo.iRenderedPixels;
}
