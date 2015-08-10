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

/// @file m3dcore.h
///

#ifndef __M3DCORE_H__
#define __M3DCORE_H__

#include "../m3dbase.h"
#include "../m3dtypes.h"

/// The Muli3D parent-class - "the root of all evil ;)"
class CMuli3D : public IBase
{
protected:
	~CMuli3D(); ///< Accessible by IBase. The destructor is called when the reference count reaches zero.

	friend result CreateMuli3D( class CMuli3D **o_ppMuli3D );
	CMuli3D(); ///< Accessible by CreateMuli3D() which is the creator-function.

public:
	/// Creates a device for rendering.
	/// @param[out] o_ppDevice receives the pointer to the device.
	/// @param[in] i_pDeviceParameters pointer to a m3ddeviceparameters-structure.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one or more parameters were invalid.
	/// @return e_outofmemory if memory allocation failed.
	result CreateDevice( class CMuli3DDevice **o_ppDevice,
		const m3ddeviceparameters *i_pDeviceParameters );
};

/// Creates a Muli3D-instance.
/// @param[out] o_ppMuli3D receives the pointer to the muli3d instance.
/// @return s_ok if the function succeeds.
/// @return e_invalidparameters if one or more parameters were invalid.
/// @return e_outofmemory if memory allocation failed.
result CreateMuli3D( class CMuli3D **o_ppMuli3D );

// Include all core-headers ---------------------------------------------------
#include "m3dcore_cubetexture.h"
#include "m3dcore_device.h"
#include "m3dcore_indexbuffer.h"
#include "m3dcore_rendertarget.h"
#include "m3dcore_shaders.h"
#include "m3dcore_surface.h"
#include "m3dcore_texture.h"
#include "m3dcore_primitiveassembler.h"
#include "m3dcore_vertexbuffer.h"
#include "m3dcore_vertexformat.h"
#include "m3dcore_volume.h"
#include "m3dcore_volumetexture.h"

#endif // __M3DCORE_H__
