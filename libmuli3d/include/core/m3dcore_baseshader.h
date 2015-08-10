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

/// @file m3dcore_baseshader.h
///

#ifndef __M3DCORE_BASESHADER_H__
#define __M3DCORE_BASESHADER_H__

#include "../m3dbase.h"
#include "../m3dtypes.h"

/// This is the shader base-class, which implements support for float, vector4 and matrix constants-registers.
class IMuli3DBaseShader : public IBase
{
public:
	/// Sets a single float-constant.
	/// @param[in] i_iIndex index of the constant.
	/// @param[in] i_fValue value of the constant.
	void SetFloat( uint32 i_iIndex, float32 i_fValue );

	/// Returns the value of a single float-constant.
	/// @param[in] i_iIndex index of the constant.
	float32 fGetFloat( uint32 i_iIndex );

	/// Sets a vector4-constant.
	/// @param[in] i_iIndex index of the constant.
	/// @param[in] i_vVector value of the constant.
	void SetVector( uint32 i_iIndex, const vector4 &i_vVector );

	/// Returns the value of a vector4-constant.
	/// @param[in] i_iIndex index of the constant.
	const vector4 &vGetVector( uint32 i_iIndex );

	/// Sets a matrix-constant.
	/// @param[in] i_iIndex index of the constant.
	/// @param[in] i_matMatrix value of the constant.
	void SetMatrix( uint32 i_iIndex, const matrix44 &i_matMatrix );

	/// Returns the value of a matrix-constant.
	/// @param[in] i_iIndex index of the constant.
	const matrix44 &matGetMatrix( uint32 i_iIndex );

protected:
	friend class CMuli3DDevice;

	/// Accessible by CMuli3D - Sets the rendering-device.
	/// @param[in] i_pDevice the device.
	void SetDevice( class CMuli3DDevice *i_pDevice );

	/// Samples the texture and returns the looked-up color. This simply functions
	/// simply forwards the sampling-call to the device.
	/// @param[out] o_vColor receives the color of the pixel to be looked up.
	/// @param[in] i_iSamplerNumber number of the sampler.
	/// @param[in] i_fU u-component of the lookup-vector.
	/// @param[in] i_fV v-component of the lookup-vector.
	/// @param[in] i_fW w-component of the lookup-vector.
	/// @param[in] i_pXGradient partial derivatives of the texture coordinates with respect to the screen-space x coordinate (optional, base for mip-level calculations).
	/// @param[in] i_pYGradient partial derivatives of the texture coordinates with respect to the screen-space y coordinate (optional, base for mip-level calculations).
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one or more parameters were invalid.
	result SampleTexture( vector4 &o_vColor, uint32 i_iSamplerNumber,
		float32 i_fU, float32 i_fV, float32 i_fW = 0.0f,
		const vector4 *i_pXGradient = 0, const vector4 *i_pYGradient = 0 );

private:
	float32				m_fConstants[c_iNumShaderConstants];	///< Single float-constants.
	vector4				m_vConstants[c_iNumShaderConstants];	///< vector4-constants.
	matrix44			m_matConstants[c_iNumShaderConstants];	///< Matrix-constants.
	class CMuli3DDevice	*m_pDevice; ///< The Muli3D-device currently used for rendering.
};

#endif // __M3DCORE_BASESHADER_H__
