
#ifndef ___SPHERICALSCALEMAPPING_H__
#define ___SPHERICALSCALEMAPPING_H__

#include "common.h"
#include "../libappframework/include/application.h"
#include "../libappframework/include/scene.h"

class CSphericalScaleMapping : public CApplication
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

#endif // ___SPHERICALSCALEMAPPING_H__
