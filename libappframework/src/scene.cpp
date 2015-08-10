
#include "../include/scene.h"
#include "../include/graphics.h"
#include "../include/application.h"
#include "../include/entity.h"
#include "../include/light.h"

CScene::CScene( IApplication *i_pParent )
{
	m_pParent = i_pParent;

	m_iNumCreatedEntities = 0;
	m_iNumCreatedLights = 0;

	SetClearColor( vector4( 0.30f, 0.25f, 0.35f, 1 ) );
	SetAmbientLightColor( vector4( 0, 0, 0, 1 ) );

	m_iCurLight = 0;
}

CScene::~CScene()
{
	while( m_SceneEntities.size() )
		ReleaseEntity( m_SceneEntities.begin()->hEntity );
	while( m_SceneLights.size() )
		ReleaseLight( m_SceneLights.begin()->hLight );
}

bool CScene::bInitialize()
{
	return true;
}

void CScene::RegisterEntityType( string i_sTypeName, PCREATEFUNCTION i_pCreateFunction )
{
	m_RegisteredEntityTypes[i_sTypeName] = i_pCreateFunction;
}

HENTITY CScene::hCreateEntity( string i_sTypeName, bool i_bSceneProcess )
{
	PCREATEFUNCTION pCreateFunction = m_RegisteredEntityTypes[i_sTypeName];
	if( !pCreateFunction )
		return 0;

	IEntity *pEntity = pCreateFunction( this );
	if( !pEntity )
		return 0;

	tSceneEntity newEntity = { ++m_iNumCreatedEntities, pEntity, i_bSceneProcess };
	m_SceneEntities.push_back( newEntity );
	return newEntity.hEntity;
}

vector<CScene::tSceneEntity>::iterator CScene::pSceneEntityIterator( HENTITY i_hEntity )
{
	if( !i_hEntity ) return m_SceneEntities.end();
	for( vector<tSceneEntity>::iterator pSceneEntity = m_SceneEntities.begin(); pSceneEntity != m_SceneEntities.end(); ++pSceneEntity )
	{
		if( pSceneEntity->hEntity == i_hEntity )
			return pSceneEntity;
	}
	return m_SceneEntities.end();
}

void CScene::ReleaseEntity( HENTITY i_hEntity )
{
	vector<tSceneEntity>::iterator pSceneEntity = pSceneEntityIterator( i_hEntity );
	if( pSceneEntity != m_SceneEntities.end() )
	{
		uint32 iOldSize = (uint32)m_SceneEntities.size();
		if( pSceneEntity->pEntity )
			delete pSceneEntity->pEntity; // must not use safe_delete here, because after delete the pointer might not be valid anymore!! safe_delete would set it to 0 ...
		
		if( iOldSize != m_SceneEntities.size() )
			pSceneEntity = pSceneEntityIterator( i_hEntity );

		m_SceneEntities.erase( pSceneEntity );
	}
}

IEntity *CScene::pGetEntity( HENTITY i_hEntity )
{
	vector<tSceneEntity>::iterator pSceneEntity = pSceneEntityIterator( i_hEntity );
	if( pSceneEntity != m_SceneEntities.end() )
		return pSceneEntity->pEntity;
	return 0;
}

HLIGHT CScene::hCreateLight()
{
	tSceneLight newLight = { ++m_iNumCreatedEntities, new CLight( this ) };
	m_SceneLights.push_back( newLight );
	return newLight.hLight;
}

vector<CScene::tSceneLight>::iterator CScene::pGetSceneLightIterator( HLIGHT i_hLight )
{
	if( !i_hLight ) return m_SceneLights.end();
	for( vector<tSceneLight>::iterator pSceneLight = m_SceneLights.begin(); pSceneLight != m_SceneLights.end(); ++pSceneLight )
	{
		if( pSceneLight->hLight == i_hLight )
			return pSceneLight;
	}
	return m_SceneLights.end();
}

void CScene::ReleaseLight( HLIGHT i_hLight )
{
	vector<tSceneLight>::iterator pSceneLight = pGetSceneLightIterator( i_hLight );
	if( pSceneLight != m_SceneLights.end() )
	{
		uint32 iOldSize = (uint32)m_SceneLights.size();
		if( pSceneLight->pLight )
			delete pSceneLight->pLight; // must not use safe_delete here, because after delete the pointer might not be valid anymore!! safe_delete would set it to 0 ...

		if( iOldSize != m_SceneLights.size() )
			pSceneLight = pGetSceneLightIterator( i_hLight );

		m_SceneLights.erase( pSceneLight );
	}
}

CLight *CScene::pGetLight( HLIGHT i_hLight )
{
	vector<tSceneLight>::iterator pSceneLight = pGetSceneLightIterator( i_hLight );
	if( pSceneLight != m_SceneLights.end() )
		return pSceneLight->pLight;
	return 0;
}

void CScene::FrameMove()
{
	for( vector<tSceneEntity>::iterator pSceneEntity = m_SceneEntities.begin(); pSceneEntity != m_SceneEntities.end(); ++pSceneEntity )
	{
		if( !pSceneEntity->bSceneProcess )
			continue;

		pSceneEntity->pEntity->bFrameMove();
		// TODO: if true, update scene-graph
	}
}

void CScene::Render( uint32 i_iPass )
{
	CGraphics *pGraphics = pGetParent()->pGetGraphics();
	for( vector<tSceneEntity>::iterator pSceneEntity = m_SceneEntities.begin(); pSceneEntity != m_SceneEntities.end(); ++pSceneEntity )
	{
		if( !pSceneEntity->bSceneProcess )
			continue;

		pGraphics->PushStateBlock();
		pSceneEntity->pEntity->Render( i_iPass );
		pGraphics->PopStateBlock();
	}
}
