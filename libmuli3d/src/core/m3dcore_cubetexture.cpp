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

#include "../../include/core/m3dcore_cubetexture.h"
#include "../../include/core/m3dcore_texture.h"
#include "../../include/core/m3dcore_device.h"

CMuli3DCubeTexture::CMuli3DCubeTexture( CMuli3DDevice *i_pParent )
	: IMuli3DBaseTexture( i_pParent )
{
	for( uint32 iFace = m3dcf_positive_x; iFace <= m3dcf_negative_z; ++iFace )
		m_ppCubeFaces[iFace] = 0;
}

result CMuli3DCubeTexture::Create( uint32 i_iEdgeLength, uint32 i_iMipLevels, m3dformat i_fmtFormat )
{
	if( !i_iEdgeLength )
	{
		FUNC_FAILING( "CMuli3DCubeTexture::Create: edge length is invalid.\n" );
		return e_invalidparameters;
	}
	
	if( i_fmtFormat < m3dfmt_r32f || i_fmtFormat > m3dfmt_r32g32b32a32f )
	{
		FUNC_FAILING( "CMuli3DCubeTexture::Create: invalid format specified.\n" );
		return e_invalidparameters;
	}

	result resCreate;
	for( uint32 iFace = m3dcf_positive_x; iFace <= m3dcf_negative_z; ++iFace )
	{
		resCreate = m_pParent->CreateTexture( &m_ppCubeFaces[iFace], i_iEdgeLength, i_iEdgeLength, i_iMipLevels, i_fmtFormat );
		if( FUNC_FAILED( resCreate ) )
			return resCreate;
	}

	return s_ok;
}

CMuli3DCubeTexture::~CMuli3DCubeTexture()
{
	for( uint32 iFace = m3dcf_positive_x; iFace <= m3dcf_negative_z; ++iFace )
		SAFE_RELEASE( m_ppCubeFaces[iFace] );
}

m3dtexsampleinput CMuli3DCubeTexture::eGetTexSampleInput()
{
	return m3dtsi_vector;
}

result CMuli3DCubeTexture::GenerateMipSubLevels( uint32 i_iSrcLevel )
{
	for( uint32 iFace = m3dcf_positive_x; iFace <= m3dcf_negative_z; ++iFace )
	{
		result resFace = m_ppCubeFaces[iFace]->GenerateMipSubLevels( i_iSrcLevel );
		if( resFace != s_ok )
			return resFace;
	}
	return s_ok;
}

result CMuli3DCubeTexture::LockRect( m3dcubefaces i_Face, uint32 i_iMipLevel, void **o_ppData, const m3drect *i_pRect )
{
	if( i_Face < 0 || i_Face >= 6 )
	{
		FUNC_FAILING( "CMuli3DCubeTexture::LockRect: invalid cube face requested.\n" );
		return e_invalidparameters;
	}

	return m_ppCubeFaces[i_Face]->LockRect( i_iMipLevel, o_ppData, i_pRect );
}

result CMuli3DCubeTexture::UnlockRect( m3dcubefaces i_Face, uint32 i_iMipLevel )
{
	if( i_Face < 0 || i_Face >= 6 )
	{
		FUNC_FAILING( "CMuli3DCubeTexture::UnlockRect: invalid cube face specified.\n" );
		return e_invalidparameters;
	}

	return m_ppCubeFaces[i_Face]->UnlockRect( i_iMipLevel );
}

result CMuli3DCubeTexture::SampleTexture( vector4 &o_vColor, float32 i_fU, float32 i_fV, float32 i_fW, const vector4 *i_pXGradient, const vector4 *i_pYGradient, const uint32 *i_pSamplerStates )
{
	// Determine face and local u/v coordinates ...
	// source: http://developer.nvidia.com/object/cube_map_ogl_tutorial.html

	// major axis 
	// direction     target                              sc     tc    ma 
	// ----------    ---------------------------------   ---    ---   --- 
	//  +rx          GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT   -rz    -ry   rx 
	//  -rx          GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT   +rz    -ry   rx 
	//  +ry          GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT   +rx    +rz   ry 
	//  -ry          GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT   +rx    -rz   ry 
	//  +rz          GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT   +rx    -ry   rz 
	//  -rz          GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT   -rx    -ry   rz
	float32 fCU, fCV, fInvMag;
	m3dcubefaces Face;

	const float32 fAbsU = fabsf( i_fU );
	const float32 fAbsV = fabsf( i_fV );
	const float32 fAbsW = fabsf( i_fW );

	if( fAbsU >= fAbsV && fAbsU >= fAbsW )
	{
		if( i_fU >= 0.0f )
		{
			// major axis direction: +rx
			Face = m3dcf_positive_x;
			fCU = -i_fW; fCV = -i_fV; fInvMag = 1.0f / fAbsU;
		}
		else
		{
			// major axis direction: -rx
			Face = m3dcf_negative_x;
			fCU = i_fW; fCV = -i_fV; fInvMag = 1.0f / fAbsU;
		}
	}
	else if( fAbsV >= fAbsU && fAbsV >= fAbsW )
	{
		if( i_fV >= 0.0f )
		{
			// major axis direction: +ry
			Face = m3dcf_positive_y;
			fCU = i_fU; fCV = i_fW; fInvMag = 1.0f / fAbsV;
		}
		else
		{
			// major axis direction: -ry
			Face = m3dcf_negative_y;
			fCU = i_fU; fCV = -i_fW; fInvMag = 1.0f / fAbsV;
		}
	}
	else //if( fAbsW >= fAbsU && fAbsW >= fAbsV )
	{
		if( i_fW >= 0.0f )
		{
			// major axis direction: +rz
			Face = m3dcf_positive_z;
			fCU = i_fU; fCV = -i_fV; fInvMag = 1.0f / fAbsW;
		}
		else
		{
			// major axis direction: -rz
			Face = m3dcf_negative_z;
			fCU = -i_fU; fCV = -i_fV; fInvMag = 1.0f / fAbsW;
		}
	}

	// s   =   ( sc/|ma| + 1 ) / 2 
	// t   =   ( tc/|ma| + 1 ) / 2
	fInvMag *= 0.5f;
	const float32 fU = /*fSaturate*/( fCU * fInvMag + 0.5f );
	const float32 fV = /*fSaturate*/( fCV * fInvMag + 0.5f );

	return m_ppCubeFaces[Face]->SampleTexture( o_vColor, fU, fV, 0, i_pXGradient, i_pYGradient, i_pSamplerStates );
}

m3dformat CMuli3DCubeTexture::fmtGetFormat()
{
	return m_ppCubeFaces[0]->fmtGetFormat();
}

uint32 CMuli3DCubeTexture::iGetFormatFloats()
{
	return m_ppCubeFaces[0]->iGetFormatFloats();
}

uint32 CMuli3DCubeTexture::iGetMipLevels()
{
	return m_ppCubeFaces[0]->iGetMipLevels();
}

uint32 CMuli3DCubeTexture::iGetEdgeLength( uint32 i_iMipLevel )
{
	return m_ppCubeFaces[0]->iGetWidth( i_iMipLevel );
}

CMuli3DTexture *CMuli3DCubeTexture::pGetCubeFace( m3dcubefaces i_Face )
{
	if( i_Face < 0 || i_Face >= 6 )
	{
		FUNC_FAILING( "CMuli3DCubeTexture::pGetCubeFace: invalid cube face requested.\n" );
		return 0;
	}

	m_ppCubeFaces[i_Face]->AddRef();
	return m_ppCubeFaces[i_Face];
}
