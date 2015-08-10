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

#include "../../include/math/m3dmath_matrix44.h"
#include "../../include/math/m3dmath_vector3.h"
#include "../../include/math/m3dmath_quaternion.h"

const matrix44 &matrix44::operator *=( const matrix44 &i_vVal )
{
	const float32 f11 = _11 * i_vVal._11 + _12 * i_vVal._21 + _13 * i_vVal._31 + _14 * i_vVal._41;
	const float32 f12 = _11 * i_vVal._12 + _12 * i_vVal._22 + _13 * i_vVal._32 + _14 * i_vVal._42;
	const float32 f13 = _11 * i_vVal._13 + _12 * i_vVal._23 + _13 * i_vVal._33 + _14 * i_vVal._43;
	const float32 f14 = _11 * i_vVal._14 + _12 * i_vVal._24 + _13 * i_vVal._34 + _14 * i_vVal._44;
	const float32 f21 = _21 * i_vVal._11 + _22 * i_vVal._21 + _23 * i_vVal._31 + _24 * i_vVal._41;
	const float32 f22 = _21 * i_vVal._12 + _22 * i_vVal._22 + _23 * i_vVal._32 + _24 * i_vVal._42;
	const float32 f23 = _21 * i_vVal._13 + _22 * i_vVal._23 + _23 * i_vVal._33 + _24 * i_vVal._43;
	const float32 f24 = _21 * i_vVal._14 + _22 * i_vVal._24 + _23 * i_vVal._34 + _24 * i_vVal._44;
	const float32 f31 = _31 * i_vVal._11 + _32 * i_vVal._21 + _33 * i_vVal._31 + _34 * i_vVal._41;
	const float32 f32 = _31 * i_vVal._12 + _32 * i_vVal._22 + _33 * i_vVal._32 + _34 * i_vVal._42;
	const float32 f33 = _31 * i_vVal._13 + _32 * i_vVal._23 + _33 * i_vVal._33 + _34 * i_vVal._43;
	const float32 f34 = _31 * i_vVal._14 + _32 * i_vVal._24 + _33 * i_vVal._34 + _34 * i_vVal._44;
	const float32 f41 = _41 * i_vVal._11 + _42 * i_vVal._21 + _43 * i_vVal._31 + _44 * i_vVal._41;
	const float32 f42 = _41 * i_vVal._12 + _42 * i_vVal._22 + _43 * i_vVal._32 + _44 * i_vVal._42;
	const float32 f43 = _41 * i_vVal._13 + _42 * i_vVal._23 + _43 * i_vVal._33 + _44 * i_vVal._43;
	const float32 f44 = _41 * i_vVal._14 + _42 * i_vVal._24 + _43 * i_vVal._34 + _44 * i_vVal._44;

	_11 = f11; _12 = f12; _13 = f13; _14 = f14;
	_21 = f21; _22 = f22; _23 = f23; _24 = f24;
	_31 = f31; _32 = f32; _33 = f33; _34 = f34;
	_41 = f41; _42 = f42; _43 = f43; _44 = f44;
	return *this;
}

matrix44 matrix44::operator *( const matrix44 &i_vVal ) const
{
	return matrix44( _11 * i_vVal._11 + _12 * i_vVal._21 + _13 * i_vVal._31 + _14 * i_vVal._41,
		_11 * i_vVal._12 + _12 * i_vVal._22 + _13 * i_vVal._32 + _14 * i_vVal._42,
		_11 * i_vVal._13 + _12 * i_vVal._23 + _13 * i_vVal._33 + _14 * i_vVal._43,
		_11 * i_vVal._14 + _12 * i_vVal._24 + _13 * i_vVal._34 + _14 * i_vVal._44,
		_21 * i_vVal._11 + _22 * i_vVal._21 + _23 * i_vVal._31 + _24 * i_vVal._41,
		_21 * i_vVal._12 + _22 * i_vVal._22 + _23 * i_vVal._32 + _24 * i_vVal._42,
		_21 * i_vVal._13 + _22 * i_vVal._23 + _23 * i_vVal._33 + _24 * i_vVal._43,
		_21 * i_vVal._14 + _22 * i_vVal._24 + _23 * i_vVal._34 + _24 * i_vVal._44,
		_31 * i_vVal._11 + _32 * i_vVal._21 + _33 * i_vVal._31 + _34 * i_vVal._41,
		_31 * i_vVal._12 + _32 * i_vVal._22 + _33 * i_vVal._32 + _34 * i_vVal._42,
		_31 * i_vVal._13 + _32 * i_vVal._23 + _33 * i_vVal._33 + _34 * i_vVal._43,
		_31 * i_vVal._14 + _32 * i_vVal._24 + _33 * i_vVal._34 + _34 * i_vVal._44,
		_41 * i_vVal._11 + _42 * i_vVal._21 + _43 * i_vVal._31 + _44 * i_vVal._41,
		_41 * i_vVal._12 + _42 * i_vVal._22 + _43 * i_vVal._32 + _44 * i_vVal._42,
		_41 * i_vVal._13 + _42 * i_vVal._23 + _43 * i_vVal._33 + _44 * i_vVal._43,
		_41 * i_vVal._14 + _42 * i_vVal._24 + _43 * i_vVal._34 + _44 * i_vVal._44 );
}

// matrix-inverse functions ---------------------------------------------------
static inline float32 fDeterminant2x2( const float32 i_fA, const float32 i_fB, const float32 i_fC, const float32 i_fD )
{
	return i_fA * i_fD - i_fB * i_fC;
}

static float32 fDeterminant3x3( const float32 i_fA1, const float32 i_fA2, const float32 i_fA3,
						 const float32 i_fB1, const float32 i_fB2, const float32 i_fB3,
						 const float32 i_fC1, const float32 i_fC2, const float32 i_fC3 )
{
	// src: http://www.acm.org/pubs/tog/GraphicsGems/gems/MatrixInvert.c
	return i_fA1 * fDeterminant2x2( i_fB2, i_fB3, i_fC2, i_fC3 )
		- i_fB1 * fDeterminant2x2( i_fA2, i_fA3, i_fC2, i_fC3 )
		+ i_fC1 * fDeterminant2x2( i_fA2, i_fA3, i_fB2, i_fB3 );
}

static float32 fMinorDeterminant( const matrix44 &i_matMatrix, const uint32 i_iRow, const uint32 i_iColumn )
{
	// src: http://www.codeproject.com/csharp/Matrix.asp
	float32 fMat3x3[3][3];

	for( uint32 r = 0, m = 0; r < 4; ++r )
	{
		if( r == i_iRow )
			continue;

		for( uint32 c = 0, n = 0; c < 4; ++c )
		{
			if( c == i_iColumn )
				continue;

			fMat3x3[m][n] = i_matMatrix.m[r][c];

			++n;
		}

		++m;
	}

	return fDeterminant3x3( fMat3x3[0][0], fMat3x3[0][1], fMat3x3[0][2],
		fMat3x3[1][0], fMat3x3[1][1], fMat3x3[1][2], fMat3x3[2][0], fMat3x3[2][1], fMat3x3[2][2] );
}

static matrix44 matAdjoint( const matrix44 *i_pMat )
{
	// src: http://www.codeproject.com/csharp/Matrix.asp
	matrix44 matReturn;
	for( uint32 r = 0; r < 4; ++r )
	{
		for( uint32 c = 0; c < 4; ++c )
			matReturn.m[c][r] = powf( -1.0f, (float32)(r + c) ) * fMinorDeterminant( *i_pMat, r, c );
	}
	return matReturn;
}
// matrix-inverse functions ---------------------------------------------------

float32 matrix44::determinant() const
{
	// src: http://www.acm.org/pubs/tog/GraphicsGems/gems/MatrixInvert.c
	return _11 * fDeterminant3x3( _22, _32, _42, _23, _33, _43, _24, _34, _44 )
    - _12 * fDeterminant3x3( _21, _31, _41, _23, _33, _43, _24, _34, _44 )
    + _13 * fDeterminant3x3( _21, _31, _41, _22, _32, _42, _24, _34, _44 )
    - _14 * fDeterminant3x3( _21, _31, _41, _22, _32, _42, _23, _33, _43 );
}

matrix44 matrix44::operator -() const
{
	const float32 fDeterminant = determinant();
	if( fabsf( fDeterminant ) < FLT_EPSILON )
		return *this;

	return matAdjoint( this ) / fDeterminant;
}

matrix44 &matMatrix44Transpose( matrix44 &o_matMatOut, const matrix44 i_matMat ) // pass i_matMat by value so we can do pMatrix44Transpose( &amat, amat );
{
	o_matMatOut._11 = i_matMat._11; o_matMatOut._12 = i_matMat._21; o_matMatOut._13 = i_matMat._31; o_matMatOut._14 = i_matMat._41;
	o_matMatOut._21 = i_matMat._12; o_matMatOut._22 = i_matMat._22; o_matMatOut._23 = i_matMat._32; o_matMatOut._24 = i_matMat._42;
	o_matMatOut._31 = i_matMat._13; o_matMatOut._32 = i_matMat._23; o_matMatOut._33 = i_matMat._33; o_matMatOut._34 = i_matMat._43;
	o_matMatOut._41 = i_matMat._14; o_matMatOut._42 = i_matMat._24; o_matMatOut._43 = i_matMat._34; o_matMatOut._44 = i_matMat._44;
	return o_matMatOut;
}

matrix44 &matMatrix44Identity( matrix44 &o_matMatOut )
{
	o_matMatOut._11 = 1.0f; o_matMatOut._12 = 0.0f; o_matMatOut._13 = 0.0f; o_matMatOut._14 = 0.0f;
	o_matMatOut._21 = 0.0f; o_matMatOut._22 = 1.0f; o_matMatOut._23 = 0.0f; o_matMatOut._24 = 0.0f;
	o_matMatOut._31 = 0.0f; o_matMatOut._32 = 0.0f; o_matMatOut._33 = 1.0f; o_matMatOut._34 = 0.0f;
	o_matMatOut._41 = 0.0f; o_matMatOut._42 = 0.0f; o_matMatOut._43 = 0.0f; o_matMatOut._44 = 1.0f;
	return o_matMatOut;
}

matrix44 &matMatrix44Scaling( matrix44 &o_matMatOut, const float32 i_fX, const float32 i_fY, const float32 i_fZ )
{
	o_matMatOut._11 = i_fX; o_matMatOut._12 = 0.0f; o_matMatOut._13 = 0.0f; o_matMatOut._14 = 0.0f;
	o_matMatOut._21 = 0.0f; o_matMatOut._22 = i_fY; o_matMatOut._23 = 0.0f; o_matMatOut._24 = 0.0f;
	o_matMatOut._31 = 0.0f; o_matMatOut._32 = 0.0f; o_matMatOut._33 = i_fZ; o_matMatOut._34 = 0.0f;
	o_matMatOut._41 = 0.0f; o_matMatOut._42 = 0.0f; o_matMatOut._43 = 0.0f; o_matMatOut._44 = 1.0f;
	return o_matMatOut;
}

matrix44 &matMatrix44Scaling( matrix44 &o_matMatOut, const vector3 &i_vScale )
{
	return matMatrix44Scaling( o_matMatOut, i_vScale.x, i_vScale.y, i_vScale.z );
}

matrix44 &matMatrix44Translation( matrix44 &o_matMatOut, const float32 i_fX, const float32 i_fY, const float32 i_fZ )
{
	o_matMatOut._11 = 1.0f; o_matMatOut._12 = 0.0f; o_matMatOut._13 = 0.0f; o_matMatOut._14 = 0.0f;
	o_matMatOut._21 = 0.0f; o_matMatOut._22 = 1.0f; o_matMatOut._23 = 0.0f; o_matMatOut._24 = 0.0f;
	o_matMatOut._31 = 0.0f; o_matMatOut._32 = 0.0f; o_matMatOut._33 = 1.0f; o_matMatOut._34 = 0.0f;
	o_matMatOut._41 = i_fX; o_matMatOut._42 = i_fY; o_matMatOut._43 = i_fZ; o_matMatOut._44 = 1.0f;
	return o_matMatOut;
}

matrix44 &matMatrix44Translation( matrix44 &o_matMatOut, const vector3 &i_vTranslation )
{
	return matMatrix44Translation( o_matMatOut, i_vTranslation.x, i_vTranslation.y, i_vTranslation.z );
}

matrix44 &matMatrix44RotationX( matrix44 &o_matMatOut, const float32 i_fRot )
{
	const float32 fSin = sinf( i_fRot ), fCos = cosf( i_fRot );
	o_matMatOut._11 = 1.0f; o_matMatOut._12 = 0.0f; o_matMatOut._13 = 0.0f; o_matMatOut._14 = 0.0f;
	o_matMatOut._21 = 0.0f; o_matMatOut._22 = fCos; o_matMatOut._23 = fSin; o_matMatOut._24 = 0.0f;
	o_matMatOut._31 = 0.0f; o_matMatOut._32 = -fSin; o_matMatOut._33 = fCos; o_matMatOut._34 = 0.0f;
	o_matMatOut._41 = 0.0f; o_matMatOut._42 = 0.0f; o_matMatOut._43 = 0.0f; o_matMatOut._44 = 1.0f;
	return o_matMatOut;
}

matrix44 &matMatrix44RotationY( matrix44 &o_matMatOut, const float32 i_fRot )
{
	const float32 fSin = sinf( i_fRot ), fCos = cosf( i_fRot );
	o_matMatOut._11 = fCos; o_matMatOut._12 = 0.0f; o_matMatOut._13 = -fSin; o_matMatOut._14 = 0.0f;
	o_matMatOut._21 = 0.0f; o_matMatOut._22 = 1.0f; o_matMatOut._23 = 0.0f; o_matMatOut._24 = 0.0f;
	o_matMatOut._31 = fSin; o_matMatOut._32 = 0.0f; o_matMatOut._33 = fCos; o_matMatOut._34 = 0.0f;
	o_matMatOut._41 = 0.0f; o_matMatOut._42 = 0.0f; o_matMatOut._43 = 0.0f; o_matMatOut._44 = 1.0f;
	return o_matMatOut;
}

matrix44 &matMatrix44RotationZ( matrix44 &o_matMatOut, const float32 i_fRot )
{
	const float32 fSin = sinf( i_fRot ), fCos = cosf( i_fRot );
	o_matMatOut._11 = fCos; o_matMatOut._12 = fSin; o_matMatOut._13 = 0.0f; o_matMatOut._14 = 0.0f;
	o_matMatOut._21 = -fSin; o_matMatOut._22 = fCos; o_matMatOut._23 = 0.0f; o_matMatOut._24 = 0.0f;
	o_matMatOut._31 = 0.0f; o_matMatOut._32 = 0.0f; o_matMatOut._33 = 1.0f; o_matMatOut._34 = 0.0f;
	o_matMatOut._41 = 0.0f; o_matMatOut._42 = 0.0f; o_matMatOut._43 = 0.0f; o_matMatOut._44 = 1.0f;
	return o_matMatOut;
}

matrix44 &matMatrix44RotationYawPitchRoll( matrix44 &o_matMatOut, const float32 i_fYaw, const float32 i_fPitch, const float32 i_fRoll )
{
	matrix44 matYaw, matPitch, matRoll;
	matMatrix44RotationY( matYaw, i_fYaw );
	matMatrix44RotationX( matPitch, i_fPitch );
	matMatrix44RotationZ( matRoll, i_fRoll );
	o_matMatOut = matRoll * matPitch * matYaw;
	return o_matMatOut;
}

matrix44 &matMatrix44RotationYawPitchRoll( matrix44 &o_matMatOut, const vector3 &i_vRot )
{
	return matMatrix44RotationYawPitchRoll( o_matMatOut, i_vRot.x, i_vRot.y, i_vRot.z );
}

matrix44 &matMatrix44RotationAxis( matrix44 &o_matMatOut, const vector3 &i_vAxis, const float32 i_fRot )
{
	// make sure incoming axis is normalized!
	// http://www.euclideanspace.com/maths/algebra/matrix/orthogonal/rotation/openforum.htm

	const float32 fSin = sinf( i_fRot ), fCos = cosf( i_fRot );
	const float32 fInvCos = 1.0f - fCos;

	o_matMatOut._11 = fInvCos * i_vAxis.x * i_vAxis.x + fCos;
	o_matMatOut._12 = fInvCos * i_vAxis.x * i_vAxis.y - i_vAxis.z * fSin;
	o_matMatOut._13 = fInvCos * i_vAxis.x * i_vAxis.z + i_vAxis.y * fSin;
	o_matMatOut._14 = 0.0f;

	o_matMatOut._21 = fInvCos * i_vAxis.x * i_vAxis.y + i_vAxis.z * fSin;
	o_matMatOut._22 = fInvCos * i_vAxis.y * i_vAxis.y + fCos;
	o_matMatOut._23 = fInvCos * i_vAxis.y * i_vAxis.z - i_vAxis.x * fSin;
	o_matMatOut._24 = 0.0f;

	o_matMatOut._31 = fInvCos * i_vAxis.x * i_vAxis.z - i_vAxis.y * fSin;
	o_matMatOut._32 = fInvCos * i_vAxis.y * i_vAxis.z + i_vAxis.x * fSin;
	o_matMatOut._33 = fInvCos * i_vAxis.z * i_vAxis.z + fCos;
	o_matMatOut._34 = 0.0f;

	o_matMatOut._41 = 0.0f; o_matMatOut._42 = 0.0f; o_matMatOut._43 = 0.0f; o_matMatOut._44 = 1.0f;
	return o_matMatOut;
}

matrix44 &matMatrix44RotationQuaternion( matrix44 &o_matMatOut, const struct quaternion &i_qQuat )
{
	// transposed version from http://www.gamedev.net/reference/articles/article1199.asp

	const float32 fSquaredX = i_qQuat.x * i_qQuat.x;
	const float32 fSquaredY = i_qQuat.y * i_qQuat.y;
	const float32 fSquaredZ = i_qQuat.z * i_qQuat.z;
	const float32 fSquaredW = i_qQuat.w * i_qQuat.w;

	o_matMatOut._11 = fSquaredW + fSquaredX - fSquaredY - fSquaredZ;
	o_matMatOut._12 = 2.0f * ( i_qQuat.x * i_qQuat.y - i_qQuat.w * i_qQuat.z );
	o_matMatOut._13 = 2.0f * ( i_qQuat.x * i_qQuat.z + i_qQuat.w * i_qQuat.y );
	o_matMatOut._14 = 0.0f;

	o_matMatOut._21 = 2.0f * ( i_qQuat.x * i_qQuat.y + i_qQuat.w * i_qQuat.z );
	o_matMatOut._22 = fSquaredW - fSquaredX + fSquaredY - fSquaredZ;
	o_matMatOut._23 = 2.0f * ( i_qQuat.y * i_qQuat.z - i_qQuat.w * i_qQuat.x );
	o_matMatOut._24 = 0.0f;

	o_matMatOut._31 = 2.0f * ( i_qQuat.x * i_qQuat.z - i_qQuat.w * i_qQuat.y );
	o_matMatOut._32 = 2.0f * ( i_qQuat.y * i_qQuat.z + i_qQuat.w * i_qQuat.x );
	o_matMatOut._33 = fSquaredW - fSquaredX - fSquaredY + fSquaredZ;
	o_matMatOut._34 = 0.0f;

	o_matMatOut._41 = 0.0f; o_matMatOut._42 = 0.0f; o_matMatOut._43 = 0.0f;
	o_matMatOut._44 = fSquaredW + fSquaredX + fSquaredY + fSquaredZ;

	return o_matMatOut;
}

matrix44 &matMatrix44LookAtLH( matrix44 &o_matMatOut, const vector3 &i_vEye, const vector3 &i_vAt, const vector3 &i_vUp )
{
	const vector3 vZAxis = (i_vAt - i_vEye).normalize();
	vector3 vXAxis; vVector3Cross( vXAxis, i_vUp, vZAxis ).normalize();
	vector3 vYAxis; vVector3Cross( vYAxis, vZAxis, vXAxis );

	o_matMatOut._11 = vXAxis.x; o_matMatOut._12 = vYAxis.x; o_matMatOut._13 = vZAxis.x; o_matMatOut._14 = 0.0f;
	o_matMatOut._21 = vXAxis.y; o_matMatOut._22 = vYAxis.y; o_matMatOut._23 = vZAxis.y; o_matMatOut._24 = 0.0f;
	o_matMatOut._31 = vXAxis.z; o_matMatOut._32 = vYAxis.z; o_matMatOut._33 = vZAxis.z; o_matMatOut._34 = 0.0f;
	o_matMatOut._41 = -fVector3Dot( vXAxis, i_vEye ); o_matMatOut._42 = -fVector3Dot( vYAxis, i_vEye); o_matMatOut._43 = -fVector3Dot( vZAxis, i_vEye ); o_matMatOut._44 = 1.0f;
	return o_matMatOut;
}

matrix44 &matMatrix44LookAtRH( matrix44 &o_matMatOut, const vector3 &i_vEye, const vector3 &i_vAt, const vector3 &i_vUp )
{
	const vector3 vZAxis = (i_vEye - i_vAt).normalize();
	vector3 vXAxis; vVector3Cross( vXAxis, i_vUp, vZAxis ).normalize();
	vector3 vYAxis; vVector3Cross( vYAxis, vZAxis, vXAxis );

	o_matMatOut._11 = vXAxis.x; o_matMatOut._12 = vYAxis.x; o_matMatOut._13 = vZAxis.x; o_matMatOut._14 = 0.0f;
	o_matMatOut._21 = vXAxis.y; o_matMatOut._22 = vYAxis.y; o_matMatOut._23 = vZAxis.y; o_matMatOut._24 = 0.0f;
	o_matMatOut._31 = vXAxis.z; o_matMatOut._32 = vYAxis.z; o_matMatOut._33 = vZAxis.z; o_matMatOut._34 = 0.0f;
	o_matMatOut._41 = -fVector3Dot( vXAxis, i_vEye ); o_matMatOut._42 = -fVector3Dot( vYAxis, i_vEye); o_matMatOut._43 = -fVector3Dot( vZAxis, i_vEye ); o_matMatOut._44 = 1.0f;
	return o_matMatOut;
}

matrix44 &matMatrix44OrthoOffCenterLH( matrix44 &o_matMatOut, const float32 i_fLeft, const float32 i_fRight, const float32 i_fBottom, const float32 i_fTop, const float32 i_fZNear, const float32 i_fZFar )
{
	o_matMatOut._11 = 2.0f / (i_fRight - i_fLeft); o_matMatOut._12 = 0.0f; o_matMatOut._13 = 0.0f; o_matMatOut._14 = 0.0f;
	o_matMatOut._21 = 0.0f; o_matMatOut._22 = 2.0f / (i_fTop - i_fBottom); o_matMatOut._23 = 0.0f; o_matMatOut._24 = 0.0f;
	o_matMatOut._31 = 0.0f; o_matMatOut._32 = 0.0f; o_matMatOut._33 = 1.0f / (i_fZFar - i_fZNear); o_matMatOut._34 = 0.0f;
	o_matMatOut._41 = (i_fLeft + i_fRight) / (i_fLeft - i_fRight); o_matMatOut._42 = (i_fBottom + i_fTop) / (i_fBottom - i_fTop); o_matMatOut._43 = i_fZNear / (i_fZNear - i_fZFar); o_matMatOut._44 = 1.0f;
	return o_matMatOut;
}

matrix44 &matMatrix44OrthoOffCenterRH( matrix44 &o_matMatOut, const float32 i_fLeft, const float32 i_fRight, const float32 i_fBottom, const float32 i_fTop, const float32 i_fZNear, const float32 i_fZFar )
{
	o_matMatOut._11 = 2.0f / (i_fRight - i_fLeft); o_matMatOut._12 = 0.0f; o_matMatOut._13 = 0.0f; o_matMatOut._14 = 0.0f;
	o_matMatOut._21 = 0.0f; o_matMatOut._22 = 2.0f / (i_fTop - i_fBottom); o_matMatOut._23 = 0.0f; o_matMatOut._24 = 0.0f;
	o_matMatOut._31 = 0.0f; o_matMatOut._32 = 0.0f; o_matMatOut._33 = 1.0f / (i_fZNear - i_fZFar); o_matMatOut._34 = 0.0f;
	o_matMatOut._41 = (i_fLeft + i_fRight) / (i_fLeft - i_fRight); o_matMatOut._42 = (i_fBottom + i_fTop) / (i_fBottom - i_fTop); o_matMatOut._43 = i_fZNear / (i_fZNear - i_fZFar); o_matMatOut._44 = 1.0f;
	return o_matMatOut;
}

matrix44 &matMatrix44OrthoLH( matrix44 &o_matMatOut, const float32 i_fWidth, const float32 i_fHeight, const float32 i_fZNear, const float32 i_fZFar )
{
	return matMatrix44OrthoOffCenterLH( o_matMatOut, -i_fWidth * 0.5f, i_fWidth * 0.5f, -i_fHeight * 0.5f, i_fHeight * 0.5f, i_fZNear, i_fZFar );
}

matrix44 &matMatrix44OrthoRH( matrix44 &o_matMatOut, const float32 i_fWidth, const float32 i_fHeight, const float32 i_fZNear, const float32 i_fZFar )
{
	return matMatrix44OrthoOffCenterRH( o_matMatOut, -i_fWidth * 0.5f, i_fWidth * 0.5f, -i_fHeight * 0.5f, i_fHeight * 0.5f, i_fZNear, i_fZFar );
}

matrix44 &matMatrix44PerspectiveFovLH( matrix44 &o_matMatOut, const float32 i_fFOVY, const float32 i_fAspect, const float32 i_fZNear, const float32 i_fZFar )
{
	const float32 fViewSpaceHeight = 1.0f / tanf( i_fFOVY * 0.5f );
	const float32 fViewSpaceWidth = fViewSpaceHeight / i_fAspect;
	o_matMatOut._11 = fViewSpaceWidth; o_matMatOut._12 = 0.0f; o_matMatOut._13 = 0.0f; o_matMatOut._14 = 0.0f;
	o_matMatOut._21 = 0.0f; o_matMatOut._22 = fViewSpaceHeight; o_matMatOut._23 = 0.0f; o_matMatOut._24 = 0.0f;
	o_matMatOut._31 = 0.0f; o_matMatOut._32 = 0.0f; o_matMatOut._33 = i_fZFar / ( i_fZFar - i_fZNear ); o_matMatOut._34 = 1.0f;
	o_matMatOut._41 = 0.0f; o_matMatOut._42 = 0.0f; o_matMatOut._43 = -i_fZNear * i_fZFar / ( i_fZFar - i_fZNear ); o_matMatOut._44 = 0.0f;
	return o_matMatOut;
}

matrix44 &matMatrix44PerspectiveFovRH( matrix44 &o_matMatOut, const float32 i_fFOVY, const float32 i_fAspect, const float32 i_fZNear, const float32 i_fZFar )
{
	const float32 fViewSpaceHeight = 1.0f / tanf( i_fFOVY * 0.5f );
	const float32 fViewSpaceWidth = fViewSpaceHeight / i_fAspect;
	o_matMatOut._11 = fViewSpaceWidth; o_matMatOut._12 = 0.0f; o_matMatOut._13 = 0.0f; o_matMatOut._14 = 0.0f;
	o_matMatOut._21 = 0.0f; o_matMatOut._22 = fViewSpaceHeight; o_matMatOut._23 = 0.0f; o_matMatOut._24 = 0.0f;
	o_matMatOut._31 = 0.0f; o_matMatOut._32 = 0.0f; o_matMatOut._33 = i_fZFar / ( i_fZNear - i_fZFar ); o_matMatOut._34 = -1.0f;
	o_matMatOut._41 = 0.0f; o_matMatOut._42 = 0.0f; o_matMatOut._43 = i_fZNear * i_fZFar / ( i_fZNear - i_fZFar ); o_matMatOut._44 = 0.0f;
	return o_matMatOut;
}

matrix44 &matMatrix44PerspectiveLH( matrix44 &o_matMatOut, const float32 i_fWidth, const float32 i_fHeight, const float32 i_fZNear, float32 const i_fZFar )
{
	o_matMatOut._11 = 2.0f * i_fZNear / i_fWidth; o_matMatOut._12 = 0.0f; o_matMatOut._13 = 0.0f; o_matMatOut._14 = 0.0f;
	o_matMatOut._21 = 0.0f; o_matMatOut._22 = 2.0f * i_fZNear / i_fHeight; o_matMatOut._23 = 0.0f; o_matMatOut._24 = 0.0f;
	o_matMatOut._31 = 0.0f; o_matMatOut._32 = 0.0f; o_matMatOut._33 = i_fZFar / (i_fZFar - i_fZNear); o_matMatOut._34 = 1.0f;
	o_matMatOut._41 = 0.0f; o_matMatOut._42 = 0.0f; o_matMatOut._43 = i_fZNear * i_fZFar / (i_fZNear - i_fZFar); o_matMatOut._44 = 0.0f;
	return o_matMatOut;
}

matrix44 &matMatrix44PerspectiveRH( matrix44 &o_matMatOut, const float32 i_fWidth, const float32 i_fHeight, const float32 i_fZNear, const float32 i_fZFar )
{
	o_matMatOut._11 = 2.0f * i_fZNear / i_fWidth; o_matMatOut._12 = 0.0f; o_matMatOut._13 = 0.0f; o_matMatOut._14 = 0.0f;
	o_matMatOut._21 = 0.0f; o_matMatOut._22 = 2.0f * i_fZNear / i_fHeight; o_matMatOut._23 = 0.0f; o_matMatOut._24 = 0.0f;
	o_matMatOut._31 = 0.0f; o_matMatOut._32 = 0.0f; o_matMatOut._33 = i_fZFar / (i_fZNear - i_fZFar); o_matMatOut._34 = -1.0f;
	o_matMatOut._41 = 0.0f; o_matMatOut._42 = 0.0f; o_matMatOut._43 = i_fZNear * i_fZFar / (i_fZNear - i_fZFar); o_matMatOut._44 = 0.0f;
	return o_matMatOut;
}

matrix44 &matMatrix44PerspectiveOffCenterLH( matrix44 &o_matMatOut, const float32 i_fLeft, const float32 i_fRight, const float32 i_fBottom, const float32 i_fTop, const float32 i_fZNear, const float32 i_fZFar )
{
	o_matMatOut._11 = 2.0f * i_fZNear / (i_fRight - i_fLeft); o_matMatOut._12 = 0.0f; o_matMatOut._13 = 0.0f; o_matMatOut._14 = 0.0f;
	o_matMatOut._21 = 0.0f; o_matMatOut._22 = 2.0f * i_fZNear / (i_fTop - i_fBottom); o_matMatOut._23 = 0.0f; o_matMatOut._24 = 0.0f;
	o_matMatOut._31 = (i_fLeft + i_fRight) / (i_fLeft - i_fRight); o_matMatOut._32 = (i_fBottom + i_fTop) / (i_fBottom - i_fTop); o_matMatOut._33 = i_fZFar / (i_fZFar - i_fZNear); o_matMatOut._34 = 1.0f;
	o_matMatOut._41 = 0.0f; o_matMatOut._42 = 0.0f; o_matMatOut._43 = i_fZNear * i_fZFar / (i_fZNear - i_fZFar); o_matMatOut._44 = 0.0f;
	return o_matMatOut;
}

matrix44 &matMatrix44PerspectiveOffCenterRH( matrix44 &o_matMatOut, const float32 i_fLeft, const float32 i_fRight, const float32 i_fBottom, const float32 i_fTop, const float32 i_fZNear, const float32 i_fZFar )
{
	o_matMatOut._11 = 2.0f * i_fZNear / (i_fRight - i_fLeft); o_matMatOut._12 = 0.0f; o_matMatOut._13 = 0.0f; o_matMatOut._14 = 0.0f;
	o_matMatOut._21 = 0.0f; o_matMatOut._22 = 2.0f * i_fZNear / (i_fTop - i_fBottom); o_matMatOut._23 = 0.0f; o_matMatOut._24 = 0.0f;
	o_matMatOut._31 = (i_fLeft + i_fRight) / (i_fLeft - i_fRight); o_matMatOut._32 = (i_fBottom + i_fTop) / (i_fBottom - i_fTop); o_matMatOut._33 = i_fZFar / (i_fZNear - i_fZFar); o_matMatOut._34 = -1.0f;
	o_matMatOut._41 = 0.0f; o_matMatOut._42 = 0.0f; o_matMatOut._43 = i_fZNear * i_fZFar / (i_fZNear - i_fZFar); o_matMatOut._44 = 0.0f;
	return o_matMatOut;
}

matrix44 &matMatrix44Viewport( matrix44 &o_matMatOut, const uint32 i_iX, const uint32 i_iY, const uint32 i_iWidth, const uint32 i_iHeight, const float32 i_fZNear, const float32 i_fZFar )
{
	o_matMatOut._11 = (float32)i_iWidth * 0.5f; o_matMatOut._12 = 0.0f; o_matMatOut._13 = 0.0f; o_matMatOut._14 = 0.0f;
	o_matMatOut._21 = 0.0f; o_matMatOut._22 = (float32)i_iHeight * -0.5f; o_matMatOut._23 = 0.0f; o_matMatOut._24 = 0.0f;
	o_matMatOut._31 = 0.0f; o_matMatOut._32 = 0.0f; o_matMatOut._33 = i_fZFar - i_fZNear; o_matMatOut._34 = 0.0f;
	o_matMatOut._41 = (float32)i_iX + (float32)i_iWidth * 0.5f; o_matMatOut._42 = (float32)i_iY + (float32)i_iHeight * 0.5f; o_matMatOut._43 = i_fZNear; o_matMatOut._44 = 1.0f;
	return o_matMatOut;
}
