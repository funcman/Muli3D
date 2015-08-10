
#ifndef __BASE_H__
#define __BASE_H__

#if _MSC_VER > 1000
#pragma warning( disable : 4786 ) // shut up compiler warning
#endif

// Basic macro definitions ----------------------------------------------------

#define SAFE_DELETE( p ) { if( p ) { delete p; p = 0; }  }
#define SAFE_DELETE_ARRAY( p ) { if( p ) { delete[] p; p = 0; }  }
#define SAFE_RELEASE( p ) { if( p ) { ( p )->Release(); p = 0; }  }

// Basic variable type-definitions --------------------------------------------

#include <string>
using namespace std;

#ifdef __amigaos4__	
#include <exec/types.h>
#else // avoid conflicts with <exec/types.h>

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


#endif // __BASE_H__
