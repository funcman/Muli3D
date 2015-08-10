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

/// @file m3dmath_plane.h
///

#ifndef __M3DMATH_PLANE_H__
#define __M3DMATH_PLANE_H__

#include "m3dmath_common.h"
#include "m3dmath_vector3.h"
#include "m3dmath_vector4.h"

struct plane
{
	vector3 normal;
	float32 d;

	plane();
	plane( const plane &i_vVal );
	plane( const float32 i_fA, const float32 i_fB, const float32 i_fC, const float32 i_fD );
	plane( const struct vector3 &i_vNormal, const float32 i_fD );

	const plane &operator =( const plane &i_vVal );

	operator float32*();
	operator const float32*() const;

	plane operator +() const;
	plane operator -() const; ///< Returns the plane with negated normal.

	float32 operator *( const struct vector3 &i_vVal ) const; ///< Calculates the dot product with the vector.
	float32 operator *( const struct vector4 &i_vVal ) const; ///< Calculates the dot product with the vector.
};

#include "m3dmath_plane.inl"

#endif // __M3DMATH_PLANE_H__
