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

/// @file m3dcore_shaders.h
///

#ifndef __M3DCORE_SHADERS_H__
#define __M3DCORE_SHADERS_H__

#include "../m3dbase.h"
#include "../m3dtypes.h"

#include "m3dcore_baseshader.h"

/// Defines the vertex shader interface.
class IMuli3DVertexShader : public IMuli3DBaseShader
{
protected:
	friend class CMuli3DDevice;
	/// Accessible by CMuli3DDevice.
	/// This is the core function of a vertex shader: It transforms vertex positions to homogeneous clipping space and sets up registers for the pixel shader.
	/// @param[in] i_pInput vertex shader input registers, data is loaded from the active vertex streams.
	/// @param[out] o_vPosition vertex position transformed to homogeneous clipping space.
	/// @param[out] o_pOutput vertex shader output registers which will be interpolated and passed to the pixel shader.
	virtual void Execute( const shaderreg *i_pInput, vector4 &o_vPosition,
		shaderreg *o_pOutput ) = 0;

	/// Returns the type of a particular output register. Member of the enumeration m3dshaderregtype; if a given register is not used, return m3dsrt_unused.
	/// @param[in] i_iRegister index of register, e [0;c_iPixelShaderRegisters[.
	virtual m3dshaderregtype GetOutputRegisters( uint32 i_iRegister ) = 0;
};

/// Defines the triangle shader interface.
class IMuli3DTriangleShader : public IMuli3DBaseShader
{
protected:
	friend class CMuli3DDevice;
	/// Accessible by CMuli3DDevice.
	/// This is the core function of a triangle shader: It performs per triangle operations on vertices.
	/// @param[in,out] io_pShaderRegs0 shader registers outputted from vertex shader for the first triangle vertex.
	/// @param[in,out] io_pShaderRegs1 shader registers outputted from vertex shader for the second triangle vertex.
	/// @param[in,out] io_pShaderRegs2 shader registers outputted from vertex shader for the third triangle vertex.
	/// @note A triangle shader may operate on the registers outputted by the vertex shader.
	/// @return false if triangle should not be rendered.
	virtual bool bExecute( shaderreg *io_pShaderRegs0,
		shaderreg *io_pShaderRegs1, shaderreg *io_pShaderRegs2 ) = 0;
};

/// Defines the pixel shader interface.
class IMuli3DPixelShader : public IMuli3DBaseShader
{
protected:
	friend class CMuli3DDevice;
	virtual m3dpixelshaderoutput GetShaderOutput() { return m3dpso_coloronly; } ///< Accessible by CMuli3DDevice. Returns the type of the pixel shader; member of the enumeration m3dpixelshaderoutput. Default: m3dpso_coloronly.
	virtual bool bMightKillPixels() { return true; }	///< Returns true incase support for pixel-killing for m3dpso_coloronly-shader-types shall be enabled.

	/// Accessible by CMuli3D - Sets the triangle info.
	/// @param[in] i_pVSOutputs pointer to the pixel shader input register-types.
	/// @param[in] i_pTriangleInfo pointer to the triangle info structure.
	void SetInfo( const m3dshaderregtype *i_pVSOutputs, const struct m3dtriangleinfo *i_pTriangleInfo );
	
	/// Accessible by CMuli3DDevice.
	/// This is the core function of a pixel shader: It receives interpolated register data from the vertex shader and can output a new color and depth value for the pixel currently being drawn.
	/// @param[in] i_pInput pixel shader input registers, which have been set up in the vertex shader and interpolated during rasterization.
	/// @param[in,out] io_vColor contains the value of the pixel in the rendertarget when Execute() is called. The pixel shader may perform blending with this value and setting it to a new color. Make sure to override GetShaderOutput() to the correct shader type.
	/// @param[in,out] io_fDepth contains the depth of the pixel in the rendertarget when Execute() is called. The pixel shader may set this to a new value. Make sure to override GetShaderOutput() to the correct shader type.
	/// @return true if the pixel shall be written to the rendertarget, false in case it shall be killed.
	virtual bool bExecute( const shaderreg *i_pInput, vector4 &io_vColor,
		float32 &io_fDepth ) = 0;

	/// This functions computes the partial derivatives of a shader register with respect to the screen space coordinates.
	/// @param[in] i_iRegister index of the source shader register.
	/// @param[out] o_vDdx partial derivative with respect to the x-screen space coordinate.
	/// @param[out] o_vDdy partial derivative with respect to the y-screen space coordinate.
	void GetDerivatives( uint32 i_iRegister, vector4 &o_vDdx, vector4 &o_vDdy ) const;

private:
	const m3dshaderregtype			*m_pVSOutputs; ///< Register type info.
	const struct m3dtriangleinfo	*m_pTriangleInfo; ///< Gradient info about the triangle that is currently being drawn.
};

#endif // __M3DCORE_SHADERS_H__
