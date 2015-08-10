
#ifndef ___ENVSPHERE_H__
#define ___ENVSPHERE_H__

#include "common.h"
#include "../libappframework/include/application.h"
#include "../libappframework/include/scene.h"

class CEnvSphere : public CApplication
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

	HENTITY m_hSphere;
	HLIGHT	m_hLight;
};

#endif // ___ENVSPHERE_H__
