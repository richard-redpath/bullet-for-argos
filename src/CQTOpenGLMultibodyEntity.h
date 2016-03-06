//
// Created by richard on 30/11/15.
//

#ifndef ARGOS3_BULLET_CQTOPENGLURDFENTITY_H
#define ARGOS3_BULLET_CQTOPENGLURDFENTITY_H

#include <GL/gl.h>
#include "CMultibodyEntity.h"

/**
 * Renderer for a multibodied entity. Simply calls the render methods for each of the component links
 */
class CQTOpenGLMultibodyEntity
{
public:
	CQTOpenGLMultibodyEntity(){};

	virtual ~CQTOpenGLMultibodyEntity(){};

	void Draw(CMultibodyEntity & c_entity);
};

#endif //ARGOS3_BULLET_CQTOPENGLURDFENTITY_H
