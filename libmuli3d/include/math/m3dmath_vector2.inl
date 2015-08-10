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

#ifndef __M3DMATH_VECTOR2_INL__
#define __M3DMATH_VECTOR2_INL__

#include "m3dmath_vector2.h"
#include "m3dmath_vector4.h"
#include "m3dmath_vector3.h"

inline vector2::vector2() {}
inline vector2::vector2( const vector2 &i_vVal )
	: x( i_vVal.x ), y( i_vVal.y )
{}
inline vector2::vector2( const float32 i_fX, const float32 i_fY )
	: x( i_fX ), y( i_fY )
{}

inline vector2::vector2( const struct vector4 &i_vVal )
{
	x = i_vVal.x; y = i_vVal.y;
}

inline vector2::vector2( const struct vector3 &i_vVal )
{
	x = i_vVal.x; y = i_vVal.y;
}

inline const vector2 &vector2::operator =( const vector2 &i_vVal )
{
	x = i_vVal.x; y = i_vVal.y;
	return *this;
}

inline const vector2 &vector2::operator =( const vector4 &i_vVal )
{
	x = i_vVal.x; y = i_vVal.y;
	return *this;
}

inline const vector2 &vector2::operator =( const vector3 &i_vVal )
{
	x = i_vVal.x; y = i_vVal.y;
	return *this;
}

inline const vector2 &vector2::operator =( const float32 i_fVal )
{
	x = i_fVal; y = 0.0f;
	return *this;
}

inline vector2::operator float32*() { return &x; }
inline vector2::operator const float32*() const { return &x; }

inline vector2 vector2::operator +() const { return *this; }
inline vector2 vector2::operator -() const { return vector2( -x, -y ); }

inline const vector2 &vector2::operator +=( const vector2 &i_vVal )
{
	x += i_vVal.x; y += i_vVal.y;
	return *this;
}

inline const vector2 &vector2::operator -=( const vector2 &i_vVal )
{
	x -= i_vVal.x; y -= i_vVal.y;
	return *this;
}

inline const vector2 &vector2::operator *=( const vector2 &i_vVal )
{
	x *= i_vVal.x; y *= i_vVal.y;
	return *this;
}

inline const vector2 &vector2::operator *=( const float32 i_fVal )
{
	x *= i_fVal; y *= i_fVal;
	return *this;
}

inline const vector2 &vector2::operator /=( const float32 i_fVal )
{
	const float32 fInvVal = 1.0f / i_fVal;
	x *= fInvVal; y *= fInvVal;
	return *this;
}

inline vector2 vector2::operator +( const vector2 &i_vVal ) const
{
	return vector2( x + i_vVal.x, y + i_vVal.y );
}

inline vector2 vector2::operator -( const vector2 &i_vVal ) const
{
	return vector2( x - i_vVal.x, y - i_vVal.y );
}

inline vector2 vector2::operator *( const vector2 &i_vVal ) const
{
	return vector2( x * i_vVal.x, y * i_vVal.y );
}

inline vector2 vector2::operator *( const float32 i_fVal ) const
{
	return vector2( x * i_fVal, y * i_fVal );
}

inline vector2 vector2::operator /( const float32 i_fVal ) const
{
	const float32 fInv = 1.0f / i_fVal;
	return vector2( x * fInv, y * fInv );
}

inline float32 vector2::length() const
{
	return sqrtf( x * x + y * y );
}

inline float32 vector2::lengthsq() const
{
	return x * x + y * y;
}

inline vector2 &vector2::normalize()
{
	const float32 fLength = length();
	//if( fLength >= FLT_EPSILON )
	{
		const float32 fInvLength = 1.0f / fLength;
		x *= fInvLength; y *= fInvLength;
	}
	return *this;
}

inline float32 fVector2Dot( const vector2 &i_vVecA, const vector2 &i_vVecB )
{
	return i_vVecA.x * i_vVecB.x + i_vVecA.y * i_vVecB.y;
}

inline vector2 &vVector2Lerp( vector2 &o_vVecOut, const vector2 &i_vVecA, const vector2 &i_vVecB, const float32 i_fInterpolation )
{
	o_vVecOut.x = i_vVecA.x + ( i_vVecB.x - i_vVecA.x ) * i_fInterpolation;
	o_vVecOut.y = i_vVecA.y + ( i_vVecB.y - i_vVecA.y ) * i_fInterpolation;
	return o_vVecOut;
}

#endif // __M3DMATH_VECTOR2_INL__
