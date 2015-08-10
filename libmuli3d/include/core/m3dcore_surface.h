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

/// @file m3dcore_surface.h
///

#ifndef __M3DCORE_SURFACE_H__
#define __M3DCORE_SURFACE_H__

#include "../m3dbase.h"
#include "../m3dtypes.h"

/// CMuli3DSurface implements a 2-dimensional image.
class CMuli3DSurface : public IBase
{
protected:
	~CMuli3DSurface(); ///< Accessible by IBase. The destructor is called when the reference count reaches zero.

	friend class CMuli3DDevice;
	/// Accessible by CMuli3DDevice which is the only class that may create a surface.
	/// @param[in] i_pParent a pointer to the parent CMuli3DDevice-object.
	CMuli3DSurface( class CMuli3DDevice *i_pParent );

	/// Accessible by CMuli3DDevice which is the only class that may create a surface.
	/// @param[in] i_iWidth width of the surface to be created in pixels.
	/// @param[in] i_iHeight height of the surface to be created in pixels.
	/// @param[in] i_fmtFormat format of the surface to be created. Member of the enumeration m3dformat; m3dfmt_r32f, m3dfmt_r32g32f, m3dfmt_r32g32b32f or m3dfmt_r32g32b32a32f.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one or more parameters were invalid.
	/// @return e_outofmemory if memory allocation failed.
	/// @return e_invalidformat if an invalid format was encountered.
	result Create( uint32 i_iWidth, uint32 i_iHeight, m3dformat i_fmtFormat );

public:
	/// Samples the surface using nearest point sampling.
	/// @param[out] o_vColor receives the color of the pixel to be looked up.
	/// @param[in] i_fU u-component of the lookup-vector.
	/// @param[in] i_fV v-component of the lookup-vector.
	void SamplePoint( vector4 &o_vColor, float32 i_fU, float32 i_fV );

	/// Samples the surface using bi-linear filtering.
	/// @param[out] o_vColor receives the color of the pixel to be looked up.
	/// @param[in] i_fU u-component of the lookup-vector.
	/// @param[in] i_fV v-component of the lookup-vector.
	void SampleLinear( vector4 &o_vColor, float32 i_fU, float32 i_fV );

	/// Clears the surface to a given color.
	/// @param[in] i_vColor color to clear the surface to.
	/// @param[in] i_pRect rectangle to restrict clearing to.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if the clear-rectangle exceeds the surface's dimensions.
	result Clear( const vector4 &i_vColor, const m3drect *i_pRect );

	/// Copies the contents of the surface to another surface using the specified filtering method.
	/// @param[in] i_pSrcRect source rectangle to copy to the other surface. (Pass 0 to copy the entire surface.)
	/// @param[in] i_pDestSurface destination surface.
	/// @param[in] i_pDestRect destination-rectangle. (Pass 0 for the entire destination-surface.)
	/// @param[in] i_Filter texture filter.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one of the two rectangles is invalid or exceeds surface-dimensions.
	/// @return e_invalidstate if the destination surface couldn't be locked.
	result CopyToSurface( const m3drect *i_pSrcRect, CMuli3DSurface *i_pDestSurface,
		const m3drect *i_pDestRect, m3dtexturefilter i_Filter );

	/// Returns a pointer to the contents of the surface.
	/// @param[out] o_ppData receives the pointer to the surface-data.
	/// @param[in] i_pRect area that will be locked and accessible. (Pass in 0 to lock the entire surface.)
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one or more parameters were invalid.
	/// @return e_invalidstate if the surface is already locked.
	/// @return e_outofmemory if memory allocation failed.
	/// @note Locking the entire surface is a lot faster than locking a sub-region, because no lock-buffer has to be created and the application may write to the surface directly.
	result LockRect( void **o_ppData, const m3drect *i_pRect );

	/// Unlocks the surface; modifications to its contents will become active.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidstate if the surface is not locked.
	result UnlockRect();

	m3dformat fmtGetFormat();	///< Returns the format of the surface. Member of the enumeration m3dformat; m3dfmt_r32f, m3dfmt_r32g32f, m3dfmt_r32g32b32f or m3dfmt_r32g32b32a32f.
	uint32 iGetFormatFloats();	///< Returns the number of floats of the format, e [1,4].
	
	uint32 iGetWidth(); ///< Returns the width of the surface in pixels.
	uint32 iGetHeight(); ///< Returns the height of the surface in pixels.

	/// Returns a pointer to the associated device. Calling this function will increase the internal reference count of the device. Failure to call Release() when finished using the pointer will result in a memory leak.
	class CMuli3DDevice *pGetDevice();

private:
	class CMuli3DDevice	*m_pParent;	///< Pointer to parent.

	m3dformat	m_fmtFormat;	///< Format of the surface. Member of the enumeration m3dformat; m3dfmt_r32f, m3dfmt_r32g32f, m3dfmt_r32g32b32f or m3dfmt_r32g32b32a32f.
	uint32		m_iWidth;		///< Width of the surface in pixels.
	uint32		m_iHeight;		///< Height of the surface in pixels.
	uint32		m_iWidthMin1;	///< Width - 1 of the surface in pixels.
	uint32		m_iHeightMin1;	///< Height - 1 of the surface in pixels.

	bool	m_bLockedComplete;		///< True if the whole surface has been locked.
	m3drect	m_PartialLockRect;		///< Information about the locked rectangle.
	float32	*m_pPartialLockData;	///< Not null if a sub-rectangle of the surface has been locked.

	float32	*m_pData;	///< Pointer to surface data.
};

#endif // __M3DCORE_SURFACE_H__
