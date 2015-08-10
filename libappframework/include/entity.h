
#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "base.h"

class IEntity
{
protected:
	friend class CScene;

public:
	virtual ~IEntity() {};

	virtual bool bFrameMove() = 0;	// returns true if the object has been moved -> scenegraph-care
	virtual void Render( uint32 i_iPass ) = 0;

protected:

public:
	inline class CScene *pGetParent() { return m_pParent; }

protected:
	class CScene *m_pParent;
};

#endif // __ENTITY_H__
