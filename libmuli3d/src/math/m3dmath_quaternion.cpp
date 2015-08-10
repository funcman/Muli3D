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

#include "../../include/math/m3dmath_quaternion.h"
#include "../../include/math/m3dmath_matrix44.h"
#include "../../include/math/m3dmath_vector3.h"

quaternion &qQuaternionIdentity( quaternion &o_qQuatOut )
{
	o_qQuatOut.x = o_qQuatOut.y = o_qQuatOut.z = 0.0f;
	o_qQuatOut.w = 1.0f;
	return o_qQuatOut;
}

quaternion &qQuaternionRotationMatrix( quaternion &o_qQuatOut, const matrix44 &i_matMatrix )
{
	// http://www.gamasutra.com/features/19980703/quaternions_01.htm
//TODO
	const float32 fDiagonal = i_matMatrix._11 + i_matMatrix._22 + i_matMatrix._33;
	if( fDiagonal > 0.0f )
	{
		float32 s = sqrtf( fDiagonal + 1.0f );
		o_qQuatOut.w = s / 2.0f;
		s = 0.5f / s;
		o_qQuatOut.x = ( i_matMatrix._32 - i_matMatrix._23 ) * s;
		o_qQuatOut.y = ( i_matMatrix._13 - i_matMatrix._31 ) * s;
		o_qQuatOut.z = ( i_matMatrix._21 - i_matMatrix._12 ) * s;
	}
	else
	{
		const uint32 iNext[3] = {1, 2, 0};

		float32 q[4];
		uint32 i = 0;
		if( i_matMatrix._22 > i_matMatrix._11 ) i = 1;
		if( i_matMatrix._33 > i_matMatrix( i, i ) ) i = 2;
		uint32 j = iNext[i];
		uint32 k = iNext[j];

		float32 s = sqrtf( i_matMatrix( i, i ) - ( i_matMatrix( j, j ) + i_matMatrix( k, k ) ) + 1.0f );
		q[i] = s * 0.5f;

		if( s >= FLT_EPSILON )
			s = 0.5f / s;

		q[3] = ( i_matMatrix( k, j ) - i_matMatrix( j, k ) ) * s;
		q[j] = ( i_matMatrix( j, i ) + i_matMatrix( i, j) ) * s;
		q[k] = ( i_matMatrix( k, i ) + i_matMatrix( i, k) ) * s;

		o_qQuatOut.x = q[0];
		o_qQuatOut.y = q[1];
		o_qQuatOut.z = q[2];
		o_qQuatOut.w = q[3];
	}

	return o_qQuatOut;
}

quaternion &qQuaternionSLerp( quaternion &o_qQuatOut, const quaternion &i_qQuatA, const quaternion &i_qQuatB, const float32 i_fLerp )
{
	// http://www.gamasutra.com/features/19980703/quaternions_01.htm

	// calc cosine
	float32 fCosine = i_qQuatA.x * i_qQuatB.x + i_qQuatA.y * i_qQuatB.y + i_qQuatA.z * i_qQuatB.z + i_qQuatA.w * i_qQuatB.w;

	// adjust signs (if necessary)
	float32 to1[4];
	if( fCosine < 0.0f )
	{
		fCosine = -fCosine;
		to1[0] = -i_qQuatB.x;
		to1[1] = -i_qQuatB.y;
		to1[2] = -i_qQuatB.z;
		to1[3] = -i_qQuatB.w;
	}
	else
	{
		to1[0] = i_qQuatB.x;
		to1[1] = i_qQuatB.y;
		to1[2] = i_qQuatB.z;
		to1[3] = i_qQuatB.w;
	}

	const float32 fOmega = acosf( fCosine );
	const float32 fInvSine = 1.0f / sinf( fOmega );
	const float32 fScale0 = sinf( ( 1.0f - i_fLerp ) * fOmega ) * fInvSine;
	const float32 fScale1 = sinf( i_fLerp * fOmega ) * fInvSine;

	// Calculate final values
	o_qQuatOut.x = fScale0 * i_qQuatB.x + fScale1 * to1[0];
	o_qQuatOut.y = fScale0 * i_qQuatB.y + fScale1 * to1[1];
	o_qQuatOut.z = fScale0 * i_qQuatB.z + fScale1 * to1[2];
	o_qQuatOut.w = fScale0 * i_qQuatB.w + fScale1 * to1[3];

	return o_qQuatOut;
}

void QuaternionToAxisAngle( quaternion &i_qQuat, vector3 &o_vAxis, float32 &o_fAngle )
{
	const float32 fScale = sqrtf( i_qQuat.x * i_qQuat.x + i_qQuat.y * i_qQuat.y + i_qQuat.z * i_qQuat.z );
	if( fScale >= FLT_EPSILON )
	{
		const float32 fInvScale = 1.0f / fScale;
		o_vAxis.x = i_qQuat.x * fInvScale;
		o_vAxis.y = i_qQuat.y * fInvScale;
		o_vAxis.z = i_qQuat.z * fInvScale;
	}
	else
	{
		o_vAxis.x = 0.0f;
		o_vAxis.y = 1.0f;
		o_vAxis.z = 0.0f;
	}

	o_fAngle = 2.0f * acosf( i_qQuat.w );
}
