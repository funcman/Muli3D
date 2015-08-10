
#include "../include/fileio.h"
#include "../include/application.h"
#include <stdio.h>
#include <stdlib.h>

CFileIO::CFileIO( IApplication *i_pParent )
{
	m_pParent = i_pParent;
}

CFileIO::~CFileIO()
{

}

bool CFileIO::bInitialize()
{
	return true;
}

uint32 CFileIO::iReadFile( string i_sFilename, byte **o_ppData, bool i_bText )
{
	const char *pFilePath = pDiskFilePath( i_sFilename );
	const char *szMode = i_bText ? "rt" : "rb";

	FILE *pFile = fopen( pFilePath, szMode );
	if( !pFile )
		return 0;

	fseek( pFile, 0, SEEK_END );
	uint32 iSize = ftell( pFile );
	rewind( pFile );

	*o_ppData = new byte[i_bText ? iSize + 1 : iSize];
	iSize = (uint32)fread( *o_ppData, 1, iSize, pFile );

	fclose( pFile );

	if( i_bText )
	{
		(*o_ppData)[iSize] = 0;
		
		#ifndef WIN32

		char *pTemp = new char[iSize + 1];
		strcpy( pTemp, (char *)(*o_ppData) );
		
		char *pSrc = pTemp, *pDest = (char *)(*o_ppData);
		iSize = 0;
		while( *pSrc )
		{
			if( *pSrc != '\r' )
			{
				*pDest++ = *pSrc;
				++iSize;
			}
			++pSrc;
		}
		*pDest = 0;
		
		SAFE_DELETE_ARRAY( pTemp );

		#endif
	}

	return iSize;
}

uint32 CFileIO::iReadFile( string i_sFilename, byte *i_pData, uint32 i_iSize, bool i_bText )
{
	const char *pFilePath = pDiskFilePath( i_sFilename );
	const char *szMode = i_bText ? "rt" : "rb";

	FILE *pFile = fopen( pFilePath, szMode );
	if( !pFile )
		return 0;

	fseek( pFile, 0, SEEK_END );
	uint32 iSize = ftell( pFile );
	rewind( pFile );

	iSize = (uint32)fread( i_pData, 1, (iSize < i_iSize) ? iSize : i_iSize, pFile );

	fclose( pFile );

	if( i_bText )
	{
		i_pData[iSize] = 0;
		
		#ifndef WIN32
		char *pTemp = new char[iSize + 1];
		strcpy( pTemp, (char *)i_pData );
		
		char *pSrc = pTemp, *pDest = (char *)i_pData;
		iSize = 0;
		while( *pSrc )
		{
			if( *pSrc != '\r' )
			{
				*pDest++ = *pSrc;
				++iSize;
			}
			++pSrc;
		}
		*pDest = 0;
		
		SAFE_DELETE_ARRAY( pTemp );

		#endif
	}
		
	return iSize;
}

uint32 CFileIO::iWriteFile( string i_sFilename, byte *i_pData, uint32 i_iSize, bool i_bText )
{
	const char *pFilePath = pDiskFilePath( i_sFilename );
	const char *szMode = i_bText ? "wt" : "wb";

	FILE *pFile = fopen( pFilePath, szMode );
	if( !pFile )
		return 0;

	uint32 iWrittenBytes = (uint32)fwrite( i_pData, 1, i_iSize, pFile );

	fclose( pFile );

	return iWrittenBytes;
}

bool CFileIO::bFileExists( string i_sFilename )
{
	const char *pFilePath = pDiskFilePath( i_sFilename );

	FILE *pFile = fopen( pFilePath, "rb" );
	if( !pFile )
		return false;

	fclose( pFile );
	return true;
}

const char *CFileIO::pDiskFilePath( string i_sFilename )
{
	static char szPath[512];
	sprintf( szPath, "%s/%s", BASE_DIR, i_sFilename.c_str() );
	return szPath;
}
