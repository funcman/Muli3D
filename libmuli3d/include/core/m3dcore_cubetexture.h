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

/// @file m3dcore_cubetexture.h
///

#ifndef __M3DCORE_CUBETEXTURE_H__
#define __M3DCORE_CUBETEXTURE_H__

#include "../m3dbase.h"
#include "../m3dtypes.h"

#include "m3dcore_basetexture.h"

/// CMuli3DCubeTexture implements support for cubemaps.
class CMuli3DCubeTexture : public IMuli3DBaseTexture
{
protected:
	~CMuli3DCubeTexture(); ///< Accessible by IBase. The destructor is called when the reference count reaches zero.
	
	friend class CMuli3DDevice;
	/// Accessible by CMuli3DDevice which is the only class that may create a cube texture.
	/// @param[in] i_pParent a pointer to the parent CMuli3DDevice-object.
	CMuli3DCubeTexture( class CMuli3DDevice *i_pParent );

	/// Accessible by CMuli3DDevice which is the only class that may create a cube texture.
	/// @param[in] i_iEdgeLength edge length of the cube texture to be created in pixels.
	/// @param[in] i_iMipLevels number of mip-levels to be created. Specify 0 to create a full mip-chain.
	/// @param[in] i_fmtFormat format of the texture to be created. Member of the enumeration m3dformat; m3dfmt_r32f, m3dfmt_r32g32f, m3dfmt_r32g32b32f or m3dfmt_r32g32b32a32f.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one or more parameters were invalid.
	/// @return e_outofmemory if memory allocation failed.
	/// @return e_invalidformat if an invalid format was encountered.
	result Create( uint32 i_iEdgeLength, uint32 i_iMipLevels,
		m3dformat i_fmtFormat );

	m3dtexsampleinput eGetTexSampleInput(); ///< Sampling this texture requires a 3-dimensional floating point vector.

	/// Accessible by CMuli3DDevice.
	/// Samples the texture and returns the looked-up color.
	/// @param[out] o_vColor receives the color of the pixel to be looked up.
	/// @param[in] i_fU u-component of the lookup-vector.
	/// @param[in] i_fV v-component of the lookup-vector.
	/// @param[in] i_fW w-component of the lookup-vector.
	/// @param[in] i_pXGradient partial derivatives of the texture coordinates with respect to the screen-space x coordinate. If 0 the base mip-level will be chosen and the minification filter will be used for texture sampling.
	/// @param[in] i_pYGradient partial derivatives of the texture coordinates with respect to the screen-space y coordinate. If 0 the base mip-level will be chosen and the minification filter will be used for texture sampling.
	/// @param[in] i_pSamplerStates texture sampler states.
	/// @return s_ok if the function succeeds.
	result SampleTexture( vector4 &o_vColor, float32 i_fU, float32 i_fV,
		float32 i_fW, const vector4 *i_pXGradient, const vector4 *i_pYGradient,
		const uint32 *i_pSamplerStates );

public:
	/// Generates mip-sublevels through downsampling (using a box-filter) a given source mip-level.
	/// @param[in] i_iSrcLevel the mip-level which will be taken as the starting point.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one or more parameters were invalid.
	result GenerateMipSubLevels( uint32 i_iSrcLevel );

	/// Returns a pointer to the contents of a given mip-level.
	/// @param[in] i_Face cube face that is requested. Member of the enumeration m3dcubefaces.
	/// @param[in] i_iMipLevel mip-level that is requested, 0 being the largest mip-level.
	/// @param[out] o_ppData receives the pointer to the texture-data.
	/// @param[in] i_pRect area that will be locked and accessible. (Pass in 0 to lock entire texture.)
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one or more parameters were invalid.
	/// @return e_invalidstate if the texture is already locked.
	/// @note Locking the entire texture is a lot faster than locking a sub-region, because no lock-buffer has to be created and the application may write to the texture directly.
	result LockRect( m3dcubefaces i_Face, uint32 i_iMipLevel, void **o_ppData,
		const m3drect *i_pRect );

	/// Unlocks the given mip-level; modifications to the texture will become active.
	/// @param[in] i_Face cube face. Member of the enumeration m3dcubefaces.
	/// @param[in] i_iMipLevel mip-level, 0 being the largest mip-level.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one or more parameters were invalid.
	result UnlockRect( m3dcubefaces i_Face, uint32 i_iMipLevel );

	m3dformat fmtGetFormat();	///< Returns the format of the texture. Member of the enumeration m3dformat; m3dfmt_r32f, m3dfmt_r32g32f, m3dfmt_r32g32b32f or m3dfmt_r32g32b32a32f.
	uint32 iGetFormatFloats();  ///< Returns the number of floats of the format, e [1,4].
	uint32 iGetMipLevels();		///< Returns the number of mip-levels this texture consists of.
	
	/// Returns the edge length of the given mip-level in pixels.
	/// @param[in] i_iMipLevel the mip-level whose edge length is requested.
	uint32 iGetEdgeLength( uint32 i_iMipLevel = 0 );
	
	/// Returns a pointer to a cube face which can then be accessed like a normal 2d texture.
	/// Calling this function will increase the internal reference count of the texture. Failure to call Release() when finished using the pointer will result in a memory leak.
	/// @param[in] i_Face member of the enumeration m3dcubefaces.
	class CMuli3DTexture *pGetCubeFace( m3dcubefaces i_Face );

private:
	class CMuli3DTexture	*m_ppCubeFaces[6]; ///< Pointer to the 6 cube faces.
};

#endif // __M3DCORE_CUBETEXTURE_H__
