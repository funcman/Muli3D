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

#include "../../include/core/m3dcore_surface.h"
#include "../../include/core/m3dcore_device.h"

CMuli3DSurface::CMuli3DSurface( CMuli3DDevice *i_pParent ) :
	m_pParent( i_pParent ), m_iWidth( 0 ), m_iHeight( 0 ), m_iWidthMin1( 0 ), m_iHeightMin1( 0 ),
	m_bLockedComplete( false ), m_pPartialLockData( 0 ), m_pData( 0 )
{}

CMuli3DSurface::~CMuli3DSurface()
{
	SAFE_DELETE_ARRAY( m_pPartialLockData ); // somebody might have forgotten to unlock the surface ;)
	SAFE_DELETE_ARRAY( m_pData );
}

result CMuli3DSurface::Create( uint32 i_iWidth, uint32 i_iHeight, m3dformat i_fmtFormat )
{
	if( !i_iWidth || !i_iHeight )
	{
		FUNC_FAILING( "CMuli3DSurface::Create: surface dimensions are invalid.\n" );
		return e_invalidparameters;
	}
	
	uint32 iFloats;
	switch( i_fmtFormat )
	{
	case m3dfmt_r32f: iFloats = 1; break;
	case m3dfmt_r32g32f: iFloats = 2; break;
	case m3dfmt_r32g32b32f: iFloats = 3; break;
	case m3dfmt_r32g32b32a32f: iFloats = 4; break;
	default: FUNC_FAILING( "CMuli3DSurface::Create: invalid format specified.\n" ); return e_invalidformat;
	}

	m_fmtFormat = i_fmtFormat;
	m_iWidth = i_iWidth;
	m_iHeight = i_iHeight;
	m_iWidthMin1 = m_iWidth - 1;
	m_iHeightMin1 = m_iHeight - 1;

	m_pData = new float32[m_iWidth * m_iHeight * iFloats];
	if( !m_pData )
	{
		FUNC_FAILING( "CMuli3DSurface::Create: out of memory, cannot create surface.\n" );
		return e_outofmemory;
	}

	return s_ok;
}

result CMuli3DSurface::Clear( const vector4 &i_vColor, const m3drect *i_pRect )
{
	m3drect ClearRect;
	if( i_pRect )
	{
		if( i_pRect->iRight > m_iWidth ||
			i_pRect->iBottom > m_iHeight )
		{
			FUNC_FAILING( "CMuli3DSurface::Clear: clear-rectangle exceeds surface's dimensions.\n" );
			return e_invalidparameters;
		}

		if( i_pRect->iLeft >= i_pRect->iRight ||
			i_pRect->iTop >= i_pRect->iBottom )
		{
			FUNC_FAILING( "CMuli3DSurface::Clear: invalid rectangle specified!\n" );
			return e_invalidparameters;
		}

		ClearRect = *i_pRect;
	}
	else
	{
		ClearRect.iLeft = 0; ClearRect.iTop = 0;
		ClearRect.iRight = m_iWidth; ClearRect.iBottom = m_iHeight;
	}

	float32 *pData;
	result resPointer = LockRect( (void **)&pData, 0 ); // lock entire surface for higher speed!
	if( FUNC_FAILED( resPointer ) )
		return resPointer;

	const uint32 iBridgeStep = ( m_iWidth - ClearRect.iRight ) + ClearRect.iLeft;

	switch( m_fmtFormat )
	{
	case m3dfmt_r32f:
		{
			float32 *pCurData = &pData[ClearRect.iTop * m_iWidth + ClearRect.iLeft];
			for( uint32 iY = ClearRect.iTop; iY < ClearRect.iBottom; ++iY, pCurData += iBridgeStep )
			{
				for( uint32 iX = ClearRect.iLeft; iX < ClearRect.iRight; ++iX, ++pCurData )
					*pCurData = i_vColor.r;
			}
		}
		break;

	case m3dfmt_r32g32f:
		{
			vector2 *pCurData = &((vector2 *)pData)[ClearRect.iTop * m_iWidth + ClearRect.iLeft];
			for( uint32 iY = ClearRect.iTop; iY < ClearRect.iBottom; ++iY, pCurData += iBridgeStep )
			{
				for( uint32 iX = ClearRect.iLeft; iX < ClearRect.iRight; ++iX, ++pCurData )
					*pCurData = i_vColor;
			}
		}
		break;

	case m3dfmt_r32g32b32f:
		{
			vector3 *pCurData = &((vector3 *)pData)[ClearRect.iTop * m_iWidth + ClearRect.iLeft];
			for( uint32 iY = ClearRect.iTop; iY < ClearRect.iBottom; ++iY, pCurData += iBridgeStep )
			{
				for( uint32 iX = ClearRect.iLeft; iX < ClearRect.iRight; ++iX, ++pCurData )
					*pCurData = i_vColor;
			}
		}
		break;

	case m3dfmt_r32g32b32a32f:
		{
			vector4 *pCurData = &((vector4 *)pData)[ClearRect.iTop * m_iWidth + ClearRect.iLeft];
			for( uint32 iY = ClearRect.iTop; iY < ClearRect.iBottom; ++iY, pCurData += iBridgeStep )
			{
				for( uint32 iX = ClearRect.iLeft; iX < ClearRect.iRight; ++iX, ++pCurData )
					*pCurData = i_vColor;
			}
		}
		break;

	default: // cannot happen
		FUNC_FAILING( "CMuli3DSurface::Clear: invalid surface format.\n" );
		UnlockRect();
		return e_invalidformat;
	}

	UnlockRect();
	return s_ok;
}

result CMuli3DSurface::LockRect( void **o_ppData, const m3drect *i_pRect )
{
	if( !o_ppData )
	{
		FUNC_FAILING( "CMuli3DSurface::LockRect: o_ppData points to null.\n" );
		return e_invalidparameters;
	}

	if( m_bLockedComplete || m_pPartialLockData )
	{
		FUNC_FAILING( "CMuli3DSurface::LockRect: mip-level is already locked!\n" );
		return e_invalidstate;
	}

	if( !i_pRect )
	{
		*o_ppData = m_pData;
		m_bLockedComplete = true;
		return s_ok;
	}

	if( i_pRect->iRight > m_iWidth ||
		i_pRect->iBottom > m_iHeight )
	{
		FUNC_FAILING( "CMuli3DSurface::LockRect: rectangle exceeds surface dimensions!\n" );
		return e_invalidparameters;
	}

	if( i_pRect->iLeft >= i_pRect->iRight ||
		i_pRect->iTop >= i_pRect->iBottom )
	{
		FUNC_FAILING( "CMuli3DSurface::LockRect: invalid rectangle specified!\n" );
		return e_invalidparameters;
	}

	m_PartialLockRect = *i_pRect;
	
	// create lock-buffer
	const uint32 iLockWidth = m_PartialLockRect.iRight - m_PartialLockRect.iLeft;
	const uint32 iLockHeight = m_PartialLockRect.iBottom - m_PartialLockRect.iTop;
	const uint32 iSurfaceFloats = iGetFormatFloats();

	m_pPartialLockData = new float32[iLockWidth * iLockHeight * iSurfaceFloats];
	if( !m_pPartialLockData )
	{
		FUNC_FAILING( "CMuli3DSurface::LockRect: memory allocation failed!\n" );
		return e_outofmemory;
	}
	
	float32 *pCurLockData = m_pPartialLockData;
	for( uint32 iY = m_PartialLockRect.iTop; iY < m_PartialLockRect.iBottom; ++iY )
	{
		const float32 *pCurSurfaceData = &m_pData[(iY * m_iWidth + m_PartialLockRect.iLeft) * iSurfaceFloats];
		memcpy( pCurLockData, pCurSurfaceData, sizeof( float32 ) * iSurfaceFloats * iLockWidth );
		pCurLockData += iSurfaceFloats * iLockWidth;
	}

	*o_ppData = m_pPartialLockData;

	return s_ok;
}

result CMuli3DSurface::UnlockRect()
{
	if( !m_bLockedComplete && !m_pPartialLockData )
	{
		FUNC_FAILING( "CMuli3DSurface::UnlockRect: cannot unlock mip-level because it isn't locked!\n" );
		return e_invalidstate;
	}

	if( m_bLockedComplete )
	{
		m_bLockedComplete = false;
		return s_ok;
	}

	// update surface
	const uint32 iLockWidth = m_PartialLockRect.iRight - m_PartialLockRect.iLeft;
	const uint32 iSurfaceFloats = iGetFormatFloats();

	const float32 *pCurLockData = m_pPartialLockData;
	for( uint32 iY = m_PartialLockRect.iTop; iY < m_PartialLockRect.iBottom; ++iY )
	{
		float32 *pCurSurfaceData = &m_pData[(iY * m_iWidth + m_PartialLockRect.iLeft) * iSurfaceFloats];
		memcpy( pCurSurfaceData, pCurLockData, sizeof( float32 ) * iSurfaceFloats * iLockWidth );
		pCurLockData += iSurfaceFloats * iLockWidth;
	}

	SAFE_DELETE_ARRAY( m_pPartialLockData );

	return s_ok;
}

uint32 CMuli3DSurface::iGetFormatFloats()
{
	switch( m_fmtFormat )
	{
	case m3dfmt_r32f: return 1;
	case m3dfmt_r32g32f: return 2;
	case m3dfmt_r32g32b32f: return 3;
	case m3dfmt_r32g32b32a32f: return 4;
	default: /* cannot happen */ return 0;
	}
}

void CMuli3DSurface::SamplePoint( vector4 &o_vColor, float32 i_fU, float32 i_fV )
{
	const float32 fX = i_fU * m_iWidthMin1, fY = i_fV * m_iHeightMin1;
	const uint32 iPixelX = ftol( fX ), iPixelY = ftol( fY );

	switch( m_fmtFormat )
	{
	case m3dfmt_r32f:
		{
			const float32 *pPixel = &m_pData[iPixelY * m_iWidth + iPixelX];
			o_vColor = vector4( pPixel[0], 0, 0, 1 );
		}
		break;
	case m3dfmt_r32g32f:
		{
			const vector2 *pPixel = (const vector2 *)&m_pData[2 * (iPixelY * m_iWidth + iPixelX)];
			o_vColor = vector4( pPixel->x, pPixel->y, 0, 1 );
		}
		break;
	case m3dfmt_r32g32b32f:
		{
			const vector3 *pPixel = (const vector3 *)&m_pData[3 * (iPixelY * m_iWidth + iPixelX)];
			o_vColor = vector4( pPixel->x, pPixel->y, pPixel->z, 1 );
		}
		break;
	case m3dfmt_r32g32b32a32f:
		{
			const vector4 *pPixel = (const vector4 *)&m_pData[4 * (iPixelY * m_iWidth + iPixelX)];
			o_vColor = *pPixel;
		}
		break;
	default: // cannot happen
		break;
	}
}

void CMuli3DSurface::SampleLinear( vector4 &o_vColor, float32 i_fU, float32 i_fV )
{
	const float32 fX = i_fU * m_iWidthMin1, fY = i_fV * m_iHeightMin1;
	const uint32 iPixelX = ftol( fX ), iPixelY = ftol( fY );

	uint32 iPixelX2 = iPixelX + 1, iPixelY2 = iPixelY + 1;
	if( iPixelX2 >= m_iWidth ) iPixelX2 = m_iWidthMin1;
	if( iPixelY2 >= m_iHeight ) iPixelY2 = m_iHeightMin1;

	const uint32 iIndexRows[2] = { iPixelY * m_iWidth, iPixelY2 * m_iWidth };
	const float32 fInterpolation[2] = { fX - iPixelX, fY - iPixelY };

	switch( m_fmtFormat )
	{
	case m3dfmt_r32f:
		{
			float32 fColorRows[2];
			fColorRows[0] = fLerp( m_pData[iIndexRows[0] + iPixelX], m_pData[iIndexRows[0] + iPixelX2], fInterpolation[0] );
			fColorRows[1] = fLerp( m_pData[iIndexRows[1] + iPixelX], m_pData[iIndexRows[1] + iPixelX2], fInterpolation[0] );
			const float32 fFinalColor = fLerp( fColorRows[0], fColorRows[1], fInterpolation[1] );
			
			o_vColor = vector4( fFinalColor, 0, 0, 1 );
		}
		break;
	case m3dfmt_r32g32f:
		{
			const vector2 *pPixelData = (const vector2 *)m_pData;

			static vector2 vColorRows[2];
			vVector2Lerp( vColorRows[0], pPixelData[iIndexRows[0] + iPixelX], pPixelData[iIndexRows[0] + iPixelX2], fInterpolation[0] );
			vVector2Lerp( vColorRows[1], pPixelData[iIndexRows[1] + iPixelX], pPixelData[iIndexRows[1] + iPixelX2], fInterpolation[0] );
			static vector2 vFinalColor; vVector2Lerp( vFinalColor, vColorRows[0], vColorRows[1], fInterpolation[1] );

			o_vColor = vector4( vFinalColor.x, vFinalColor.y, 0, 1 );
		}
		break;
	case m3dfmt_r32g32b32f:
		{
			const vector3 *pPixelData = (const vector3 *)m_pData;

			static vector3 vColorRows[2];
			vVector3Lerp( vColorRows[0], pPixelData[iIndexRows[0] + iPixelX], pPixelData[iIndexRows[0] + iPixelX2], fInterpolation[0] );
			vVector3Lerp( vColorRows[1], pPixelData[iIndexRows[1] + iPixelX], pPixelData[iIndexRows[1] + iPixelX2], fInterpolation[0] );
			static vector3 vFinalColor; vVector3Lerp( vFinalColor, vColorRows[0], vColorRows[1], fInterpolation[1] );

			o_vColor = vector4( vFinalColor.x, vFinalColor.y, vFinalColor.z, 1 );
		}
		break;
	case m3dfmt_r32g32b32a32f:
		{
			const vector4 *pPixelData = (const vector4 *)m_pData;

			static vector4 vColorRows[2];
			vVector4Lerp( vColorRows[0], pPixelData[iIndexRows[0] + iPixelX], pPixelData[iIndexRows[0] + iPixelX2], fInterpolation[0] );
			vVector4Lerp( vColorRows[1], pPixelData[iIndexRows[1] + iPixelX], pPixelData[iIndexRows[1] + iPixelX2], fInterpolation[0] );
			vVector4Lerp( o_vColor, vColorRows[0], vColorRows[1], fInterpolation[1] );
		}
		break;
	default: // cannot happen
		break;
	}
}

m3dformat CMuli3DSurface::fmtGetFormat()
{
	return m_fmtFormat;
}

uint32 CMuli3DSurface::iGetWidth()
{
	return m_iWidth;
}

uint32 CMuli3DSurface::iGetHeight()
{
	return m_iHeight;
}

result CMuli3DSurface::CopyToSurface( const m3drect *i_pSrcRect, CMuli3DSurface *i_pDestSurface, const m3drect *i_pDestRect, m3dtexturefilter i_Filter )
{
	if( !i_pDestSurface )
	{
		FUNC_FAILING( "CMuli3DSurface::CopyToSurface: no destination surface specified!\n" );
		return e_invalidparameters;
	}

	if( i_Filter != m3dtf_point || i_Filter != m3dtf_linear )
	{
		FUNC_FAILING( "CMuli3DSurface::CopyToSurface: invalid filter specified!\n" );
		return e_invalidparameters;
	}

	m3drect SrcRect;
	if( i_pSrcRect )
	{
		if( i_pSrcRect->iRight > m_iWidth ||
			i_pSrcRect->iBottom > m_iHeight )
		{
			FUNC_FAILING( "CMuli3DSurface::CopyToSurface: source rectangle exceeds surface dimensions!\n" );
			return e_invalidparameters;
		}

		if( i_pSrcRect->iLeft >= i_pSrcRect->iRight ||
			i_pSrcRect->iTop >= i_pSrcRect->iBottom )
		{
			FUNC_FAILING( "CMuli3DSurface::CopyToSurface: invalid source rectangle specified!\n" );
			return e_invalidparameters;
		}

		SrcRect = *i_pSrcRect;
	}
	else
	{
		SrcRect.iLeft = 0; SrcRect.iTop = 0;
		SrcRect.iRight = m_iWidth; SrcRect.iBottom = m_iHeight;
	}

	m3drect DestRect;
	if( i_pDestRect )
	{
		if( i_pDestRect->iRight > i_pDestSurface->iGetWidth() ||
			i_pDestRect->iBottom > i_pDestSurface->iGetHeight() )
		{
			FUNC_FAILING( "CMuli3DSurface::CopyToSurface: destination rectangle exceeds surface dimensions!\n" );
			return e_invalidparameters;
		}

		if( i_pDestRect->iLeft >= i_pDestRect->iRight ||
			i_pDestRect->iTop >= i_pDestRect->iBottom )
		{
			FUNC_FAILING( "CMuli3DSurface::CopyToSurface: invalid destination rectangle specified!\n" );
			return e_invalidparameters;
		}

		DestRect = *i_pDestRect;
	}
	else
	{
		DestRect.iLeft = 0; DestRect.iTop = 0;
		DestRect.iRight = i_pDestSurface->iGetWidth(); DestRect.iBottom = i_pDestSurface->iGetHeight();
	}

	float32 *pDestData = 0;
	result resLock = i_pDestSurface->LockRect( (void **)&pDestData, i_pDestRect );
	if( FUNC_FAILED( resLock ) )
	{
		FUNC_FAILING( "CMuli3DSurface::CopyToSurface: couldn't lock destination surface!\n" );
		return resLock;
	}

	const uint32 iDestFloats = i_pDestSurface->iGetFormatFloats();
	const uint32 iDestWidth = DestRect.iRight - DestRect.iLeft;
	const uint32 iDestHeight = DestRect.iBottom - DestRect.iTop;

	// direct copy possible?
	if( !i_pSrcRect && !i_pDestRect && iDestFloats == iGetFormatFloats() &&
		iDestWidth == m_iWidth && iDestHeight == m_iHeight )
	{
		memcpy( pDestData, m_pData, sizeof( float32 ) * iDestFloats * iDestWidth * iDestHeight );
		i_pDestSurface->UnlockRect();
		return s_ok;
	}
	
	const float32 fStepU = 1.0f / m_iWidthMin1;
	const float32 fStepV = 1.0f / m_iHeightMin1;

	float32 fSrcV = SrcRect.iTop * fStepV;
	for( uint32 y = 0; y < iDestHeight; ++y, fSrcV += fStepV )
	{
		float32 fSrcU = SrcRect.iLeft * fStepU;
		for( uint32 x = 0; x < iDestWidth; ++x, fSrcU += fStepU, pDestData += iDestFloats )
		{
			vector4 vSrcColor;
			if( i_Filter == m3dtf_linear )
				SampleLinear( vSrcColor, fSrcU, fSrcV );
			else
				SamplePoint( vSrcColor, fSrcU, fSrcV );
			
			switch( iDestFloats )
			{
			case 4: pDestData[3] = vSrcColor.a;
			case 3: pDestData[2] = vSrcColor.b;
			case 2: pDestData[1] = vSrcColor.g;
			case 1: pDestData[0] = vSrcColor.r;
			}
		}
	}

	i_pDestSurface->UnlockRect();

	return s_ok;
}
