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

#include "../../include/core/m3dcore_texture.h"
#include "../../include/core/m3dcore_device.h"
#include "../../include/core/m3dcore_surface.h"

CMuli3DTexture::CMuli3DTexture( CMuli3DDevice *i_pParent )
	: IMuli3DBaseTexture( i_pParent ),
	m_iMipLevels( 0 ), m_ppMipLevels( 0 )
{
	
}

CMuli3DTexture::~CMuli3DTexture()
{
	for( uint32 iLevel = 0; iLevel < m_iMipLevels; ++iLevel )
		SAFE_RELEASE( m_ppMipLevels[iLevel] );
	SAFE_DELETE_ARRAY( m_ppMipLevels );
}

result CMuli3DTexture::Create( uint32 i_iWidth, uint32 i_iHeight, uint32 i_iMipLevels, m3dformat i_fmtFormat )
{
	if( !i_iWidth || !i_iHeight )
	{
		FUNC_FAILING( "CMuli3DTexture::Create: texture dimensions are invalid.\n" );
		return e_invalidparameters;
	}
	
	if( i_fmtFormat < m3dfmt_r32f || i_fmtFormat > m3dfmt_r32g32b32a32f )
	{
		FUNC_FAILING( "CMuli3DTexture::Create: invalid format specified.\n" );
		return e_invalidformat;
	}

	m_fSquaredWidth = (float32)(i_iWidth * i_iWidth);
	m_fSquaredHeight = (float32)(i_iHeight * i_iHeight);

	if( !i_iMipLevels )
	{
		// Create a full chain ...
		uint32 iWidth = i_iWidth, iHeight = i_iHeight;
		do
		{
			++i_iMipLevels;
			iWidth >>= 1; iHeight >>= 1;
		}
		while( iWidth && iHeight );
	}

	m_ppMipLevels = new CMuli3DSurface *[i_iMipLevels];
	if( !m_ppMipLevels )
	{
		FUNC_FAILING( "CMuli3DTexture::Create: out of memory, cannot create mip-levels.\n" );
		return e_outofmemory;
	}

	memset( m_ppMipLevels, 0, sizeof( CMuli3DSurface * ) * i_iMipLevels );

	CMuli3DSurface **pCurMipLevel = m_ppMipLevels;
	do
	{
		result resMipLevel = m_pParent->CreateSurface( pCurMipLevel, i_iWidth, i_iHeight, i_fmtFormat );
		if( FUNC_FAILED( resMipLevel ) )
		{
			// destructor will perform cleanup
			FUNC_FAILING( "CMuli3DTexture::Create: creation of mip-level failed.\n" );
			return resMipLevel;
		}

		++m_iMipLevels; ++pCurMipLevel;

		if( --i_iMipLevels == 0 )
			break;

		i_iWidth >>= 1; i_iHeight >>= 1;
	}
	while( i_iWidth && i_iHeight );

	return s_ok;
}

m3dtexsampleinput CMuli3DTexture::eGetTexSampleInput()
{
	return m3dtsi_2coords;
}

result CMuli3DTexture::Clear( uint32 i_iMipLevel, const vector4 &i_vColor, const m3drect *i_pRect )
{
	if( i_iMipLevel >= m_iMipLevels )
	{
		FUNC_FAILING( "CMuli3DTexture::Clear: invalid mip-level specified.\n" );
		return e_invalidparameters;
	}

	return m_ppMipLevels[i_iMipLevel]->Clear( i_vColor, i_pRect );
}

result CMuli3DTexture::GenerateMipSubLevels( uint32 i_iSrcLevel )
{
	if( i_iSrcLevel + 1 >= m_iMipLevels )
	{
		FUNC_FAILING( "CMuli3DTexture::GenerateMipSubLevels: i_iSrcLevel refers either to last mip-level or is larger than the number of mip-levels.\n" );
		return e_invalidparameters;
	}

	for( uint32 iLevel = i_iSrcLevel + 1; iLevel < m_iMipLevels; ++iLevel )
	{
		const float32 *pSrcData = 0;
		result resLock = LockRect( iLevel - 1, (void **)&pSrcData, 0 );
		if( FUNC_FAILED( resLock ) )
			return resLock;
		
		float32 *pDestData = 0;
		resLock = LockRect( iLevel, (void **)&pDestData, 0 );
		if( FUNC_FAILED( resLock ) )
		{
			UnlockRect( iLevel );
			return resLock;
		}

		const uint32 iSrcHeight = iGetHeight( iLevel - 1 );
		const uint32 iSrcWidth = iGetWidth( iLevel - 1 );

		switch( fmtGetFormat() )
		{
		case m3dfmt_r32f:
			{
				for( uint32 iY = 0; iY < iSrcHeight; iY += 2 )
				{
					const uint32 iIndexRows[2] = { iY * iSrcWidth, ( iY + 1 ) * iSrcWidth };
					for( uint32 iX = 0; iX < iSrcWidth; iX += 2, ++pDestData )
					{
						const float32 fSrcPixels[4] = 
						{
							pSrcData[iIndexRows[0] + iX],
							pSrcData[iIndexRows[0] + iX + 1],
							pSrcData[iIndexRows[1] + iX],
							pSrcData[iIndexRows[1] + iX + 1]
						};
						*pDestData = ( fSrcPixels[0] + fSrcPixels[1] + fSrcPixels[2] + fSrcPixels[3] ) * 0.25f;
					}
				}
			}
			break;

		case m3dfmt_r32g32f:
			{
				for( uint32 iY = 0; iY < iSrcHeight; iY += 2 )
				{
					const uint32 iIndexRows[2] = { iY * iSrcWidth, ( iY + 1 ) * iSrcWidth };
					for( uint32 iX = 0; iX < iSrcWidth; iX += 2, pDestData += 2 )
					{
						const vector2 *pSrcPixels[4] = 
						{
							&((vector2 *)pSrcData)[iIndexRows[0] + iX],
							&((vector2 *)pSrcData)[iIndexRows[0] + iX + 1],
							&((vector2 *)pSrcData)[iIndexRows[1] + iX],
							&((vector2 *)pSrcData)[iIndexRows[1] + iX + 1]
						};
						*(vector2 *)pDestData = ( *pSrcPixels[0] + *pSrcPixels[1] + *pSrcPixels[2] + *pSrcPixels[3] ) * 0.25f;
					}
				}
			}
			break;

		case m3dfmt_r32g32b32f:
			{
				for( uint32 iY = 0; iY < iSrcHeight; iY += 2 )
				{
					const uint32 iIndexRows[2] = { iY * iSrcWidth, ( iY + 1 ) * iSrcWidth };
					for( uint32 iX = 0; iX < iSrcWidth; iX += 2, pDestData += 3 )
					{
						const vector3 *pSrcPixels[4] = 
						{
							&((vector3 *)pSrcData)[iIndexRows[0] + iX],
							&((vector3 *)pSrcData)[iIndexRows[0] + iX + 1],
							&((vector3 *)pSrcData)[iIndexRows[1] + iX],
							&((vector3 *)pSrcData)[iIndexRows[1] + iX + 1]
						};
						*(vector3 *)pDestData = ( *pSrcPixels[0] + *pSrcPixels[1] + *pSrcPixels[2] + *pSrcPixels[3] ) * 0.25f;
					}
				}
			}
			break;

		case m3dfmt_r32g32b32a32f:
			{
				for( uint32 iY = 0; iY < iSrcHeight; iY += 2 )
				{
					const uint32 iIndexRows[2] = { iY * iSrcWidth, ( iY + 1 ) * iSrcWidth };
					for( uint32 iX = 0; iX < iSrcWidth; iX += 2, pDestData += 4 )
					{
						const vector4 *pSrcPixels[4] = 
						{
							&((vector4 *)pSrcData)[iIndexRows[0] + iX],
							&((vector4 *)pSrcData)[iIndexRows[0] + iX + 1],
							&((vector4 *)pSrcData)[iIndexRows[1] + iX],
							&((vector4 *)pSrcData)[iIndexRows[1] + iX + 1]
						};
						*(vector4 *)pDestData = ( *pSrcPixels[0] + *pSrcPixels[1] + *pSrcPixels[2] + *pSrcPixels[3] ) * 0.25f;
					}
				}
			}
			break;

		default: // cannot happen
			break;
		}

		UnlockRect( iLevel );
		UnlockRect( iLevel - 1 );
	}

	return s_ok;
}

result CMuli3DTexture::LockRect( uint32 i_iMipLevel, void **o_ppData, const m3drect *i_pRect )
{
	if( i_iMipLevel >= m_iMipLevels )
	{
		FUNC_FAILING( "CMuli3DTexture::LockRect: invalid mip-level specified.\n" );
		return e_invalidparameters;
	}

	return m_ppMipLevels[i_iMipLevel]->LockRect( o_ppData, i_pRect );
}

result CMuli3DTexture::UnlockRect( uint32 i_iMipLevel )
{
	if( i_iMipLevel >= m_iMipLevels )
	{
		FUNC_FAILING( "CMuli3DTexture::UnlockRect: invalid mip-level specified.\n" );
		return e_invalidparameters;
	}

	return m_ppMipLevels[i_iMipLevel]->UnlockRect();
}

CMuli3DSurface *CMuli3DTexture::pGetMipLevel( uint32 i_iMipLevel )
{
	if( i_iMipLevel >= m_iMipLevels )
	{
		FUNC_FAILING( "CMuli3DTexture::pGetMipLevel: invalid mip-level specified.\n" );
		return 0;
	}

	m_ppMipLevels[i_iMipLevel]->AddRef();

	return m_ppMipLevels[i_iMipLevel];
}

result CMuli3DTexture::SampleTexture( vector4 &o_vColor, float32 i_fU, float32 i_fV, float32 i_fW, const vector4 *i_pXGradient, const vector4 *i_pYGradient, const uint32 *i_pSamplerStates )
{
	uint32 iTexFilter = i_pSamplerStates[m3dtss_minfilter];
	float32 fTexMipLevel = 0.0f;
	
	if( i_pXGradient && i_pYGradient )
	{
		// Compute the mip-level and determine the texture filter type.
		const float32 fLenXGrad = i_pXGradient->x * i_pXGradient->x * m_fSquaredWidth + i_pXGradient->y * i_pXGradient->y * m_fSquaredHeight;
		const float32 fLenYGrad = i_pYGradient->x * i_pYGradient->x * m_fSquaredWidth + i_pYGradient->y * i_pYGradient->y * m_fSquaredHeight;
		const float32 fTexelsPerScreenPixel = sqrtf( fLenXGrad > fLenYGrad ? fLenXGrad : fLenYGrad );

		if( fTexelsPerScreenPixel <= 1.0f )
		{
			 // if fTexelsPerScreenPixel < 1.0f -> magnification, no mipmapping needed
			fTexMipLevel = 0.0f;
			iTexFilter = i_pSamplerStates[m3dtss_magfilter];
		}
		else
		{
			// minification, need mipmapping
			static const float32 fInvLog2 = 1.0f / logf( 2.0f ); // calculate log2
			fTexMipLevel = logf( fTexelsPerScreenPixel ) * fInvLog2;
			iTexFilter = i_pSamplerStates[m3dtss_minfilter];
		}
	}

	const float32 fMipLODBias = *(float32 *)&i_pSamplerStates[m3dtss_miplodbias];
	const float32 fMaxMipLevel = *(float32 *)&i_pSamplerStates[m3dtss_maxmiplevel];
	fTexMipLevel = fClamp( fTexMipLevel + fMipLODBias, 0.0f, fMaxMipLevel );

	if( i_pSamplerStates[m3dtss_mipfilter] == m3dtf_linear )
	{
		uint32 iMipLevelA = ftol( fTexMipLevel ), iMipLevelB = iMipLevelA + 1;
		if( iMipLevelA >= m_iMipLevels ) iMipLevelA = m_iMipLevels - 1;
		if( iMipLevelB >= m_iMipLevels ) iMipLevelB = m_iMipLevels - 1;

		vector4 vColorA, vColorB;
		if( iTexFilter == m3dtf_linear )
		{
			m_ppMipLevels[iMipLevelA]->SampleLinear( vColorA, i_fU, i_fV );
			m_ppMipLevels[iMipLevelB]->SampleLinear( vColorB, i_fU, i_fV );
		}
		else
		{
			m_ppMipLevels[iMipLevelA]->SamplePoint( vColorA, i_fU, i_fV );
			m_ppMipLevels[iMipLevelB]->SamplePoint( vColorB, i_fU, i_fV );
		}

		const float32 fInterpolation = fTexMipLevel - iMipLevelA; // TODO: not accurate
		vVector4Lerp( o_vColor, vColorA, vColorB, fInterpolation );
	}
	else
	{
		uint32 iMipLevel = ftol( fTexMipLevel );
		if( iMipLevel >= m_iMipLevels ) iMipLevel = m_iMipLevels - 1;

		if( iTexFilter == m3dtf_linear )
			m_ppMipLevels[iMipLevel]->SampleLinear( o_vColor, i_fU, i_fV );
		else
			m_ppMipLevels[iMipLevel]->SamplePoint( o_vColor, i_fU, i_fV );
	}

	return s_ok;
}

m3dformat CMuli3DTexture::fmtGetFormat()
{
	return m_ppMipLevels[0]->fmtGetFormat();
}

uint32 CMuli3DTexture::iGetFormatFloats()
{
	return m_ppMipLevels[0]->iGetFormatFloats();
}

uint32 CMuli3DTexture::iGetMipLevels()
{
	return m_iMipLevels;
}

uint32 CMuli3DTexture::iGetWidth( uint32 i_iMipLevel )
{
	if( i_iMipLevel >= m_iMipLevels )
	{
		FUNC_FAILING( "CMuli3DTexture::iGetWidth: invalid mip-level specified.\n" );
		return 0;
	}

	return m_ppMipLevels[i_iMipLevel]->iGetWidth();
}

uint32 CMuli3DTexture::iGetHeight( uint32 i_iMipLevel )
{
	if( i_iMipLevel >= m_iMipLevels )
	{
		FUNC_FAILING( "CMuli3DTexture::iGetHeight: invalid mip-level specified.\n" );
		return 0;
	}

	return m_ppMipLevels[i_iMipLevel]->iGetHeight();
}
