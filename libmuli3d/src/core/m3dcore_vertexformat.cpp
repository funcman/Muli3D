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

#include "../../include/core/m3dcore_vertexformat.h"
#include "../../include/core/m3dcore_device.h"

CMuli3DVertexFormat::CMuli3DVertexFormat( CMuli3DDevice *i_pParent )
	: m_pParent( i_pParent ), m_pElements( 0 )
{
	m_pParent->AddRef();
}

result CMuli3DVertexFormat::Create( const m3dvertexelement *i_pVertexDeclaration,
		uint32 i_iVertexDeclSize )
{
	if( !i_pVertexDeclaration )
	{
		FUNC_FAILING( "CMuli3DVertexFormat::Create: parameter i_pVertexDeclaration points to null.\n" );
		return e_invalidparameters;
	}

	if( !i_iVertexDeclSize )
	{
		FUNC_FAILING( "CMuli3DVertexFormat::Create: parameter i_iVertexDeclSize is 0.\n" );
		return e_invalidparameters;
	}

	m_iNumVertexElements = i_iVertexDeclSize / sizeof( m3dvertexelement );
	m_iHighestStream = 0;

	const m3dvertexelement *pCurElement = i_pVertexDeclaration;
	for( uint32 iElement = 0; iElement < m_iNumVertexElements; ++iElement, ++pCurElement )
	{
		if( pCurElement->iStream >= c_iMaxVertexStreams )
		{
			FUNC_FAILING( "CMuli3DVertexFormat::Create: vertex element's stream number exceeds number of available streams.\n" );
			return e_invalidparameters;
		}

		if( pCurElement->iRegister >= c_iVertexShaderRegisters )
		{
			FUNC_FAILING( "CMuli3DVertexFormat::Create: vertex element's target register exceeds number of available vertex shader registers.\n" );
			return e_invalidparameters;
		}

		if( pCurElement->Type < m3dvet_float32 || pCurElement->Type > m3dvet_vector4 )
		{
			FUNC_FAILING( "CMuli3DVertexFormat::Create: vertex element has invalid type.\n" );
			return e_invalidparameters;
		}

		if( pCurElement->iStream > m_iHighestStream )
			m_iHighestStream = pCurElement->iStream;
	}

	m_pElements = new m3dvertexelement[m_iNumVertexElements];
	if( !m_pElements )
	{
		FUNC_FAILING( "CMuli3DVertexFormat::Create: out of memory, cannot create vertex element data.\n" );
		return e_outofmemory;
	}

	memcpy( m_pElements, i_pVertexDeclaration, sizeof( m3dvertexelement ) * m_iNumVertexElements );

	return s_ok;
}

CMuli3DVertexFormat::~CMuli3DVertexFormat()
{
	SAFE_DELETE_ARRAY( m_pElements );

	SAFE_RELEASE( m_pParent );
}

CMuli3DDevice *CMuli3DVertexFormat::pGetDevice()
{
	if( m_pParent )
		m_pParent->AddRef();
	return m_pParent;
}

uint32 CMuli3DVertexFormat::iGetNumVertexElements()
{
	return m_iNumVertexElements;
}

uint32 CMuli3DVertexFormat::iGetHighestStream()
{
	return m_iHighestStream;
}

m3dvertexelement *CMuli3DVertexFormat::pGetElements()
{
	return m_pElements;
}
