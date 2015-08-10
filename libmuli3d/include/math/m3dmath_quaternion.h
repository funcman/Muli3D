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

/// @file m3dmath_quaternion.h
///

#ifndef __M3DMATH_QUATERNION_H__
#define __M3DMATH_QUATERNION_H__

#include "m3dmath_common.h"

struct quaternion
{
	float32 x, y, z, w;

	quaternion();
	quaternion( const quaternion &i_vVal );
	quaternion( const float32 i_fX, const float32 i_fY, const float32 i_fZ, const float32 i_fW );

	const quaternion &operator =( const quaternion &i_vVal );

	operator float32*();
	operator const float32*() const;

	quaternion operator +() const;
	quaternion operator -() const; ///< Returns the quaternion with negative rotation.

	const quaternion &operator +=( const quaternion &i_vVal );
	const quaternion &operator -=( const quaternion &i_vVal );
	const quaternion &operator *=( const quaternion &i_qVal );
	const quaternion &operator *=( const float32 i_fVal );
	const quaternion &operator /=( const float32 i_fVal );

	quaternion operator +( const quaternion &i_vVal ) const;
	quaternion operator -( const quaternion &i_vVal ) const;
	quaternion operator *( const quaternion &i_qVal ) const;
	quaternion operator *( const float32 i_fVal ) const;
	quaternion operator /( const float32 i_fVal ) const;

	float32 length() const;
	float32 lengthsq() const;

	quaternion &normalize();
};

quaternion &qQuaternionIdentity( quaternion &o_qQuatOut );
quaternion &qQuaternionRotationMatrix( quaternion &o_qQuatOut,
									  const struct matrix44 &i_matMatrix );
quaternion &qQuaternionSLerp( quaternion &o_qQuatOut, const quaternion &i_qQuatA,
							 const quaternion &i_qQuatB, const float32 i_fLerp );
void QuaternionToAxisAngle( quaternion &i_qQuat, struct vector3 &o_vAxis,
						   float32 &o_fAngle );

#include "m3dmath_quaternion.inl"

#endif // __M3DMATH_QUATERNION_H__
