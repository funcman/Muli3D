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

#ifndef __M3DMATH_PLANE_INL__
#define __M3DMATH_PLANE_INL__

#include "m3dmath_plane.h"

inline plane::plane() {}
inline plane::plane( const plane &i_vVal )
	: normal( i_vVal.normal ), d( i_vVal.d )
{}
inline plane::plane( const float32 i_fA, const float32 i_fB, const float32 i_fC, const float32 i_fD )
	: d( i_fD )
{
	normal = vector3( i_fA, i_fB, i_fC );
}
inline plane::plane( const vector3 &i_vNormal, const float32 i_fD )
	: normal( i_vNormal ), d( i_fD )
{}

inline const plane &plane::operator =( const plane &i_vVal )
{
	normal = i_vVal.normal;
	d = i_vVal.d;
	return *this;
}

inline plane::operator float32*() { return &normal.x; }
inline plane::operator const float32*() const { return &normal.x; }

inline plane plane::operator +() const { return *this; }
inline plane plane::operator -() const { return plane( -normal, d ); }

inline float32 plane::operator *( const vector3 &i_vVal ) const
{
	return fVector3Dot( normal, i_vVal ) + d;
}

inline float32 plane::operator *( const vector4 &i_vVal ) const
{
	return fVector3Dot( normal, (vector3)i_vVal ) + d * i_vVal.w;
}

#endif // __M3DMATH_PLANE_INL__
