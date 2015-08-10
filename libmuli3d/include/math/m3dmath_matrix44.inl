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

#ifndef __M3DMATH_MATRIX44_INL__
#define __M3DMATH_MATRIX44_INL__

#include "m3dmath_matrix44.h"
#include "m3dmath_vector4.h"

inline matrix44::matrix44() {}
inline matrix44::matrix44( const matrix44 &i_matVal )
	: _11( i_matVal._11 ), _12( i_matVal._12 ), _13( i_matVal._13 ), _14( i_matVal._14 ),
	_21( i_matVal._21 ), _22( i_matVal._22 ), _23( i_matVal._23 ), _24( i_matVal._24 ),
	_31( i_matVal._31 ), _32( i_matVal._32 ), _33( i_matVal._33 ), _34( i_matVal._34 ),
	_41( i_matVal._41 ), _42( i_matVal._42 ), _43( i_matVal._43 ), _44( i_matVal._44 )
{}
inline matrix44::matrix44( const float32 i_f11, const float32 i_f12, const float32 i_f13, const float32 i_f14,
	const float32 i_f21, const float32 i_f22, const float32 i_f23, const float32 i_f24,
	const float32 i_f31, const float32 i_f32, const float32 i_f33, const float32 i_f34,
	const float32 i_f41, const float32 i_f42, const float32 i_f43, const float32 i_f44 )
	: _11( i_f11 ), _12( i_f12 ), _13( i_f13 ), _14( i_f14 ),
	_21( i_f21 ), _22( i_f22 ), _23( i_f23 ), _24( i_f24 ),
	_31( i_f31 ), _32( i_f32 ), _33( i_f33 ), _34( i_f34 ),
	_41( i_f41 ), _42( i_f42 ), _43( i_f43 ), _44( i_f44 )
{}
inline matrix44::matrix44( const vector4 &i_vRow0, const vector4 &i_vRow1, const vector4 &i_vRow2, const vector4 &i_vRow3 )
{
	_11 = i_vRow0.x; _12 = i_vRow0.y; _13 = i_vRow0.z; _14 = i_vRow0.w;
	_21 = i_vRow1.x; _22 = i_vRow1.y; _23 = i_vRow1.z; _24 = i_vRow1.w;
	_31 = i_vRow2.x; _32 = i_vRow2.y; _33 = i_vRow2.z; _34 = i_vRow2.w;
	_41 = i_vRow3.x; _42 = i_vRow3.y; _43 = i_vRow3.z; _44 = i_vRow3.w;
}

inline const matrix44 &matrix44::operator =( const matrix44 &i_matVal )
{
	_11 = i_matVal._11; _12 = i_matVal._12; _13 = i_matVal._13; _14 = i_matVal._14;
	_21 = i_matVal._21; _22 = i_matVal._22; _23 = i_matVal._23; _24 = i_matVal._24;
	_31 = i_matVal._31; _32 = i_matVal._32; _33 = i_matVal._33; _34 = i_matVal._34;
	_41 = i_matVal._41; _42 = i_matVal._42; _43 = i_matVal._43; _44 = i_matVal._44;
	return *this;
}

inline matrix44::operator float32*() { return &_11; }
inline matrix44::operator const float32*() const { return &_11; }
inline float32 matrix44::operator()( const uint32 i_iRow, const uint32 i_iCol ) const { return m[i_iRow][i_iCol]; }

inline matrix44 matrix44::operator +() const { return *this; }

inline const matrix44 &matrix44::operator +=( const matrix44 &i_matVal )
{
	_11 += i_matVal._11; _12 += i_matVal._12; _13 += i_matVal._13; _14 += i_matVal._14;
	_21 += i_matVal._21; _22 += i_matVal._22; _23 += i_matVal._23; _24 += i_matVal._24;
	_31 += i_matVal._31; _32 += i_matVal._32; _33 += i_matVal._33; _34 += i_matVal._34;
	_41 += i_matVal._41; _42 += i_matVal._42; _43 += i_matVal._43; _44 += i_matVal._44;
	return *this;
}

inline const matrix44 &matrix44::operator -=( const matrix44 &i_matVal )
{
	_11 -= i_matVal._11; _12 -= i_matVal._12; _13 -= i_matVal._13; _14 -= i_matVal._14;
	_21 -= i_matVal._21; _22 -= i_matVal._22; _23 -= i_matVal._23; _24 -= i_matVal._24;
	_31 -= i_matVal._31; _32 -= i_matVal._32; _33 -= i_matVal._33; _34 -= i_matVal._34;
	_41 -= i_matVal._41; _42 -= i_matVal._42; _43 -= i_matVal._43; _44 -= i_matVal._44;
	return *this;
}

inline const matrix44 &matrix44::operator *=( const float32 i_fVal )
{
	_11 *= i_fVal; _12 *= i_fVal; _13 *= i_fVal; _14 *= i_fVal;
	_21 *= i_fVal; _22 *= i_fVal; _23 *= i_fVal; _24 *= i_fVal;
	_31 *= i_fVal; _32 *= i_fVal; _33 *= i_fVal; _34 *= i_fVal;
	_41 *= i_fVal; _42 *= i_fVal; _43 *= i_fVal; _44 *= i_fVal;
	return *this;
}

inline const matrix44 &matrix44::operator /=( const float32 i_fVal )
{
	const float32 fInvVal = 1.0f / i_fVal;
	_11 *= fInvVal; _12 *= fInvVal; _13 *= fInvVal; _14 *= fInvVal;
	_21 *= fInvVal; _22 *= fInvVal; _23 *= fInvVal; _24 *= fInvVal;
	_31 *= fInvVal; _32 *= fInvVal; _33 *= fInvVal; _34 *= fInvVal;
	_41 *= fInvVal; _42 *= fInvVal; _43 *= fInvVal; _44 *= fInvVal;
	return *this;
}

inline matrix44 matrix44::operator +( const matrix44 &i_matVal ) const
{
	return matrix44( _11 + i_matVal._11, _12 + i_matVal._12, _13 + i_matVal._13, _14 + i_matVal._14,
			 _21 + i_matVal._21, _22 + i_matVal._22, _23 + i_matVal._23, _24 + i_matVal._24,
			 _31 + i_matVal._31, _32 + i_matVal._32, _33 + i_matVal._33, _34 + i_matVal._34,
			 _41 + i_matVal._41, _42 + i_matVal._42, _43 + i_matVal._43, _44 + i_matVal._44 );
}

inline matrix44 matrix44::operator -( const matrix44 &i_matVal ) const
{
	return matrix44( _11 - i_matVal._11, _12 - i_matVal._12, _13 - i_matVal._13, _14 - i_matVal._14,
			 _21 - i_matVal._21, _22 - i_matVal._22, _23 - i_matVal._23, _24 - i_matVal._24,
			 _31 - i_matVal._31, _32 - i_matVal._32, _33 - i_matVal._33, _34 - i_matVal._34,
			 _41 - i_matVal._41, _42 - i_matVal._42, _43 - i_matVal._43, _44 - i_matVal._44 );
}

inline matrix44 matrix44::operator *( const float32 i_fVal ) const
{
	return matrix44( _11 * i_fVal, _12 * i_fVal, _13 * i_fVal, _14 * i_fVal,
			 _21 * i_fVal, _22 * i_fVal, _23 * i_fVal, _24 * i_fVal,
			 _31 * i_fVal, _32 * i_fVal, _33 * i_fVal, _34 * i_fVal,
			 _41 * i_fVal, _42 * i_fVal, _43 * i_fVal, _44 * i_fVal );
}

inline matrix44 matrix44::operator /( const float32 i_fVal ) const
{
	const float32 fInvVal = 1.0f / i_fVal;
	return matrix44( _11 * fInvVal, _12 * fInvVal, _13 * fInvVal, _14 * fInvVal,
			 _21 * fInvVal, _22 * fInvVal, _23 * fInvVal, _24 * fInvVal,
			 _31 * fInvVal, _32 * fInvVal, _33 * fInvVal, _34 * fInvVal,
			 _41 * fInvVal, _42 * fInvVal, _43 * fInvVal, _44 * fInvVal );
}

#endif // __M3DMATH_MATRIX44_INL__
