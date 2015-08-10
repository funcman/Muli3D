
#ifndef __LIGHT_H__
#define __LIGHT_H__

#include "base.h"

class CLight
{
private:
	friend class CScene;
	inline CLight( class CScene *i_pParent ) { m_pParent = i_pParent; }

public:
	inline ~CLight() {};

private:

public:
	inline class CScene *pGetParent() { return m_pParent; }

	inline void SetPosition( const vector3 &i_vPosition ) { m_vPosition = i_vPosition; }
	inline const vector3 &vGetPosition() { return m_vPosition; }

	inline void SetRange( float32 i_fRange ) { m_fRange = i_fRange; }
	inline float32 fGetRange() { return m_fRange; }

	inline void SetColor( const vector4 &i_vColor ) { m_vColor = i_vColor; }
	inline const vector4 &vGetColor() { return m_vColor; }

private:
	class CScene *m_pParent;

	vector3	m_vPosition;
	float32	m_fRange;
	vector4 m_vColor;
};

#endif // __LIGHT_H__
