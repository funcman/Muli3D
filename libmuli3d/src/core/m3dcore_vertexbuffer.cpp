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

#include "../../include/core/m3dcore_vertexbuffer.h"
#include "../../include/core/m3dcore_device.h"

CMuli3DVertexBuffer::CMuli3DVertexBuffer( CMuli3DDevice *i_pParent )
	: m_pParent( i_pParent ), m_pData( 0 )
{
	m_pParent->AddRef();
}

result CMuli3DVertexBuffer::Create( uint32 i_iLength )
{
	if( !i_iLength )
	{
		FUNC_FAILING( "CMuli3DVertexBuffer::Create: parameter i_iLength is 0.\n" );
		return e_invalidparameters;
	}

	m_iLength = i_iLength;

	m_pData = new byte[i_iLength];
	if( !m_pData )
	{
		FUNC_FAILING( "CMuli3DVertexBuffer::Create: out of memory, cannot create vertex buffer.\n" );
		return e_outofmemory;
	}

	return s_ok;
}

CMuli3DVertexBuffer::~CMuli3DVertexBuffer()
{
	SAFE_DELETE_ARRAY( m_pData );

	SAFE_RELEASE( m_pParent );
}

result CMuli3DVertexBuffer::GetPointer( uint32 i_iOffset, void **o_ppData )
{
	if( !o_ppData )
	{
		FUNC_FAILING( "CMuli3DVertexBuffer::GetPointer: parameter o_ppData points to null.\n" );
		return e_invalidparameters;
	}

	if( i_iOffset >= m_iLength )
	{
		*o_ppData = 0;
		FUNC_FAILING( "CMuli3DVertexBuffer::GetPointer: i_iOffset exceeds vertex buffer length.\n" );
		return e_invalidparameters;
	}

	*o_ppData = &m_pData[i_iOffset];
	return s_ok;
}

CMuli3DDevice *CMuli3DVertexBuffer::pGetDevice()
{
	if( m_pParent )
		m_pParent->AddRef();
	return m_pParent;
}

uint32 CMuli3DVertexBuffer::iGetLength()
{
	return m_iLength;
}
