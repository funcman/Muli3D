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

/// @file m3dcore_volume.h
///

#ifndef __M3DCORE_VOLUME_H__
#define __M3DCORE_VOLUME_H__

#include "../m3dbase.h"
#include "../m3dtypes.h"

/// CMuli3DVolume implements a 3-dimensional image.
class CMuli3DVolume : public IBase
{
protected:
	~CMuli3DVolume(); ///< Accessible by IBase. The destructor is called when the reference count reaches zero.

	friend class CMuli3DDevice;
	/// Accessible by CMuli3DDevice which is the only class that may create a volume.
	/// @param[in] i_pParent a pointer to the parent CMuli3DDevice-object.
	CMuli3DVolume( class CMuli3DDevice *i_pParent );

	/// Accessible by CMuli3DDevice which is the only class that may create a volume.
	/// @param[in] i_iWidth width of the volume to be created in pixels.
	/// @param[in] i_iHeight height of the volume to be created in pixels.
	/// @param[in] i_iDepth depth of the volume to be created in pixels.
	/// @param[in] i_fmtFormat format of the volume to be created. Member of the enumeration m3dformat; m3dfmt_r32f, m3dfmt_r32g32f, m3dfmt_r32g32b32f or m3dfmt_r32g32b32a32f.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one or more parameters were invalid.
	/// @return e_outofmemory if memory allocation failed.
	/// @return e_invalidformat if an invalid format was encountered.
	result Create( uint32 i_iWidth, uint32 i_iHeight, uint32 i_iDepth,
		m3dformat i_fmtFormat );

public:
	/// Samples the volume using nearest point sampling.
	/// @param[out] o_vColor receives the color of the pixel to be looked up.
	/// @param[in] i_fU u-component of the lookup-vector.
	/// @param[in] i_fV v-component of the lookup-vector.
	/// @param[in] i_fW w-component of the lookup-vector.
	void SamplePoint( vector4 &o_vColor, float32 i_fU, float32 i_fV, float32 i_fW );

	/// Samples the volume using bi-linear filtering.
	/// @param[out] o_vColor receives the color of the pixel to be looked up.
	/// @param[in] i_fU u-component of the lookup-vector.
	/// @param[in] i_fV v-component of the lookup-vector.
	/// @param[in] i_fW w-component of the lookup-vector.
	void SampleLinear( vector4 &o_vColor, float32 i_fU, float32 i_fV, float32 i_fW );

	/// Clears the volume to a given color.
	/// @param[in] i_vColor color to clear the volume to.
	/// @param[in] i_pBox box to restrict clearing to.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if the clear-box exceeds the volume's dimensions.
	result Clear( const vector4 &i_vColor, const m3dbox *i_pBox );

	/// Copies the contents of the volume to another volume using the specified filtering method.
	/// @param[in] i_pSrcBox source box to copy to the other volume. (Pass 0 to copy the entire volume.)
	/// @param[in] i_pDestVolume destination volume.
	/// @param[in] i_pDestBox destination-box. (Pass 0 for the entire destination-volume.)
	/// @param[in] i_Filter texture filter.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one of the two boxes is invalid or exceeds volume-dimensions.
	/// @return e_invalidstate if the destination volume couldn't be locked.
	result CopyToVolume( const m3dbox *i_pSrcBox, CMuli3DVolume *i_pDestVolume,
		const m3dbox *i_pDestBox, m3dtexturefilter i_Filter );

	/// Returns a pointer to the contents of the volume.
	/// @param[out] o_ppData receives the pointer to the volume-data.
	/// @param[in] i_pBox box that will be locked and accessible. (Pass in 0 to lock the entire volume.)
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one or more parameters were invalid.
	/// @return e_invalidstate if the volume is already locked.
	/// @return e_outofmemory if memory allocation failed.
	/// @note Locking the entire volume is a lot faster than locking a sub-region, because no lock-buffer has to be created and the application may write to the volume directly.
	result LockBox( void **o_ppData, const m3dbox *i_pBox );

	/// Unlocks the volume; modifications to its contents will become active.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidstate if the volume is not locked.
	result UnlockBox();

	m3dformat fmtGetFormat();	///< Returns the format of the volume. Member of the enumeration m3dformat; m3dfmt_r32f, m3dfmt_r32g32f, m3dfmt_r32g32b32f or m3dfmt_r32g32b32a32f.
	uint32 iGetFormatFloats();	///< Returns the number of floats of the format, e [1,4].
	
	uint32 iGetWidth(); ///< Returns the width of the volume in pixels.
	uint32 iGetHeight(); ///< Returns the height of the volume in pixels.
	uint32 iGetDepth(); ///< Returns the depth of the volume in pixels.

	/// Returns a pointer to the associated device. Calling this function will increase the internal reference count of the device. Failure to call Release() when finished using the pointer will result in a memory leak.
	class CMuli3DDevice *pGetDevice();

private:
	class CMuli3DDevice	*m_pParent;	///< Pointer to parent.

	m3dformat	m_fmtFormat;	///< Format of the volume. Member of the enumeration m3dformat; m3dfmt_r32f, m3dfmt_r32g32f, m3dfmt_r32g32b32f or m3dfmt_r32g32b32a32f.
	uint32		m_iWidth;		///< Width of the volume in pixels.
	uint32		m_iHeight;		///< Height of the volume in pixels.
	uint32		m_iDepth;		///< Depth of the volume in pixels.
	uint32		m_iWidthMin1;	///< Width - 1 of the volume in pixels.
	uint32		m_iHeightMin1;	///< Height - 1 of the volume in pixels.
	uint32		m_iDepthMin1;	///< Depth - 1 of the volume in pixels.

	bool	m_bLockedComplete;		///< True if the whole volume has been locked.
	m3dbox	m_PartialLockBox;		///< Information about the locked box.
	float32	*m_pPartialLockData;	///< Not null if a sub-box of the volume has been locked.

	float32	*m_pData;	///< Pointer to volume data.
};

#endif // __M3DCORE_VOLUME_H__
