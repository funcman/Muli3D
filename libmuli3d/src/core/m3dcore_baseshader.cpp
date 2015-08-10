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

#include "../../include/core/m3dcore_baseshader.h"
#include "../../include/core/m3dcore_device.h"

void IMuli3DBaseShader::SetFloat( uint32 i_iIndex, float32 i_fValue )
{
	m_fConstants[i_iIndex] = i_fValue;
}

float32 IMuli3DBaseShader::fGetFloat( uint32 i_iIndex )
{
	return m_fConstants[i_iIndex];
}

void IMuli3DBaseShader::SetVector( uint32 i_iIndex, const vector4 &i_vVector )
{
	m_vConstants[i_iIndex] = i_vVector;
}

const vector4 &IMuli3DBaseShader::vGetVector( uint32 i_iIndex )
{
	return m_vConstants[i_iIndex];
}

void IMuli3DBaseShader::SetMatrix( uint32 i_iIndex, const matrix44 &i_matMatrix )
{
	m_matConstants[i_iIndex] = i_matMatrix;
}

const matrix44 &IMuli3DBaseShader::matGetMatrix( uint32 i_iIndex )
{
	return m_matConstants[i_iIndex];
}

void IMuli3DBaseShader::SetDevice( CMuli3DDevice *i_pDevice )
{
	m_pDevice = i_pDevice;
}

result IMuli3DBaseShader::SampleTexture( vector4 &o_vColor, uint32 i_iSamplerNumber, float32 i_fU, float32 i_fV, float32 i_fW, const vector4 *i_pXGradient, const vector4 *i_pYGradient )
{
	/* if( !m_pDevice )
		return e_unknown; */
	
	return m_pDevice->SampleTexture( o_vColor, i_iSamplerNumber, i_fU, i_fV, i_fW, i_pXGradient, i_pYGradient );
}
