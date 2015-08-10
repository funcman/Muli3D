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

/// @file m3dmath_vector4.h
///

#ifndef __M3DMATH_VECTOR4_H__
#define __M3DMATH_VECTOR4_H__

#include "m3dmath_common.h"

struct vector4
{
	union
	{
		struct { float32 x, y, z, w; };
		struct { float32 r, g, b, a; };
	};

	vector4();
	vector4( const vector4 &i_vVal );
	vector4( const float32 i_fX, const float32 i_fY, const float32 i_fZ, const float32 i_fW );
	vector4( const struct vector3 &i_vVal ); ///< Sets this vector to vector4( i_vVal.x, i_vVal.y, i_vVal.z, 0.0f );
	vector4( const struct vector2 &i_vVal ); ///< Sets this vector to vector4( i_vVal.x, i_vVal.y, 0.0f, 0.0f );

	const vector4 &operator =( const vector4 &i_vVal );
	const vector4 &operator =( const struct vector3 &i_vVal ); ///< Sets this vector to vector4( i_vVal.x, i_vVal.y, i_vVal.z, 0.0f );
	const vector4 &operator =( const struct vector2 &i_vVal ); ///< Sets this vector to vector4( i_vVal.x, i_vVal.y, 0.0f, 0.0f );
	const vector4 &operator =( const float32 i_fVal ); ///< Sets this vector to vector4( i_fVal, 0.0f, 0.0f, 0.0f );

	operator float32*();
	operator const float32*() const;

	vector4 operator +() const;
	vector4 operator -() const;

	const vector4 &operator +=( const vector4 &i_vVal );
	const vector4 &operator -=( const vector4 &i_vVal );
	const vector4 &operator *=( const vector4 &i_vVal );
	const vector4 &operator *=( const struct matrix44 &i_matVal );
	const vector4 &operator *=( const float32 i_fVal );
	const vector4 &operator /=( const float32 i_fVal );

	vector4 operator +( const vector4 &i_vVal ) const;
	vector4 operator -( const vector4 &i_vVal ) const;
	vector4 operator *( const vector4 &i_vVal ) const;
	vector4 operator *( const struct matrix44 &i_matVal ) const;
	vector4 operator *( const float32 i_fVal ) const;
	vector4 operator /( const float32 i_fVal ) const;

	float32 length() const;
	float32 lengthsq() const;

	vector4 &normalize();
	vector4 &homogenize(); ///< Divides the vector by its w-coordinate.

	vector4 &clamp( const float32 i_fLower, const float32 i_fUpper );
	vector4 &saturate();
};

float32 fVector4Dot( const vector4 &i_vVecA, const vector4 &i_vVecB );
vector4 &vVector4Lerp( vector4 &o_vVecOut, const vector4 &i_vVecA, const vector4 &i_vVecB, const float32 i_fInterpolation );

#include "m3dmath_vector4.inl"

#endif // __M3DMATH_VECTOR4_H__
