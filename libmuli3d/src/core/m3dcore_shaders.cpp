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

#include "../../include/core/m3dcore_shaders.h"

void IMuli3DPixelShader::SetInfo( const m3dshaderregtype *i_pVSOutputs, const struct m3dtriangleinfo *i_pTriangleInfo )
{
	m_pVSOutputs = i_pVSOutputs;
	m_pTriangleInfo = i_pTriangleInfo;
}

// Partial derivative equations taken from
// "MIP-Map Level Selection for Texture Mapping",
// Jon P. Ewins, Member, IEEE, Marcus D. Waller,
// Martin White, and Paul F. Lister, Member, IEEE

void IMuli3DPixelShader::GetDerivatives( uint32 i_iRegister, vector4 &o_vDdx, vector4 &o_vDdy ) const
{
	o_vDdx = vector4( 0, 0, 0, 0 ); o_vDdy = vector4( 0, 0, 0, 0 );
	if( i_iRegister < 0 || i_iRegister >= c_iPixelShaderRegisters )
		return;

	const shaderreg &A = m_pTriangleInfo->ShaderOutputsDdx[i_iRegister];
	const shaderreg &B = m_pTriangleInfo->ShaderOutputsDdy[i_iRegister];
	const shaderreg &C = m_pTriangleInfo->pBaseVertex->ShaderOutputs[i_iRegister];

	const float32 D = m_pTriangleInfo->fWDdx;
	const float32 E = m_pTriangleInfo->fWDdy;
	const float32 F = m_pTriangleInfo->pBaseVertex->vPosition.w;

	const float32 fRelPixelX = m_pTriangleInfo->iCurPixelX - m_pTriangleInfo->pBaseVertex->vPosition.x;
	const float32 fRelPixelY = m_pTriangleInfo->iCurPixelY - m_pTriangleInfo->pBaseVertex->vPosition.y;
	const float32 fInvWSquare = m_pTriangleInfo->fCurPixelInvW * m_pTriangleInfo->fCurPixelInvW;

	// Compute partial derivative with respect to the x-screen space coordinate.
	switch( m_pVSOutputs[i_iRegister] )
	{
	case m3dsrt_vector4:
		o_vDdx.w = ( (A.w * F - C.w * D) + (A.w * E - B.w * D) * fRelPixelY ) * fInvWSquare;
	case m3dsrt_vector3:
		o_vDdx.z = ( (A.z * F - C.z * D) + (A.z * E - B.z * D) * fRelPixelY ) * fInvWSquare;
	case m3dsrt_vector2:
		o_vDdx.y = ( (A.y * F - C.y * D) + (A.y * E - B.y * D) * fRelPixelY ) * fInvWSquare;
	case m3dsrt_float32:
		o_vDdx.x = ( (A.x * F - C.x * D) + (A.x * E - B.x * D) * fRelPixelY ) * fInvWSquare;
	case m3dsrt_unused:
	default:
		break;
	}

	// Compute partial derivative with respect to the y-screen space coordinate.
	switch( m_pVSOutputs[i_iRegister] )
	{
	case m3dsrt_vector4:
		o_vDdy.w = ( (B.w * F - C.w * E) + (B.w * D - A.w * E) * fRelPixelX ) * fInvWSquare;
	case m3dsrt_vector3:
		o_vDdy.z = ( (B.z * F - C.z * E) + (B.z * D - A.z * E) * fRelPixelX ) * fInvWSquare;
	case m3dsrt_vector2:
		o_vDdy.y = ( (B.y * F - C.y * E) + (B.y * D - A.y * E) * fRelPixelX ) * fInvWSquare;
	case m3dsrt_float32:
		o_vDdy.x = ( (B.x * F - C.x * E) + (B.x * D - A.x * E) * fRelPixelX ) * fInvWSquare;
	case m3dsrt_unused:
	default:
		break;
	}
}
