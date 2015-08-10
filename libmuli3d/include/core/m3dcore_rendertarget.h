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

/// @file m3dcore_rendertarget.h
///

#ifndef __M3DCORE_RENDERTARGET_H__
#define __M3DCORE_RENDERTARGET_H__

#include "../m3dbase.h"
#include "../m3dtypes.h"

/// This class defines a Muli3D rendertarget.
class CMuli3DRenderTarget : public IBase
{
protected:
	~CMuli3DRenderTarget(); ///< Accessible by IBase. The destructor is called when the reference count reaches zero.

	friend class CMuli3DDevice;
	/// Accessible by CMuli3DDevice which is the only class that may create a render target.
	/// @param[in] i_pParent a pointer to the parent CMuli3DDevice-object.
	CMuli3DRenderTarget( class CMuli3DDevice *i_pParent );

public:
	class CMuli3DDevice *pGetDevice(); ///< Returns a pointer to the associated device. Calling this function will increase the internal reference count of the device. Failure to call Release() when finished using the pointer will result in a memory leak.

	/// Clears the colorbuffer, which is associated with this rendertarget, to a given color.
	/// @param[in] i_vColor color to clear the colorbuffer to.
	/// @param[in] i_pRect rectangle to restrict clearing to.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidstate if no colorbuffer has been set.
	/// @return e_invalidparameters if the clear-rectangle exceeds the colorbuffer's dimensions.
	result ClearColorBuffer( const vector4 &i_vColor, const m3drect *i_pRect );

	/// Clears the depthbuffer, which is associated with this rendertarget, to a given depth-value.
	/// @param[in] i_fDepth depth to clear the depthbuffer to.
	/// @param[in] i_pRect rectangle to restrict clearing to.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidstate if no depthbuffer has been set.
	/// @return e_invalidparameters if the clear-rectangle exceeds the depthbuffer's dimensions.
	result ClearDepthBuffer( float32 i_fDepth, const m3drect *i_pRect );

	/// Associates a CMuli3DSurface as colorbuffer with this rendertarget, releasing the currently set colorbuffer.
	/// Calling this function will increase the internal reference count of the surface.
	/// @param[in] i_pColorBuffer new colorbuffer.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidformat if an invalid format was encountered.
	result SetColorBuffer( class CMuli3DSurface *i_pColorBuffer );

	/// Associates a CMuli3DSurface as depthbuffer with this rendertarget, releasing the currently set depthbuffer.
	/// Calling this function will increase the internal reference count of the surface.
	/// @param[in] i_pDepthBuffer new depthbuffer.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidformat if an invalid format was encountered.
	result SetDepthBuffer( class CMuli3DSurface *i_pDepthBuffer );
	
	class CMuli3DSurface *pGetColorBuffer(); ///< Returns a pointer to the rendertarget's colorbuffer. Calling this function will increase the internal reference count of the texture. Failure to call Release() when finished using the pointer will result in a memory leak.
	
	class CMuli3DSurface *pGetDepthBuffer(); ///< Returns a pointer to the rendertarget's depthbuffer. Calling this function will increase the internal reference count of the texture. Failure to call Release() when finished using the pointer will result in a memory leak.

	/// Sets the rendertarget's viewport matrix.
	/// @param[in] i_matViewport the viewport matrix.
	void SetViewportMatrix( const matrix44 &i_matViewport );
	const matrix44 &matGetViewportMatrix(); ///< Returns the rendertarget's viewport matrix.

private:
	class CMuli3DDevice		*m_pParent;			///< Pointer to parent.
	class CMuli3DSurface	*m_pColorBuffer;	///< Pointer to the colorbuffer.
	class CMuli3DSurface	*m_pDepthBuffer;	///< Pointer to the depthbuffer.
	matrix44				m_matViewport;		///< Viewport matrix.
};

#endif // __M3DCORE_RENDERTARGET_H__
