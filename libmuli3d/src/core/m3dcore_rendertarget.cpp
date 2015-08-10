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

#include "../../include/core/m3dcore_rendertarget.h"
#include "../../include/core/m3dcore_device.h"
#include "../../include/core/m3dcore_surface.h"

CMuli3DRenderTarget::CMuli3DRenderTarget( CMuli3DDevice *i_pParent )
	: m_pParent( i_pParent ), m_pColorBuffer( 0 ), m_pDepthBuffer( 0 )
{
	m_pParent->AddRef();
}

CMuli3DRenderTarget::~CMuli3DRenderTarget()
{
	SAFE_RELEASE( m_pColorBuffer );
	SAFE_RELEASE( m_pDepthBuffer );

	SAFE_RELEASE( m_pParent );
}

CMuli3DDevice *CMuli3DRenderTarget::pGetDevice()
{
	if( m_pParent )
		m_pParent->AddRef();
	return m_pParent;
}

result CMuli3DRenderTarget::ClearColorBuffer( const vector4 &i_vColor, const m3drect *i_pRect )
{
	if( !m_pColorBuffer )
	{
		FUNC_FAILING( "CMuli3DRenderTarget::ClearColorBuffer: no framebuffer has been set.\n" );
		return e_invalidstate;
	}

	return m_pColorBuffer->Clear( i_vColor, i_pRect );
}

result CMuli3DRenderTarget::ClearDepthBuffer( float32 i_fDepth, const m3drect *i_pRect )
{
	if( !m_pDepthBuffer )
	{
		FUNC_FAILING( "CMuli3DRenderTarget::ClearDepthBuffer: no depthbuffer has been set.\n" );
		return e_invalidstate;
	}

	return m_pDepthBuffer->Clear( vector4( i_fDepth, 0, 0, 0 ), i_pRect );
}

result CMuli3DRenderTarget::SetColorBuffer( CMuli3DSurface *i_pColorBuffer )
{
	if( i_pColorBuffer )
	{
		if( i_pColorBuffer->fmtGetFormat() < m3dfmt_r32f || i_pColorBuffer->fmtGetFormat() > m3dfmt_r32g32b32a32f )
		{
			FUNC_FAILING( "CMuli3DRenderTarget::SetColorBuffer: invalid texture format.\n" );
			return e_invalidformat;
		}

		if( m_pDepthBuffer )
		{
			if( m_pDepthBuffer->iGetWidth() != i_pColorBuffer->iGetWidth() ||
				m_pDepthBuffer->iGetHeight() != i_pColorBuffer->iGetHeight() )
			{
				FUNC_FAILING( "CMuli3DDevice::SetColorBuffer: framebuffer and depthbuffer dimensions are not equal.\n" );
				return e_invalidformat;
			}
		}
	}

	SAFE_RELEASE( m_pColorBuffer );
	m_pColorBuffer = i_pColorBuffer;
	if( m_pColorBuffer ) m_pColorBuffer->AddRef();
	return s_ok;
}

result CMuli3DRenderTarget::SetDepthBuffer( CMuli3DSurface *i_pDepthBuffer )
{
	if( i_pDepthBuffer )
	{
		if( i_pDepthBuffer->fmtGetFormat() != m3dfmt_r32f )
		{
			FUNC_FAILING( "CMuli3DRenderTarget::SetDepthBuffer: invalid texture format.\n" );
			return e_invalidformat;
		}

		if( m_pColorBuffer )
		{
			if( i_pDepthBuffer->iGetWidth() != m_pColorBuffer->iGetWidth() ||
				i_pDepthBuffer->iGetHeight() != m_pColorBuffer->iGetHeight() )
			{
				FUNC_FAILING( "CMuli3DDevice::SetDepthBuffer: depthbuffer and framebuffer dimensions are not equal.\n" );
				return e_invalidformat;
			}
		}
	}

	SAFE_RELEASE( m_pDepthBuffer );
	m_pDepthBuffer = i_pDepthBuffer;
	if( m_pDepthBuffer ) m_pDepthBuffer->AddRef();
	return s_ok;
}

CMuli3DSurface *CMuli3DRenderTarget::pGetColorBuffer()
{
	if( m_pColorBuffer )
		m_pColorBuffer->AddRef();
	
	return m_pColorBuffer;
}

CMuli3DSurface *CMuli3DRenderTarget::pGetDepthBuffer()
{
	if( m_pDepthBuffer )
		m_pDepthBuffer->AddRef();
	
	return m_pDepthBuffer;
}

void CMuli3DRenderTarget::SetViewportMatrix( const matrix44 &i_matViewport )
{
	m_matViewport = i_matViewport;
}

const matrix44 &CMuli3DRenderTarget::matGetViewportMatrix()
{
	return m_matViewport;
}
