//
// Created by richard on 19/08/15.
//

#include "CQTOpenGLBall.h"
#include "CSphereEntity.h"
#include <argos3/plugins/simulator/visualizations/qt-opengl/qtopengl_widget.h>

using namespace argos;

/**
 * Default colour for primitive
 */
const GLfloat MOVABLE_COLOR[]    = { 1.0f, 0.0f, 0.0f, 1.0f };
const GLfloat NONMOVABLE_COLOR[] = { 0.7f, 0.7f, 0.7f, 1.0f };
const GLfloat SPECULAR[]         = { 0.0f, 0.0f, 0.0f, 1.0f };
const GLfloat SHININESS[]        = { 0.0f                   };
const GLfloat EMISSION[]         = { 0.0f, 0.0f, 0.0f, 1.0f };

/**
 * Constructor reserves a call list and initialises it
 */
CQTOpenGLBall::CQTOpenGLBall()
{
	/* Reserve the needed display lists */
	drawListId = glGenLists(1);

	/* Make body list */
	glNewList(drawListId, GL_COMPILE);
	MakeBody();
	glEndList();
}

/**
 * Destructor releases the call list
 */
CQTOpenGLBall::~CQTOpenGLBall() {
	glDeleteLists(drawListId, 1);
}

/**
 * Call the list with the appropriate colour/material set
 */
void CQTOpenGLBall::Draw(const CSphereEntity & c_entity) {

	// Set the appropriate visual properties
	if(c_entity.GetEmbodiedEntity().IsMovable()) {
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, MOVABLE_COLOR);
	}
	else {
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, NONMOVABLE_COLOR);
	}

	// Push the state matrix
	glPushMatrix();

	// Scale to the size of the sphere
	glScalef(c_entity.GetRadius(), c_entity.GetRadius(), c_entity.GetRadius());

	// Draw a generic sphere
	glCallList(drawListId);

	// Restore the state matrix
	glPopMatrix();
}

#define PI_FLOAT ((float)(M_PI))

/**
 * Draws a sphere as 2 end caps and a bunch of quad layers
 */
void CQTOpenGLBall::MakeBody() {
	// Set the material
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, SPECULAR);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, SHININESS);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, EMISSION);

	glShadeModel(GL_SMOOTH);

	// What resolution should we draw at?
	int segments = 32;
	int layers = 32;

	// Top cap
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0, 0, 1);
	for(int i = 0; i < segments; ++i)
	{
		double phi = PI_FLOAT/layers;
		double theta = i * 2 * PI_FLOAT/segments;
		glVertex3f(cos(theta)*sin(phi), sin(theta)*sin(phi), cos(phi));
	}
	glEnd();

	// Middle layers
	glBegin(GL_QUADS);
	for(int layer = 1; layer < layers; ++layer)
	{
		double phi = layer * PI_FLOAT/layers;
		double nextPhi = (layer + 1) * PI_FLOAT/layers;

		for(int segment = 0; segment < segments; ++segment)
		{
			double theta = segment * 2 * PI_FLOAT / segments;
			double nextTheta = (segment + 1) * 2 * PI_FLOAT / segments;

			glVertex3f(cos(theta) * sin(phi), sin(theta) * sin(phi), cos(phi));
			glVertex3f(cos(theta) * sin(nextPhi), sin(theta) * sin(nextPhi), cos(nextPhi));
			glVertex3f(cos(nextTheta) * sin(nextPhi), sin(nextTheta) * sin(nextPhi), cos(nextPhi));
			glVertex3f(cos(nextTheta) * sin(phi), sin(nextTheta) * sin(phi), cos(phi));
		}
	}
	glEnd();

	// End cap
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0, 0, -1);
	for(int i = 0; i < segments; ++i)
	{
		double phi = (layers-1) * PI_FLOAT/layers;
		double theta = i * 2 * PI_FLOAT/segments;
		glVertex3f(cos(theta)*sin(phi), sin(theta)*sin(phi), cos(phi));
	}
	glEnd();
}

/**
 * Operation to draw the provided sphere
 */
class CQTOpenGLOperationDrawBallNormal : public CQTOpenGLOperationDrawNormal {
public:
	void ApplyTo(CQTOpenGLWidget& c_visualization,
				 CSphereEntity & c_entity) {
		static CQTOpenGLBall m_cModel;
		c_visualization.DrawEntity(c_entity.GetEmbodiedEntity());
		m_cModel.Draw(c_entity);
	}
};

/**
 * Operation to draw the selected sphere with a bounding box
 */
class CQTOpenGLOperationDrawBallSelected : public CQTOpenGLOperationDrawSelected {
public:
	void ApplyTo(CQTOpenGLWidget& c_visualization,
				 CSphereEntity & c_entity) {
		c_visualization.DrawBoundingBox(c_entity.GetEmbodiedEntity());
	}
};


REGISTER_QTOPENGL_ENTITY_OPERATION(CQTOpenGLOperationDrawNormal, CQTOpenGLOperationDrawBallNormal, CSphereEntity);

REGISTER_QTOPENGL_ENTITY_OPERATION(CQTOpenGLOperationDrawSelected, CQTOpenGLOperationDrawBallSelected, CSphereEntity);