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

#include "../../include/core/m3dcore_volumetexture.h"
#include "../../include/core/m3dcore_device.h"
#include "../../include/core/m3dcore_volume.h"

CMuli3DVolumeTexture::CMuli3DVolumeTexture( CMuli3DDevice *i_pParent )
	: IMuli3DBaseTexture( i_pParent ),
	m_iMipLevels( 0 ), m_ppMipLevels( 0 )
{
	
}

CMuli3DVolumeTexture::~CMuli3DVolumeTexture()
{
	for( uint32 iLevel = 0; iLevel < m_iMipLevels; ++iLevel )
		SAFE_RELEASE( m_ppMipLevels[iLevel] );
	SAFE_DELETE_ARRAY( m_ppMipLevels );
}

result CMuli3DVolumeTexture::Create( uint32 i_iWidth, uint32 i_iHeight, uint32 i_iDepth, uint32 i_iMipLevels, m3dformat i_fmtFormat )
{
	if( !i_iWidth || !i_iHeight || !i_iDepth )
	{
		FUNC_FAILING( "CMuli3DVolumeTexture::Create: texture dimensions are invalid.\n" );
		return e_invalidparameters;
	}
	
	if( i_fmtFormat < m3dfmt_r32f || i_fmtFormat > m3dfmt_r32g32b32a32f )
	{
		FUNC_FAILING( "CMuli3DVolumeTexture::Create: invalid format specified.\n" );
		return e_invalidformat;
	}

	m_fSquaredWidth = (float32)(i_iWidth * i_iWidth);
	m_fSquaredHeight = (float32)(i_iHeight * i_iHeight);
	m_fSquaredDepth = (float32)(i_iDepth * i_iDepth);

	if( !i_iMipLevels )
	{
		// Create a full chain ...
		uint32 iWidth = i_iWidth, iHeight = i_iHeight, iDepth = i_iDepth;
		do
		{
			++i_iMipLevels;
			iWidth >>= 1; iHeight >>= 1; iDepth >>= 1;
		}
		while( iWidth && iHeight && iDepth );
	}

	m_ppMipLevels = new CMuli3DVolume *[i_iMipLevels];
	if( !m_ppMipLevels )
	{
		FUNC_FAILING( "CMuli3DVolumeTexture::Create: out of memory, cannot create mip-levels.\n" );
		return e_outofmemory;
	}

	memset( m_ppMipLevels, 0, sizeof( CMuli3DVolume * ) * i_iMipLevels );

	CMuli3DVolume **pCurMipLevel = m_ppMipLevels;
	do
	{
		result resMipLevel = m_pParent->CreateVolume( pCurMipLevel, i_iWidth, i_iHeight, i_iDepth, i_fmtFormat );
		if( FUNC_FAILED( resMipLevel ) )
		{
			// destructor will perform cleanup
			FUNC_FAILING( "CMuli3DVolumeTexture::Create: creation of mip-level failed.\n" );
			return resMipLevel;
		}

		++m_iMipLevels; ++pCurMipLevel;

		if( --i_iMipLevels == 0 )
			break;

		i_iWidth >>= 1; i_iHeight >>= 1; i_iDepth >>= 1;
	}
	while( i_iWidth && i_iHeight && i_iDepth );

	return s_ok;
}

m3dtexsampleinput CMuli3DVolumeTexture::eGetTexSampleInput()
{
	return m3dtsi_3coords;
}

result CMuli3DVolumeTexture::Clear( uint32 i_iMipLevel, const vector4 &i_vColor, const m3dbox *i_pBox )
{
	if( i_iMipLevel >= m_iMipLevels )
	{
		FUNC_FAILING( "CMuli3DVolumeTexture::Clear: invalid mip-level specified.\n" );
		return e_invalidparameters;
	}

	return m_ppMipLevels[i_iMipLevel]->Clear( i_vColor, i_pBox );
}

result CMuli3DVolumeTexture::GenerateMipSubLevels( uint32 i_iSrcLevel )
{
	if( i_iSrcLevel + 1 >= m_iMipLevels )
	{
		FUNC_FAILING( "CMuli3DVolumeTexture::GenerateMipSubLevels: i_iSrcLevel refers either to last mip-level or is larger than the number of mip-levels.\n" );
		return e_invalidparameters;
	}

	for( uint32 iLevel = i_iSrcLevel + 1; iLevel < m_iMipLevels; ++iLevel )
	{
		const float32 *pSrcData = 0;
		result resLock = LockBox( iLevel - 1, (void **)&pSrcData, 0 );
		if( FUNC_FAILED( resLock ) )
			return resLock;
		
		float32 *pDestData = 0;
		resLock = LockBox( iLevel, (void **)&pDestData, 0 );
		if( FUNC_FAILED( resLock ) )
		{
			UnlockBox( iLevel );
			return resLock;
		}

		const uint32 iSrcDepth = iGetDepth( iLevel - 1 );
		const uint32 iSrcHeight = iGetHeight( iLevel - 1 );
		const uint32 iSrcWidth = iGetWidth( iLevel - 1 );

		switch( fmtGetFormat() )
		{
		case m3dfmt_r32f:
			{
				for( uint32 iZ = 0; iZ < iSrcDepth; iZ += 2 )
				{
					const uint32 iIndexSlices[2] = { iZ * iSrcWidth * iSrcHeight, ( iZ + 1 ) * iSrcWidth * iSrcHeight };
					for( uint32 iY = 0; iY < iSrcHeight; iY += 2 )
					{
						const uint32 iIndexRows[2] = { iY * iSrcWidth, ( iY + 1 ) * iSrcWidth };
						for( uint32 iX = 0; iX < iSrcWidth; iX += 2, ++pDestData )
						{
							const float32 fSrcPixels[8] = 
							{
								pSrcData[iIndexSlices[0] + iIndexRows[0] + iX],
								pSrcData[iIndexSlices[0] + iIndexRows[0] + iX + 1],
								pSrcData[iIndexSlices[0] + iIndexRows[1] + iX],
								pSrcData[iIndexSlices[0] + iIndexRows[1] + iX + 1],
								pSrcData[iIndexSlices[1] + iIndexRows[0] + iX],
								pSrcData[iIndexSlices[1] + iIndexRows[0] + iX + 1],
								pSrcData[iIndexSlices[1] + iIndexRows[1] + iX],
								pSrcData[iIndexSlices[1] + iIndexRows[1] + iX + 1]
							};
							*pDestData = ( fSrcPixels[0] + fSrcPixels[1] + fSrcPixels[2] + fSrcPixels[3] +
								fSrcPixels[4] + fSrcPixels[5] + fSrcPixels[6] + fSrcPixels[7] ) * 0.125f;
						}
					}
				}
			}
			break;

		case m3dfmt_r32g32f:
			{
				for( uint32 iZ = 0; iZ < iSrcDepth; iZ += 2 )
				{
					const uint32 iIndexSlices[2] = { iZ * iSrcWidth * iSrcHeight, ( iZ + 1 ) * iSrcWidth * iSrcHeight };
					for( uint32 iY = 0; iY < iSrcHeight; iY += 2 )
					{
						const uint32 iIndexRows[2] = { iY * iSrcWidth, ( iY + 1 ) * iSrcWidth };
						for( uint32 iX = 0; iX < iSrcWidth; iX += 2, pDestData += 2 )
						{
							const vector2 *pSrcPixels[8] = 
							{
								&((vector2 *)pSrcData)[iIndexSlices[0] + iIndexRows[0] + iX],
								&((vector2 *)pSrcData)[iIndexSlices[0] + iIndexRows[0] + iX + 1],
								&((vector2 *)pSrcData)[iIndexSlices[0] + iIndexRows[1] + iX],
								&((vector2 *)pSrcData)[iIndexSlices[0] + iIndexRows[1] + iX + 1],
								&((vector2 *)pSrcData)[iIndexSlices[1] + iIndexRows[0] + iX],
								&((vector2 *)pSrcData)[iIndexSlices[1] + iIndexRows[0] + iX + 1],
								&((vector2 *)pSrcData)[iIndexSlices[1] + iIndexRows[1] + iX],
								&((vector2 *)pSrcData)[iIndexSlices[1] + iIndexRows[1] + iX + 1]
							};
							*(vector2 *)pDestData = ( *pSrcPixels[0] + *pSrcPixels[1] + *pSrcPixels[2] + *pSrcPixels[3] +
								*pSrcPixels[4] + *pSrcPixels[5] + *pSrcPixels[6] + *pSrcPixels[7] ) * 0.125f;
						}
					}
				}
			}
			break;

		case m3dfmt_r32g32b32f:
			{
				for( uint32 iZ = 0; iZ < iSrcDepth; iZ += 2 )
				{
					const uint32 iIndexSlices[2] = { iZ * iSrcWidth * iSrcHeight, ( iZ + 1 ) * iSrcWidth * iSrcHeight };
					for( uint32 iY = 0; iY < iSrcHeight; iY += 2 )
					{
						const uint32 iIndexRows[2] = { iY * iSrcWidth, ( iY + 1 ) * iSrcWidth };
						for( uint32 iX = 0; iX < iSrcWidth; iX += 2, pDestData += 3 )
						{
							const vector3 *pSrcPixels[8] = 
							{
								&((vector3 *)pSrcData)[iIndexSlices[0] + iIndexRows[0] + iX],
								&((vector3 *)pSrcData)[iIndexSlices[0] + iIndexRows[0] + iX + 1],
								&((vector3 *)pSrcData)[iIndexSlices[0] + iIndexRows[1] + iX],
								&((vector3 *)pSrcData)[iIndexSlices[0] + iIndexRows[1] + iX + 1],
								&((vector3 *)pSrcData)[iIndexSlices[1] + iIndexRows[0] + iX],
								&((vector3 *)pSrcData)[iIndexSlices[1] + iIndexRows[0] + iX + 1],
								&((vector3 *)pSrcData)[iIndexSlices[1] + iIndexRows[1] + iX],
								&((vector3 *)pSrcData)[iIndexSlices[1] + iIndexRows[1] + iX + 1]
							};
							*(vector3 *)pDestData = ( *pSrcPixels[0] + *pSrcPixels[1] + *pSrcPixels[2] + *pSrcPixels[3] +
								*pSrcPixels[4] + *pSrcPixels[5] + *pSrcPixels[6] + *pSrcPixels[7] ) * 0.125f;
						}
					}
				}
			}
			break;

		case m3dfmt_r32g32b32a32f:
			{
				for( uint32 iZ = 0; iZ < iSrcDepth; iZ += 2 )
				{
					const uint32 iIndexSlices[2] = { iZ * iSrcWidth * iSrcHeight, ( iZ + 1 ) * iSrcWidth * iSrcHeight };
					for( uint32 iY = 0; iY < iSrcHeight; iY += 2 )
					{
						const uint32 iIndexRows[2] = { iY * iSrcWidth, ( iY + 1 ) * iSrcWidth };
						for( uint32 iX = 0; iX < iSrcWidth; iX += 2, pDestData += 4 )
						{
							const vector4 *pSrcPixels[8] = 
							{
								&((vector4 *)pSrcData)[iIndexSlices[0] + iIndexRows[0] + iX],
								&((vector4 *)pSrcData)[iIndexSlices[0] + iIndexRows[0] + iX + 1],
								&((vector4 *)pSrcData)[iIndexSlices[0] + iIndexRows[1] + iX],
								&((vector4 *)pSrcData)[iIndexSlices[0] + iIndexRows[1] + iX + 1],
								&((vector4 *)pSrcData)[iIndexSlices[1] + iIndexRows[0] + iX],
								&((vector4 *)pSrcData)[iIndexSlices[1] + iIndexRows[0] + iX + 1],
								&((vector4 *)pSrcData)[iIndexSlices[1] + iIndexRows[1] + iX],
								&((vector4 *)pSrcData)[iIndexSlices[1] + iIndexRows[1] + iX + 1]
							};
							*(vector4 *)pDestData = ( *pSrcPixels[0] + *pSrcPixels[1] + *pSrcPixels[2] + *pSrcPixels[3] +
								*pSrcPixels[4] + *pSrcPixels[5] + *pSrcPixels[6] + *pSrcPixels[7] ) * 0.125f;
						}
					}
				}
			}
			break;

		default: // cannot happen
			break;
		}

		UnlockBox( iLevel );
		UnlockBox( iLevel - 1 );
	}

	return s_ok;
}

result CMuli3DVolumeTexture::LockBox( uint32 i_iMipLevel, void **o_ppData, const m3dbox *i_pBox )
{
	if( i_iMipLevel >= m_iMipLevels )
	{
		FUNC_FAILING( "CMuli3DVolumeTexture::LockRect: invalid mip-level specified.\n" );
		return e_invalidparameters;
	}

	return m_ppMipLevels[i_iMipLevel]->LockBox( o_ppData, i_pBox );
}

result CMuli3DVolumeTexture::UnlockBox( uint32 i_iMipLevel )
{
	if( i_iMipLevel >= m_iMipLevels )
	{
		FUNC_FAILING( "CMuli3DVolumeTexture::UnlockRect: invalid mip-level specified.\n" );
		return e_invalidparameters;
	}

	return m_ppMipLevels[i_iMipLevel]->UnlockBox();
}

CMuli3DVolume *CMuli3DVolumeTexture::pGetMipLevel( uint32 i_iMipLevel )
{
	if( i_iMipLevel >= m_iMipLevels )
	{
		FUNC_FAILING( "CMuli3DVolumeTexture::pGetMipLevel: invalid mip-level specified.\n" );
		return 0;
	}

	m_ppMipLevels[i_iMipLevel]->AddRef();

	return m_ppMipLevels[i_iMipLevel];
}

result CMuli3DVolumeTexture::SampleTexture( vector4 &o_vColor, float32 i_fU, float32 i_fV, float32 i_fW, const vector4 *i_pXGradient, const vector4 *i_pYGradient, const uint32 *i_pSamplerStates )
{
	uint32 iTexFilter = i_pSamplerStates[m3dtss_minfilter];
	float32 fTexMipLevel = 0.0f;
	
	if( i_pXGradient && i_pYGradient )
	{
		// Compute the mip-level and determine the texture filter type.
		const float32 fLenXGrad = i_pXGradient->x * i_pXGradient->x * m_fSquaredWidth + i_pXGradient->y * i_pXGradient->y * m_fSquaredHeight + i_pXGradient->z * i_pXGradient->z * m_fSquaredDepth;
		const float32 fLenYGrad = i_pYGradient->x * i_pYGradient->x * m_fSquaredWidth + i_pYGradient->y * i_pYGradient->y * m_fSquaredHeight + i_pYGradient->z * i_pYGradient->z * m_fSquaredDepth;
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

	if( i_pSamplerStates[m3dtss_mipfilter] == m3dtf_linear )
	{
		uint32 iMipLevelA = ftol( fTexMipLevel );
		uint32 iMipLevelB = iMipLevelA + 1;
		if( iMipLevelA >= m_iMipLevels ) iMipLevelA = m_iMipLevels - 1;
		if( iMipLevelB >= m_iMipLevels ) iMipLevelB = m_iMipLevels - 1;

		vector4 vColorA, vColorB;
		if( iTexFilter == m3dtf_linear )
		{
			m_ppMipLevels[iMipLevelA]->SampleLinear( vColorA, i_fU, i_fV, i_fW );
			m_ppMipLevels[iMipLevelB]->SampleLinear( vColorB, i_fU, i_fV, i_fW );
		}
		else
		{
			m_ppMipLevels[iMipLevelA]->SamplePoint( vColorA, i_fU, i_fV, i_fW );
			m_ppMipLevels[iMipLevelB]->SamplePoint( vColorB, i_fU, i_fV, i_fW );
		}

		float32 fInterpolation = fTexMipLevel - iMipLevelA;  // TODO: not accurate
		vVector4Lerp( o_vColor, vColorA, vColorB, fInterpolation );
	}
	else
	{
		uint32 iMipLevel = ftol( fTexMipLevel );
		if( iMipLevel >= m_iMipLevels ) iMipLevel = m_iMipLevels - 1;

		if( iTexFilter == m3dtf_linear )
			m_ppMipLevels[iMipLevel]->SampleLinear( o_vColor, i_fU, i_fV, i_fW );
		else
			m_ppMipLevels[iMipLevel]->SamplePoint( o_vColor, i_fU, i_fV, i_fW );
	}

	return s_ok;
}

m3dformat CMuli3DVolumeTexture::fmtGetFormat()
{
	return m_ppMipLevels[0]->fmtGetFormat();
}

uint32 CMuli3DVolumeTexture::iGetFormatFloats()
{
	return m_ppMipLevels[0]->iGetFormatFloats();
}

uint32 CMuli3DVolumeTexture::iGetMipLevels()
{
	return m_iMipLevels;
}

uint32 CMuli3DVolumeTexture::iGetWidth( uint32 i_iMipLevel )
{
	if( i_iMipLevel >= m_iMipLevels )
	{
		FUNC_FAILING( "CMuli3DVolumeTexture::iGetWidth: invalid mip-level specified.\n" );
		return 0;
	}

	return m_ppMipLevels[i_iMipLevel]->iGetWidth();
}

uint32 CMuli3DVolumeTexture::iGetHeight( uint32 i_iMipLevel )
{
	if( i_iMipLevel >= m_iMipLevels )
	{
		FUNC_FAILING( "CMuli3DVolumeTexture::iGetHeight: invalid mip-level specified.\n" );
		return 0;
	}

	return m_ppMipLevels[i_iMipLevel]->iGetHeight();
}

uint32 CMuli3DVolumeTexture::iGetDepth( uint32 i_iMipLevel )
{
	if( i_iMipLevel >= m_iMipLevels )
	{
		FUNC_FAILING( "CMuli3DVolumeTexture::iGetDepth: invalid mip-level specified.\n" );
		return 0;
	}

	return m_ppMipLevels[i_iMipLevel]->iGetDepth();
}
