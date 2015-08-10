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

#include "../../include/math/m3dmath_vector4.h"
#include "../../include/math/m3dmath_matrix44.h"

const vector4 &vector4::operator *=( const matrix44 &i_matVal )
{
	const float32 fX = i_matVal._11 * x + i_matVal._21 * y + i_matVal._31 * z + i_matVal._41 * w;
	const float32 fY = i_matVal._12 * x + i_matVal._22 * y + i_matVal._32 * z + i_matVal._42 * w;
	const float32 fZ = i_matVal._13 * x + i_matVal._23 * y + i_matVal._33 * z + i_matVal._43 * w;
	const float32 fW = i_matVal._14 * x + i_matVal._24 * y + i_matVal._34 * z + i_matVal._44 * w;
	x = fX; y = fY; z = fZ; w = fW;
	return *this;
}

vector4 vector4::operator *( const matrix44 &i_matVal ) const
{
	const float32 fX = i_matVal._11 * x + i_matVal._21 * y + i_matVal._31 * z + i_matVal._41 * w;
	const float32 fY = i_matVal._12 * x + i_matVal._22 * y + i_matVal._32 * z + i_matVal._42 * w;
	const float32 fZ = i_matVal._13 * x + i_matVal._23 * y + i_matVal._33 * z + i_matVal._43 * w;
	const float32 fW = i_matVal._14 * x + i_matVal._24 * y + i_matVal._34 * z + i_matVal._44 * w;
	return vector4( fX, fY, fZ, fW );
}
