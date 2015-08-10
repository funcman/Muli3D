
#ifndef __SCENE_H__
#define __SCENE_H__

#include "base.h"
#include "../../libmuli3d/include/m3d.h"
#include <map>
#include <vector>

#include "light.h"

// TODO: implement scene-graph

typedef uint32 HENTITY;
typedef class IEntity *(*PCREATEFUNCTION)( class CScene *i_pParent );

typedef uint32 HLIGHT;

class CScene
{
protected:
	friend class IApplication;
	friend class CApplication;
	CScene( class IApplication *i_pParent );
	~CScene();

	bool bInitialize();
	void FrameMove();

public:
	void RegisterEntityType( string i_sTypeName, PCREATEFUNCTION i_pCreateFunction );
	
	HENTITY hCreateEntity( string i_sTypeName, bool i_bSceneProcess = true ); // if i_bSceneProcess is false, then the scene will not automatically call framemove/render - suited for sub-models
	class IEntity *pGetEntity( HENTITY i_hEntity );
	void ReleaseEntity( HENTITY i_hEntity );

	HLIGHT hCreateLight();
	CLight *pGetLight( HLIGHT i_hLight );
	void ReleaseLight( HLIGHT i_hLight );

	void Render( uint32 i_iPass );

private:

public:
	inline class IApplication *pGetParent() { return m_pParent; }

	inline void SetClearColor( vector4 i_vClearColor ) { m_vClearColor = i_vClearColor; }
	inline vector4 &vGetClearColor() { return m_vClearColor; }

	inline void SetAmbientLightColor( vector4 i_vAmbientLightColor ) { m_vAmbientLightColor = i_vAmbientLightColor; }
	inline const vector4 &vGetAmbientLightColor() { return m_vAmbientLightColor; }

	inline uint32 iGetNumLights() { return (uint32)m_SceneLights.size(); }
	inline CLight *pGetLightFromNum( uint32 i_iNum ) { return m_SceneLights[i_iNum].pLight; }
	
	inline void SetCurrentLight( uint32 i_iNum ) { m_iCurLight = i_iNum; }
	inline CLight *pGetCurrentLight() { return pGetLightFromNum( m_iCurLight ); }

private:
	class IApplication *m_pParent;

	vector4	m_vClearColor;
	vector4	m_vAmbientLightColor;

	map<string, PCREATEFUNCTION>	m_RegisteredEntityTypes;

	struct tSceneEntity
	{
		HENTITY			hEntity;
		class IEntity	*pEntity;
		bool			bSceneProcess;
	};
	vector<tSceneEntity>	m_SceneEntities;
	uint32					m_iNumCreatedEntities;

	struct tSceneLight
	{
		HLIGHT	hLight;
		CLight	*pLight;
	};
	vector<tSceneLight>	m_SceneLights;
	uint32				m_iNumCreatedLights;
	uint32				m_iCurLight;

private:
	vector<tSceneEntity>::iterator pSceneEntityIterator( HENTITY i_hEntity );
	vector<tSceneLight>::iterator pGetSceneLightIterator( HLIGHT i_hLight );
};

#endif // __SCENE_H__
