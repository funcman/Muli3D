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

#ifndef __M3DMATH_VECTOR3_INL__
#define __M3DMATH_VECTOR3_INL__

#include "m3dmath_vector3.h"
#include "m3dmath_vector4.h"
#include "m3dmath_vector2.h"

inline vector3::vector3() {}
inline vector3::vector3( const vector3 &i_vVal )
	: x( i_vVal.x ), y( i_vVal.y ), z( i_vVal.z )
{}
inline vector3::vector3( const float32 i_fX, const float32 i_fY, const float32 i_fZ )
	: x( i_fX ), y( i_fY ), z( i_fZ )
{}

inline vector3::vector3( const struct vector4 &i_vVal )
{
	x = i_vVal.x; y = i_vVal.y;
	z = i_vVal.z;
}

inline vector3::vector3( const struct vector2 &i_vVal )
{
	x = i_vVal.x; y = i_vVal.y;
	z = 0.0f;
}

inline const vector3 &vector3::operator =( const vector3 &i_vVal )
{
	x = i_vVal.x; y = i_vVal.y; z = i_vVal.z;
	return *this;
}

inline const vector3 &vector3::operator=( const vector4 &i_vVal )
{
	x = i_vVal.x; y = i_vVal.y; z = i_vVal.z;
	return *this;
}

inline const vector3 &vector3::operator =( const vector2 &i_vVal )
{
	x = i_vVal.x; y = i_vVal.y; z = 0.0f;
	return *this;
}

inline const vector3 &vector3::operator =( const float32 i_fVal )
{
	x = i_fVal; y = 0.0f; z = 0.0f;
	return *this;
}

inline vector3::operator float32*() { return &x; }
inline vector3::operator const float32*() const { return &x; }

inline vector3 vector3::operator +() const { return *this; }
inline vector3 vector3::operator -() const { return vector3( -x, -y, -z ); }

inline const vector3 &vector3::operator +=( const vector3 &i_vVal )
{
	x += i_vVal.x; y += i_vVal.y; z += i_vVal.z;
	return *this;
}

inline const vector3 &vector3::operator -=( const vector3 &i_vVal )
{
	x -= i_vVal.x; y -= i_vVal.y; z -= i_vVal.z;
	return *this;
}

inline const vector3 &vector3::operator *=( const vector3 &i_vVal )
{
	x *= i_vVal.x; y *= i_vVal.y; z *= i_vVal.z;
	return *this;
}

inline const vector3 &vector3::operator *=( const struct matrix44 &i_matVal )
{
	*this = ( vector4( x, y, z, 1 ) * i_matVal ).homogenize();
	return *this;
}

inline const vector3 &vector3::operator *=( const float32 i_fVal )
{
	x *= i_fVal; y *= i_fVal; z *= i_fVal;
	return *this;
}

inline const vector3 &vector3::operator /=( const float32 i_fVal )
{
	const float32 fInvVal = 1.0f / i_fVal;
	x *= fInvVal; y *= fInvVal; z *= fInvVal;
	return *this;
}

inline vector3 vector3::operator +( const vector3 &i_vVal ) const
{
	return vector3( x + i_vVal.x, y + i_vVal.y, z + i_vVal.z );
}

inline vector3 vector3::operator -( const vector3 &i_vVal ) const
{
	return vector3( x - i_vVal.x, y - i_vVal.y, z - i_vVal.z );
}

inline vector3 vector3::operator *( const vector3 &i_vVal ) const
{
	return vector3( x * i_vVal.x, y * i_vVal.y, z * i_vVal.z );
}

inline vector3 vector3::operator *( const struct matrix44 &i_matVal ) const
{
	return ( vector4( x, y, z, 1 ) * i_matVal ).homogenize();
}

inline vector3 vector3::operator *( const float32 i_fVal ) const
{
	return vector3( x * i_fVal, y * i_fVal, z * i_fVal );
}

inline vector3 vector3::operator /( const float32 i_fVal ) const
{
	const float32 fInv = 1.0f / i_fVal;
	return vector3( x * fInv, y * fInv, z * fInv );
}

inline float32 vector3::length() const
{
	return sqrtf( x * x + y * y + z * z );
}

inline float32 vector3::lengthsq() const
{
	return x * x + y * y + z * z;
}

inline vector3 &vector3::normalize()
{
	const float32 fLength = length();
	//if( fLength >= FLT_EPSILON )
	{
		const float32 fInvLength = 1.0f / fLength;
		x *= fInvLength; y *= fInvLength; z *= fInvLength;
	}
	return *this;
}

inline vector3 &vector3::clamp( const float32 i_fLower, const float32 i_fUpper )
{
	x = fClamp( x, i_fLower, i_fUpper );
	y = fClamp( y, i_fLower, i_fUpper );
	z = fClamp( z, i_fLower, i_fUpper );
	return *this;
}

inline vector3 &vector3::saturate() { return clamp( 0, 1 ); }

inline vector3 &vVector3Cross( vector3 &o_vVecOut, const vector3 &i_vVecA, const vector3 &i_vVecB )
{
	o_vVecOut.x = i_vVecA.y * i_vVecB.z - i_vVecA.z * i_vVecB.y;
	o_vVecOut.y = i_vVecA.z * i_vVecB.x - i_vVecA.x * i_vVecB.z;
	o_vVecOut.z = i_vVecA.x * i_vVecB.y - i_vVecA.y * i_vVecB.x;
	return o_vVecOut;
}

inline float32 fVector3Dot( const vector3 &i_vVecA, const vector3 &i_vVecB )
{
	return i_vVecA.x * i_vVecB.x + i_vVecA.y * i_vVecB.y + i_vVecA.z * i_vVecB.z;
}

inline vector3 &vVector3Lerp( vector3 &o_vVecOut, const vector3 &i_vVecA, const vector3 &i_vVecB, const float32 i_fInterpolation )
{
	o_vVecOut.x = i_vVecA.x + ( i_vVecB.x - i_vVecA.x ) * i_fInterpolation;
	o_vVecOut.y = i_vVecA.y + ( i_vVecB.y - i_vVecA.y ) * i_fInterpolation;
	o_vVecOut.z = i_vVecA.z + ( i_vVecB.z - i_vVecA.z ) * i_fInterpolation;
	return o_vVecOut;
}

inline vector3 &vVector3TransformNormal( vector3 &o_vVecOut, const vector3 &i_vVector, const struct matrix44 &i_matVal )
{
	o_vVecOut = vector4( i_vVector.x, i_vVector.y, i_vVector.z, 0 ) * i_matVal;
	return o_vVecOut;
}

#endif // __M3DMATH_VECTOR3_INL__
