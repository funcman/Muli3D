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

#include "../../include/core/m3dcore_volume.h"
#include "../../include/core/m3dcore_device.h"

CMuli3DVolume::CMuli3DVolume( CMuli3DDevice *i_pParent ) :
	m_pParent( i_pParent ), m_iWidth( 0 ), m_iHeight( 0 ), m_iDepth( 0 ),
	m_iWidthMin1( 0 ), m_iHeightMin1( 0 ), m_iDepthMin1( 0 ), 
	m_bLockedComplete( false ), m_pPartialLockData( 0 ), m_pData( 0 )
{}

CMuli3DVolume::~CMuli3DVolume()
{
	SAFE_DELETE_ARRAY( m_pPartialLockData ); // somebody might have forgotten to unlock the volume ;)
	SAFE_DELETE_ARRAY( m_pData );
}

result CMuli3DVolume::Create( uint32 i_iWidth, uint32 i_iHeight, uint32 i_iDepth, m3dformat i_fmtFormat )
{
	if( !i_iWidth || !i_iHeight || !i_iDepth )
	{
		FUNC_FAILING( "CMuli3DVolume::Create: volume dimensions are invalid.\n" );
		return e_invalidparameters;
	}
	
	uint32 iFloats;
	switch( i_fmtFormat )
	{
	case m3dfmt_r32f: iFloats = 1; break;
	case m3dfmt_r32g32f: iFloats = 2; break;
	case m3dfmt_r32g32b32f: iFloats = 3; break;
	case m3dfmt_r32g32b32a32f: iFloats = 4; break;
	default: FUNC_FAILING( "CMuli3DVolume::Create: invalid format specified.\n" ); return e_invalidformat;
	}

	m_fmtFormat = i_fmtFormat;
	m_iWidth = i_iWidth;
	m_iHeight = i_iHeight;
	m_iDepth = i_iDepth;
	m_iWidthMin1 = m_iWidth - 1;
	m_iHeightMin1 = m_iHeight - 1;
	m_iDepthMin1 = m_iDepth - 1;

	m_pData = new float32[m_iWidth * m_iHeight * m_iDepth * iFloats];
	if( !m_pData )
	{
		FUNC_FAILING( "CMuli3DVolume::Create: out of memory, cannot create volume.\n" );
		return e_outofmemory;
	}

	return s_ok;
}

result CMuli3DVolume::Clear( const vector4 &i_vColor, const m3dbox *i_pBox )
{
	m3dbox ClearBox;
	if( i_pBox )
	{
		if( i_pBox->iRight > m_iWidth ||
			i_pBox->iBottom > m_iHeight ||
			i_pBox->iBack > m_iDepth )
		{
			FUNC_FAILING( "CMuli3DVolume::Clear: clear-box exceeds volume's dimensions.\n" );
			return e_invalidparameters;
		}

		if( i_pBox->iLeft >= i_pBox->iRight ||
			i_pBox->iTop >= i_pBox->iBottom ||
			i_pBox->iFront >= i_pBox->iBack )
		{
			FUNC_FAILING( "CMuli3DVolume::Clear: invalid box specified!\n" );
			return e_invalidparameters;
		}

		ClearBox = *i_pBox;
	}
	else
	{
		ClearBox.iLeft = 0; ClearBox.iTop = 0; ClearBox.iFront= 0;
		ClearBox.iRight = m_iWidth; ClearBox.iBottom = m_iHeight; ClearBox.iBack = m_iDepth;
	}

	float32 *pData;
	result resPointer = LockBox( (void **)&pData, 0 );
	if( FUNC_FAILED( resPointer ) )
		return resPointer;

	switch( m_fmtFormat )
	{
	case m3dfmt_r32f:
		{
			for( uint32 iZ = ClearBox.iFront; iZ < ClearBox.iBack; ++iZ )
			{
				float32 *pCurData2 = &pData[iZ * m_iWidth * m_iHeight];
				for( uint32 iY = ClearBox.iTop; iY < ClearBox.iBottom; ++iY )
				{
					float32 *pCurData = &pCurData2[iY * m_iWidth + ClearBox.iLeft];
					for( uint32 iX = ClearBox.iLeft; iX < ClearBox.iRight; ++iX, ++pCurData )
						*pCurData = i_vColor.r;
				}
			}
		}
		break;

	case m3dfmt_r32g32f:
		{
			for( uint32 iZ = ClearBox.iFront; iZ < ClearBox.iBack; ++iZ )
			{
				vector2 *pCurData2 = &((vector2 *)pData)[iZ * m_iWidth * m_iHeight];
				for( uint32 iY = ClearBox.iTop; iY < ClearBox.iBottom; ++iY )
				{
					vector2 *pCurData = &pCurData2[iY * m_iWidth + ClearBox.iLeft];
					for( uint32 iX = ClearBox.iLeft; iX < ClearBox.iRight; ++iX, ++pCurData )
						*pCurData = i_vColor;
				}
			}
		}
		break;

	case m3dfmt_r32g32b32f:
		{
			for( uint32 iZ = ClearBox.iFront; iZ < ClearBox.iBack; ++iZ )
			{
				vector3 *pCurData2 = &((vector3 *)pData)[iZ * m_iWidth * m_iHeight];
				for( uint32 iY = ClearBox.iTop; iY < ClearBox.iBottom; ++iY )
				{
					vector3 *pCurData = &pCurData2[iY * m_iWidth + ClearBox.iLeft];
					for( uint32 iX = ClearBox.iLeft; iX < ClearBox.iRight; ++iX, ++pCurData )
						*pCurData = i_vColor;
				}
			}
		}
		break;

	case m3dfmt_r32g32b32a32f:
		{
			for( uint32 iZ = ClearBox.iFront; iZ < ClearBox.iBack; ++iZ )
			{
				vector4 *pCurData2 = &((vector4 *)pData)[iZ * m_iWidth * m_iHeight];
				for( uint32 iY = ClearBox.iTop; iY < ClearBox.iBottom; ++iY )
				{
					vector4 *pCurData = &pCurData2[iY * m_iWidth + ClearBox.iLeft];
					for( uint32 iX = ClearBox.iLeft; iX < ClearBox.iRight; ++iX, ++pCurData )
						*pCurData = i_vColor;
				}
			}
		}
		break;

	default: // cannot happen
		FUNC_FAILING( "CMuli3DVolume::Clear: invalid volume format.\n" );
		UnlockBox();
		return e_invalidformat;
	}

	UnlockBox();
	return s_ok;
}

result CMuli3DVolume::LockBox( void **o_ppData, const m3dbox *i_pBox )
{
	if( !o_ppData )
	{
		FUNC_FAILING( "CMuli3DVolume::LockBox: o_ppData points to null.\n" );
		return e_invalidparameters;
	}

	if( m_bLockedComplete || m_pPartialLockData )
	{
		FUNC_FAILING( "CMuli3DVolume::LockBox: mip-level is already locked!\n" );
		return e_invalidstate;
	}

	if( !i_pBox )
	{
		*o_ppData = m_pData;
		m_bLockedComplete = true;
		return s_ok;
	}

	if( i_pBox->iRight > m_iWidth ||
		i_pBox->iBottom > m_iHeight ||
		i_pBox->iBack > m_iDepth )
	{
		FUNC_FAILING( "CMuli3DVolume::LockBox: box exceeds volume dimensions!\n" );
		return e_invalidparameters;
	}

	if( i_pBox->iLeft >= i_pBox->iRight ||
		i_pBox->iTop >= i_pBox->iBottom ||
		i_pBox->iFront >= i_pBox->iBack )
	{
		FUNC_FAILING( "CMuli3DVolume::LockBox: invalid box specified!\n" );
		return e_invalidparameters;
	}

	m_PartialLockBox = *i_pBox;
	
	// create lock-buffer
	const uint32 iLockWidth = m_PartialLockBox.iRight - m_PartialLockBox.iLeft;
	const uint32 iLockHeight = m_PartialLockBox.iBottom - m_PartialLockBox.iTop;
	const uint32 iLockDepth = m_PartialLockBox.iBack - m_PartialLockBox.iFront;
	const uint32 iVolumeFloats = iGetFormatFloats();

	m_pPartialLockData = new float32[iLockWidth * iLockHeight * iLockDepth * iVolumeFloats];
	if( !m_pPartialLockData )
	{
		FUNC_FAILING( "CMuli3DVolume::LockBox: memory allocation failed!\n" );
		return e_outofmemory;
	}
	
	float32 *pCurLockData = m_pPartialLockData;
	for( uint32 iZ = m_PartialLockBox.iFront; iZ < m_PartialLockBox.iBack; ++iZ )
	{
		const float32 *pCurVolumeData2 = &m_pData[(iZ * m_iWidth * m_iHeight) * iVolumeFloats];
		for( uint32 iY = m_PartialLockBox.iTop; iY < m_PartialLockBox.iBottom; ++iY )
		{
			const float32 *pCurVolumeData = &pCurVolumeData2[(iY * m_iWidth + m_PartialLockBox.iLeft) * iVolumeFloats];
			memcpy( pCurLockData, pCurVolumeData, sizeof( float32 ) * iVolumeFloats * iLockWidth );
			pCurLockData += iVolumeFloats * iLockWidth;
		}
	}

	*o_ppData = m_pPartialLockData;

	return s_ok;
}

result CMuli3DVolume::UnlockBox()
{
	if( !m_bLockedComplete && !m_pPartialLockData )
	{
		FUNC_FAILING( "CMuli3DVolume::UnlockBox: cannot unlock mip-level because it isn't locked!\n" );
		return e_invalidstate;
	}

	if( m_bLockedComplete )
	{
		m_bLockedComplete = false;
		return s_ok;
	}

	// update volume
	const uint32 iLockWidth = m_PartialLockBox.iRight - m_PartialLockBox.iLeft;
	const uint32 iVolumeFloats = iGetFormatFloats();

	const float32 *pCurLockData = m_pPartialLockData;
	for( uint32 iZ = m_PartialLockBox.iFront; iZ < m_PartialLockBox.iBack; ++iZ )
	{
		float32 *pCurVolumeData2 = &m_pData[(iZ * m_iWidth * m_iHeight) * iVolumeFloats];
		for( uint32 iY = m_PartialLockBox.iTop; iY < m_PartialLockBox.iBottom; ++iY )
		{
			float32 *pCurVolumeData = &pCurVolumeData2[(iY * m_iWidth + m_PartialLockBox.iLeft) * iVolumeFloats];
			memcpy( pCurVolumeData, pCurLockData, sizeof( float32 ) * iVolumeFloats * iLockWidth );
			pCurLockData += iVolumeFloats * iLockWidth;
		}
	}

	SAFE_DELETE_ARRAY( m_pPartialLockData );

	return s_ok;
}

uint32 CMuli3DVolume::iGetFormatFloats()
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

void CMuli3DVolume::SamplePoint( vector4 &o_vColor, float32 i_fU, float32 i_fV, float32 i_fW )
{
	const float32 fX = i_fU * m_iWidthMin1, fY = i_fV * m_iHeightMin1, fZ = i_fW * m_iDepthMin1;
	const uint32 iPixelX = ftol( fX ), iPixelY = ftol( fY ), iPixelZ = ftol( fZ );

	switch( m_fmtFormat )
	{
	case m3dfmt_r32f:
		{
			const float32 *pPixel = &m_pData[iPixelZ * m_iWidth * m_iHeight + iPixelY * m_iWidth + iPixelX];
			o_vColor = vector4( pPixel[0], 0, 0, 1 );
		}
		break;
	case m3dfmt_r32g32f:
		{
			const vector2 *pPixel = (const vector2 *)&m_pData[2 * (iPixelZ * m_iWidth * m_iHeight + iPixelY * m_iWidth + iPixelX)];
			o_vColor = vector4( pPixel->x, pPixel->y, 0, 1 );
		}
		break;
	case m3dfmt_r32g32b32f:
		{
			const vector3 *pPixel = (const vector3 *)&m_pData[3 * (iPixelZ * m_iWidth * m_iHeight + iPixelY * m_iWidth + iPixelX)];
			o_vColor = vector4( pPixel->x, pPixel->y, pPixel->z, 1 );
		}
		break;
	case m3dfmt_r32g32b32a32f:
		{
			const vector4 *pPixel = (const vector4 *)&m_pData[4 * (iPixelZ * m_iWidth * m_iHeight + iPixelY * m_iWidth + iPixelX)];
			o_vColor = *pPixel;
		}
		break;
	default: // cannot happen
		break;
	}
}

void CMuli3DVolume::SampleLinear( vector4 &o_vColor, float32 i_fU, float32 i_fV, float32 i_fW )
{
	const float32 fX = i_fU * m_iWidthMin1, fY = i_fV * m_iHeightMin1, fZ = i_fW * m_iDepthMin1;
	const uint32 iPixelX = ftol( fX ), iPixelY = ftol( fY ), iPixelZ = ftol( fZ );

	uint32 iPixelX2 = iPixelX + 1, iPixelY2 = iPixelY + 1, iPixelZ2 = iPixelZ + 1;
	if( iPixelX2 >= m_iWidth ) iPixelX2 = m_iWidthMin1;
	if( iPixelY2 >= m_iHeight ) iPixelY2 = m_iHeightMin1;
	if( iPixelZ2 >= m_iDepth ) iPixelZ2 = m_iDepthMin1;

	const uint32 iIndexRows[2] = { iPixelY * m_iWidth, iPixelY2 * m_iWidth };
	const uint32 iIndexSlices[2] = { iPixelZ * m_iWidth * m_iHeight, iPixelZ2 * m_iWidth * m_iHeight };
	const float32 fInterpolation[3] = { fX - iPixelX, fY - iPixelY, fZ - iPixelZ };

	switch( m_fmtFormat )
	{
	case m3dfmt_r32f:
		{
			float32 fColorSlices[2], fColorRows[2];

			fColorRows[0] = fLerp( m_pData[iIndexSlices[0] + iIndexRows[0] + iPixelX], m_pData[iIndexSlices[0] + iIndexRows[0] + iPixelX2], fInterpolation[0] );
			fColorRows[1] = fLerp( m_pData[iIndexSlices[0] + iIndexRows[1] + iPixelX], m_pData[iIndexSlices[0] + iIndexRows[1] + iPixelX2], fInterpolation[0] );
			fColorSlices[0] = fLerp( fColorRows[0], fColorRows[1], fInterpolation[1] );

			fColorRows[0] = fLerp( m_pData[iIndexSlices[1] + iIndexRows[0] + iPixelX], m_pData[iIndexSlices[1] + iIndexRows[0] + iPixelX2], fInterpolation[0] );
			fColorRows[1] = fLerp( m_pData[iIndexSlices[1] + iIndexRows[1] + iPixelX], m_pData[iIndexSlices[1] + iIndexRows[1] + iPixelX2], fInterpolation[0] );
			fColorSlices[1] = fLerp( fColorRows[1], fColorRows[1], fInterpolation[1] );

			const float32 fFinalColor = fLerp( fColorSlices[0], fColorSlices[1], fInterpolation[2] );
			
			o_vColor = vector4( fFinalColor, 0, 0, 1 );
		}
		break;
	case m3dfmt_r32g32f:
		{
			const vector2 *pPixelData = (const vector2 *)m_pData;

			static vector2 vColorSlices[2], vColorRows[2];

			vVector2Lerp( vColorRows[0], pPixelData[iIndexSlices[0] + iIndexRows[0] + iPixelX], pPixelData[iIndexSlices[0] + iIndexRows[0] + iPixelX2], fInterpolation[0] );
			vVector2Lerp( vColorRows[1], pPixelData[iIndexSlices[0] + iIndexRows[1] + iPixelX], pPixelData[iIndexSlices[0] + iIndexRows[1] + iPixelX2], fInterpolation[0] );
			vVector2Lerp( vColorSlices[0], vColorRows[0], vColorRows[1], fInterpolation[1] );
			
			vVector2Lerp( vColorRows[0], pPixelData[iIndexSlices[1] + iIndexRows[0] + iPixelX], pPixelData[iIndexSlices[1] + iIndexRows[0] + iPixelX2], fInterpolation[0] );
			vVector2Lerp( vColorRows[1], pPixelData[iIndexSlices[1] + iIndexRows[1] + iPixelX], pPixelData[iIndexSlices[1] + iIndexRows[1] + iPixelX2], fInterpolation[0] );
			vVector2Lerp( vColorSlices[1], vColorRows[0], vColorRows[1], fInterpolation[1] );

			static vector2 vFinalColor; vVector2Lerp( vFinalColor, vColorSlices[0], vColorSlices[1], fInterpolation[2] );

			o_vColor = vector4( vFinalColor.x, vFinalColor.y, 0, 1 );
		}
		break;
	case m3dfmt_r32g32b32f:
		{
			const vector3 *pPixelData = (const vector3 *)m_pData;

			static vector3 vColorSlices[2], vColorRows[2];

			vVector3Lerp( vColorRows[0], pPixelData[iIndexSlices[0] + iIndexRows[0] + iPixelX], pPixelData[iIndexSlices[0] + iIndexRows[0] + iPixelX2], fInterpolation[0] );
			vVector3Lerp( vColorRows[1], pPixelData[iIndexSlices[0] + iIndexRows[1] + iPixelX], pPixelData[iIndexSlices[0] + iIndexRows[1] + iPixelX2], fInterpolation[0] );
			vVector3Lerp( vColorSlices[0], vColorRows[0], vColorRows[1], fInterpolation[1] );
			
			vVector3Lerp( vColorRows[0], pPixelData[iIndexSlices[1] + iIndexRows[0] + iPixelX], pPixelData[iIndexSlices[1] + iIndexRows[0] + iPixelX2], fInterpolation[0] );
			vVector3Lerp( vColorRows[1], pPixelData[iIndexSlices[1] + iIndexRows[1] + iPixelX], pPixelData[iIndexSlices[1] + iIndexRows[1] + iPixelX2], fInterpolation[0] );
			vVector3Lerp( vColorSlices[1], vColorRows[0], vColorRows[1], fInterpolation[1] );

			static vector3 vFinalColor; vVector3Lerp( vFinalColor, vColorSlices[0], vColorSlices[1], fInterpolation[2] );

			o_vColor = vector4( vFinalColor.x, vFinalColor.y, vFinalColor.z, 1 );
		}
		break;
	case m3dfmt_r32g32b32a32f:
		{
			const vector4 *pPixelData = (const vector4 *)m_pData;

			static vector4 vColorSlices[2], vColorRows[2];

			vVector4Lerp( vColorRows[0], pPixelData[iIndexSlices[0] + iIndexRows[0] + iPixelX], pPixelData[iIndexSlices[0] + iIndexRows[0] + iPixelX2], fInterpolation[0] );
			vVector4Lerp( vColorRows[1], pPixelData[iIndexSlices[0] + iIndexRows[1] + iPixelX], pPixelData[iIndexSlices[0] + iIndexRows[1] + iPixelX2], fInterpolation[0] );
			vVector4Lerp( vColorSlices[0], vColorRows[0], vColorRows[1], fInterpolation[1] );
			
			vVector4Lerp( vColorRows[0], pPixelData[iIndexSlices[1] + iIndexRows[0] + iPixelX], pPixelData[iIndexSlices[1] + iIndexRows[0] + iPixelX2], fInterpolation[0] );
			vVector4Lerp( vColorRows[1], pPixelData[iIndexSlices[1] + iIndexRows[1] + iPixelX], pPixelData[iIndexSlices[1] + iIndexRows[1] + iPixelX2], fInterpolation[0] );
			vVector4Lerp( vColorSlices[1], vColorRows[0], vColorRows[1], fInterpolation[1] );

			vVector4Lerp( o_vColor, vColorSlices[0], vColorSlices[1], fInterpolation[2] );
		}
		break;
	default: // cannot happen
		break;
	}
}

m3dformat CMuli3DVolume::fmtGetFormat()
{
	return m_fmtFormat;
}

uint32 CMuli3DVolume::iGetWidth()
{
	return m_iWidth;
}

uint32 CMuli3DVolume::iGetHeight()
{
	return m_iHeight;
}

uint32 CMuli3DVolume::iGetDepth()
{
	return m_iDepth;
}

result CMuli3DVolume::CopyToVolume( const m3dbox *i_pSrcBox, CMuli3DVolume *i_pDestVolume, const m3dbox *i_pDestBox, m3dtexturefilter i_Filter )
{
	if( !i_pDestVolume )
	{
		FUNC_FAILING( "CMuli3DVolume::CopyToSurface: no destination volume specified!\n" );
		return e_invalidparameters;
	}

	if( i_Filter != m3dtf_point || i_Filter != m3dtf_linear )
	{
		FUNC_FAILING( "CMuli3DVolume::CopyToSurface: invalid filter specified!\n" );
		return e_invalidparameters;
	}

	m3dbox SrcBox;
	if( i_pSrcBox )
	{
		if( i_pSrcBox->iRight > m_iWidth ||
			i_pSrcBox->iBottom > m_iHeight ||
			i_pSrcBox->iBack > m_iDepth )
		{
			FUNC_FAILING( "CMuli3DVolume::CopyToSurface: source box exceeds volume dimensions!\n" );
			return e_invalidparameters;
		}

		if( i_pSrcBox->iLeft >= i_pSrcBox->iRight ||
			i_pSrcBox->iTop >= i_pSrcBox->iBottom ||
			i_pSrcBox->iFront >= i_pSrcBox->iBack )
		{
			FUNC_FAILING( "CMuli3DVolume::CopyToSurface: invalid source box specified!\n" );
			return e_invalidparameters;
		}

		SrcBox = *i_pSrcBox;
	}
	else
	{
		SrcBox.iLeft = 0; SrcBox.iTop = 0; SrcBox.iFront = 0;
		SrcBox.iRight = m_iWidth; SrcBox.iBottom = m_iHeight; SrcBox.iBack = m_iDepth;
	}

	m3dbox DestBox;
	if( i_pDestBox )
	{
		if( i_pDestBox->iRight > i_pDestVolume->iGetWidth() ||
			i_pDestBox->iBottom > i_pDestVolume->iGetHeight() ||
			i_pDestBox->iBack > i_pDestVolume->iGetDepth() )
		{
			FUNC_FAILING( "CMuli3DVolume::CopyToSurface: destination box exceeds volume dimensions!\n" );
			return e_invalidparameters;
		}

		if( i_pDestBox->iLeft >= i_pDestBox->iRight ||
			i_pDestBox->iTop >= i_pDestBox->iBottom ||
			i_pDestBox->iFront >= i_pDestBox->iBack )
		{
			FUNC_FAILING( "CMuli3DVolume::CopyToSurface: invalid destination box specified!\n" );
			return e_invalidparameters;
		}

		DestBox = *i_pDestBox;
	}
	else
	{
		DestBox.iLeft = 0; DestBox.iTop = 0; DestBox.iFront = 0;
		DestBox.iRight = i_pDestVolume->iGetWidth(); DestBox.iBottom = i_pDestVolume->iGetHeight(); DestBox.iBack = i_pDestVolume->iGetDepth();
	}

	float32 *pDestData = 0;
	result resLock = i_pDestVolume->LockBox( (void **)&pDestData, i_pDestBox );
	if( FUNC_FAILED( resLock ) )
	{
		FUNC_FAILING( "CMuli3DVolume::CopyToSurface: couldn't lock destination volume!\n" );
		return resLock;
	}

	const uint32 iDestFloats = i_pDestVolume->iGetFormatFloats();
	const uint32 iDestWidth = DestBox.iRight - DestBox.iLeft;
	const uint32 iDestHeight = DestBox.iBottom - DestBox.iTop;
	const uint32 iDestDepth = DestBox.iBack - DestBox.iFront;
	
	// direct copy possible?
	if( !i_pSrcBox && !i_pDestBox && iDestFloats == iGetFormatFloats() &&
		iDestWidth == m_iWidth && iDestHeight == m_iHeight && iDestDepth == m_iDepth )
	{
		memcpy( pDestData, m_pData, sizeof( float32 ) * iDestFloats * iDestWidth * iDestHeight * iDestDepth );
		i_pDestVolume->UnlockBox();
		return s_ok;
	}

	const float32 fStepU = 1.0f / m_iWidthMin1;
	const float32 fStepV = 1.0f / m_iHeightMin1;
	const float32 fStepW = 1.0f / m_iDepthMin1;

	float32 fSrcW = SrcBox.iFront * fStepW;
	for( uint32 z = 0; z < iDestDepth; ++z, fSrcW += fStepW )
	{
		float32 fSrcV = SrcBox.iTop * fStepV;
		for( uint32 y = 0; y < iDestHeight; ++y, fSrcV += fStepV )
		{
			float32 fSrcU = SrcBox.iLeft * fStepU;
			for( uint32 x = 0; x < iDestWidth; ++x, fSrcU += fStepU, pDestData += iDestFloats )
			{
				vector4 vSrcColor;
				if( i_Filter == m3dtf_linear )
					SampleLinear( vSrcColor, fSrcU, fSrcV, fSrcW );
				else
					SamplePoint( vSrcColor, fSrcU, fSrcV, fSrcW );
				
				switch( iDestFloats )
				{
				case 4: pDestData[3] = vSrcColor.a;
				case 3: pDestData[2] = vSrcColor.b;
				case 2: pDestData[1] = vSrcColor.g;
				case 1: pDestData[0] = vSrcColor.r;
				}
			}
		}
	}

	i_pDestVolume->UnlockBox();

	return s_ok;
}
