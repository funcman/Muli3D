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

/// @file m3dcore_primitiveassembler.h
///

#ifndef __M3DCORE_PRIMITIVEASSEMBLER_H__
#define __M3DCORE_PRIMITIVEASSEMBLER_H__

#include "../m3dbase.h"
#include "../m3dtypes.h"

/// Defines the triangle assembler interface.
class IMuli3DPrimitiveAssembler : public IBase
{
protected:
	friend class CMuli3DDevice;
	/// Accessible by CMuli3DDevice.
	/// This is the core function of a primitive assembler: It is used for DrawDynamicPrimitive() and executed after all vertices have been transformed. A primitive assembler returns indices to form primitives, which are in turn processed and rendered.
	/// @param[out] o_VertexIndices output vector which receives three indices.
	/// @param[in] i_iNumVertices number of vertices.
	/// @return type of assembled primitives: member of the enumeration m3dprimitivetype.
	virtual m3dprimitivetype Execute( std::vector<uint32> &o_VertexIndices, uint32 i_iNumVertices ) = 0;
};

#endif // __M3DCORE_PRIMITIVEASSEMBLER_H__
