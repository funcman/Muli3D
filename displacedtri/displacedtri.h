
#ifndef ___DISPLACEDTRI_H__
#define ___DISPLACEDTRI_H__

#include "common.h"
#include "../libappframework/include/application.h"
#include "../libappframework/include/scene.h"

class CDisplacedTri : public CApplication
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

#endif // ___DISPLACEDTRI_H__
