
#ifndef ___PARALLAXTRI_H__
#define ___PARALLAXTRI_H__

#include "common.h"
#include "../libappframework/include/application.h"
#include "../libappframework/include/scene.h"

class CParallaxTri : public CApplication
{
public:
	bool bCreateWorld();
	void DestroyWorld();

	void FrameMove();
	void RenderWorld();

private:

public:

private:
	class CMyCamera *m_pCamera;

	HENTITY m_hTriangle;
	HLIGHT	m_hLight;
};

#endif // ___PARALLAXTRI_H__
