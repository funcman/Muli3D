
#ifndef __MYCAMERA_H__
#define __MYCAMERA_H__

#include "../libappframework/include/camera.h"
#include "common.h"

enum ePass
{
	ePass_Lighting			// per-light: calculate per-pixel-lighting
};

class CMyCamera : public CCamera
{
public:
	CMyCamera( class CGraphics *i_pParent );

	void RenderPass( int32 i_iPass = -1 );

private:

public:

private:

};

#endif // __MYCAMERA_H__
