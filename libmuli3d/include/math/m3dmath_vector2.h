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

/// @file m3dmath_vector2.h
///

#ifndef __M3DMATH_VECTOR2_H__
#define __M3DMATH_VECTOR2_H__

#include "m3dmath_common.h"

struct vector2
{
	float32 x, y;

	vector2();
	vector2( const vector2 &i_vVal );
	vector2( const float32 i_fX, const float32 i_fY );
	vector2( const struct vector4 &i_vVal ); ///< Sets this vector to vector2( i_vVal.x, i_vVal.y );
	vector2( const struct vector3 &i_vVal ); ///< Sets this vector to vector2( i_vVal.x, i_vVal.y );

	const vector2 &operator =( const vector2 &i_vVal );
	const vector2 &operator =( const struct vector4 &i_vVal ); ///< Sets this vector to vector2( i_vVal.x, i_vVal.y );
	const vector2 &operator =( const struct vector3 &i_vVal ); ///< Sets this vector to vector2( i_vVal.x, i_vVal.y );
	const vector2 &operator =( const float32 i_fVal ); ///< Sets this vector to vector2( i_fVal, 0.0f );

	operator float32*();
	operator const float32*() const;

	vector2 operator +() const;
	vector2 operator -() const;

	const vector2 &operator +=( const vector2 &i_vVal );
	const vector2 &operator -=( const vector2 &i_vVal );
	const vector2 &operator *=( const vector2 &i_vVal );
	const vector2 &operator *=( const float32 i_fVal );
	const vector2 &operator /=( const float32 i_fVal );

	vector2 operator +( const vector2 &i_vVal ) const;
	vector2 operator -( const vector2 &i_vVal ) const;
	vector2 operator *( const vector2 &i_vVal ) const;
	vector2 operator *( const float32 i_fVal ) const;
	vector2 operator /( const float32 i_fVal ) const;

	float32 length() const;
	float32 lengthsq() const;

	vector2 &normalize();
};

float32 fVector2Dot( const vector2 &i_vVecA, const vector2 &i_vVecB );
vector2 &vVector2Lerp( vector2 &o_vVecOut, const vector2 &i_vVecA, const vector2 &i_vVecB, const float32 i_fInterpolation );

#include "m3dmath_vector2.inl"

#endif // __M3DMATH_VECTOR2_H__
