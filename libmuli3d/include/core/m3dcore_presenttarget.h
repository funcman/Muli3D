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

/// @file m3dcore_presenttarget.h
///

#ifndef __M3DCORE_PRESENTTARGET_H__
#define __M3DCORE_PRESENTTARGET_H__

#include "../m3dbase.h"
#include "../m3dtypes.h"

/// Present-targets provide the base for platform-independet rendering output to the screen.
class IMuli3DPresentTarget : public IBase
{
protected:
	/// Accessible by IBase. The destructor is called when the reference count reaches zero.
	virtual ~IMuli3DPresentTarget();

	/// Present-target constructor.
	/// @param[in] i_pParent a pointer to the parent CMuli3DDevice-object.
	IMuli3DPresentTarget( class CMuli3DDevice *i_pParent );

public:
	/// Creates and initializes the presenttarget.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one or more parameters were invalid.
	/// @return e_unknown if a presenttarget-specific problem was encountered.
	virtual result Create() = 0;

	/// Presents the contents of a given rendertarget's colorbuffer.
	/// @param[in] i_pSource pointer to the data of the colorbuffer to be presented (backbuffer dimensions).
	/// @param[in] i_iFloats format of the data (number of float32s).
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one or more parameters were invalid.
	/// @return e_invalidformat if an invalid format was encountered.
	/// @return e_invalidstate if an invalid state was encountered.
	/// @return e_unknown if a present-target related problem was encountered.
	virtual result Present( const float32 *i_pSource, uint32 i_iFloats ) = 0;

	/// Returns a pointer to the associated device. Calling this function will increase the internal reference count of the device. Failure to call Release() when finished using the pointer will result in a memory leak.
	class CMuli3DDevice *pGetDevice();

protected:
	class CMuli3DDevice	*m_pParent;	///< Pointer to parent.
};

// Platform-dependent code ----------------------------------------------------

#ifdef WIN32

#include <ddraw.h>
#pragma comment( lib, "ddraw.lib" )
#pragma comment( lib, "dxguid.lib" )

/// This class defines a Muli3D presenttarget for the Windows-platform.
class CMuli3DPresentTargetWin32 : public IMuli3DPresentTarget
{
protected:
	virtual ~CMuli3DPresentTargetWin32(); ///< Accessible by IBase. The destructor is called when the reference count reaches zero.

	friend class CMuli3DDevice;
	/// Accessible by CMuli3DDevice which is the only class that may create a present target.
	/// @param[in] i_pParent a pointer to the parent CMuli3DDevice-object.
	CMuli3DPresentTargetWin32( class CMuli3DDevice *i_pParent );

public:
	/// Creates and initializes the presenttarget.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one or more parameters were invalid.
	/// @return e_unknown if a presenttarget-specific problem was encountered.
	result Create();

	/// Presents the contents of a given rendertarget's colorbuffer.
	/// @param[in] i_pSource pointer to the data of the colorbuffer to be presented (backbuffer dimensions).
	/// @param[in] i_iFloats format of the data (number of float32s).
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one or more parameters were invalid.
	/// @return e_invalidformat if an invalid format was encountered.
	/// @return e_invalidstate if an invalid state was encountered.
	/// @return e_unknown if a present-target related problem was encountered.
	result Present( const float32 *i_pSource, uint32 i_iFloats );

private:
	/// Returns low-bit and number of bits for a given color-channel mask.
	/// @param[in] i_iMask color-channel mask.
	/// @param[out] o_iLowBit lowbit of the channel.
	/// @param[out] o_iNumBits number of bits of the channel.
	void ProcessBits( uint32 i_iMask, uint16 &o_iLowBit, uint16 &o_iNumBits );

private:
	LPDIRECTDRAW7			m_pDirectDraw;				///< Pointer to DirectDraw.
	LPDIRECTDRAWCLIPPER		m_pDirectDrawClipper;		///< Pointer to a DirectDraw clipper.
	LPDIRECTDRAWSURFACE7	m_pDirectDrawSurfaces[2];	///< Pointer to DirectDraw surfaces.
	bool					m_bDDSurfaceLost;			///< True if DirectDraw surfaces have been lost and need to be restored.

	uint16	m_i16bitMaxVal[3];	///< Used when presenting to a 16-bit backbuffer. Masks and maximum color-values per channel, e.g. (31,63,31) for 16-bit 565 mode.
	uint16	m_i16bitShift[3];	///< Used when presenting to a 16-bit backbuffer. Shifts for the individual color channels, e.g. (
};

#endif

#ifdef LINUX_X11

/// This class defines a Muli3D presenttarget for the Linux-platform.
class CMuli3DPresentTargetLinuxX11 : public IMuli3DPresentTarget
{
protected:
	virtual ~CMuli3DPresentTargetLinuxX11(); ///< Accessible by IBase. The destructor is called when the reference count reaches zero.

	friend class CMuli3DDevice;
	/// Accessible by CMuli3DDevice which is the only class that may create a present target.
	/// @param[in] i_pParent a pointer to the parent CMuli3DDevice-object.
	CMuli3DPresentTargetLinuxX11( class CMuli3DDevice *i_pParent );

public:
	/// Creates and initializes the presenttarget.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one or more parameters were invalid.
	/// @return e_unknown if a presenttarget-specific problem was encountered.
	result Create();

	/// Presents the contents of a given rendertarget's colorbuffer.
	/// @param[in] i_pSource pointer to the data of the colorbuffer to be presented (backbuffer dimensions).
	/// @param[in] i_iFloats format of the data (number of float32s).
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one or more parameters were invalid.
	/// @return e_invalidformat if an invalid format was encountered.
	/// @return e_invalidstate if an invalid state was encountered.
	/// @return e_unknown if a present-target related problem was encountered.
	result Present( const float32 *i_pSource, uint32 i_iFloats );

private:
	Display	*m_pDisplay;	///< The X11 display.
	GC		m_WindowGC;		///< The graphic context.
	XImage	*m_pXImage;		///< The X-image used for output to the screen.
	uint32	m_iPixelBytes;	///< Number of bytes per pixel for output.

	uint16	m_i16bitMaxVal[3];	///< Used when presenting to a 16-bit backbuffer. Masks and maximum color-values per channel, e.g. (31,63,31) for 16-bit 565 mode.
	uint16	m_i16bitShift[3];	///< Used when presenting to a 16-bit backbuffer. Shifts for the individual color channels, e.g. (
};

#endif

#ifdef __amigaos4__

/// This class defines a Muli3D presenttarget for the AmigaOS-platform.
class CMuli3DPresentTargetAmigaOS4 : public IMuli3DPresentTarget
{
protected:
	virtual ~CMuli3DPresentTargetAmigaOS4(); ///< Accessible by IBase. The destructor is called when the reference count reaches zero.

	friend class CMuli3DDevice;
	/// Accessible by CMuli3DDevice which is the only class that may create a present target.
	/// @param[in] i_pParent a pointer to the parent CMuli3DDevice-object.
	CMuli3DPresentTargetAmigaOS4( class CMuli3DDevice *i_pParent );

public:
	/// Creates and initializes the presenttarget.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one or more parameters were invalid.
	/// @return e_unknown if a presenttarget-specific problem was encountered.
	result Create();

	/// Presents the contents of a given rendertarget's colorbuffer.
	/// @param[in] i_pSource pointer to the data of the colorbuffer to be presented (backbuffer dimensions).
	/// @param[in] i_iFloats format of the data (number of float32s).
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one or more parameters were invalid.
	/// @return e_invalidformat if an invalid format was encountered.
	/// @return e_invalidstate if an invalid state was encountered.
	/// @return e_unknown if a present-target related problem was encountered.
	result Present( const float32 *i_pSource, uint32 i_iFloats );

private:
	struct BitMap *m_pBitMap; ///< Pointer to the bitmap to be blitted into Window's RastPort
};

#endif

#endif // __M3DCORE_PRESENTTARGET_H__
