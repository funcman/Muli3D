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

#ifndef __M3DMATH_QUATERNION_INL__
#define __M3DMATH_QUATERNION_INL__

#include "m3dmath_quaternion.h"

inline quaternion::quaternion() {}
inline quaternion::quaternion( const quaternion &i_vVal )
	: x( i_vVal.x ), y( i_vVal.y ), z( i_vVal.z ), w( i_vVal.w )
{}
inline quaternion::quaternion( const float32 i_fX, const float32 i_fY, const float32 i_fZ, const float32 i_fW )
	: x( i_fX ), y( i_fY ), z( i_fZ ), w( i_fW )
{}

inline const quaternion &quaternion::operator =( const quaternion &i_vVal )
{
	x = i_vVal.x; y = i_vVal.y;
	z = i_vVal.z; w = i_vVal.w;
	return *this;
}

inline quaternion::operator float32*() { return &x; }
inline quaternion::operator const float32*() const { return &x; }

inline quaternion quaternion::operator +() const { return *this; }
inline quaternion quaternion::operator -() const { return quaternion( -x, -y, -z, w ); }

inline const quaternion &quaternion::operator +=( const quaternion &i_vVal )
{
	x += i_vVal.x; y += i_vVal.y;
	z += i_vVal.z; w += i_vVal.w;
	return *this;
}

inline const quaternion &quaternion::operator -=( const quaternion &i_vVal )
{
	x -= i_vVal.x; y -= i_vVal.y;
	z -= i_vVal.z; w -= i_vVal.w;
	return *this;
}

inline const quaternion &quaternion::operator *=( const quaternion &i_qVal )
{
	quaternion qResult;
	qResult.x = w * i_qVal.x + x * i_qVal.w + y * i_qVal.z - z * i_qVal.y;
	qResult.y = w * i_qVal.y - x * i_qVal.z + y * i_qVal.w + z * i_qVal.x;
	qResult.z = w * i_qVal.z + x * i_qVal.y - y * i_qVal.x + z * i_qVal.w;
	qResult.w = w * i_qVal.w - x * i_qVal.x - y * i_qVal.y - z * i_qVal.z;
	*this = qResult;

	return *this;
}

inline const quaternion &quaternion::operator *=( const float32 i_fVal )
{
	x *= i_fVal; y *= i_fVal;
	z *= i_fVal; w *= i_fVal;
	return *this;
}

inline const quaternion &quaternion::operator /=( const float32 i_fVal )
{
	const float32 fInvVal = 1.0f / i_fVal;
	x *= fInvVal; y *= fInvVal;
	z *= fInvVal; w *= fInvVal;
	return *this;
}

inline quaternion quaternion::operator +( const quaternion &i_vVal ) const
{
	return quaternion( x + i_vVal.x, y + i_vVal.y, z + i_vVal.z, w + i_vVal.w );
}

inline quaternion quaternion::operator -( const quaternion &i_vVal ) const
{
	return quaternion( x - i_vVal.x, y - i_vVal.y, z - i_vVal.z, w - i_vVal.w );
}

inline quaternion quaternion::operator *( const quaternion &i_qVal ) const
{
	quaternion qResult;
	qResult.x = w * i_qVal.x + x * i_qVal.w + y * i_qVal.z - z * i_qVal.y;
	qResult.y = w * i_qVal.y - x * i_qVal.z + y * i_qVal.w + z * i_qVal.x;
	qResult.z = w * i_qVal.z + x * i_qVal.y - y * i_qVal.x + z * i_qVal.w;
	qResult.w = w * i_qVal.w - x * i_qVal.x - y * i_qVal.y - z * i_qVal.z;
	return qResult;
}

inline quaternion quaternion::operator *( const float32 i_fVal ) const
{
	return quaternion( x * i_fVal, y * i_fVal, z * i_fVal, w * i_fVal );
}

inline quaternion quaternion::operator /( const float32 i_fVal ) const
{
	const float32 fInv = 1.0f / i_fVal;
	return quaternion( x * fInv, y * fInv, z * fInv, w * fInv );
}

inline float32 quaternion::length() const
{
	return sqrtf( x * x + y * y + z * z + w * w );
}

inline float32 quaternion::lengthsq() const
{
	return x * x + y * y + z * z + w * w;
}

inline quaternion &quaternion::normalize()
{
	const float32 fLength = length();
	//if( fLength >= FLT_EPSILON )
	{
		const float32 fInvLength = 1.0f / fLength;
		x *= fInvLength; y *= fInvLength;
		z *= fInvLength; w *= fInvLength;
	}
	return *this;
}

#endif // __M3DMATH_QUATERNION_INL__
