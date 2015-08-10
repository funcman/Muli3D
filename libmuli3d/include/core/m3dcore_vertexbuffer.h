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

/// @file m3dcore_vertexbuffer.h
///

#ifndef __M3DCORE_VERTEXBUFFER_H__
#define __M3DCORE_VERTEXBUFFER_H__

#include "../m3dbase.h"
#include "../m3dtypes.h"

/// Vertex buffers are memory buffers that contain vertex-data, which is described by a vertex format.
class CMuli3DVertexBuffer : public IBase
{
protected:
	~CMuli3DVertexBuffer();		///< Accessible by IBase. The destructor is called when the reference count reaches zero.

	friend class CMuli3DDevice;
	/// Accessible by CMuli3DDevice which is the only class that may create a vertex buffer.
	/// @param[in] i_pParent a pointer to the parent CMuli3DDevice-object.
	CMuli3DVertexBuffer( class CMuli3DDevice *i_pParent );

	/// Accessible by CMuli3DDevice which is the only class that may create a vertex buffer.
	/// @param[in] i_iLength length of the vertex buffer to be created in bytes.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one or more parameters were invalid.
	/// @return e_outofmemory if memory allocation failed.
	result Create( uint32 i_iLength );

public:
	class CMuli3DDevice *pGetDevice(); ///< Returns a pointer to the associated device. Calling this function will increase the internal reference count of the device. Failure to call Release() when finished using the pointer will result in a memory leak.

	/// Returns a pointer to the desired position in the buffer.
	/// @param[in] i_iOffset has to be specified in bytes.
	/// @param[out] o_ppData receives the pointer to the vertex buffer.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one or more parameters were invalid.
	result GetPointer( uint32 i_iOffset, void **o_ppData );

	uint32 iGetLength(); ///< Returns the length of the buffer in bytes.

private:
	class CMuli3DDevice	*m_pParent;	///< Pointer to parent.
	uint32				m_iLength;	///< Length of the vertex buffer in bytes.
	byte				*m_pData;	///< Pointer to the vertex buffer data.
};

#endif // __M3DCORE_VERTEXBUFFER_H__
