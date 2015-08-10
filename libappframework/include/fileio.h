
#ifndef __FILEIO_H__
#define __FILEIO_H__

#include "base.h"

#ifdef __amigaos4__
#define BASE_DIR "data"
#else
#define BASE_DIR "./data"
#endif

class CFileIO
{
protected:
	friend class IApplication;
	friend class CApplication;
	CFileIO( class IApplication *i_pParent );
	~CFileIO();

	bool bInitialize();

public:
	uint32 iReadFile( string i_sFilename, byte **o_ppData, bool i_bText = false );
	uint32 iReadFile( string i_sFilename, byte *i_pData, uint32 i_iSize, bool i_bText = false );

	uint32 iWriteFile( string i_sFilename, byte *i_pData, uint32 i_iSize, bool i_bText = false );

	bool bFileExists( string i_sFilename );

private:

public:
	inline class IApplication *pGetParent() { return m_pParent; }

	const char *pDiskFilePath( string i_sFilename );

private:
	class IApplication *m_pParent;
};

#endif // __FILEIO_H__
