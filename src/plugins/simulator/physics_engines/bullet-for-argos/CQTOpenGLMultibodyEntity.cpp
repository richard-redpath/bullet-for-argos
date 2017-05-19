//
// Created by rar500 on 20/11/15.
//

#include "CQTOpenGLMultibodyEntity.h"

#include <argos3/plugins/simulator/visualizations/qt-opengl/qtopengl_widget.h>

using namespace argos;

void CQTOpenGLMultibodyEntity::Draw(CMultibodyEntity & entity)
{
}

/**
 * Renderer for a multibodied entity. Delegates all drawing to the link renderer for each part
 */
class CQTOpenGLOperationDrawMultibodyNormal : public CQTOpenGLOperationDrawNormal {
public:
	void ApplyTo(CQTOpenGLWidget& c_visualization,
				 CMultibodyEntity & entity) {
		for(auto pair : entity.getLinkEntityMap())
		{
			CallEntityOperation<CQTOpenGLOperationDrawNormal, CQTOpenGLWidget, void>(c_visualization, *pair.second);
		}
	}
};

class CQTOpenGLOperationDrawMultibodySelected : public CQTOpenGLOperationDrawSelected {
public:
	void ApplyTo(CQTOpenGLWidget& c_visualization,
				 CMultibodyEntity & entity) {
	}
};


REGISTER_QTOPENGL_ENTITY_OPERATION(CQTOpenGLOperationDrawNormal, CQTOpenGLOperationDrawMultibodyNormal, CMultibodyEntity);

REGISTER_QTOPENGL_ENTITY_OPERATION(CQTOpenGLOperationDrawSelected, CQTOpenGLOperationDrawMultibodySelected, CMultibodyEntity);