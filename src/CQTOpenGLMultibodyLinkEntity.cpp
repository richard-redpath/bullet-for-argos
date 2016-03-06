//
// Created by rar500 on 20/11/15.
//

#include "CQTOpenGLMultibodyLinkEntity.h"

#include <argos3/plugins/simulator/visualizations/qt-opengl/qtopengl_widget.h>

using namespace argos;

/**
 * Issue the appropriate OpenGL calls to render a box
 */
inline void drawBox(BoxAttributes &box)
{
	// Generate the vertex coordinates
	float verts[] = { -0.5f * box.x, -0.5f * box.y, -0.5f * box.z,	// Bottom
					   0.5f * box.x, -0.5f * box.y, -0.5f * box.z,	// Bottom
					   0.5f * box.x, -0.5f * box.y,  0.5f * box.z,	// Bottom
					  -0.5f * box.x, -0.5f * box.y,  0.5f * box.z,	// Bottom
					  -0.5f * box.x,  0.5f * box.y, -0.5f * box.z,	// Top
					   0.5f * box.x,  0.5f * box.y, -0.5f * box.z,	// Top
					   0.5f * box.x,  0.5f * box.y,  0.5f * box.z,	// Top
					  -0.5f * box.x,  0.5f * box.y,  0.5f * box.z};	// Top

	// Face indices
	static unsigned short indices[] = {	0, 3, 2, 1,		// Bottom
										7, 4, 5, 6,		// Top
										7, 3, 0, 4,		// Left
										5, 1, 2, 6,		// Right
										6, 2, 3, 7,		// Back
										4, 0, 1, 5};	// Front

	glBegin(GL_QUADS);

	// For each vertex to be drawn (4 for each face = 24 total)
	for(int i = 0; i < 24; i++)
	{
		// Get a reference to the X coordinate of this vertex
		float* vert = &verts[3*indices[i]];

		// And issue the draw call with the appropriate X, Y and Z
		glVertex3f(vert[0], vert[1], vert[2]);
	}

	glEnd();
}

#define PI_FLOAT ((float)(M_PI))

/**
 * Draw a sphere as 2 end caps and a series of square strips
 */
inline void drawSphere(SphereAttributes& sphere)
{
	float radius = sphere.radius;

	glPushMatrix();
	glEnable(GL_NORMALIZE);
	glScalef(radius, radius, radius);

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

	glDisable(GL_NORMALIZE);
	glPopMatrix();
}

/**
 * Draw a cylinder as 2 end segments and a collection of rectangles
 */
inline void drawCylinder(CylinderAttributes& cylinder)
{
	float radius = cylinder.radius;
	float length = cylinder.length;

	int segments = 64;

	// Draw bottom
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0, 0, 0);
	for(int i = 0; i <= segments; ++i)
	{
		float angle = i * (2*PI_FLOAT/segments);
		float x = sin(angle) * radius;
		float z = cos(angle) * radius;
		glVertex3f(x, 0, z);
	}
	glEnd();

	// Draw top
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0, 0, length);
	for(int i = 0; i <= segments; ++i)
	{
		float angle = i * (2*PI_FLOAT/segments);
		float x = sin(angle) * radius;
		float z = cos(angle) * radius;
		glVertex3f(x, length, z);
	}
	glEnd();

	// Draw sides
	glBegin(GL_QUADS);
	for(int i = 0; i <= segments; ++i)
	{
		float angle1 = i * (2*PI_FLOAT/segments);
		float x1 = sin(angle1) * radius;
		float z1 = cos(angle1) * radius;

		float angle2 = (i+1) * (2*PI_FLOAT/segments);
		float x2 = sin(angle2) * radius;
		float z2 = cos(angle2) * radius;

		glVertex3f(x1, 0, z1);
		glVertex3f(x1, length, z1);
		glVertex3f(x2, length, z2);
		glVertex3f(x2, 0, z2);
	}
	glEnd();
}

/**
 * Draw an arbitrary trimesh with the provided texture
 */
inline void drawMesh(MeshAttributes& mesh)
{
	// Get the mesh info
	MeshInfo* info = mesh.mesh;

	// Iterate over all sub-meshes
	for(auto key : info->keys)
	{
		// Get the verts, indices and texture coordinates of this sub-mesh
		float *verts = info->vertsMap[key];
		int *inds = info->indicesMap[key];
		float* uvs = info->uvsMap[key];

		// Get the texture info
		MeshTexture* texture = info->textures[key];

		// If this mesh should have a texture then its "renderId" will be non-zero
		bool hasTexture = texture->renderId > 0;

		// If it has one then bind it
		if(hasTexture)
			glBindTexture(GL_TEXTURE_2D, texture->renderId);

		// Start drawing triangles
		glBegin(GL_TRIANGLES);
		for (int i = 0; i < info->numIndices[key]; ++i)
		{
			// Get the coordinates for the current vertex
			int idx = inds[i];
			float x = verts[3 * idx + 0];
			float y = verts[3 * idx + 1];
			float z = verts[3 * idx + 2];

			// Set the texture coordinates if they are provided
			if(hasTexture)
				glTexCoord2f(uvs[idx*2], uvs[idx*2+1]);

			// And draw the vertex
			glVertex3f(x, y, z);
		}

		// Finish drawing this submesh
		glEnd();

		// And unbind the texture
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

static const GLfloat SPECULAR[] = {0.0f, 0.0f, 0.0f, 1.0f};
static const GLfloat SHININESS[] = {0.0f};
static const GLfloat EMISSION[] = {0.0f, 0.0f, 0.0f, 1.0f};

/**
 * Bind the appropriate colour for this mesh
 */
inline void setupMaterial(GeometrySpecification& item)
{
	// Default reflective properties
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, SPECULAR);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, SHININESS);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, EMISSION);

	// Specified colour
	GLfloat colour[] = {item.materialColour.red, item.materialColour.green, item.materialColour.blue, item.materialColour.alpha};
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, colour);
}

/**
 * Bind any mesh images to OpenGL textures
 */
inline void setupMeshTextures(MeshInfo* mesh)
{
	// Setup textures
	for(auto key : mesh->keys)
	{
		// Key the texture info
		MeshTexture* texture = mesh->textures[key];

		// As long as there is image data
		if (texture->data != nullptr)
		{
			// Unbind a any previous texture
			glBindTexture(GL_TEXTURE_2D, 0);

			// Create a new texture id
			glGenTextures(1, &texture->renderId);

			// Bind it
			glBindTexture(GL_TEXTURE_2D, texture->renderId);

			// Setup filtering and coordinate handling
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			// Bind the image data
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texture->width, texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
						 texture->data);

			// Unbind it
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}
}

/**
 * Checks if a model has had its draw lists already created and returns the
 * first ID immediately if so, otherwise a draw list will be created and set up
 */
GLuint CQTOpenGLMultibodyLinkEntity::LazyLoadModel(CMultibodyLinkEntity & entity)
{
	// Check if the entity already has a draw list
	GLuint firstListId = modelDrawListIds[entity.GetId()];

	// Return it if so
	if(firstListId > 0)
		return firstListId;

	// Otherwise get our link
	auto link = entity.getCurrentState();

	// Create our OpenGL display lists
	auto& visualVector = link.visual;
	firstListId = glGenLists(visualVector.size());

	// For each visual element
	for(int i = 0; i < visualVector.size(); ++i)
	{
		// Get the item
		auto& item = visualVector[i];

		// Create a new list
		glNewList(firstListId + i, GL_COMPILE);

		// Setup our material
		setupMaterial(item);

		// Then call the appropriate draw method to issue the correct calls
		switch (item.type)
		{
			case Box:
				drawBox(item.box);
				break;
			case Sphere:
				drawSphere(item.sphere);
				break;
			case Cylinder:
				drawCylinder(item.cylinder);
				break;
			case Mesh:
				setupMeshTextures(item.mesh.mesh);
				drawMesh(item.mesh);
				break;
		}

		// End this list
		glEndList();
	}

	// Save the id of the first list
	modelDrawListIds[entity.GetId()] = firstListId;

	// And return it
	return firstListId;
}

/**
 * Draw an entity
 */
void CQTOpenGLMultibodyLinkEntity::Draw(CMultibodyLinkEntity & entity)
{
	// Ensure textures are enabled
	if(!glIsEnabled(GL_TEXTURE_2D))
		glEnable(GL_TEXTURE_2D);

	// Unbind any previously bound texture
	glBindTexture(GL_TEXTURE_2D, 0);

	// Lazy load the model and get the first draw list
	GLuint firstListId = LazyLoadModel(entity);

	// Get the link
	const Link& link = entity.getCurrentState();

	// Disable face culling and save its current state to restore later
	GLboolean cullingEnabled = glIsEnabled(GL_CULL_FACE);
	glDisable(GL_CULL_FACE);

	// Get the visual elements
	auto& visualVector = link.visual;

	// For each one
	for(int i = 0; i < visualVector.size(); ++i)
	{
		// Save the state matrix
		glPushMatrix();

		// Rotate by the appropriate amount
		auto& item = visualVector[i];
		glTranslatef(item.originX, item.originY, item.originZ);
		glRotatef(item.pitch, 1, 0, 0);
		glRotatef(item.roll, 0, 1, 0);
		glRotatef(item.yaw, 0, 0, 1);

		// Call the correct call list
		glCallList(firstListId + i);

		// Restore the state matrix
		glPopMatrix();
	}

	// Re-enable culling if necessary
	if(cullingEnabled)
		glEnable(GL_CULL_FACE);
}

/**
 * Draw operation for link entities
 */
class CQTOpenGLOperationDrawURDFLinkNormal : public CQTOpenGLOperationDrawNormal {
public:
	void ApplyTo(CQTOpenGLWidget& c_visualization,
				 CMultibodyLinkEntity & c_entity) {
		static CQTOpenGLMultibodyLinkEntity model;

		// Save state matrix
		glPushMatrix();

		// Translate to the correct location
		c_visualization.DrawEntity(c_entity.GetEmbodiedEntity());

		// Draw the entity
		model.Draw(c_entity);

		// And restore the OpenGL state
		glPopMatrix();
	}
};

/**
 * Selected renderer for link entities
 */
class CQTOpenGLOperationDrawURDFLinkSelected : public CQTOpenGLOperationDrawSelected {
public:
	void ApplyTo(CQTOpenGLWidget& c_visualization,
				 CMultibodyLinkEntity & c_entity) {
		c_visualization.DrawBoundingBox(c_entity.GetEmbodiedEntity());
	}
};


REGISTER_QTOPENGL_ENTITY_OPERATION(CQTOpenGLOperationDrawNormal, CQTOpenGLOperationDrawURDFLinkNormal, CMultibodyLinkEntity);

REGISTER_QTOPENGL_ENTITY_OPERATION(CQTOpenGLOperationDrawSelected, CQTOpenGLOperationDrawURDFLinkSelected, CMultibodyLinkEntity);