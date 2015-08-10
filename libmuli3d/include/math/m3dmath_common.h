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

/// @file m3dmath_common.h
///

#ifndef __M3DMATH_COMMON_H__
#define __M3DMATH_COMMON_H__

#include "../m3dbase.h"
#include <float.h>
#include <math.h>

#define M3D_PI 3.141592654f ///< Pi

/// Converts radians to degrees.
/// @param[in] i_fVal radians.
/// @return degrees.
inline float32 fRadToDeg( const float32 i_fVal )
{
	return 180.0f * i_fVal / M3D_PI;
}

/// Converts degrees to radians.
/// @param[in] i_fVal degrees.
/// @return radians.
inline float32 fDegToRad( const float32 i_fVal )
{
	return M3D_PI * i_fVal / 180.0f;
}

/// Clamps a floating-point value.
/// @param[in] i_fVal value to clamp.
/// @param[in] i_fLower minimum value.
/// @param[in] i_fUpper maximum value.
/// @return clamped value e [i_fLower,i_fUpper].
inline float32 fClamp( const float32 i_fVal, const float32 i_fLower,
			    const float32 i_fUpper )
{
	if( i_fVal <= i_fLower ) return i_fLower;
	else if( i_fVal >= i_fUpper ) return i_fUpper;
	else return i_fVal;
}

/// Clamps an integer value.
/// @param[in] i_iVal value to clamp.
/// @param[in] i_iLower minimum value.
/// @param[in] i_iUpper maximum value.
/// @return clamped value e [i_iLower,i_iUpper].
inline int32 iClamp( const int32 i_iVal, const int32 i_iLower,
			  const int32 i_iUpper )
{
	if( i_iVal <= i_iLower ) return i_iLower;
	else if( i_iVal >= i_iUpper ) return i_iUpper;
	else return i_iVal;
}

/// Clamps a floating-point value to [0.0f,1.0f].
/// @param[in] i_fVal value to saturate.
/// @return saturated value e [0.0f,1.0f].
inline float32 fSaturate( const float32 i_fVal )
{
	return fClamp( i_fVal, 0.0f, 1.0f );
}

/// Linearly interpolates between two values
/// @param[in] i_fValA first value.
/// @param[in] i_fValB second value.
/// @param[in] i_fInterpolation interpolation factor e [0.0f,1.0f].
/// @return interpolated value.
inline float32 fLerp( const float32 i_fValA, const float32 i_fValB,
			   const float32 i_fInterpolation )
{
	return i_fValA + ( i_fValB - i_fValA ) * i_fInterpolation;
}

#endif // __M3DMATH_COMMON_H__
