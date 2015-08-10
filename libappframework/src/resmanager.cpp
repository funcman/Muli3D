
#include "../include/resmanager.h"
#include "../include/application.h"

CResManager::CResManager( IApplication *i_pParent )
{
	m_pParent = i_pParent;
	
	m_iNumLoadedResources = 0;
}

CResManager::~CResManager()
{
	while( m_ManagedResources.size() )
		ReleaseResource( m_ManagedResources.begin()->hResource );
}

// Resource loaders -----------------------------------------------------------
static CResManager *g_pResManager = 0;
#include "../include/graphics.h"
#include "../include/fileio.h"

// Models ---------------------------------------------------------------------

#include "../include/model.h"

void *pLoadModel( CResManager *i_pParent, string i_sFilename )
{
	CGraphics *pGraphics = i_pParent->pGetParent()->pGetGraphics();
	CFileIO *pFileIO = pGraphics->pGetParent()->pGetFileIO();
	
	char *pData = 0;
	uint32 iLength = pFileIO->iReadFile( i_sFilename, (byte **)&pData, true );
	if( !iLength )
		return 0;

	CModel *pModel = new CModel( g_pResManager );
	bool bResult = pModel->bLoadModel( pData );
	SAFE_DELETE_ARRAY( pData );

	if( !bResult )
	{
		SAFE_DELETE( pModel );
		return 0;
	}
	else
		return pModel;
}

void UnloadModel( CResManager *i_pParent, void *i_pResource )
{
	CModel *pModel = (CModel *)i_pResource;
	SAFE_DELETE( pModel );
}

// Textures -------------------------------------------------------------------

#include "../include/texture.h"

#ifdef WIN32
#include "../libpng/png.h"
#elif __amigaos4__
#include <libpng/png.h>
#else
#include <png.h>
#endif

static void png_read_data( png_structp png_ptr, png_bytep data, png_size_t length )
{
	memcpy( data, png_ptr->io_ptr, length );
	png_ptr->io_ptr = (byte *)png_ptr->io_ptr + length;
}

bool bLoadPNGTexture( CMuli3DTexture **o_ppTexture, const byte *i_pData, CMuli3DDevice *i_pDevice )
{
	png_structp png_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING, 0, 0, 0 );
	if( !png_ptr )
		return false;

	png_infop info_ptr = png_create_info_struct( png_ptr );
	if( !info_ptr )
	{
		png_destroy_read_struct( &png_ptr, (png_infopp)0, (png_infopp)0 );
		return false;
	}

	png_infop end_info = png_create_info_struct( png_ptr );
	if( !end_info )
	{
		png_destroy_read_struct( &png_ptr, &info_ptr, (png_infopp)0 );
		return false;
	}

	if( setjmp( png_ptr->jmpbuf ) ) 
	{
		png_destroy_read_struct( &png_ptr, &info_ptr, &end_info );
        return false;
	}

	png_set_read_fn( png_ptr, (png_voidp *)i_pData, png_read_data );
	png_read_info( png_ptr, info_ptr );

	uint32 iDimX = png_get_image_width( png_ptr, info_ptr );
	uint32 iDimY = png_get_image_height( png_ptr, info_ptr );

	int32 color_type = png_get_color_type( png_ptr, info_ptr );
	if( color_type == PNG_COLOR_TYPE_PALETTE && png_get_bit_depth( png_ptr, info_ptr ) <= 8) png_set_expand( png_ptr );
	if( color_type == PNG_COLOR_TYPE_GRAY && png_get_bit_depth( png_ptr, info_ptr ) < 8) png_set_expand( png_ptr );
	if( color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA ) png_set_gray_to_rgb( png_ptr );
	if( png_get_bit_depth( png_ptr, info_ptr ) == 16 ) png_set_strip_16( png_ptr );
	png_read_update_info( png_ptr, info_ptr );

	color_type = png_get_color_type( png_ptr, info_ptr );
	bool bHasAlpha = (color_type == PNG_COLOR_TYPE_RGB_ALPHA);
	m3dformat fmtTextureFormat = bHasAlpha ? m3dfmt_r32g32b32a32f : m3dfmt_r32g32b32f;
	
	if( FUNC_FAILED( i_pDevice->CreateTexture( o_ppTexture, iDimX, iDimY, 0, fmtTextureFormat ) ) )
	{
		png_destroy_read_struct( &png_ptr, &info_ptr, &end_info );
        return false;
	}

	byte *pData = new byte[ iDimY * png_get_rowbytes( png_ptr, info_ptr ) ];
	byte *pCurData = pData;
	byte **pRows = new byte *[iDimY];
	for( uint32 i = 0; i < iDimY; ++i )
	{
		pRows[i] = pCurData;
		pCurData += png_get_rowbytes( png_ptr, info_ptr );
	}

	// read the image
	png_read_image( png_ptr, pRows );

	SAFE_DELETE_ARRAY( pRows );

	png_read_end( png_ptr, end_info );
	png_destroy_read_struct( &png_ptr, &info_ptr, &end_info );

	float32 *pTexData = 0;
	result resLock = (*o_ppTexture)->LockRect( 0, (void **)&pTexData, 0 );
	if( FUNC_FAILED( resLock ) )
	{
		SAFE_DELETE_ARRAY( pData );
		return false;
	}

	const float32 fColorScale = 1.0f / 255.0f;

	pCurData = pData;
	for( uint32 iY = 0; iY < iDimY; ++iY )
	{
		for( uint32 iX = 0; iX < iDimX; ++iX )
		{
			(*pTexData++) = (float32)(*pCurData++) * fColorScale;
			(*pTexData++) = (float32)(*pCurData++) * fColorScale;
			(*pTexData++) = (float32)(*pCurData++) * fColorScale;
			if( bHasAlpha )
				(*pTexData++) = (float32)(*pCurData++) * fColorScale;
		}
	}

	SAFE_DELETE_ARRAY( pData );

	(*o_ppTexture)->UnlockRect( 0 );

	return true;
}

void *pLoadTexture( CResManager *i_pParent, string i_sFilename )
{
	CGraphics *pGraphics = i_pParent->pGetParent()->pGetGraphics();
	CFileIO *pFileIO = pGraphics->pGetParent()->pGetFileIO();
	
	byte *pData = 0;
	uint32 iLength = pFileIO->iReadFile( i_sFilename, &pData );
	if( !iLength )
		return 0;

	CMuli3DTexture *pTexture = 0;
	bool bResult = bLoadPNGTexture( &pTexture, pData, pGraphics->pGetM3DDevice() );
	SAFE_DELETE_ARRAY( pData );
	if( !bResult )
		return 0;

	pTexture->GenerateMipSubLevels( 0 );

	return new CTexture( g_pResManager, pTexture );
}

void *pLoadCubeTexture( CResManager *i_pParent, string i_sFilename )
{
	CGraphics *pGraphics = i_pParent->pGetParent()->pGetGraphics();
	CFileIO *pFileIO = pGraphics->pGetParent()->pGetFileIO();
	
	byte *pData = 0;
	uint32 iLength = pFileIO->iReadFile( i_sFilename, &pData, true );
	if( !iLength )
		return 0;

	vector<string> sFilenames;

	char *pCurPosition = (char *)pData;

	// read in textures
	while( true )
	{
		char *pEndOfLine = strchr( pCurPosition, '\n' );
		if( pEndOfLine ) *pEndOfLine = 0;

		if( !strlen( pCurPosition ) ) break;
		string sTexture = pCurPosition;

		sFilenames.push_back( sTexture );

		if( pEndOfLine )
			pCurPosition = pEndOfLine + 1;
		else
			break;
	}

	SAFE_DELETE_ARRAY( pData );

	uint32 iNumTextures = (uint32)sFilenames.size();
	if( iNumTextures != 6 )
		return 0;

	uint32 iEdgeLength = 0;
	m3dformat fmtCubeFormat = m3dfmt_r32g32b32f;

	CMuli3DTexture **ppTextures = new CMuli3DTexture *[iNumTextures];
	uint32 i;
	for( i = 0; i < iNumTextures; ++i )
	{
		byte *pTexData = 0;
		uint32 iTexLength = pFileIO->iReadFile( sFilenames[i], &pTexData );
	
		if( !iTexLength )
		{
			// release created textures up to now
			for( uint32 j = 0; j < i; ++j )
				SAFE_RELEASE( ppTextures[j] );
			SAFE_DELETE_ARRAY( ppTextures );
			return 0;
		}

		bool bResult = bLoadPNGTexture( &ppTextures[i], pTexData, pGraphics->pGetM3DDevice() );
		SAFE_DELETE_ARRAY( pTexData );
		if( !bResult )
		{
			// release created textures up to now
			for( uint32 j = 0; j < i; ++j )
				SAFE_RELEASE( ppTextures[j] );
			SAFE_DELETE_ARRAY( ppTextures );
			return 0;
		}

		if( i == 0 )
		{
			iEdgeLength = ppTextures[i]->iGetWidth();
			fmtCubeFormat = ppTextures[i]->fmtGetFormat();
		}

		// make sure that we're building a valid cubemap ...
		if( ppTextures[i]->iGetWidth() != ppTextures[i]->iGetHeight() ||
			ppTextures[i]->iGetWidth() != iEdgeLength ||
			ppTextures[i]->fmtGetFormat() != fmtCubeFormat )
		{
			// release created textures up to now
			for( uint32 j = 0; j < i; ++j )
				SAFE_RELEASE( ppTextures[j] );
			SAFE_DELETE_ARRAY( ppTextures );
			return 0;
		}
	}

	CMuli3DCubeTexture *pCubeTexture = 0;
	if( FUNC_FAILED( pGraphics->pGetM3DDevice()->CreateCubeTexture( &pCubeTexture,
		iEdgeLength, 0, fmtCubeFormat ) ) )
	{
		// release created textures up to now
		for( uint32 j = 0; j < i; ++j )
			SAFE_RELEASE( ppTextures[j] );
		SAFE_DELETE_ARRAY( ppTextures );
		return 0;
	}
	
	uint32 iNumBytes = iEdgeLength * iEdgeLength;
	switch( fmtCubeFormat )
	{
	case m3dfmt_r32f: iNumBytes *= sizeof( float32 ); break;
	case m3dfmt_r32g32f: iNumBytes *= 2 * sizeof( float32 ); break;
	case m3dfmt_r32g32b32f: iNumBytes *= 3 * sizeof( float32 ); break;
	case m3dfmt_r32g32b32a32f: iNumBytes *= 4 * sizeof( float32 ); break;
	default: /* cannot happen */ break;
	}

	for( uint32 iFace = m3dcf_positive_x; iFace <= m3dcf_negative_z; ++iFace )
	{
		uint8 *pDest = 0;
		pCubeTexture->LockRect( (m3dcubefaces)iFace, 0, (void **)&pDest, 0 );

		uint8 *pSrc = 0;
		ppTextures[iFace]->LockRect( 0, (void **)&pSrc, 0 );
		memcpy( pDest, pSrc, iNumBytes );
		ppTextures[iFace]->UnlockRect( 0 );
		SAFE_RELEASE( ppTextures[iFace] );
		
		pCubeTexture->UnlockRect( (m3dcubefaces)iFace, 0 );
	}

	SAFE_DELETE_ARRAY( ppTextures );

	pCubeTexture->GenerateMipSubLevels( 0 );

	return new CTexture( g_pResManager, pCubeTexture );
}

void *pLoadAnimatedTexture( CResManager *i_pParent, string i_sFilename )
{
	CGraphics *pGraphics = i_pParent->pGetParent()->pGetGraphics();
	CFileIO *pFileIO = pGraphics->pGetParent()->pGetFileIO();
	
	byte *pData = 0;
	uint32 iLength = pFileIO->iReadFile( i_sFilename, &pData, true );
	if( !iLength )
		return 0;

	float32 fFPS = 0.0f;
	vector<string> sFilenames;

	char *pCurPosition = (char *)pData;

	// read in frames per second
	sscanf( pCurPosition, "%f", &fFPS );
	pCurPosition = strchr( pCurPosition, '\n' );
	if( !pCurPosition ) { SAFE_DELETE_ARRAY( pData ); return 0; }
	else pCurPosition += 1;

	if( fFPS <= 0.0f )
	{
		SAFE_DELETE_ARRAY( pData );
		return 0;
	}

	// read in textures
	while( true )
	{
		char *pEndOfLine = strchr( pCurPosition, '\n' );
		if( pEndOfLine ) *pEndOfLine = 0;

		if( !strlen( pCurPosition ) ) break;
		sFilenames.push_back( pCurPosition );

		if( pEndOfLine )
			pCurPosition = pEndOfLine + 1;
		else
			break;
	}

	SAFE_DELETE_ARRAY( pData );

	uint32 iNumTextures = (uint32)sFilenames.size();
	if( !iNumTextures )
		return 0;

	CMuli3DTexture **ppTextures = new CMuli3DTexture *[iNumTextures];
	for( uint32 i = 0; i < iNumTextures; ++i )
	{
		byte *pTexData = 0;
		uint32 iTexLength = pFileIO->iReadFile( sFilenames[i], &pTexData );
		if( !iTexLength )
		{
			// release created textures up to now
			for( uint32 j = 0; j < i; ++j )
				SAFE_RELEASE( ppTextures[j] );
			SAFE_DELETE_ARRAY( ppTextures );
			return 0;
		}

		bool bResult = bLoadPNGTexture( &ppTextures[i], pTexData, pGraphics->pGetM3DDevice() );
		SAFE_DELETE_ARRAY( pTexData );
		if( !bResult )
		{
			// release created textures up to now
			for( uint32 j = 0; j < i; ++j )
				SAFE_RELEASE( ppTextures[j] );
			SAFE_DELETE_ARRAY( ppTextures );
			return 0;
		}

		ppTextures[i]->GenerateMipSubLevels( 0 );
	}

	return new CTexture( g_pResManager, iNumTextures, fFPS, ppTextures );
}

void UnloadTexture( CResManager *i_pParent, void *i_pResource )
{
	CTexture *pTexture = (CTexture *)i_pResource;
	SAFE_DELETE( pTexture );
}

// Resource loaders -----------------------------------------------------------

bool CResManager::bInitialize()
{
	RegisterResourceExtension( "obj", pLoadModel, UnloadModel );

	RegisterResourceExtension( "png", pLoadTexture, UnloadTexture );
	RegisterResourceExtension( "cube", pLoadCubeTexture, UnloadTexture );
	RegisterResourceExtension( "anim", pLoadAnimatedTexture, UnloadTexture );

	return true;
}

void CResManager::RegisterResourceExtension( string i_sExtension, PLOADFUNCTION i_pLoadFunction, PUNLOADFUNCTION i_pUnloadFunction )
{
	m_RegisteredEntityExtensionsLoad[i_sExtension] = i_pLoadFunction;
	m_RegisteredEntityExtensionsUnload[i_sExtension] = i_pUnloadFunction;
}

HRESOURCE CResManager::hLoadResource( string i_sFilename )
{
	// Look if we have already loaded this resource ---------------------------
	for( vector<tManagedResource>::iterator pManagedResource = m_ManagedResources.begin(); pManagedResource != m_ManagedResources.end(); ++pManagedResource )
	{
		if( pManagedResource->sFilename == i_sFilename )
		{
			++pManagedResource->iReferences;
			return pManagedResource->hResource;
		}
	} 

	// We have to load it from the disk ---------------------------------------
	const char *pCheck = i_sFilename.c_str();
	const char *pExt = 0;
	while( *pCheck )
	{
		if( *pCheck == '.' )
			pExt = pCheck;
		++pCheck;
	}

	if( !pExt )
		return 0;

	string sExtension = (pExt + 1);
	PLOADFUNCTION pLoadFunction = m_RegisteredEntityExtensionsLoad[sExtension];
	if( !pLoadFunction )
		return 0;

	g_pResManager = this;

	tManagedResource newResource;
	newResource.pResource = pLoadFunction( this, i_sFilename );
	if( !newResource.pResource )
		return 0;

	newResource.hResource = ++m_iNumLoadedResources;
	newResource.iReferences = 1;
	newResource.sFilename = i_sFilename;
	newResource.sExtension = sExtension;
	m_ManagedResources.push_back( newResource );
	return newResource.hResource;
}

vector<CResManager::tManagedResource>::iterator CResManager::pGetManagedResourceIterator( HRESOURCE i_hResource )
{
	if( !i_hResource ) return m_ManagedResources.end();
	for( vector<tManagedResource>::iterator pManagedResource = m_ManagedResources.begin(); pManagedResource != m_ManagedResources.end(); ++pManagedResource )
	{
		if( pManagedResource->hResource == i_hResource )
			return pManagedResource;
	}
	return m_ManagedResources.end();
}

void CResManager::ReleaseResource( HRESOURCE i_hResource )
{
	vector<tManagedResource>::iterator pManagedResource = pGetManagedResourceIterator( i_hResource );
	if( pManagedResource != m_ManagedResources.end() )
	{
		if( --pManagedResource->iReferences == 0 )
		{
			PUNLOADFUNCTION pUnloadFunction = m_RegisteredEntityExtensionsUnload[pManagedResource->sExtension];

			uint32 iOldSize = (uint32)m_ManagedResources.size();
			if( pUnloadFunction )
				pUnloadFunction( this, pManagedResource->pResource );

			if( iOldSize != m_ManagedResources.size() )
				pManagedResource = pGetManagedResourceIterator( i_hResource );
			m_ManagedResources.erase( pManagedResource );
		}
	}
}

void *CResManager::pGetResource( HRESOURCE i_hResource )
{
	vector<tManagedResource>::iterator pManagedResource = pGetManagedResourceIterator( i_hResource );
	if( pManagedResource != m_ManagedResources.end() )
		return pManagedResource->pResource;
	return 0;
}
