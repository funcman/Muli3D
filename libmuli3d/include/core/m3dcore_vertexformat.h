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

/// @file m3dcore_vertexformat.h
///

#ifndef __M3DCORE_VERTEXFORMAT_H__
#define __M3DCORE_VERTEXFORMAT_H__

#include "../m3dbase.h"
#include "../m3dtypes.h"

/// A vertex format describes the data-layout of vertex streams.
class CMuli3DVertexFormat : public IBase
{
protected:
	~CMuli3DVertexFormat(); ///< Accessible by IBase. The destructor is called when the reference count reaches zero.

	friend class CMuli3DDevice;
	/// Accessible by CMuli3DDevice which is the only class that may create a vertex format.
	/// @param[in] i_pParent a pointer to the parent CMuli3DDevice-object.
	CMuli3DVertexFormat( class CMuli3DDevice *i_pParent );

	/// Accessible by CMuli3DDevice which is the only class that may create a vertex format.
	/// @param[in] i_pVertexDeclaration pointer to the vertex format declaration.
	/// @param[in] i_iVertexDeclSize length of the vertex format declaration in bytes.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one or more parameters were invalid.
	/// @return e_outofmemory if memory allocation failed.
	result Create( const m3dvertexelement *i_pVertexDeclaration,
		uint32 i_iVertexDeclSize );

public:
	class CMuli3DDevice *pGetDevice(); ///< Returns a pointer to the associated device. Calling this function will increase the internal reference count of the device. Failure to call Release() when finished using the pointer will result in a memory leak.

protected:
	uint32 iGetNumVertexElements();		///< Accessible by CMuli3DDevice. Returns the number of vertex elements described in this vertex format.
	uint32 iGetHighestStream();			///< Accessible by CMuli3DDevice. Returns the highest index of the used vertex streams.
	m3dvertexelement *pGetElements();	///< Accessible by CMuli3DDevice. Returns a pointer to the vertex elements description.

private:
	class CMuli3DDevice	*m_pParent;				///< Pointer to parent.
	uint32				m_iNumVertexElements;	///< Number of vertex elements.
	uint32				m_iHighestStream;		///< Highest index of the used vertex streams.
	m3dvertexelement	*m_pElements;			///< Pointer to the vertex elements.
};

#endif // __M3DCORE_VERTEXFORMAT_H__
