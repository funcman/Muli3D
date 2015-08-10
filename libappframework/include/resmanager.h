
#ifndef __RESMANAGER_H__
#define __RESMANAGER_H__

#include "base.h"
#include <map>
#include <vector>

typedef uint32 HRESOURCE;
typedef void *(*PLOADFUNCTION)( class CResManager *i_pParent, string i_sFilename );
typedef void (*PUNLOADFUNCTION)( class CResManager *i_pParent, void *i_pResource );

class CResManager
{
protected:
	friend class IApplication;
	friend class CApplication;
	CResManager( class IApplication *i_pParent );
	~CResManager();

	bool bInitialize();

public:
	void RegisterResourceExtension( string i_sExtension, PLOADFUNCTION i_pLoadFunction, PUNLOADFUNCTION i_pUnloadFunction );
	
	HRESOURCE hLoadResource( string i_sFilename );
	void ReleaseResource( HRESOURCE i_hResource );

	void *pGetResource( HRESOURCE i_hResource );

private:

public:
	inline class IApplication *pGetParent() { return m_pParent; }

private:
	class IApplication *m_pParent;

	map<string, PLOADFUNCTION>			m_RegisteredEntityExtensionsLoad;
	map<string, PUNLOADFUNCTION>		m_RegisteredEntityExtensionsUnload;

	struct tManagedResource
	{
		HRESOURCE	hResource;
		uint32		iReferences;
		string		sFilename, sExtension;
		void		*pResource;
	};
	vector<tManagedResource>	m_ManagedResources;
	uint32						m_iNumLoadedResources;

private:
	vector<tManagedResource>::iterator pGetManagedResourceIterator( HRESOURCE i_hResource );
};

#endif // __RESMANAGER_H__
