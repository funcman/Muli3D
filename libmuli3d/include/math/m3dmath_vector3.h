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

/// @file m3dmath_vector3.h
///

#ifndef __M3DMATH_VECTOR3_H__
#define __M3DMATH_VECTOR3_H__

#include "m3dmath_common.h"

struct vector3
{
	union
	{
		struct { float32 x, y, z; };
		struct { float32 r, g, b; };
	};

	vector3();
	vector3( const vector3 &i_vVal );
	vector3( const float32 i_fX, const float32 i_fY, const float32 i_fZ );
	vector3( const struct vector4 &i_vVal ); ///< Sets this vector to vector3( i_vVal.x, i_vVal.y, i_vVal.z );
	vector3( const struct vector2 &i_vVal ); ///< Sets this vector to vector3( i_vVal.x, i_vVal.y, 0.0f );

	const vector3 &operator =( const vector3 &i_vVal );
	const vector3 &operator =( const struct vector4 &i_vVal ); ///< Sets this vector to vector3( i_vVal.x, i_vVal.y, i_vVal.z );
	const vector3 &operator =( const struct vector2 &i_vVal ); ///< Sets this vector to vector3( i_vVal.x, i_vVal.y, 0.0f );
	const vector3 &operator =( const float32 i_fVal ); ///< Sets this vector to vector3( i_fVal, 0.0f, 0.0f );

	operator float32*();
	operator const float32*() const;

	vector3 operator +() const;
	vector3 operator -() const;

	const vector3 &operator +=( const vector3 &i_vVal );
	const vector3 &operator -=( const vector3 &i_vVal );
	const vector3 &operator *=( const vector3 &i_vVal );
	const vector3 &operator *=( const struct matrix44 &i_matVal );
	const vector3 &operator *=( const float32 i_fVal );
	const vector3 &operator /=( const float32 i_fVal );

	vector3 operator +( const vector3 &i_vVal ) const;
	vector3 operator -( const vector3 &i_vVal ) const;
	vector3 operator *( const vector3 &i_vVal ) const;
	vector3 operator *( const struct matrix44 &i_matVal ) const;
	vector3 operator *( const float32 i_fVal ) const;
	vector3 operator /( const float32 i_fVal ) const;

	float32 length() const;
	float32 lengthsq() const;

	vector3 &normalize();

	vector3 &clamp( const float32 i_fLower, const float32 i_fUpper );
	vector3 &saturate();
};

vector3 &vVector3Cross( vector3 &o_vVecOut, const vector3 &i_vVecA, const vector3 &i_vVecB );
float32 fVector3Dot( const vector3 &i_vVecA, const vector3 &i_vVecB );
vector3 &vVector3Lerp( vector3 &o_vVecOut, const vector3 &i_vVecA, const vector3 &i_vVecB, const float32 i_fInterpolation );
vector3 &vVector3TransformNormal( vector3 &o_vVecOut, const vector3 &i_vVector, const struct matrix44 &i_matVal );

#include "m3dmath_vector3.inl"

#endif // __M3DMATH_VECTOR3_H__
