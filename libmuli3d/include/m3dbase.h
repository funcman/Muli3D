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

/// @file m3dbase.h
///

#ifndef __M3DBASE_H__
#define __M3DBASE_H__

// Basic header includes ------------------------------------------------------

#include <stdlib.h>
#ifndef __amigaos4__
#	include <memory.h>
#else
#	include <exec/types.h>
#endif
#include <vector>

// Basic macro definitions ----------------------------------------------------

/// If the pointer is not null, the memory it is pointing to is deleted and the pointer is set to null to ease debugging and to make sure it is not deleted again using SAFE_DELETE().
/// @param[in] p a pointer.
#define SAFE_DELETE( p )		{ if( p ) { delete p; p = 0; } }

/// If the pointer is not null, the memory it is pointing to is deleted and the pointer is set to null to ease debugging and to make sure it is not deleted again using SAFE_DELETE_ARRAY().
/// @param[in] p a pointer to an array.
#define SAFE_DELETE_ARRAY( p )	{ if( p ) { delete[] p; p = 0; } }

/// If the pointer is not null, the object it is pointing to is released and the pointer is set to null to ease debugging and to make sure it is not released again using SAFE_RELEASE().
/// @param[in] p a pointer to a reference counted object.
#define SAFE_RELEASE( p )		{ if( p ) { ( p )->Release(); p = 0; } }


// Basic variable definitions -------------------------------------------------

#ifndef __amigaos4__	// avoid conflicts with <exec/types.h>

typedef signed char         int8;		///< 8-bit signed integer
typedef signed short        int16;		///< 16-bit signed integer
typedef signed int          int32;		///< 32-bit signed integer
typedef unsigned char       uint8;		///< 8-bit unsigned integer
typedef unsigned short      uint16;		///< 16-bit unsigned integer
typedef unsigned int        uint32;		///< 32-bit unsigned integer

typedef float				float32;	///< 32-bit float
typedef double				float64;	///< 64-bit float

#endif

typedef unsigned char		byte;		///< byte

// Floating pointer helper-functions ------------------------------------------

#ifdef WIN32

#include <float.h>
#pragma warning( disable: 4996 ) // Disable warning about _controlfp being deprecated.
inline void fpuTruncate() { _controlfp( _RC_DOWN + _PC_24, _MCW_RC|_MCW_PC ); }	///< Sets FPU to truncation-mode and single precision.
inline void fpuReset() { _controlfp( _CW_DEFAULT, _MCW_RC|_MCW_PC ); }		///< Resets FPU to default.

#endif

#ifdef LINUX_X11

#include <fpu_control.h>

/// Sets FPU to truncation-mode and single precision.
inline void fpuTruncate()
{
	fpu_control_t cwChop = _FPU_RC_DOWN | _FPU_IEEE | _FPU_SINGLE;
	_FPU_SETCW( cwChop );
}

/// Resets FPU to default.
inline void fpuReset()
{
	fpu_control_t cwDefault = _FPU_DEFAULT;
	_FPU_SETCW( cwDefault );
}

#endif

#ifdef __amigaos4__

inline void fpuTruncate()
{
	// #warning: fpuTruncate() currently not implemented on AmigaOS 4 
}

inline void fpuReset()
{
	// #warning: fpuReset() currently not implemented on AmigaOS 4
}

#endif

/// ftol() performs fast float to integer-conversion.
/// Ensure that fpuTruncate() has been called before using this function, otherwise
/// ftol() will round to the nearest integer. After calling fpuTruncate() it is advised to 
/// reset the fpu to rounding-mode using fpuReset().
/// @param[in] f The floating pointer number to be converted to an integer.
/// @return an integer.

#ifdef __amigaos4__
typedef union
{
	struct
	{
		unsigned long hi;
		unsigned long lo;
	} i;
	
	double d;
} hexdouble;
#endif

inline int32 ftol( float32 f )
{
#ifdef __amigaos4__
	static hexdouble hd;
	__asm__ ( "fctiw %0, %1" : "=f" (hd.d) : "f" (f) );
	return hd.i.lo;
#else
	static int32 tmp;

	#if _MSC_VER > 1000
	__asm
	{
		fld f
		fistp tmp
	}
	#else
	asm volatile( "fistpl %0" : "=m" ( tmp ) : "t" ( f ) : "st" );
	#endif

	return tmp;
#endif
}

// Functions and return-values ------------------------------------------------

/// Describes function return values.
enum result
{
	s_ok = 0,				///< No errors were encountered, function returned successfully.
	e_unknown,				///< An unknown error has occured.
	e_invalidparameters,	///< One or more parameters were invalid.
	e_outofmemory,			///< Allocation of memory within the function failed.
	
	e_invalidformat,		///< A format is invalid for a specific task.
	e_invalidstate			///< An invalid state has been found and the function cannot continue.
};

/// Macro to test for success.
/// @param[in] res a function return value.
/// @return true if the function succeeded.
#define FUNC_SUCCESSFUL( res )	( res == s_ok )

/// Macro to test for failure.
/// @param[in] res a function return value.
/// @return true if the function failed.
#define FUNC_FAILED( res )		( res != s_ok )

// Debugging-tools ------------------------------------------------------------

#ifdef _DEBUG

#ifndef WIN32
#define OutputDebugString printf
#endif

/// Outputs an error-description in case a function fails, only available in debug-builds.
/// @param[in] errdesc error-description, c-string.
#define FUNC_FAILING( errdesc ) { OutputDebugString( errdesc ); }

/// Outputs an information-string, only available in debug-builds.
/// @param[in] errdesc error-description, c-string.
#define FUNC_NOTIFY( infostring ) { OutputDebugString( infostring ); }

#else
#define FUNC_FAILING ( void )	///< Function is only available in debug-builds.
#define FUNC_NOTIFY ( void )	///< Function is only available in debug-builds.
#endif // _DEBUG

// Muli3D base-class definition -----------------------------------------------

/// IBase is the base class for all Muli3D classes. It implements a reference-counter with functions AddRef() and Release() known from COM interfaces.
class IBase
{
protected:
	inline IBase() : m_iRefCount( 1 ) {}	///< The constructor initializes the reference count to 1.
	virtual inline ~IBase() {}

public:
	inline void AddRef() { ++m_iRefCount; }	///< AddRef() increases the reference count.
	inline void Release() { if( --m_iRefCount == 0 ) delete this; }	///< Release() decreases the reference count and calls the destructor when it is 0.

private:
	IBase( const IBase & ) {}								///< Private copy-operator to avoid object copying.
	IBase &operator =( const IBase & ) { return *this; }	///< Private assignment-operator to avoid object copying. Returns a value to avoid compiler warnings.

private:
	uint32 m_iRefCount;
};

#endif // __M3DBASE_H__
