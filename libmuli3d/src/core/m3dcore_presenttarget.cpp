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

#include "../../include/core/m3dcore_presenttarget.h"
#include "../../include/core/m3dcore_device.h"

IMuli3DPresentTarget::IMuli3DPresentTarget( CMuli3DDevice *i_pParent ) :
	m_pParent( i_pParent )
{
	//	note: cannot add a reference to parent or the presenttarget will never be freed
	//	m_pParent->AddRef();
}

IMuli3DPresentTarget::~IMuli3DPresentTarget()
{
	//	note: see note in constructor.
	//	SAFE_RELEASE( m_pParent );
}

CMuli3DDevice *IMuli3DPresentTarget::pGetDevice() {
	if( m_pParent )
		m_pParent->AddRef();

	return m_pParent;
}

// ----------------------------------------------------------------------------

#ifdef WIN32

CMuli3DPresentTargetWin32::CMuli3DPresentTargetWin32( CMuli3DDevice *i_pParent )
	: IMuli3DPresentTarget( i_pParent ),
	m_pDirectDraw( 0 ), m_pDirectDrawClipper( 0 ), m_bDDSurfaceLost( false )
{
	m_pDirectDrawSurfaces[0] = m_pDirectDrawSurfaces[1] = 0;
}

CMuli3DPresentTargetWin32::~CMuli3DPresentTargetWin32()
{
	m3ddeviceparameters DeviceParameters = m_pParent->GetDeviceParameters();

	// Release DirectDraw -----------------------------------------------------
	if( m_pDirectDrawSurfaces[1] )
	{
		if( DeviceParameters.bWindowed ) // Only may release the back surface if we allocated it seperately from the primary surface (which we do in windowed mode)
			m_pDirectDrawSurfaces[1]->Release();
		m_pDirectDrawSurfaces[1] = 0;
	}

	SAFE_RELEASE( m_pDirectDrawSurfaces[0] );
	SAFE_RELEASE( m_pDirectDrawClipper );
	if( m_pDirectDraw )
	{
		if( DeviceParameters.bWindowed )
		{
			m_pDirectDraw->RestoreDisplayMode();
			m_pDirectDraw->SetCooperativeLevel( DeviceParameters.hDeviceWindow, DDSCL_NORMAL );
		}
		m_pDirectDraw->Release();
		m_pDirectDraw = 0;
	}
}

result CMuli3DPresentTargetWin32::Create()
{
	m3ddeviceparameters DeviceParameters = m_pParent->GetDeviceParameters();

	if( !DeviceParameters.iBackbufferWidth || !DeviceParameters.iBackbufferHeight )
	{
		FUNC_FAILING( "CMuli3DPresentTargetWin32::Create: invalid backbuffer dimensions have been supplied.\n" );
		return e_invalidparameters;
	}

	if( FAILED( DirectDrawCreateEx( 0, (void **)&m_pDirectDraw, IID_IDirectDraw7, 0 ) ) )
	{
		FUNC_FAILING( "CMuli3DPresentTargetWin32::Create: couldn't create DirectDraw 7 instance.\n" );
		return e_unknown;
	}

	uint32 iDDFlags = DeviceParameters.bWindowed ? DDSCL_NORMAL : DDSCL_EXCLUSIVE|DDSCL_FULLSCREEN|DDSCL_ALLOWREBOOT;
    if( FAILED( m_pDirectDraw->SetCooperativeLevel( DeviceParameters.hDeviceWindow, iDDFlags ) ) )
	{
		FUNC_FAILING( "CMuli3DPresentTargetWin32::Create: couldn't set cooperative level.\n" );
		return e_unknown;
	}

	if( !DeviceParameters.bWindowed )
	{
		switch( DeviceParameters.iFullscreenColorBits )
		{
		case 16:
		case 24:
		case 32:
			break;

		default:
			FUNC_FAILING( "CMuli3DPresentTargetWin32::Create: invalid backbuffer bit-depth specified.\n" );
			return e_unknown;
		}

		// Go to fullscreen mode
		if( FAILED( m_pDirectDraw->SetDisplayMode( DeviceParameters.iBackbufferWidth, DeviceParameters.iBackbufferHeight, DeviceParameters.iFullscreenColorBits, 0, 0 ) ) )
		{
			FUNC_FAILING( "CMuli3DPresentTargetWin32::Create: couldn't set display mode.\n" );
			return e_unknown;
		}
	}
	else
	{
		// Determine display mode.
		DDSURFACEDESC2 descSurface;
		descSurface.dwSize = sizeof( descSurface );
		if( FAILED( m_pDirectDraw->GetDisplayMode( &descSurface ) ) )
		{
			FUNC_FAILING( "CMuli3DPresentTargetWin32::Create: couldn't get display mode.\n" );
			return e_unknown;
		}

		switch( descSurface.ddpfPixelFormat.dwRGBBitCount )
		{
		case 16: // also handles formats 555(15-bit) and 444(12-bit)
			{
				uint16 iNumRedBits, iNumGreenBits, iNumBlueBits;
				ProcessBits( descSurface.ddpfPixelFormat.dwRBitMask, m_i16bitShift[0], iNumRedBits );
				ProcessBits( descSurface.ddpfPixelFormat.dwGBitMask, m_i16bitShift[1], iNumGreenBits );
				ProcessBits( descSurface.ddpfPixelFormat.dwBBitMask, m_i16bitShift[2], iNumBlueBits );

				m_i16bitMaxVal[0] = (uint16)(( 1 << iNumRedBits ) - 1);
				m_i16bitMaxVal[1] = (uint16)(( 1 << iNumGreenBits ) - 1);
				m_i16bitMaxVal[2] = (uint16)(( 1 << iNumBlueBits ) - 1);
			}
			break;

		case 24:
		case 32:
			break;

		default:
			FUNC_FAILING( "CMuli3DPresentTargetWin32::Create: display modes with colors other than 16-, 24- or 32-bit are not supported!\n" );
			return e_unknown;
		}
	}

	if( DeviceParameters.bWindowed )
	{
		// Create a primary surface without a backbuffer - a secondary surface will be used as backbuffer.
		DDSURFACEDESC2 descSurface;
		ZeroMemory( &descSurface, sizeof( descSurface ) );
		descSurface.dwSize = sizeof( descSurface );
		descSurface.dwFlags = DDSD_CAPS;
		descSurface.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
		if( FAILED( m_pDirectDraw->CreateSurface( &descSurface, &m_pDirectDrawSurfaces[0], 0 ) ) )
		{
			FUNC_FAILING( "CMuli3DPresentTargetWin32::Create: couldn't create primary surface.\n" );
			return e_unknown;
		}

		// Create the secondary surface
		descSurface.dwFlags = DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH;
		descSurface.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
		descSurface.dwWidth = DeviceParameters.iBackbufferWidth;
		descSurface.dwHeight = DeviceParameters.iBackbufferHeight;
		if( FAILED( m_pDirectDraw->CreateSurface( &descSurface, &m_pDirectDrawSurfaces[1], 0 ) ) )
		{
			FUNC_FAILING( "CMuli3DPresentTargetWin32::Create: couldn't create secondary surface.\n" );
			return e_unknown;
		}

		// Create clipper
		if( FAILED( m_pDirectDraw->CreateClipper( 0, &m_pDirectDrawClipper, 0 ) ) )
		{
			FUNC_FAILING( "CMuli3DPresentTargetWin32::Create: couldn't create clipper.\n" );
			return e_unknown;
		}

		// Set clipper to window
		if( FAILED( m_pDirectDrawClipper->SetHWnd( 0, DeviceParameters.hDeviceWindow ) ) )
		{
			FUNC_FAILING( "CMuli3DPresentTargetWin32::Create: couldn't set clipper window handle.\n" );
			return e_unknown;
		}

		// Attach clipper object to primary surface
		if( FAILED( m_pDirectDrawSurfaces[0]->SetClipper( m_pDirectDrawClipper ) ) )
		{
			FUNC_FAILING( "CMuli3DPresentTargetWin32::Create: couldn't attach clipper to primary surface.\n" );
			return e_unknown;
		}
	}
	else
	{
		// Create a primary surface with one backbuffer if in fullscreen mode.
		DDSURFACEDESC2 descSurface;
		ZeroMemory( &descSurface, sizeof( descSurface ) );
		descSurface.dwSize = sizeof( descSurface );
		descSurface.dwFlags = DDSD_CAPS|DDSD_BACKBUFFERCOUNT;
		descSurface.dwBackBufferCount = 1;
		descSurface.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE|DDSCAPS_COMPLEX|DDSCAPS_FLIP;
		if( FAILED( m_pDirectDraw->CreateSurface( &descSurface, &m_pDirectDrawSurfaces[0], 0 ) ) )
		{
			FUNC_FAILING( "CMuli3DPresentTargetWin32::Create: couldn't create primary surface.\n" );
			return e_unknown;
		}

		// Get DD backbuffer
		DDSCAPS2 capsDirectDraw = { DDSCAPS_BACKBUFFER };
		if( FAILED( m_pDirectDrawSurfaces[0]->GetAttachedSurface( &capsDirectDraw, &m_pDirectDrawSurfaces[1] ) ) )
		{
			FUNC_FAILING( "CMuli3DPresentTargetWin32::Create: couldn't access secondary surface.\n" );
			return e_unknown;
		}
	}

	return s_ok;
}

// Small utility function to find the LowBit and Number of Bits in a supplied Mask
// Thank you, Nathan Davies! - http://www.gamedev.net/reference/articles/article588.asp
void CMuli3DPresentTargetWin32::ProcessBits( uint32 i_iMask, uint16 &o_iLowBit, uint16 &o_iNumBits )
{
	uint32 iTestMask = 1;
	for( o_iLowBit = 0; o_iLowBit < 32; ++o_iLowBit )
	{
		if( i_iMask & iTestMask )
			break;

		iTestMask <<= 1;
	}

	iTestMask <<= 1;
	for( o_iNumBits = 1; o_iNumBits < 32; ++o_iNumBits )
	{
		if( !( i_iMask & iTestMask ) )
			break;

		iTestMask <<= 1;
	}
}

result CMuli3DPresentTargetWin32::Present( const float32 *i_pSource, uint32 i_iFloats )
{
	m3ddeviceparameters DeviceParameters = m_pParent->GetDeviceParameters();

	// Check for lost DirectDraw surfaces -------------------------------------
	if( m_bDDSurfaceLost )
	{
		m_pDirectDrawSurfaces[0]->Restore();
		if( DeviceParameters.bWindowed )
			m_pDirectDrawSurfaces[1]->Restore();
		m_bDDSurfaceLost = false;
	}

    // Lock backbuffer-surface-------------------------------------------------
	DDSURFACEDESC2 descSurface;
    descSurface.dwSize = sizeof( DDSURFACEDESC2 );
    if( FAILED( m_pDirectDrawSurfaces[1]->Lock( 0, &descSurface, DDLOCK_WAIT|DDLOCK_NOSYSLOCK|DDLOCK_WRITEONLY, 0 ) ) )
	{
		FUNC_FAILING( "CMuli3DPresentTargetWin32::Present: couldn't lock secondary surface.\n" );
		return e_unknown;
	}

	const uint32 iDestBytes = descSurface.ddpfPixelFormat.dwRGBBitCount / 8;

    // Copy pixels to the backbuffer-surface ----------------------------------
	const uint32 iDestRowJump = descSurface.lPitch - iDestBytes * DeviceParameters.iBackbufferWidth;
	uint8 *pDestination = (uint8 *)descSurface.lpSurface;

	fpuTruncate();

	if( iDestBytes == 2 )
	{
		// 16-bit
		uint32 iHeight = DeviceParameters.iBackbufferHeight;
		while( iHeight-- )
		{
			uint32 iWidth = DeviceParameters.iBackbufferWidth;
			while( iWidth-- )
			{
				const int32 iR = iClamp( ftol( i_pSource[0] * m_i16bitMaxVal[0] ), 0, m_i16bitMaxVal[0] );
				const int32 iG = iClamp( ftol( i_pSource[1] * m_i16bitMaxVal[1] ), 0, m_i16bitMaxVal[1] );
				const int32 iB = iClamp( ftol( i_pSource[2] * m_i16bitMaxVal[2] ), 0, m_i16bitMaxVal[2] );
				i_pSource += i_iFloats;

				*((uint16 *)pDestination) = ( iR << m_i16bitShift[0] ) | ( iG << m_i16bitShift[1] ) | ( iB << m_i16bitShift[2] );
				pDestination += 2;
			}
			pDestination += iDestRowJump;
		}
	}
	else
	{
		// 24- or 32-bit
		uint32 iHeight = DeviceParameters.iBackbufferHeight;
		while( iHeight-- )
		{
			uint32 iWidth = DeviceParameters.iBackbufferWidth;
			while( iWidth-- )
			{
				pDestination[0] = iClamp( ftol( i_pSource[2] * 255.0f ), 0, 255 ); // b
				pDestination[1] = iClamp( ftol( i_pSource[1] * 255.0f ), 0, 255 ); // g
				pDestination[2] = iClamp( ftol( i_pSource[0] * 255.0f ), 0, 255 ); // r

				i_pSource += i_iFloats;
				pDestination += iDestBytes;
			}
			pDestination += iDestRowJump;
		}
	}

	fpuReset();

    // Unlock backbuffer-surface and surface
    m_pDirectDrawSurfaces[1]->Unlock( 0 );

	// Present the image to the screen ----------------------------------------
	if( DeviceParameters.bWindowed )
	{
		POINT pntTopLeft = { 0, 0 };
        ClientToScreen( DeviceParameters.hDeviceWindow, &pntTopLeft );

		RECT rctDestination;
        GetClientRect( DeviceParameters.hDeviceWindow, &rctDestination );
		OffsetRect( &rctDestination, pntTopLeft.x, pntTopLeft.y );

		RECT rctSource;
		SetRect( &rctSource, 0, 0, DeviceParameters.iBackbufferWidth, DeviceParameters.iBackbufferHeight );

        // Blt secondary to primary surface
        if( FAILED( m_pDirectDrawSurfaces[0]->Blt( &rctDestination, m_pDirectDrawSurfaces[1], &rctSource, DDBLT_WAIT, 0 ) ) )
		{
			m_bDDSurfaceLost = true;
			FUNC_FAILING( "CMuli3DPresentTargetWin32::Present: primary surfaces have been lost!\n" );
			return e_unknown;
		}
	}
	else
	{
		if( FAILED( m_pDirectDrawSurfaces[0]->Flip( 0, DDFLIP_WAIT ) ) )
		{
			m_bDDSurfaceLost = true;
			FUNC_FAILING( "CMuli3DPresentTargetWin32::Present: surfaces have been lost!\n" );
			return e_unknown;
		}
	}

	return s_ok;
}

#endif

// ----------------------------------------------------------------------------

#ifdef LINUX_X11

CMuli3DPresentTargetLinuxX11::CMuli3DPresentTargetLinuxX11( CMuli3DDevice *i_pParent )
	: IMuli3DPresentTarget( i_pParent ),
	m_pDisplay( 0 ), m_pXImage( 0 ), m_iPixelBytes( 0 )
{}

CMuli3DPresentTargetLinuxX11::~CMuli3DPresentTargetLinuxX11()
{
	if( m_pXImage )
	{
		SAFE_DELETE_ARRAY( m_pXImage->data );
		XDestroyImage( m_pXImage );
		m_pXImage = 0;
	}

	if( m_pDisplay )
	{
		XCloseDisplay( m_pDisplay );
		m_pDisplay = 0;
	}
}

result CMuli3DPresentTargetLinuxX11::Create()
{
	m3ddeviceparameters DeviceParameters = m_pParent->GetDeviceParameters();

	if( !DeviceParameters.iBackbufferWidth || !DeviceParameters.iBackbufferHeight )
	{
		FUNC_FAILING( "CMuli3DPresentTargetLinuxX11::Create: invalid backbuffer dimensions have been supplied.\n" );
		return e_invalidparameters;
	}

	m_pDisplay = XOpenDisplay( 0 );
	if( !m_pDisplay )
	{
		FUNC_FAILING( "CMuli3DPresentTargetLinuxX11::Create: couldn't open X-display.\n" );
		return e_unknown;
	}

	const int32 iScreen = DefaultScreen( m_pDisplay );
	const int32 iDepth = DefaultDepth( m_pDisplay, iScreen );

	m_iPixelBytes = 0;
	int32 iPixmapFormatCount = 0;
	XPixmapFormatValues *pPixmapFormats = XListPixmapFormats( m_pDisplay, &iPixmapFormatCount );
	for( int32 iPixmapFormat = 0; iPixmapFormat < iPixmapFormatCount && !m_iPixelBytes; ++iPixmapFormat )
    {
		if( pPixmapFormats[iPixmapFormat].depth == iDepth )
		{
			switch( pPixmapFormats[iPixmapFormat].bits_per_pixel )
			{
			case 12:
				m_iPixelBytes = 2;
				m_i16bitMaxVal[0] = 15; m_i16bitMaxVal[1] = 15; m_i16bitMaxVal[2] = 15;
				m_i16bitShift[0] = 8; m_i16bitShift[1] = 4; m_i16bitShift[2] = 0;
				break;

			case 15:
				m_iPixelBytes = 2;
				m_i16bitMaxVal[0] = 31; m_i16bitMaxVal[1] = 31; m_i16bitMaxVal[2] = 31;
				m_i16bitShift[0] = 10; m_i16bitShift[1] = 5; m_i16bitShift[2] = 0;
				break;

			case 16:
				m_iPixelBytes = 2;
				m_i16bitMaxVal[0] = 31; m_i16bitMaxVal[1] = 63; m_i16bitMaxVal[2] = 31;
				m_i16bitShift[0] = 11; m_i16bitShift[1] = 5; m_i16bitShift[2] = 0;
				break;

			case 24: m_iPixelBytes = 3; break;
			case 32: m_iPixelBytes = 4; break;
			default: break;
			}
		}
    }
	XFree( pPixmapFormats );

	if( !m_iPixelBytes )
	{
		XCloseDisplay( m_pDisplay ); m_pDisplay = 0;
		FUNC_FAILING( "CMuli3DPresentTargetLinuxX11::Create: invalid pixel-depth! Make sure you're running in 16-, 24- or 32bits color-mode.\n" );
		return e_unknown;
	}

	m_WindowGC = DefaultGC( m_pDisplay, iScreen );
	m_pXImage = XCreateImage( m_pDisplay, CopyFromParent, iDepth, ZPixmap, 0, 0,
		  DeviceParameters.iBackbufferWidth, DeviceParameters.iBackbufferHeight, 32,
		  DeviceParameters.iBackbufferWidth * m_iPixelBytes );

	if( !m_pXImage )
	{
		FUNC_FAILING( "CMuli3DPresentTargetLinuxX11::Create: couldn't create X-image.\n" );
		XCloseDisplay( m_pDisplay ); m_pDisplay = 0;
		return e_unknown;
	}

	m_pXImage->data = new char[DeviceParameters.iBackbufferWidth * DeviceParameters.iBackbufferHeight * m_iPixelBytes];
	if( !m_pXImage->data )
	{
		XDestroyImage( m_pXImage ); m_pXImage = 0;
		XCloseDisplay( m_pDisplay ); m_pDisplay = 0;
		FUNC_FAILING( "CMuli3DPresentTargetLinuxX11::Create: couldn't allocate memory for backbuffer.\n" );
		return e_outofmemory;
	}

	return s_ok;
}

result CMuli3DPresentTargetLinuxX11::Present( const float32 *i_pSource, uint32 i_iFloats )
{
	m3ddeviceparameters DeviceParameters = m_pParent->GetDeviceParameters();

	// Copy pixels to the ximage-buffer ---------------------------------------

	fpuTruncate();

	if( m_iPixelBytes == 2 )
	{
		// 16-bit
		uint16 *pDestination = (uint16 *)m_pXImage->data;
		uint32 iPixels = DeviceParameters.iBackbufferWidth * DeviceParameters.iBackbufferHeight;
		while( iPixels-- )
		{
			const int32 iR = iClamp( ftol( i_pSource[0] * m_i16bitMaxVal[0] ), 0, m_i16bitMaxVal[0] );
			const int32 iG = iClamp( ftol( i_pSource[1] * m_i16bitMaxVal[1] ), 0, m_i16bitMaxVal[1] );
			const int32 iB = iClamp( ftol( i_pSource[2] * m_i16bitMaxVal[2] ), 0, m_i16bitMaxVal[2] );
			i_pSource += i_iFloats;

			*pDestination++ = ( iR << m_i16bitShift[0] ) | ( iG << m_i16bitShift[1] ) | ( iB << m_i16bitShift[2] );
		}
	}
	else
	{
		// 24- or 32-bit
		uint8 *pDestination = (uint8 *)m_pXImage->data;
		uint32 iPixels = DeviceParameters.iBackbufferWidth * DeviceParameters.iBackbufferHeight;
		while( iPixels-- )
		{
			pDestination[0] = iClamp( ftol( i_pSource[2] * 255.0f ), 0, 255 ); // b
			pDestination[1] = iClamp( ftol( i_pSource[1] * 255.0f ), 0, 255 ); // g
			pDestination[2] = iClamp( ftol( i_pSource[0] * 255.0f ), 0, 255 ); // r

			i_pSource += i_iFloats;
			pDestination += m_iPixelBytes;
		}
	}

	fpuReset();

	// Present to window/screen
	XPutImage( m_pDisplay, DeviceParameters.hDeviceWindow, m_WindowGC, m_pXImage, 0, 0,
		0, 0, DeviceParameters.iBackbufferWidth, DeviceParameters.iBackbufferHeight );

	return s_ok;
}

#endif

// ---------------------------------------------------------------------------------

#ifdef __amigaos4__

#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/picasso96api.h>

CMuli3DPresentTargetAmigaOS4::CMuli3DPresentTargetAmigaOS4( CMuli3DDevice *i_pParent )
	: IMuli3DPresentTarget( i_pParent ),
	m_pBitMap( 0 )
{}

CMuli3DPresentTargetAmigaOS4::~CMuli3DPresentTargetAmigaOS4()
{
	if ( m_pBitMap )
	{
		// Delete bitmap
		IGraphics->WaitBlit();
		IP96->p96FreeBitMap( m_pBitMap );
		m_pBitMap = 0;
	}
}

result CMuli3DPresentTargetAmigaOS4::Create()
{
	m3ddeviceparameters DeviceParameters = m_pParent->GetDeviceParameters();

	if( !DeviceParameters.iBackbufferWidth || !DeviceParameters.iBackbufferHeight )
	{
		FUNC_FAILING( "CMuli3DPresentTargetAmigaOS4::Create: invalid backbuffer dimensions have been supplied.\n" );
		return e_invalidparameters;
	}

	// Allocate buffer:
	if ( !( m_pBitMap = IP96->p96AllocBitMap(
		DeviceParameters.iBackbufferWidth,
		DeviceParameters.iBackbufferHeight,
		24,
		BMF_CLEAR|BMF_DISPLAYABLE,
		0,
		RGBFB_R8G8B8 ) ) )
	{
		FUNC_FAILING( "CMuli3dPresentTargetAmigaOS4::Create: couldn't allocate bitmap.\n");
		return e_unknown;
	}

	return s_ok;
}

result CMuli3DPresentTargetAmigaOS4::Present( const float32 *i_pSource, uint32 i_iFloats )
{
	m3ddeviceparameters DeviceParameters = m_pParent->GetDeviceParameters();

	// Copy pixels to the image-buffer ---------------------------------------

	// Doesn't currently do anything...
	fpuTruncate();

	// Picasso96 RenderInfo for locking the bitmap
	struct RenderInfo ri;

	// Lock bitmap for drawing
	ULONG lock = IP96->p96LockBitMap( m_pBitMap, (UBYTE*)&ri, sizeof(struct RenderInfo) );

	if ( lock )
	{
		// Pointer to allocated buffer
		uint8 *pDestination = (uint8 *)ri.Memory;
	
		// Help pointer to a row in bitmap
		uint8 *_pDestination;
	
		for ( uint32 y = 0; y < DeviceParameters.iBackbufferHeight; y++ )
		{
			// Next row
			_pDestination = pDestination + y * ri.BytesPerRow; 

			for ( uint32 x = 0; x < DeviceParameters.iBackbufferWidth; x++ )
			{
		 		//*_pDestination++ = iClamp( (int32)( *i_pSource++ * 255.0f ), 0, 255 ); // r
				//*_pDestination++ = iClamp( (int32)( *i_pSource++ * 255.0f ), 0, 255 ); // g
				//*_pDestination++ = iClamp( (int32)( *i_pSource++ * 255.0f ), 0, 255 ); // b

				// Set r, g and b bytes in our buffer
		 		_pDestination[0] = iClamp( ftol( i_pSource[0] * 255.0f ), 0, 255 ); // r
				_pDestination[1] = iClamp( ftol( i_pSource[1] * 255.0f ), 0, 255 ); // g
				_pDestination[2] = iClamp( ftol( i_pSource[2] * 255.0f ), 0, 255 ); // b

		 		//*_pDestination++ = (iClamp( ftol( i_pSource[0] * 255.0f ), 0, 255 ) << 16) |
				//	(iClamp( ftol( i_pSource[1] * 255.0f ), 0, 255 ) << 8) |
				//	iClamp( ftol( i_pSource[2] * 255.0f ), 0, 255 );

				i_pSource += i_iFloats;
				_pDestination += 3;
			}
		}

		IP96->p96UnlockBitMap( m_pBitMap, lock );

		// Blit bitmap to window
		IGraphics->BltBitMapRastPort( m_pBitMap, 0, 0, DeviceParameters.hDeviceWindow->RPort, DeviceParameters.hDeviceWindow->BorderLeft, DeviceParameters.hDeviceWindow->BorderTop, DeviceParameters.iBackbufferWidth, DeviceParameters.iBackbufferHeight, 0xC0);
	
		IGraphics->WaitTOF();
	} // lock

	// Doesn't currently do anything...
	fpuReset();

	return s_ok;
}

#endif
