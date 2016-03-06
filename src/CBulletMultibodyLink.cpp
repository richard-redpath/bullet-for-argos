//
// Created by richard on 25/11/15.
//

#include <BulletCollision/Gimpact/btGImpactShape.h>
#include "CBulletMultibodyLink.h"

bool propertiesSet(const MaterialInstance& instance)
{
    return (!isnan(instance.properties.dampening) && !isnan(instance.properties.friction));
}

/**
 * Constructor for a multibodied entity part.
 *
 * Extracts the required information from the provided ARGoS entity and adds an appropriate
 * rigid body shape to the simulation
 */
CBulletMultibodyLink::CBulletMultibodyLink(CBulletEngine &engine, CMultibodyLinkEntity &entity) : CBulletModel(engine, entity.GetEmbodiedEntity())
{
	// Retain a reference to the entity and engine
    this->engine = &engine;
    this->entity = &entity;

    const Link& link = entity.getCurrentState();

	// Create a new compound collision shape.
	// This may only hold 1 actual shape but it is easier to create a
	// compound shape and add to it as necessary
    btCompoundShape* collisionShape = new btCompoundShape{};

	// Cycle over each definition in the link's collision definition
    for(auto part : link.collision)
    {
		// Add the appropriate collision shape depending on it's type
        switch(part.type)
        {
            case Box:
                addBoxToCollisionShape(collisionShape, part);
                break;
            case Cylinder:
                addCylinderToCollisionShape(collisionShape, part);
                break;
            case Sphere:
                addSphereToCollisionShape(collisionShape, part);
                break;
            case Mesh:
                addMeshToCollisionShape(collisionShape, part);
                break;
        }
    }

	// Initialise the inertia tensor to the provided value or let bullet calculate it
    btVector3 inertia;
    if(link.inertiaProvided)
        inertia.setValue(link.ixx, link.iyy, link.izz);
    else
        collisionShape->calculateLocalInertia(link.mass, inertia);

	// Get the required orientation and position from the ARGoS entity
	CQuaternion argosOrientation = entity.GetEmbodiedEntity().GetOriginAnchor().Orientation;
    CVector3 argosPosition = entity.GetEmbodiedEntity().GetOriginAnchor().Position;

	// Build the required bullet transform
    btTransform transform = bulletTransformFromARGoS(argosPosition, argosOrientation);

	// Create a rigid body with the calculated shape
    btMotionState* motionState = new btDefaultMotionState{transform};
    btRigidBody* body = new btRigidBody{link.mass, motionState, collisionShape, inertia};

	// Check we have some physical properties and fail if not
    if(!propertiesSet(link.collisionMaterial))
        THROW_ARGOSEXCEPTION("Attempting to use a material without necessary properties in link "<<link.name<<". Collision materials are currently only supported up to the level of links, not individual collision shapes.");

	// Setup our body
    body->setFriction(link.collisionMaterial.properties.friction);
    body->setRestitution(1-link.collisionMaterial.properties.dampening);
    body->setActivationState(DISABLE_DEACTIVATION);

	// Retain a reference to it
    rigidBody = body;

	// Initial sync for the physics engine and simulator
    UpdateEntityStatus();
    UpdateFromEntityStatus();
}

/**
 * Add a new shape to the provided compound shape in the position specified
 */
inline void addShapeToCompound(btCompoundShape* compund, btCollisionShape* toAdd, GeometrySpecification &spec)
{
	// Create the transform
    btVector3 offset{spec.originX, spec.originY, spec.originZ};
    btQuaternion rotation{spec.yaw, spec.pitch, spec.roll};
    btTransform transform {rotation, offset};

	// And do the add
    compund->addChildShape(transform, toAdd);
}

/**
 * Add a box to the collision shape
 */
void CBulletMultibodyLink::addBoxToCollisionShape(btCompoundShape *pShape, GeometrySpecification& spec) {
	// Create the box shape
    btCollisionShape* shape = new btBoxShape{0.5f * btVector3{spec.box.x, spec.box.y, spec.box.z}};

	// And add it
	addShapeToCompound(pShape, shape, spec);
}

/**
 * Add a cylinder to the collision shape
 */
void CBulletMultibodyLink::addCylinderToCollisionShape(btCompoundShape *pShape, GeometrySpecification& spec) {
	// Build the cylinder model
    btCollisionShape* shape = new btCylinderShape{btVector3{spec.cylinder.radius, 0.5f * spec.cylinder.length, spec.cylinder.radius}};

	// And add it
	addShapeToCompound(pShape, shape, spec);
}

/**
 * Add a sphere to the provided collision shape
 */
void CBulletMultibodyLink::addSphereToCollisionShape(btCompoundShape *pShape, GeometrySpecification& spec) {
	// Build the shape
    btCollisionShape* shape = new btSphereShape{spec.sphere.radius};

	// And add it
	addShapeToCompound(pShape, shape, spec);
}

/**
 * Add a mesh to the collision shape
 */
void CBulletMultibodyLink::addMeshToCollisionShape(btCompoundShape *pShape, GeometrySpecification& spec) {

	// This may, itself, be built up of multiple sub shapes
    btCompoundShape *meshShape = new btCompoundShape;

	// Get the mesh info
    auto meshInfo = *spec.mesh.mesh;

	// For every sub-shape in the mesh
    for(auto key : meshInfo.keys)
    {
		// Create an accessor for the trimesh data
        btTriangleIndexVertexArray *vert
                = new btTriangleIndexVertexArray((int)meshInfo.numIndices[key] / 3, meshInfo.indicesMap[key],
                                                 3 * sizeof(int), (int)meshInfo.numVerts[key], meshInfo.vertsMap[key],
                                                 3 * sizeof(float));

		// Create the collision sub-shape
        btGImpactMeshShape *shape = new btGImpactMeshShape {vert};

		// Set its scale factor and update its bounds
        shape->setLocalScaling(btVector3{1, 1, 1});
        shape->updateBound();

		// Add the sub-shape to the mesh with not transformation
        meshShape->addChildShape(btTransform{btQuaternion{0,0,0}, btVector3{0, 0, 0}}, shape);
    }

	// Scale the mesh by the specified amount
	meshShape->setLocalScaling(btVector3{spec.mesh.sx, spec.mesh.sy, spec.mesh.sz});

	// And add it to the main compound shape
	addShapeToCompound(pShape, meshShape, spec);
}

/**
 * Update the physics model from the ARGoS entity
 */
void CBulletMultibodyLink::UpdateFromEntityStatus() {
	position = entity->GetEmbodiedEntity().GetOriginAnchor().Position;
	orientation = entity->GetEmbodiedEntity().GetOriginAnchor().Orientation;
	rigidBody->setWorldTransform(bulletTransformFromARGoS(position, orientation));
}

/**
 * Update the ARGoS entity from the physics model
 */
void CBulletMultibodyLink::UpdateEntityStatus() {
	bulletTransformToARGoS(rigidBody->getWorldTransform(), position, orientation);
	entity->GetEmbodiedEntity().GetOriginAnchor().Position = position;
	entity->GetEmbodiedEntity().GetOriginAnchor().Orientation = orientation;
}

/**
 * Get the AABB from the physics engine and mirror it to the ARGoS AABB
 */
void CBulletMultibodyLink::CalculateBoundingBox() {
    btVector3 min, max;
    GetRigidBody()->getAabb(min, max);
    GetBoundingBox().MinCorner.Set(min.getX(), min.getY(), min.getZ());
    GetBoundingBox().MaxCorner.Set(max.getX(), max.getY(), max.getZ());
}

/**
 * Check for ray intersection. Not yet implemented.
 */
bool CBulletMultibodyLink::CheckIntersectionWithRay(Real &f_t_on_ray, const CRay3 &ray) const {
    return false;
}

REGISTER_BULLET_ENTITY_OPS(CMultibodyLinkEntity, CBulletMultibodyLink)