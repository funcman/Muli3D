
#ifndef ___DISPLACEDSPHERE_H__
#define ___DISPLACEDSPHERE_H__

#include "common.h"
#include "../libappframework/include/application.h"
#include "../libappframework/include/scene.h"

class CDisplacedSphere : public CApplication
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
};

#endif // ___DISPLACEDSPHERE_H__
