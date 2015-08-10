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

/// @file m3dcore_device.h
///

#ifndef __M3DCORE_DEVICE_H__
#define __M3DCORE_DEVICE_H__

#include "../m3dbase.h"
#include "../m3dtypes.h"

/// The Muli3D device.
class CMuli3DDevice : public IBase
{
protected:
	~CMuli3DDevice(); ///< Accessible by IBase. The destructor is called when the reference count reaches zero.

	friend class CMuli3D;
	/// Accessible by CMuli3D which is the only class that may create a device.
	/// @param[in] i_pParent a pointer to the parent CMuli3D-object.
	/// @param[in] i_pDeviceParameters pointer to a m3ddeviceparameters-structure.
	CMuli3DDevice( class CMuli3D *i_pParent,
		const m3ddeviceparameters *i_pDeviceParameters );

	/// Creates and initializes the device.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one or more parameters were invalid.
	/// @return e_outofmemory if memory allocation failed.
	/// @return e_unknown if a presenttarget-specific problem was encountered.
	result Create();

public:
	class CMuli3D *pGetMuli3D(); ///< Returns a pointer to the Muli3D instance. Calling this function will increase the internal reference count of the Muli3D instance. Failure to call Release() when finished using the pointer will result in a memory leak.

	const m3ddeviceparameters &GetDeviceParameters(); ///< Returns the device parameters.

	// Drawing ----------------------------------------------------------------
	
	/// Presents the contents of a given rendertarget's colorbuffer.
	/// @param[in] i_pRenderTarget the rendertarget to be presented.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one or more parameters were invalid.
	/// @return e_invalidformat if an invalid format was encountered.
	/// @return e_invalidstate if an invalid state was encountered.
	/// @return e_unknown if a present-target related problem was encountered.
	result Present( class CMuli3DRenderTarget *i_pRenderTarget );

	/// Renders nonindexed primitives of the specified type from the currently set vertex streams.
	/// @param[in] i_PrimitiveType member of the enumeration m3dprimitivetype, specifies the primitives' type.
	/// @param[in] i_iStartVertex Beginning at this vertex the correct number used for rendering this batch will be read from the vertex streams.
	/// @param[in] i_iPrimitiveCount Amount of primitives to render.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one or more parameters were invalid.
	/// @return e_invalidstate if an invalid state was encountered.
	result DrawPrimitive( m3dprimitivetype i_PrimitiveType,
		uint32 i_iStartVertex, uint32 i_iPrimitiveCount );

	/// Renders geometric primitives through indexing into an array of vertices.
	/// @param[in] i_PrimitiveType member of the enumeration m3dprimitivetype, specifies the primitives' type.
	/// @param[in] i_iBaseVertexIndex added to each index before accessing a vertex from the array.
	/// @param[in] i_iMinIndex specifies the minimum index for vertices used during this batch.
	/// @param[in] i_iNumVertices specifies the number of vertices that will be used beginning from i_iBaseVertexIndex + i_iMinIndex.
	/// @param[in] i_iStartIndex Location in the index buffer to start reading from.
	/// @param[in] i_iPrimitiveCount Amount of primitives to render.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one or more parameters were invalid.
	/// @return e_invalidstate if an invalid state was encountered.
	result DrawIndexedPrimitive( m3dprimitivetype i_PrimitiveType,
		int32 i_iBaseVertexIndex, uint32 i_iMinIndex,
		uint32 i_iNumVertices, uint32 i_iStartIndex, uint32 i_iPrimitiveCount );

	/// Renders primitives assembled through the triangle assembler from the currently set vertex streams.
	/// @param[in] i_iStartVertex Beginning at this vertex the correct number used for rendering this batch will be read from the vertex streams.
	/// @param[in] i_iNumVertices specifies the number of vertices that will be used beginning from i_iStartVertex.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one or more parameters were invalid.
	/// @return e_invalidstate if an invalid state was encountered.
	result DrawDynamicPrimitive( uint32 i_iStartVertex, uint32 i_iNumVertices );

	// Resource creation ------------------------------------------------------

	/// Creates a vertex format from a vertex declaration. A vertex format describes the layout of vertex data in the vertex streams.
	/// @param[out] o_ppVertexFormat receives a pointer to the created vertex format.
	/// @param[in] i_pVertexDeclaration pointer to the vertex declaration.
	/// @param[in] i_iVertexDeclSize size of the vertex declaration in bytes.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one or more parameters were invalid.
	/// @return e_outofmemory if memory allocation failed.
	result CreateVertexFormat( class CMuli3DVertexFormat **o_ppVertexFormat,
		const m3dvertexelement *i_pVertexDeclaration,
		uint32 i_iVertexDeclSize );

	/// Creates an index buffer for index storage.
	/// @param[out] o_ppIndexBuffer receives a pointer to the created index buffer.
	/// @param[in] i_iLength length of the index buffer to be created in bytes.
	/// @param[in] i_fmtFormat format of the index buffer to be created. Member of the enumeration m3dformat; either m3dfmt_index16 or m3dfmt_index32.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one or more parameters were invalid.
	/// @return e_outofmemory if memory allocation failed.
	result CreateIndexBuffer( class CMuli3DIndexBuffer **o_ppIndexBuffer,
		uint32 i_iLength, m3dformat i_fmtFormat );

	/// Creates a vertex buffer for vertex storage.
	/// @param[out] o_ppVertexBuffer receives a pointer to the created vertex buffer.
	/// @param[in] i_iLength length of the vertex buffer to be created in bytes.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one or more parameters were invalid.
	/// @return e_outofmemory if memory allocation failed.
	result CreateVertexBuffer( class CMuli3DVertexBuffer **o_ppVertexBuffer,
		uint32 i_iLength );

	/// Creates a surface.
	/// @param[out] o_ppSurface receives a pointer to the created surface.
	/// @param[in] i_iWidth width of the surface in pixels.
	/// @param[in] i_iHeight height of the surface in pixels.
	/// @param[in] i_fmtFormat format of the new surface. Member of the enumeration m3dformat; either m3dfmt_index16 or m3dfmt_index32.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one or more parameters were invalid.
	/// @return e_outofmemory if memory allocation failed.
	result CreateSurface( class CMuli3DSurface **o_ppSurface, uint32 i_iWidth,
		uint32 i_iHeight, m3dformat i_fmtFormat );

	/// Creates a standard 2d texture, which may either be used for texture data storage or as a target for rendering-operations (as frame- or depthbuffer).
	/// @param[out] o_ppTexture receives a pointer to the created texture.
	/// @param[in] i_iWidth width of the texture in pixels.
	/// @param[in] i_iHeight height of the texture in pixels.
	/// @param[in] i_iMipLevels number of miplevels of the new texture; specify 0 to create a full mip-chain.
	/// @param[in] i_fmtFormat format of the new texture. Member of the enumeration m3dformat; either m3dfmt_index16 or m3dfmt_index32.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one or more parameters were invalid.
	/// @return e_outofmemory if memory allocation failed.
	result CreateTexture( class CMuli3DTexture **o_ppTexture, uint32 i_iWidth,
		uint32 i_iHeight, uint32 i_iMipLevels, m3dformat i_fmtFormat );

	/// Creates a cube texture. A pointer to each of the 6 faces can be obtained and used as a target for renderin-operations like a standard 2d texture.
	/// @param[out] o_ppCubeTexture receives a pointer to the created texture.
	/// @param[in] i_iEdgeLength edge length of the texture in pixels.
	/// @param[in] i_iMipLevels number of miplevels of the new texture; specify 0 to create a full mip-chain.
	/// @param[in] i_fmtFormat format of the new texture. Member of the enumeration m3dformat; either m3dfmt_index16 or m3dfmt_index32.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one or more parameters were invalid.
	/// @return e_outofmemory if memory allocation failed.
	result CreateCubeTexture( class CMuli3DCubeTexture **o_ppCubeTexture,
		uint32 i_iEdgeLength, uint32 i_iMipLevels, m3dformat i_fmtFormat );

	/// Creates a volume.
	/// @param[out] o_ppVolume receives a pointer to the created volume.
	/// @param[in] i_iWidth width of the volume in pixels.
	/// @param[in] i_iHeight height of the volume in pixels.
	/// @param[in] i_iDepth depth of the volume in pixels.
	/// @param[in] i_fmtFormat format of the new surface. Member of the enumeration m3dformat; either m3dfmt_index16 or m3dfmt_index32.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one or more parameters were invalid.
	/// @return e_outofmemory if memory allocation failed.
	result CreateVolume( class CMuli3DVolume **o_ppVolume, uint32 i_iWidth,
		uint32 i_iHeight, uint32 i_iDepth, m3dformat i_fmtFormat );

	/// Creates a volume texture. Volume texture cannot be used as a target for rendering-operations.
	/// @param[out] o_ppVolumeTexture receives a pointer to the created texture.
	/// @param[in] i_iWidth width of the texture in pixels.
	/// @param[in] i_iHeight height of the texture in pixels.
	/// @param[in] i_iDepth depth of the texture in pixels.
	/// @param[in] i_iMipLevels number of miplevels of the new texture; specify 0 to create a full mip-chain.
	/// @param[in] i_fmtFormat format of the new texture. Member of the enumeration m3dformat; either m3dfmt_index16 or m3dfmt_index32.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one or more parameters were invalid.
	/// @return e_outofmemory if memory allocation failed.
	result CreateVolumeTexture( class CMuli3DVolumeTexture **o_ppVolumeTexture,
		uint32 i_iWidth, uint32 i_iHeight, uint32 i_iDepth,
		uint32 i_iMipLevels, m3dformat i_fmtFormat );

	/// Creates a render target.
	/// @param[out] o_ppVertexFormat receives a pointer to the created render target.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one or more parameters were invalid.
	/// @return e_outofmemory if memory allocation failed.
	result CreateRenderTarget( class CMuli3DRenderTarget **o_ppVertexFormat );

	// State management -------------------------------------------------------
	/// Sets a renderstate.
	/// @param[in] i_RenderState member of the enumeration m3drenderstate.
	/// @param[in] i_iValue value to be set. Some renderstates require floating point values, which can be set through casting to a uint32-pointer and dereferencing it.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one or more parameters were invalid.
	result SetRenderState( m3drenderstate i_RenderState, uint32 i_iValue );
	
	/// Retrieves the value of a renderstate.
	/// @param[in] i_RenderState member of the enumeration m3drenderstate.
	/// @param[out] o_iValue receives the value of the renderstate.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one or more parameters were invalid.
	result GetRenderState( m3drenderstate i_RenderState, uint32 &o_iValue );

	/// Sets the vertex format.
	/// @param[in] i_pVertexFormat pointer to the vertex format.
	/// @return s_ok if the function succeeds.
	result SetVertexFormat( class CMuli3DVertexFormat *i_pVertexFormat );
	class CMuli3DVertexFormat *pGetVertexFormat(); ///< Returns a pointer to the active vertex format. Calling this function will increase the internal reference count of the vertex format. Failure to call Release() when finished using the pointer will result in a memory leak.

	/// Sets the triangle assembler.
	/// @param[in] i_pPrimitiveAssembler pointer to the primitive assembler.
	void SetPrimitiveAssembler( class IMuli3DPrimitiveAssembler *i_pPrimitiveAssembler );
	class IMuli3DPrimitiveAssembler *pGetPrimitiveAssembler(); ///< Returns a pointer to the active triangle assembler. Calling this function will increase the internal reference count of the primitive assembler. Failure to call Release() when finished using the pointer will result in a memory leak.

	/// Sets the vertex shader.
	/// @param[in] i_pVertexShader pointer to the vertex shader.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidstate if the type of a vertex shader output register is invalid.
	result SetVertexShader( class IMuli3DVertexShader *i_pVertexShader );
	class IMuli3DVertexShader *pGetVertexShader(); ///< Returns a pointer to the active vertex shader. Calling this function will increase the internal reference count of the vertex shader. Failure to call Release() when finished using the pointer will result in a memory leak.

	/// Sets the triangle shader.
	/// @param[in] i_pTriangleShader pointer to the triangle shader.
	void SetTriangleShader( class IMuli3DTriangleShader *i_pTriangleShader );
	class IMuli3DTriangleShader *pGetTriangleShader(); ///< Returns a pointer to the active triangle shader. Calling this function will increase the internal reference count of the triangle shader. Failure to call Release() when finished using the pointer will result in a memory leak.

	/// Sets the pixel shader.
	/// @param[in] i_pPixelShader pointer to the pixel shader.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidstate if the type of pixelshader is invalid.
	result SetPixelShader( class IMuli3DPixelShader *i_pPixelShader );
	class IMuli3DPixelShader *pGetPixelShader(); ///< Returns a pointer to the active pixel shader. Calling this function will increase the internal reference count of the pixel shader. Failure to call Release() when finished using the pointer will result in a memory leak.

	/// Sets the index buffer.
	/// @param[in] i_pIndexBuffer pointer to the index buffer.
	/// @return s_ok if the function succeeds.
	result SetIndexBuffer( class CMuli3DIndexBuffer *i_pIndexBuffer );
	class CMuli3DIndexBuffer *pGetIndexBuffer(); ///< Returns a pointer to the active index buffer. Calling this function will increase the internal reference count of the index buffer. Failure to call Release() when finished using the pointer will result in a memory leak.

	/// Sets a vertex buffer to a given stream.
	/// @param[in] i_iStreamNumber number of the stream.
	/// @param[in] i_pVertexBuffer pointer to the vertex buffer.
	/// @param[in] i_iOffset offset from the start of the vertex buffer in bytes.
	/// @param[in] i_iStride stride in bytes.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one or more parameters were invalid.
	result SetVertexStream( uint32 i_iStreamNumber,
		class CMuli3DVertexBuffer *i_pVertexBuffer, uint32 i_iOffset,
		uint32 i_iStride );

	/// Returns a pointer to the active vertex buffer of a given stream. Calling this function will increase the internal reference count of the vertex buffer. Failure to call Release() when finished using the pointer will result in a memory leak.
	/// @param[in] i_iStreamNumber number of the stream.
	/// @param[out] o_ppVertexBuffer receives a pointer to the vertex buffer.
	/// @param[out] o_pOffset receives the offset from the start of the vertex buffer in bytes. (In case this value doesn't need to be retrieved, pass 0 as parameter.)
	/// @param[out] o_pStride receives the stride in bytes. (In case this value doesn't need to be retrieved, pass 0 as parameter.)
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one or more parameters were invalid.
	result GetVertexStream( uint32 i_iStreamNumber,
		class CMuli3DVertexBuffer **o_ppVertexBuffer, uint32 *o_pOffset,
		uint32 *o_pStride );

	/// Sets a vertex buffer to a given sampler.
	/// @param[in] i_iSamplerNumber number of the sampler.
	/// @param[in] i_pTexture pointer to the texture.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one or more parameters were invalid.
	result SetTexture( uint32 i_iSamplerNumber,
		class IMuli3DBaseTexture *i_pTexture );

	/// Returns a pointer to the active texture of a given sampler. Calling this function will increase the internal reference count of the texture. Failure to call Release() when finished using the pointer will result in a memory leak.
	/// @param[in] i_iSamplerNumber number of the sampler.
	/// @param[out] o_ppTexture receives a pointer to the texture.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one or more parameters were invalid.
	result GetTexture( uint32 i_iSamplerNumber,
		class IMuli3DBaseTexture **o_ppTexture );

	/// Sets a sampler state.
	/// @param[in] i_iSamplerNumber number of the sampler.
	/// @param[in] i_TextureSamplerState sampler state. Member of the enumeration m3dtexturesamplerstate.
	/// @param[in] i_iState value to be set. Some sampler states require floating point values, which can be set through casting to a uint32-pointer and dereferencing it.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one or more parameters were invalid.
	result SetTextureSamplerState( uint32 i_iSamplerNumber,
		m3dtexturesamplerstate i_TextureSamplerState, uint32 i_iState );

	/// Retrieves a sampler state.
	/// @param[in] i_iSamplerNumber number of the sampler.
	/// @param[in] i_TextureSamplerState sampler state. Member of the enumeration m3dtexturesamplerstate.
	/// @param[out] o_iState receives the value of the sampler state.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one or more parameters were invalid.
	result GetTextureSamplerState( uint32 i_iSamplerNumber,
		m3dtexturesamplerstate i_TextureSamplerState, uint32 &o_iState );

	/// Samples the texture and returns the looked-up color.
	/// @param[out] o_vColor receives the color of the pixel to be looked up.
	/// @param[in] i_iSamplerNumber number of the sampler.
	/// @param[in] i_fU u-component of the lookup-vector.
	/// @param[in] i_fV v-component of the lookup-vector.
	/// @param[in] i_fW w-component of the lookup-vector.
	/// @param[in] i_pXGradient partial derivatives of the texture coordinates with respect to the screen-space x coordinate. If 0 the base mip-level will be chosen and the minification filter will be used for texture sampling.
	/// @param[in] i_pYGradient partial derivatives of the texture coordinates with respect to the screen-space y coordinate. If 0 the base mip-level will be chosen and the minification filter will be used for texture sampling.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one or more parameters were invalid.
	result SampleTexture( vector4 &o_vColor, uint32 i_iSamplerNumber,
		float32 i_fU, float32 i_fV, float32 i_fW,
		const vector4 *i_pXGradient, const vector4 *i_pYGradient );

	/// Sets the render target.
	/// @param[in] i_pRenderTarget pointer to the render target.
	void SetRenderTarget( class CMuli3DRenderTarget *i_pRenderTarget );
	class CMuli3DRenderTarget *pGetRenderTarget(); ///< Returns a pointer to the active render target. Calling this function will increase the internal reference count of the render target. Failure to call Release() when finished using the pointer will result in a memory leak.

	/// Sets the scissor rect.
	/// @param[in] i_ScissorRect the scissor rect.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if the scissor rectangle is invalid.
	result SetScissorRect( const m3drect &i_ScissorRect );
	m3drect GetScissorRect(); ///< Returns the currently set scissor rect.

	/// Sets the bounding values for depth.
	/// @param[in] i_fMinZ minimum allowed depth value, e [0;1], default: 0.
	/// @param[in] i_fMaxZ maximum allowed depth value, e [0;1], default: 1.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if the bound values are invalid.
	result SetDepthBounds( float32 i_fMinZ, float32 i_fMaxZ );
	void GetDepthBounds( float32 &o_fMinZ, float32 &o_fMaxZ ); ///< Returns the currently set depth bounding values.

	/// Sets an user-specified clipping plane.
	/// @param[in] i_eIndex clipping plane index (member of the enumeration m3dclippingplanes) starting from m3dcp_user0.
	/// @param[in] i_pPlane pointer to clipping plane. Pass 0 to disable a clipping plane.
	/// @return e_invalidparameters if the index is invalid.
	result SetClippingPlane( m3dclippingplanes i_eIndex, const plane *i_pPlane );
	
	// Gets an user-specified clipping plane.
	/// @param[in] i_eIndex clipping plane index (member of the enumeration m3dclippingplanes) starting from m3dcp_user0.
	/// @param[out] o_plane reference to the output plane.
	/// @return e_invalidparameters if the index is invalid.
	/// @return e_invalidstate if the specified clipping has not been set.
	result GetClippingPlane( m3dclippingplanes i_eIndex, plane &o_plane );

	uint32 iGetRenderedPixels(); ///< Returns the number of pixels that passed the depth-test during the last Draw*Primitive() call.

private:
	void SetDefaultRenderStates();	///< Initializes renderstates to default values.
	void SetDefaultTextureSamplerStates();	///< Initializes samplerstates to default values.
	void SetDefaultClippingPlanes(); ///< Initializes the frustum clipping planes.

	/// Prepares internal structure with information used for rendering.
	/// Checks if all necessary objects (vertexbuffer, vertex format, etc.) have been set + if renderstates are valid.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidstate if an invalid state was encountered.
	result PreRender();

	/// Performs cleanup: Unlocking frame- and depthbuffer, etc.
	void PostRender();

	/// Loads data of a particular vertex from the vertex streams using the active vertex format as a description.
	/// @param[out] o_VertexShaderInput filled with vertex data from the streams.
	/// @param[in] i_iVertex index of the vertex.
	/// @return s_ok if the function succeeds.
	/// @return e_invalidparameters if one or more parameters were invalid.
	/// @return e_invalidstate if an invalid state was encountered.
	result DecodeVertexStream( m3dvsinput &o_VertexShaderInput,
		uint32 i_iVertex );

	/// Fetches a vertex from the current vertex streams and transforms it by calling the vertex shader.
	/// This function also takes care of caching transformed vertices.
	/// @param[in,out] io_ppVertex receives a pointer to the cache-entry holding the transformed vertex. (in-parameter, because a check is performed to see if the pointer already points to the desired vertex)
	/// @param[in] i_iVertex index of the vertex.
	result FetchVertex( m3dvertexcacheentry **io_ppVertex, uint32 i_iVertex );

	/// Begins the processing-pipeline that works on a per-triangle base. Either continues to the clipping-stage or takes care of subdivision.
	/// @param[in] i_pVSOutput0 vertex A.
	/// @param[in] i_pVSOutput1 vertex B.
	/// @param[in] i_pVSOutput2 vertex C.
	void ProcessTriangle( const m3dvsoutput *i_pVSOutput0,
		const m3dvsoutput *i_pVSOutput1, const m3dvsoutput *i_pVSOutput2 );

	/// Interpolates between two vertex shader inputs (used for subdivision).
	/// @param[out] o_pVSInput output.
	/// @param[in] i_pVSInputA input A.
	/// @param[in] i_pVSInputB input B.
	/// @param[in] i_fInterpolation interpolation value e [0;1].
	void InterpolateVertexShaderInput( m3dvsinput *o_pVSInput,
		const m3dvsinput *i_pVSInputA, const m3dvsinput *i_pVSInputB,
		float32 i_fInterpolation );

	/// Interpolates between two vertex shader outputs.
	/// @param[out] o_pVSOutput output.
	/// @param[in] i_pVSOutputA input A.
	/// @param[in] i_pVSOutputB input B.
	/// @param[in] i_fInterpolation interpolation value e [0;1].
	void InterpolateVertexShaderOutput( m3dvsoutput *o_pVSOutput,
		const m3dvsoutput *i_pVSOutputA, const m3dvsoutput *i_pVSOutputB,
		float32 i_fInterpolation );

	/// Multiples a vertex shader output's registers by a floating point value.
	/// @param[out] o_pDest vertex shader output destination.
	/// @param[in] i_pSrc vertex shader output source.
	/// @param[in] i_fVal floating point value to multiply registers with.
	void MultiplyVertexShaderOutputRegisters( m3dvsoutput *o_pDest, const m3dvsoutput *i_pSrc, float32 i_fVal );

	/// Performs simple-subdivision.
	/// @param[in] i_iSubdivisionLevel number of times to subdivide.
	/// @param[in] i_pVSOutput0 vertex A.
	/// @param[in] i_pVSOutput1 vertex B.
	/// @param[in] i_pVSOutput2 vertex C.
	void SubdivideTriangle_Simple( uint32 i_iSubdivisionLevel,
		const m3dvsoutput *i_pVSOutput0, const m3dvsoutput *i_pVSOutput1,
		const m3dvsoutput *i_pVSOutput2 );

	/// Performs smooth-subdivision.
	/// @param[in] i_iSubdivisionLevel number of times to subdivide.
	/// @param[in] i_pVSOutput0 vertex A.
	/// @param[in] i_pVSOutput1 vertex B.
	/// @param[in] i_pVSOutput2 vertex C.
	void SubdivideTriangle_Smooth( uint32 i_iSubdivisionLevel,
		const m3dvsoutput *i_pVSOutput0, const m3dvsoutput *i_pVSOutput1,
		const m3dvsoutput *i_pVSOutput2 );

	/// Performs adaptive-subdivision: Finds the triangle's center vertex and initiates splitting of triangle edges.
	/// @param[in] i_pVSOutput0 vertex A.
	/// @param[in] i_pVSOutput1 vertex B.
	/// @param[in] i_pVSOutput2 vertex C.
	void SubdivideTriangle_Adaptive( const m3dvsoutput *i_pVSOutput0,
		const m3dvsoutput *i_pVSOutput1, const m3dvsoutput *i_pVSOutput2 );

	/// Helper function for adaptive-subdivision: Recursively splits triangle edges.
	/// @param[in] i_iSubdivisionLevel number of times to subdivide.
	/// @param[in] i_pVSOutput0 vertex A.
	/// @param[in] i_pVSOutput1 vertex B.
	/// @param[in] i_pVSOutput2 vertex C.
	void SubdivideTriangle_Adaptive_SubdivideEdges( uint32 i_iSubdivisionLevel,
		const m3dvsoutput *i_pVSOutputEdge0, const m3dvsoutput *i_pVSOutputEdge1,
		const m3dvsoutput *i_pVSOutputCenter );
	
	/// Helper function for adaptive-subdivision: Recursively subdivides triangles until their screen-area falls below a user-defined threshold.
	/// @param[in] i_iSubdivisionLevel number of times to subdivide.
	/// @param[in] i_pVSOutput0 vertex A.
	/// @param[in] i_pVSOutput1 vertex B.
	/// @param[in] i_pVSOutput2 vertex C.
	void SubdivideTriangle_Adaptive_SubdivideInnerPart(
		uint32 i_iSubdivisionLevel, const m3dvsoutput *i_pVSOutput0,
		const m3dvsoutput *i_pVSOutput1, const m3dvsoutput *i_pVSOutput2 );
	
	/// iClipToPlane() clippes a polygon to the specified clipping plane.
	/// @param[in] i_iNumVertices number of vertices of the polygon to clip.
	/// @param[in] i_iStage stage in the clipping pipeline.
	/// @param[in] i_plane clipping plane.
	/// @param[in] i_bHomogenous if true the w-coordinate of vertex position is taken into account.
	/// @return number of vertices of the clipped polygon.
	uint32 iClipToPlane( uint32 i_iNumVertices, uint32 i_iStage,
		const plane &i_plane, bool i_bHomogenous );

	/// DrawTriangle() takes care of backface culling, triangle clipping, vertex projection and begins rasterization.
	/// @param[in] i_pVSOutput0 vertex A.
	/// @param[in] i_pVSOutput1 vertex B.
	/// @param[in] i_pVSOutput2 vertex C.
	void DrawTriangle( const m3dvsoutput *i_pVSOutput0,
		const m3dvsoutput *i_pVSOutput1, const m3dvsoutput *i_pVSOutput2 );
	
	/// Performs back face culling in screen space.
	/// @param[in] i_pVSOutput0 vertex A.
	/// @param[in] i_pVSOutput1 vertex B.
	/// @param[in] i_pVSOutput2 vertex C.
	bool bCullTriangle( const m3dvsoutput *i_pVSOutput0,
		const m3dvsoutput *i_pVSOutput1, const m3dvsoutput *i_pVSOutput2 );

	/// Projects a vertex and prepares it for interpolation during rasterization.
	/// @param[in,out] io_pVSOutput the vertex.
	void ProjectVertex( m3dvsoutput *io_pVSOutput );

	/// Calculates gradients for shader registers.
	/// @param[in] i_pVSOutput0 vertex A.
	/// @param[in] i_pVSOutput1 vertex B.
	/// @param[in] i_pVSOutput2 vertex C.
	void CalculateTriangleGradients( const m3dvsoutput *i_pVSOutput0,
		const m3dvsoutput *i_pVSOutput1, const m3dvsoutput *i_pVSOutput2 );

	/// Sets shader registers from triangle gradients.
	/// @param[in,out] io_pVSOutput vertex shader output.
	/// @param[in] i_fX screen space x-coordinate.
	/// @param[in] i_fY screen space y-coordinate.
	void SetVSOutputFromGradient( m3dvsoutput *o_pVSOutput, float32 i_fX, float32 i_fY );

	/// Updates shader registers from triangle gradients performing a step to the next pixel in the current scanline.
	/// @param[in,out] io_pVSOutput vertex shader output.
	void StepXVSOutputFromGradient( m3dvsoutput *io_pVSOutput );

	/// Rasterizes a single triangle: Performs triangle setup and does scanline-conversion.
	/// @param[in] i_pVSOutput0 vertex A.
	/// @param[in] i_pVSOutput1 vertex B.
	/// @param[in] i_pVSOutput2 vertex C.
	void RasterizeTriangle( const m3dvsoutput *i_pVSOutput0,
		const m3dvsoutput *i_pVSOutput1, const m3dvsoutput *i_pVSOutput2 );

	/// Rasterizes a line.
	/// @param[in] i_pVSOutput0 vertex A.
	/// @param[in] i_pVSOutput1 vertex B.
	void RasterizeLine( const m3dvsoutput *i_pVSOutput0, const m3dvsoutput *i_pVSOutput1 );

	/// Rasterizes a scanline span on screen. Writes the pixel color, which is outputted by the pixel shader, to the colorbuffer; writes the pixel depth, which has been interpolated from the base triangle's vertices to the depth buffer. Does not support pixel-killing.
	/// @param[in] i_iY position in rendertarget along y-axis.
	/// @param[in] i_iX left position in rendertarget along x-axis.
	/// @param[in] i_iX2 right position in rendertarget along x-axis.
	/// @param[in,out] io_pVSOutput interpolated vertex data.
	void RasterizeScanline_ColorOnly( uint32 i_iY,
		uint32 i_iX, uint32 i_iX2, m3dvsoutput *io_pVSOutput );

	/// Rasterizes a scanline span on screen. Writes the pixel color, which is outputted by the pixel shader, to the colorbuffer; writes the pixel depth, which has been interpolated from the base triangle's vertices to the depth buffer.
	/// @param[in] i_iY position in rendertarget along y-axis.
	/// @param[in] i_iX left position in rendertarget along x-axis.
	/// @param[in] i_iX2 right position in rendertarget along x-axis.
	/// @param[in,out] io_pVSOutput interpolated vertex data.
	void RasterizeScanline_ColorOnly_MightKillPixels( uint32 i_iY,
		uint32 i_iX, uint32 i_iX2, m3dvsoutput *io_pVSOutput );

	/// Rasterizes a scanline span on screen. Writes the pixel color, which is outputted by the pixel shader, to the colorbuffer; writes the pixel depth, which has been computed by the pixel shader to the depth buffer.
	/// @note Early depth-testing is disabled, which may lead to worse performance because regardless of the depth value the pixel shader will always be called for a given pixel.
	/// @param[in] i_iY position in rendertarget along y-axis.
	/// @param[in] i_iX left position in rendertarget along x-axis.
	/// @param[in] i_iX2 right position in rendertarget along x-axis.
	/// @param[in,out] io_pVSOutput interpolated vertex data.
	void RasterizeScanline_ColorDepth( uint32 i_iY,
		uint32 i_iX, uint32 i_iX2, m3dvsoutput *io_pVSOutput );

	/// Draws a single pixels. Writes the pixel color, which is outputted by the pixel shader, to the colorbuffer; writes the pixel depth, which has been interpolated from the vertices to the depth buffer. Does not support pixel-killing.
	/// @param[in] i_iX position in rendertarget along x-axis.
	/// @param[in] i_iY position in rendertarget along y-axis.
	/// @param[in] i_pVSOutput interpolated vertex data, already divided by position w component.
	void DrawPixel_ColorOnly( uint32 i_iX,
		uint32 i_iY, const m3dvsoutput *i_pVSOutput );

	/// Rasterizes a scanline span on screen. Writes the pixel color, which is outputted by the pixel shader, to the colorbuffer; writes the pixel depth, which has been computed by the pixel shader to the depth buffer.
	/// @note Early depth-testing is disabled, which may lead to worse performance because regardless of the depth value the pixel shader will always be called for a given pixel.
	/// @param[in] i_iX position in rendertarget along x-axis.
	/// @param[in] i_iY position in rendertarget along y-axis.
	/// @param[in] i_pVSOutput interpolated vertex data, already divided by position w component.
	void DrawPixel_ColorDepth( uint32 i_iX,
		uint32 i_iY, const m3dvsoutput *i_pVSOutput );

private:
	class CMuli3D				*m_pParent;			///< Pointer to parent.
	m3ddeviceparameters			m_DeviceParameters;	///< Device parameters, initialize at device-creation time.
	class IMuli3DPresentTarget	*m_pPresentTarget;	///< Base for rendering to screen.
	
	uint32	m_iRenderStates[m3drs_numrenderstates];	///< The renderstates.

	class CMuli3DVertexFormat		*m_pVertexFormat;		///< The vertex format.
	class IMuli3DPrimitiveAssembler	*m_pPrimitiveAssembler;	///< The primitive assembler; used for DrawDynamicPrimitive().
	class IMuli3DVertexShader		*m_pVertexShader;		///< The vertex shader.
	class IMuli3DTriangleShader		*m_pTriangleShader;		///< The triangle shader (optional).
	class IMuli3DPixelShader		*m_pPixelShader;		///< The pixel shader.
	class CMuli3DIndexBuffer		*m_pIndexBuffer;		///< The index buffer.
	
	/// @internal Describes a vertex stream.
	/// @note This structure is used internally by devices.
	struct vertexstream
	{
		class CMuli3DVertexBuffer *pVertexBuffer;	///< Pointer to the vertex buffer.
		uint32	iOffset;	///< Offset from the beginning of the vertex buffer in bytes.
		uint32	iStride;	///< Stride in bytes.
	} m_VertexStreams[c_iMaxVertexStreams];	///< The vertex streams;

	/// @internal Describes a texture sampler.
	/// @note This structure is used internally by devices.
	struct texturesampler
	{
		class IMuli3DBaseTexture *pTexture;		///< Pointer to the texture.
		uint32 iTextureSamplerStates[m3dtss_numtexturesamplerstates];	///< The samplers states.
		m3dtexsampleinput TextureSampleInput;	///< Type of texture coordinates for sampling.
	} m_TextureSamplers[c_iMaxTextureSamplers];	///< The texture samplers.
	
	class CMuli3DRenderTarget	*m_pRenderTarget;	///< The render target.

	m3drect	m_ScissorRect;	///< The active scissor rect.

	struct m3drenderinfo
	{
		m3dshaderregtype VSInputs[c_iVertexShaderRegisters]; ///< Holds information about the type of a particular input-register.
		m3dshaderregtype VSOutputs[c_iPixelShaderRegisters]; ///< Type of vertex shader output-registers.

		float32 *pFrameData;		///< Holds a pointer to the colorbuffer data.
		uint32 iColorFloats;		///< Number of floats in colorbuffer, e.g. 2 for a vector2-texture.
		uint32 iColorBufferPitch;	///< Colorbuffer width * number of floats; pitch in multiples of sizeof( float32 ).
		bool bColorWrite;			///< True if writing to the colorbuffer has been enabled + if a colorbuffer is available.

		float32 *pDepthData;		///< Holds a pointer to the depthbuffer data.
		uint32 iDepthBufferPitch;	///< Depthbuffer width * 1 (depthbuffers may only contain a single float); pitch in multiples of sizeof( float32 ).
		m3dcmpfunc DepthCompare;	///< Depth compare-function. If no depthbuffer is available this is m3dcmp_always.
		bool bDepthWrite;			///< True if writing to the depthbuffer has been enabled + if a depthbuffer is available.

		void (CMuli3DDevice::*fpRasterizeScanline)( uint32, uint32, uint32,
			m3dvsoutput * );	///< Rasterization-function for scanlines (triangle-drawing).

		void (CMuli3DDevice::*fpDrawPixel)( uint32, uint32, const m3dvsoutput * );	///< Drawing-function for individual pixels.

		uint32 iRenderedPixels;		///< Counts the number of pixels that pass the depth-test.

		m3drect ViewportRect;	///< Active viewport rectangle.

		plane ClippingPlanes[m3dcp_numplanes];	///< Planes used for clipping, frustum planes are initialized at device creation time.
		bool bClippingPlaneEnabled[m3dcp_numplanes]; ///< Signals if a particular clipping plane is enabled.
		plane ScissorPlanes[4];					///< Scissor planes used for clipping created from m_ScissorRect;

	} m_RenderInfo;	///< Contains information that serves as the base for rendering-processes.

	m3dtriangleinfo m_TriangleInfo; ///< Contains gradient information that serves as the base for scanline-conversion.

	uint32 m_iNumValidCacheEntries;	///< Number of valid vertex cache entries - reset before each draw-call.
	uint32 m_iFetchedVertices;		///< Amount of fetched vertices - reset before each draw-call.
	m3dvertexcacheentry m_VertexCache[c_iVertexCacheSize];	///< Vertex cache contents.

	m3dvsoutput m_ClipVertices[20];	///< Storage for vertices, that are created during clipping.
	uint32		m_iNextFreeClipVertex;	///< Keeps the next index of m_ClipVertices that can be used for the creation of vertices during clipping.
	m3dvsoutput *m_pClipVertices[2][20];	///< Pointers to polygon vertices, two stages: ping-pong during clipping.
};

#endif // __M3DCORE_DEVICE_H__
