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

/// @file m3dcore_volumetexture.h
///

#ifndef __M3DCORE_VOLUMETEXTURE_H__
#define __M3DCORE_VOLUMETEXTURE_H__

#include "../m3dbase.h"
#include "../m3dtypes.h"

#include "m3dcore_basetexture.h"

/// CMuli3DTexture implements a 3-dimensional texture.
class CMuli3DVolumeTexture : public IMuli3DBaseTexture
{
protected:
	~CMuli3DVolumeTexture(); ///< Accessible by IBase. The destructor is called when the reference count reaches zero.

	friend class CMuli3DDevice;
	/// Accessible by CMuli3DDevice which is the only class that may create a texture.
	/// @param[in] i_pParent a pointer to the parent CMuli3DDevice-object.
	CMuli3DVolumeTexture( class CMuli3DDevice *i_pParent );

	/// Accessible by CMuli3DDevice which is the only class that may create a texture.
	/// @param[in] i_iWidth width of the texture to be created in pixels.
	/// @param[in] i_iHeight height of the texture to be created in pixels.
	/// @param[in] i_iDepth depth of the texture to be created in pixels.
	/// @param[in] i_iMipLevels number of mip-levels to be created. Specify 0 to create a full mip-chain.
	/// @param[in] i_fmtFormat format of the texture to be created. Member of the enumeration m3dformat; m3dfmt_r32f, m3dfmt_r32g32f, m3dfmt_r32g32b32f or m3dfmt_r32g32b32a32f.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one or more parameters were invalid.
	/// @return e_outofmemory if memory allocation failed.
	/// @return e_invalidformat if an invalid format was encountered.
	result Create( uint32 i_iWidth, uint32 i_iHeight, uint32 i_iDepth,
		uint32 i_iMipLevels, m3dformat i_fmtFormat );

	m3dtexsampleinput eGetTexSampleInput(); ///< Sampling this texture requires 3 floating point coordinates.

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

	/// Clears the texture to a given color.
	/// @param[in] i_iMipLevel the mip-level to be cleared.
	/// @param[in] i_vColor color to clear the texture to.
	/// @param[in] i_pBox box to restrict clearing to.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if the clear-box exceeds the texture's dimensions.
	result Clear( uint32 i_iMipLevel, const vector4 &i_vColor, const m3dbox *i_pBox );

	/// Returns a pointer to the contents of a given mip-level.
	/// @param[in] i_iMipLevel mip-level that is requested, 0 being the largest mip-level.
	/// @param[out] o_ppData receives the pointer to the texture-data.
	/// @param[in] i_pBox volume that will be locked and accessible. (Pass in 0 to lock entire texture.)
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one or more parameters were invalid.
	/// @return e_invalidstate if the texture is already locked.
	/// @return e_outofmemory if memory allocation failed.
	/// @note Locking the entire texture is a lot faster than locking a sub-region, because no lock-buffer has to be created and the application may write to the texture directly.
	result LockBox( uint32 i_iMipLevel, void **o_ppData, const m3dbox *i_pBox );

	/// Unlocks the given mip-level; modifications to the texture will become active.
	/// @param[in] i_iMipLevel mip-level, 0 being the largest mip-level.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one or more parameters were invalid.
	result UnlockBox( uint32 i_iMipLevel );

	/// Returns a volume-pointer to a given texture mip-level.
	/// Calling this function will increase the internal reference count of the volume. Failure to call Release() when finished using the pointer will result in a memory leak.
	/// @param[in] i_iMipLevel mip-level, 0 being the largest mip-level.
	class CMuli3DVolume *pGetMipLevel( uint32 i_iMipLevel );

	m3dformat fmtGetFormat();	///< Returns the format of the texture. Member of the enumeration m3dformat; m3dfmt_r32f, m3dfmt_r32g32f, m3dfmt_r32g32b32f or m3dfmt_r32g32b32a32f.
	uint32 iGetFormatFloats();	///< Returns the number of floats of the format, e [1,4].
	uint32 iGetMipLevels();		///< Returns the number of mip-levels this texture consists of.
	
	/// Returns the width of the given mip-level in pixels.
	/// @param[in] i_iMipLevel the mip-level whose width is requested.
	uint32 iGetWidth( uint32 i_iMipLevel = 0 );
	
	/// Returns the height of the given mip-level in pixels.
	/// @param[in] i_iMipLevel the mip-level whose height is requested.
	uint32 iGetHeight( uint32 i_iMipLevel = 0 );

	/// Returns the depth of the given mip-level in pixels.
	/// @param[in] i_iMipLevel the mip-level whose depth is requested.
	uint32 iGetDepth( uint32 i_iMipLevel = 0 );

private:
	uint32				m_iMipLevels;			///< Number of mip-levels.
	float32				m_fSquaredWidth, m_fSquaredHeight, m_fSquaredDepth; ///< Squared dimensions of the base mip-level, used for mip-calculations.
	class CMuli3DVolume	**m_ppMipLevels;		///< The texture's mip-levels.
};

#endif // __M3DCORE_VOLUMETEXTURE_H__
