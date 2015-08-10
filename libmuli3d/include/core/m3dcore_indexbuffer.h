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

/// @file m3dcore_indexbuffer.h
///

#ifndef __M3DCORE_INDEXBUFFER_H__
#define __M3DCORE_INDEXBUFFER_H__

#include "../m3dbase.h"
#include "../m3dtypes.h"

/// Index buffers contain a list of vertex indices either in 16-bit or 32-bit format.
class CMuli3DIndexBuffer : public IBase
{
protected:
	~CMuli3DIndexBuffer(); ///< Accessible by IBase. The destructor is called when the reference count reaches zero.

	friend class CMuli3DDevice;
	/// Accessible by CMuli3DDevice which is the only class that may create an index buffer.
	/// @param[in] i_pParent a pointer to the parent CMuli3DDevice-object.
	CMuli3DIndexBuffer( class CMuli3DDevice *i_pParent );

	/// Accessible by CMuli3DDevice which is the only class that may create an index buffer.
	/// @param[in] i_iLength length of the index buffer to be created in bytes.
	/// @param[in] i_fmtFormat format of the index buffer to be created. Member of the enumeration m3dformat; either m3dfmt_index16 or m3dfmt_index32.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one or more parameters were invalid.
	/// @return e_outofmemory if memory allocation failed.
	/// @return e_invalidformat if an invalid format was encountered.
	result Create( uint32 i_iLength, m3dformat i_fmtFormat );

public:
	class CMuli3DDevice *pGetDevice(); ///< Returns a pointer to the associated device. Calling this function will increase the internal reference count of the device. Failure to call Release() when finished using the pointer will result in a memory leak.

	/// Returns a pointer to the desired position in the buffer.
	/// @param[in] i_iOffset has to be specified in bytes.
	/// @param[out] o_ppData receives the pointer to the vertex buffer.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one or more parameters were invalid.
	result GetPointer( uint32 i_iOffset, void **o_ppData );

	uint32 iGetLength();		///< Returns the length of the buffer in bytes.
	m3dformat fmtGetFormat();	///< Returns the format of the buffer. Member of the enumeration m3dformat; either m3dfmt_index16 or m3dfmt_index32.

protected:
	/// Accessible by CMuli3DDevice: This function returns the index-value at a given index in the buffer, that means an index of 3 returns the third stored vertex-index in the buffer regardless of its format.
	/// @param[in] i_iArrayIndex index of the value in the ib-array.
	/// @param[out] o_iValue receives the value of the index to be looked up.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one or more parameters were invalid.
	result GetVertexIndex( uint32 i_iArrayIndex, uint32 &o_iValue );

private:
	class CMuli3DDevice	*m_pParent;		///< Pointer to parent.
	uint32				m_iLength;		///< Length of the index buffer in bytes.
	m3dformat			m_fmtFormat;	///< Format of the index buffer. Member of the enumeration m3dformat; either m3dfmt_index16 or m3dfmt_index32.
	byte				*m_pData;		///< Pointer to the index buffer data.
};

#endif // __M3DCORE_INDEXBUFFER_H__
