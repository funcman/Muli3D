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

/// @file m3dtypes.h
///

#ifndef __M3DTYPES_H__
#define __M3DTYPES_H__

#include "m3dbase.h"
#include "math/m3dmath.h"

// Platform-dependent definitions and includes --------------------------------

#ifdef WIN32

#define WIN32_LEAN_AND_MEAN		///< Restrict inclusions.
#include <windows.h>
typedef HWND windowhandle;		///< Define window-handle for the Win32-platform.

#endif


#ifdef LINUX_X11

#include <X11/Xlib.h>
#include <X11/Xutil.h>
typedef Window windowhandle;		///< Define window-handle for the Linux-platform.

#endif


#ifdef __amigaos4__

#include <intuition/intuition.h>
typedef struct Window *windowhandle; ///< Define window-handle for the AmigaOS-platform

#endif

// Constants ------------------------------------------------------------------

const uint32 c_iVertexCacheSize = 32;		///< Specifies the size of the vertex cache. Minimum is 3!
const uint32 c_iVertexShaderRegisters = 8;	///< Specifies the amount of available vertex shader input registers.
const uint32 c_iPixelShaderRegisters = 8;	///< Specifies the amount of available vertex shader output registers, which are simulateously used as pixel shader input registers.
const uint32 c_iNumShaderConstants = 32;	///< Specifies the amount of available shader constants-registers for both vertex and pixel shaders.
const uint32 c_iMaxVertexStreams = 8;		///< Specifies the amount of available vertex streams.
const uint32 c_iMaxTextureSamplers = 16;	///< Specifies the amount of available texture samplers.

// Enumerations ---------------------------------------------------------------

/// Defines the available renderstates.
enum m3drenderstate
{
	m3drs_zenable = 0,		///< Set this to true to enable depth-buffering or to false to disable it.
	m3drs_zwriteenable,		///< Set this to true(default) to enable writing to the depth-buffer during rasterization. If no depth-buffer is available or has been disabled by setting m3drs_zenable to false, this renderstate has no effect.
	m3drs_zfunc,			///< Compare-function used for depth-buffer. Set this renderstate to a member of the enumeration m3dcmpfunc. Default: m3dcmp_less.

	m3drs_colorwriteenable,	///< Set this to true(default) to enable writing to the color-buffer during rasteriation. If no color-buffer is available this renderstate has no effect.
	m3drs_fillmode,			///< Fillmode. Set this renderstate to a member of the enumeration m3dfill. Default: m3dfill_solid.

	m3drs_cullmode,			///< Cullmode. Set this renderstate to a member of the enumeration m3dcull. Default: m3dcull_ccw.

	m3drs_subdivisionmode,				///< Subdivisionmode. Set this renderstate to a member of the enumeration m3dsubdiv. Default: m3dsubdiv_none.
	m3drs_subdivisionlevels,			///< This renderstate specifies the number of recursive subdivision when using simple or smooth subdivision. It specifies the maximum number of recursive subdivisions of triangles' edges when using adaptive subdivision. In case subdivision has been disabled, this renderstate has no effect. Valid values are integers > 0 - if this renderstate has been set to 0, DrawPrimitive()-calls will fail. Default: 1.
	m3drs_subdivisionpositionregister,	///< This renderstate is only used when using smooth subdivision. It specifies the vertex shader input register which holds position-data. Make sure vertex positions have been homogenized (w=1)! In case subdivision has been disabled, this renderstate has no effect. Valid values are integers e [0,c_iVertexShaderRegisters[. Default: 0.
	m3drs_subdivisionnormalregister,	///< This renderstate is only used when using smooth subdivision. It specifies the vertex shader input register which holds normal-data. For best results make sure that the normals have been normalized. In case subdivision has been disabled, this renderstate has no effect. Valid values are integers e [0,c_iVertexShaderRegisters[. Default: 1.
	m3drs_subdivisionmaxscreenarea,		///< This renderstate is only used when using adaptive subdivision. Triangles, which cover more than the set area in screenspace (rendertarget's viewport is respected), are recursivly subdividied. In case subdivision has been disabled, this renderstate has no effect. Valid values are floats > 0.0f - if this renderstate has been set to 0.0f, DrawPrimitive()-calls will fail. Default: 1.0f.
	m3drs_subdivisionmaxinnerlevels,	///< This renderstate is only used when using adaptive subdivision. It specifies the maximum number of recursive subdivisions of triangles. In case subdivision has been disabled, this renderstate has no effect. Valid values are integers > 0 - if this renderstate has been set to 0, DrawPrimitive()-calls will fail. Default: 1.

	m3drs_scissortestenable,	///< Set this to true to enable scissor testing. Make sure that a scissor rect has been set before calling DrawPrimitive()-functions. Set this to false(default) to disable scissor testing.
	
	m3drs_linethickness,			///< Controls the thickness of rendered lines Valid values are integers >= 1. Default: 1.

	m3drs_numrenderstates
};

/// Defines the supported compare functions.
enum m3dcmpfunc
{
	m3dcmp_never,			///< Compares will never pass.
    m3dcmp_equal,			///< Compares of two values will pass if they are equal.
    m3dcmp_notequal,		///< Compares of two values will pass if they are not equal.
    m3dcmp_less,			///< Compares of two values a and b will pass if a < b.
    m3dcmp_lessequal,		///< Compares of two values a and b will pass if a <= b.
    m3dcmp_greaterequal,	///< Compares of two values a and b will pass if a >= b.
    m3dcmp_greater,			///< Compares of two values a and b will pass if a > b.
    m3dcmp_always			///< Compares will always pass.
};

/// Defines the supported culling modes.
enum m3dcull
{
	m3dcull_none,	///< Back faces won't be culled.
	m3dcull_cw,		///< Culls faces with clockwise vertices.
	m3dcull_ccw		///< Culls faces with counterclockwise vertices (default).
};

/// Defines the supported fill modes.
enum m3dfill
{
	m3dfill_solid,		///< Triangles are filled during rasterization (default).
	m3dfill_wireframe	///< Only triangle's edges are drawn.
};

/// Defines the available texturesamplerstates.
enum m3dtexturesamplerstate
{
	m3dtss_addressu = 0,	///< Texture addressing mode for the u-coordinate. Set this renderstate to a member of the enumeration m3dta. Default: m3dta_wrap.
	m3dtss_addressv,		///< Texture addressing mode for the v-coordinate. Set this renderstate to a member of the enumeration m3dta. Default: m3dta_wrap.
	m3dtss_addressw,		///< Texture addressing mode for the w-coordinate. Set this renderstate to a member of the enumeration m3dta. Default: m3dta_wrap.
	m3dtss_minfilter,		///< Texture minification filtering mode. Set this renderstate to a member of the enumeration m3dtf. Default: m3dtf_linear.
	m3dtss_magfilter,		///< Texture magnification filtering mode. Set this renderstate to a member of the enumeration m3dtf. Default: m3dtf_linear.
	m3dtss_mipfilter,		///< Mipmap filtering mode. Set this renderstate to a member of the enumeration m3dtf. Default: m3dtf_point.
	m3dtss_miplodbias,		///< Floating point value added to the mip-level when sampling textures. Default: 0.0f.
	m3dtss_maxmiplevel,		///< Floating point value which specifies the smallest mip-level to be used, e.g. 3.0f would mean that the third mip-level is the smallest to be used. Set this to 0.0f to force the use of the largest mip-level. default: 16.0f

	m3dtss_numtexturesamplerstates
};

/// Defines the supported texture adressing-modes.
enum m3dtextureaddress
{
	m3dta_wrap,
	m3dta_clamp
};

/// Defines the supported texturefilters.
enum m3dtexturefilter
{
	m3dtf_point,	///< Specifies nearest point sampling.
	m3dtf_linear	///< Specifies linear filtering.
};

/// Specifies the supported subdivision modes.
enum m3dsubdiv
{
	m3dsubdiv_none,		///< Triangle subdivision is disabled.
	m3dsubdiv_simple,	///< This subdivision mode simply subdivides each triangle an user specified number of times recursively.
	m3dsubdiv_smooth,	///< This subdivision mode is a simplified implementation of ATI's TruForm: It subdivides each triangle an user specified number of times recursively and attempts to generate a smooth triangle-surface using vertex normals as a base. This subdivision mode requires access to vertex position and normal. (For best results make sure that normal-vectors are normalized.)
	m3dsubdiv_adaptive	///< This subdivision mode splits each triangle's edges an user specified number of times recursively. The triangle is then subdivided until its sub-triangles cover no more than a user specified area in clipping space.
};

/// Defines the supported texture and buffer formats.
/// The default value for formats that contain undefined channels is 1.0f for the undefined alpha channel and 0.0f for undefined color channels.
/// E.g. m3dfmt_r32g32b32f doesn't define the alpha channel, which is therefore set to 1.0f.
enum m3dformat
{
	// Texture formats
	m3dfmt_r32f,			///< 32-bit texture format, one float mapped to the red channel.
	m3dfmt_r32g32f,			///< 64-bit texture format, two floats mapped to the red and green channel.
	m3dfmt_r32g32b32f,		///< 96-bit texture format, three floats mapped to the three color channel.
	m3dfmt_r32g32b32a32f,	///< 128-bit texture format, four floats mapped to the three color channel plus the alpha channel.

	// Indexbuffer formats
	m3dfmt_index16,			///< 16-bit indexbuffer format, indices are shorts.
	m3dfmt_index32			///< 32-bit indexbuffer format, indices are integers.
};

/// Defines the supported primitive types.
enum m3dprimitivetype
{
	m3dpt_trianglefan,		///< Triangle fan.
	m3dpt_trianglestrip,	///< Triangle strip.
	m3dpt_trianglelist		///< Triangle lists.
};

/// Defines the supported vertex elements.
enum m3dvertexelementtype
{
	m3dvet_float32,		///< Specifies a single float. The vertex element is expanded to vector4( float, 0, 0, 1 ) and mapped to a vertex shader input register.
	m3dvet_vector2,		///< Specifies two floats. The vertex element is expanded to vector4( float, float, 0, 1 ) and mapped to a vertex shader input register.
	m3dvet_vector3,		///< Specifies three floats. The vertex element is expanded to vector4( float, float, float, 1 ) and mapped to a vertex shader input register.
	m3dvet_vector4		///< Specifies four floats. The vertex element is directly mapped to a vertex shader input register.
};

/// Specifies indices for commonly used shader constants.
enum m3dshaderconstant
{
	m3dsc_worldmatrix		= 0,	///< World matrix for vertex transformation.
	m3dsc_viewmatrix		= 1,	///< View matrix for vertex transformation.
	m3dsc_projectionmatrix	= 2,	///< Projection matrix for vertex transformation.
	m3dsc_wvpmatrix			= 3,	///< Concatenated world, view and projection matrix for vertex transformation.
};

/// Specifies the available types of vectors used for sampling different textures.
enum m3dtexsampleinput
{
	m3dtsi_2coords,	///< 2 floating point coordinates used for standard 2d texture-sampling.
	m3dtsi_3coords,	///< 3 floating point coordinates used for volume texture-sampling.
	m3dtsi_vector	///< 3-dimensional vector used for cubemap-sampling.
};

/// Specifies indices for the 6 cubemap faces.
enum m3dcubefaces
{
	m3dcf_positive_x = 0,	///< face +x
	m3dcf_negative_x,		///< face -x
	m3dcf_positive_y,		///< face +y
	m3dcf_negative_y,		///< face -y
	m3dcf_positive_z,		///< face +z
	m3dcf_negative_z		///< face -z
};

/// Specifies the supported pixelshader types.
enum m3dpixelshaderoutput
{
	m3dpso_coloronly,	///< Specifies that a given pixelshader only outputs color (default). A pixel's depth values will be automatically computed by the rasterizer through interpolation of depth values of a triangle's vertices.
	m3dpso_colordepth,	///< Specifies that a given pixelshader outputs both color and depth. If you want to output only depth use renderstate m3drs_colorwriteenable to disable writing color.
};

/// Specifies the type of a particular shader register.
enum m3dshaderregtype
{
	m3dsrt_unused = 0,		///< Specifies that the register is unused.
	m3dsrt_float32,			///< Specifies that the register should be treated as a single float.
	m3dsrt_vector2,			///< Specifies that the register should be treated as a 2-dimensional vector.
	m3dsrt_vector3,			///< Specifies that the register should be treated as a 3-dimensional vector.
	m3dsrt_vector4			///< Specifies that the register should be treated as a 4-dimensional vector.
};

enum m3dclippingplanes
{
	m3dcp_left = 0,			///< Left frustum clipping plane.
	m3dcp_right,			///< Right frustum clipping plane.
	m3dcp_top,				///< Top frustum clipping plane.
	m3dcp_bottom,			///< Bottom frustum clipping plane.
	m3dcp_near,				///< Near frustum clipping plane.
	m3dcp_far,				///< Far frustum clipping plane.

	/// User specified clipping planes.
	m3dcp_user0,
	m3dcp_user1,
	m3dcp_user2,
	m3dcp_user3,

	m3dcp_numplanes
};

// Structures -----------------------------------------------------------------

/// Defines a rectangle.
struct m3drect
{
	uint32 iLeft, iTop;
	uint32 iRight, iBottom;
};

/// Defines a box. Added for volume texture support.
struct m3dbox
{
	uint32 iLeft, iTop, iFront;
	uint32 iRight, iBottom, iBack;
};

/// This structure defines the device parameters.
struct m3ddeviceparameters
{
	windowhandle	hDeviceWindow;			///< Handle to the output window.
	bool			bWindowed;				///< True if the application runs windowed, false if it runs in full-screen.
	uint32			iFullscreenColorBits;	///< Bit-depth of backbuffer in fullscreen mode (ignored in windowed mode). Valid values: 32, 24, 16.
	
	/// Dimension of the backbuffer in Pixels.
	uint32	iBackbufferWidth, iBackbufferHeight;
};

/// Describes a vertex element.
struct m3dvertexelement
{
    uint32					iStream;	///< Index of the stream this element is loaded from.
    m3dvertexelementtype	Type;		///< Type of this vertex element. Set this field to a member of the enumeration m3dvertexelementtype.
	uint32					iRegister;	///< The register of the vertex shader the vertex element's value will be passed to.
};

#define M3DVERTEXFORMATDECL( i_iStream, i_Type, i_Register ) \
	{ i_iStream, i_Type, i_Register } ///< Helper-macro for vertex format declaration.


// Internal structures --------------------------------------------------------

typedef vector4 shaderreg; ///< A shader register is 128-bits wide and is divided into four floating-point-values.

/// Describes the vertex shader input.
/// @note This structure is used internally by devices.
struct m3dvsinput
{
	shaderreg	ShaderInputs[c_iVertexShaderRegisters];	///< Vertex shader input registers.
};

/// Describes the vertex shader output.
/// @note This structure is used internally by devices.
struct m3dvsoutput
{
	shaderreg	ShaderOutputs[c_iPixelShaderRegisters];	///< Vertex shader output registers, which are in turn used as pixel shader input registers.
	vector4		vPosition;								///< Position of this vertex.

	m3dvsinput	SourceInput;	///< Original vertex shader input fetched from vertex streams; added for triangle subdivision.
};

/// Describes a structure that is used for triangle gradient storage.
/// @note This structure is used internally by devices.
struct m3dtriangleinfo
{
	float32				fCommonGradient;	///< Gradient constant.
	const m3dvsoutput	*pBaseVertex;	///< Base vertex for gradient computations.

	/// z partial derivatives with respect to the screen-space x- and y-coordinates.
	float32		fZDdx, fZDdy;

	/// w partial derivatives with respect to the screen-space x- and y-coordinates.
	float32		fWDdx, fWDdy;

	shaderreg	ShaderOutputsDdx[c_iPixelShaderRegisters];	///< Shader register partial derivatives with respect to the screen-space x-coordinate.
	shaderreg	ShaderOutputsDdy[c_iPixelShaderRegisters];	///< Shader register partial derivatives with respect to the screen-space y-coordinate.

	/// Integer-coordinates of current pixel; needed by pixel shader for computation of partial derivatives.
	uint32 iCurPixelX, iCurPixelY;

	float32 fCurPixelInvW; ///< 1.0f / w of the current pixel; needed by pixel shader for computation of partial derivatives.
};

/// Describes a structure that is used for vertex caching.
/// @note This structure is used internally by devices.
struct m3dvertexcacheentry
{
	uint32		iVertexIndex;	///< Index of the contained vertex in the vertex buffer.
	m3dvsoutput	VertexOutput;	///< Vertex shader output, vertex data.
	uint32		iFetchTime;		///< Whenever a vertex cache entry is reserved for drawing (updated or simply 'touched and returned') its fetch-time is set to m_iFetchedVertices.
};

#endif // __M3DTYPES_H__
