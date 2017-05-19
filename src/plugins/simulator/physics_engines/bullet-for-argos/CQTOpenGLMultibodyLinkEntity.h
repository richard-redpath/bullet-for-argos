//
// Created by rar500 on 20/11/15.
//

#ifndef ARGOS3_BULLET_CQTOPENGLURDFLINKENTITY_H
#define ARGOS3_BULLET_CQTOPENGLURDFLINKENTITY_H

#include <GL/gl.h>
#include "CMultibodyLinkEntity.h"

/**
 * Renderer for Multibody link entities
 */
class CQTOpenGLMultibodyLinkEntity
{
public:
	CQTOpenGLMultibodyLinkEntity(){ }

	virtual ~CQTOpenGLMultibodyLinkEntity(){};

	void Draw(CMultibodyLinkEntity & c_entity);

private:
	GLuint LazyLoadModel(CMultibodyLinkEntity & entity);
	std::map<std::string, GLuint> modelDrawListIds;
};


#endif //ARGOS3_BULLET_CQTOPENGLURDFENTITY_H
