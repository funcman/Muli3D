
#ifndef __APP_H__
#define __APP_H__

#include "common.h"
#include "../libappframework/include/application.h"
#include "../libappframework/include/scene.h"

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

	HENTITY m_hBubble;
};

#endif // __APP_H__
