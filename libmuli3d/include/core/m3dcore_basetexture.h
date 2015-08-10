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

/// @file m3dcore_basetexture.h
///

#ifndef __M3DCORE_BASETEXTURE_H__
#define __M3DCORE_BASETEXTURE_H__

#include "../m3dbase.h"
#include "../m3dtypes.h"

/// This is the texture base-class, which defines functions that are common to all different sorts of textures.
class IMuli3DBaseTexture : public IBase
{
protected:
	/// Accessible by IBase. The destructor is called when the reference count reaches zero.
	virtual ~IMuli3DBaseTexture();

	/// Texture constructor.
	/// @param[in] i_pParent a pointer to the parent CMuli3DDevice-object.
	IMuli3DBaseTexture( class CMuli3DDevice *i_pParent );

	friend class CMuli3DDevice;
	virtual m3dtexsampleinput eGetTexSampleInput() = 0;	///< Returns a member of enum m3dtexsampleinput, which determines the type of input vector SampleTexture() will receive when sampling the texture.

	/// Samples the texture and returns the looked-up color.
	/// @param[out] o_vColor receives the color of the pixel to be looked up.
	/// @param[in] i_fU u-component of the lookup-vector.
	/// @param[in] i_fV v-component of the lookup-vector.
	/// @param[in] i_fW w-component of the lookup-vector.
	/// @param[in] i_pXGradient partial derivatives of the texture coordinates with respect to the screen-space x coordinate. If 0 the base mip-level will be chosen and the minification filter will be used for texture sampling.
	/// @param[in] i_pYGradient partial derivatives of the texture coordinates with respect to the screen-space y coordinate. If 0 the base mip-level will be chosen and the minification filter will be used for texture sampling.
	/// @param[in] i_pSamplerStates texture sampler states.
	/// @return s_ok if the function succeeds.
	virtual result SampleTexture( vector4 &o_vColor, float32 i_fU, float32 i_fV,
		float32 i_fW, const vector4 *i_pXGradient, const vector4 *i_pYGradient,
		const uint32 *i_pSamplerStates ) = 0;

public:
	/// Returns a pointer to the associated device. Calling this function will increase the internal reference count of the device. Failure to call Release() when finished using the pointer will result in a memory leak.
	class CMuli3DDevice *pGetDevice();

protected:
	class CMuli3DDevice	*m_pParent;	///< Pointer to parent.
};

#endif // __M3DCORE_BASETEXTURE_H__
