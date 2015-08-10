
#ifndef ___APP_H__
#define ___APP_H__

#include "common.h"
#include "../libappframework/include/application.h"
#include "../libappframework/include/scene.h"
#include "../libappframework/include/resmanager.h"

class CApp : public CApplication
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

	HENTITY		m_hRaytracer;
	HRESOURCE	m_hEarth, m_hMoon;
};

#endif // ___APP_H__
