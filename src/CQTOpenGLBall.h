//
// Created by richard on 19/08/15.
//

#ifndef ARGOS3_BULLET_CQTOPENGLBALL_H
#define ARGOS3_BULLET_CQTOPENGLBALL_H

class CSphereEntity;
class CQTOpenGLBall;

#include <GL/gl.h>

/**
 * Renderer for a ball
 */
class CQTOpenGLBall
{
public:
	CQTOpenGLBall();

	virtual ~CQTOpenGLBall();

	virtual void Draw(const CSphereEntity &c_entity);

private:
	void MakeBody();

private:
	GLuint drawListId;
};

#endif //ARGOS3_BULLET_CQTOPENGLBALL_H
