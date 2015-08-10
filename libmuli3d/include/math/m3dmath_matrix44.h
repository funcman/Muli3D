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

/// @file m3dmath_matrix44.h
///

#ifndef __M3DMATH_MATRIX44_H__
#define __M3DMATH_MATRIX44_H__

#include "m3dmath_common.h"
#include "m3dmath_vector4.h"

struct matrix44
{
	union
	{
        struct
		{
            float32 _11, _12, _13, _14;
            float32 _21, _22, _23, _24;
            float32 _31, _32, _33, _34;
            float32 _41, _42, _43, _44;
        };
        struct { float32 m[4][4]; };
	};

	matrix44();
	matrix44( const matrix44 &i_matVal );
	matrix44( const float32 i_f11, const float32 i_f12, const float32 i_f13, const float32 i_f14,
		const float32 i_f21, const float32 i_f22, const float32 i_f23, const float32 i_f24,
		const float32 i_f31, const float32 i_f32, const float32 i_f33, const float32 i_f34,
		const float32 i_f41, const float32 i_f42, const float32 i_f43, const float32 i_f44 );
	matrix44( const struct vector4 &i_vRow0, const struct vector4 &i_vRow1, const struct vector4 &i_vRow2, const struct vector4 &i_vRow3 );

	const matrix44 &operator =( const matrix44 &i_matVal );

	operator float32*();
	operator const float32*() const;
	float32 operator()( const uint32 i_iRow, const uint32 i_iCol ) const;

	matrix44 operator +() const;
	matrix44 operator -() const; ///< Returns the inverse of the matrix.

	const matrix44 &operator +=( const matrix44 &i_matVal );
	const matrix44 &operator -=( const matrix44 &i_matVal );
	const matrix44 &operator *=( const matrix44 &i_matVal );
	const matrix44 &operator *=( const float32 i_fVal );
	const matrix44 &operator /=( const float32 i_fVal );

	matrix44 operator +( const matrix44 &i_matVal ) const;
	matrix44 operator -( const matrix44 &i_matVal ) const;
	matrix44 operator *( const matrix44 &i_matVal ) const;
	matrix44 operator *( const float32 i_fVal ) const;
	matrix44 operator /( const float32 i_fVal ) const;

	float32 determinant() const;
};

matrix44 &matMatrix44Transpose( matrix44 &o_matMatOut, const matrix44 i_matMat );
matrix44 &matMatrix44Identity( matrix44 &o_matMatOut );
matrix44 &matMatrix44Scaling( matrix44 &o_matMatOut, const float32 i_fX,
							 const float32 i_fY, const float32 i_fZ );
matrix44 &matMatrix44Scaling( matrix44 &o_matMatOut,
							 const struct vector3 &i_vScale );
matrix44 &matMatrix44Translation( matrix44 &o_matMatOut,
								 const float32 i_fX, const float32 i_fY,
								 const float32 i_fZ );
matrix44 &matMatrix44Translation( matrix44 &o_matMatOut,
								 const struct vector3 &i_vTranslation );
matrix44 &matMatrix44RotationX( matrix44 &o_matMatOut, const float32 i_fRot );
matrix44 &matMatrix44RotationY( matrix44 &o_matMatOut, const float32 i_fRot );
matrix44 &matMatrix44RotationZ( matrix44 &o_matMatOut, const float32 i_fRot );
matrix44 &matMatrix44RotationYawPitchRoll( matrix44 &o_matMatOut,
										  const float32 i_fYaw,
										  const float32 i_fPitch,
										  const float32 i_fRoll );
matrix44 &matMatrix44RotationYawPitchRoll( matrix44 &o_matMatOut,
										  const struct vector3 &i_vRot );
matrix44 &matMatrix44RotationAxis( matrix44 &o_matMatOut,
								  const struct vector3 &i_vAxis, const float32 i_fRot );
matrix44 &matMatrix44RotationQuaternion( matrix44 &o_matMatOut,
										const struct quaternion &i_qQuat );
matrix44 &matMatrix44LookAtLH( matrix44 &o_matMatOut,
							  const struct vector3 &i_vEye,
							  const struct vector3 &i_vAt,
							  const struct vector3 &i_vUp );
matrix44 &matMatrix44LookAtRH( matrix44 &o_matMatOut,
							  const struct vector3 &i_vEye,
							  const struct vector3 &i_vAt,
							  const struct vector3 &i_vUp );
matrix44 &matMatrix44OrthoLH( matrix44 &o_matMatOut, const float32 i_fWidth,
							 const float32 i_fHeight, const float32 i_fZNear,
							 const float32 i_fZFar );
matrix44 &matMatrix44OrthoRH( matrix44 &o_matMatOut, const float32 i_fWidth,
							 const float32 i_fHeight, const float32 i_fZNear,
							 const float32 i_fZFar );
matrix44 &matMatrix44OrthoOffCenterLH( matrix44 &o_matMatOut, const float32 i_fLeft,
									  const float32 i_fRight, const float32 i_fBottom,
									  const float32 i_fTop, const float32 i_fZNear,
									  const float32 i_fZFar );
matrix44 &matMatrix44OrthoOffCenterRH( matrix44 &o_matMatOut, const float32 i_fLeft,
									  const float32 i_fRight, const float32 i_fBottom,
									  const float32 i_fTop, const float32 i_fZNear,
									  const float32 i_fZFar );
matrix44 &matMatrix44PerspectiveFovLH( matrix44 &o_matMatOut, const float32 i_fFOVY,
									  const float32 i_fAspect, const float32 i_fZNear,
									  const float32 i_fZFar );
matrix44 &matMatrix44PerspectiveFovRH( matrix44 &o_matMatOut, const float32 i_fFOVY,
									  const float32 i_fAspect, const float32 i_fZNear,
									  const float32 i_fZFar );
matrix44 &matMatrix44PerspectiveLH( matrix44 &o_matMatOut, const float32 i_fWidth,
								   const float32 i_fHeight, const float32 i_fZNear,
								   const float32 i_fZFar );
matrix44 &matMatrix44PerspectiveRH( matrix44 &o_matMatOut, const float32 i_fWidth,
								   const float32 i_fHeight, const float32 i_fZNear,
								   const float32 i_fZFar );
matrix44 &matMatrix44PerspectiveOffCenterLH( matrix44 &o_matMatOut,
											const float32 i_fLeft,
											const float32 i_fRight,
											const float32 i_fBottom,
											const float32 i_fTop,
											const float32 i_fZNear,
											const float32 i_fZFar );
matrix44 &matMatrix44PerspectiveOffCenterRH( matrix44 &o_matMatOut,
											const float32 i_fLeft,
											const float32 i_fRight,
											const float32 i_fBottom,
											const float32 i_fTop,
											const float32 i_fZNear,
											const float32 i_fZFar );
matrix44 &matMatrix44Viewport( matrix44 &o_matMatOut, const uint32 i_iX,
							  const uint32 i_iY, const uint32 i_iWidth,
							  const uint32 i_iHeight, const float32 i_fZNear,
							  const float32 i_fZFar );

#include "m3dmath_matrix44.inl"

#endif // __M3DMATH_MATRIX44_H__
