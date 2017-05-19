//
// Created by richard on 17/08/15.
//

#include "CBulletCubeModel.h"
#include <argos3/core/utility/math/box.h>

/**
 * Creates a box to be added to the world. Created boxes have a mass and restitution of 0.5
 */
CBulletCubeModel::CBulletCubeModel(CBulletEngine& engine, CBoxEntity& entity)
		: CBulletModel(engine, entity.GetEmbodiedEntity()),
		  positionOffset(0,0,entity.GetSize().GetZ() * 0.5)
{
	// Retain a reference to the engine and associated ARGoS entity
	this->entity = &entity;
	this->engine = &engine;

	// Bullet requires boxes to be initialised with half side lengths
	CVector3 dims = entity.GetSize() * 0.5 * engine.worldScale;
	collisionShape = new btBoxShape{btVector3{(btScalar)dims.GetX(), (btScalar)dims.GetY(), (btScalar)dims.GetZ()}};

	// Setup the location of the box in bullet compensating for the difference between ARGoS and Bullet coordinate origins
	position = entity.GetEmbodiedEntity().GetOriginAnchor().Position;
	orientation = entity.GetEmbodiedEntity().GetOriginAnchor().Orientation;
	btTransform t = bulletTransformFromARGoS(position - rotateARGoSVector(positionOffset, orientation), orientation);
	motionState = new btDefaultMotionState{t};

	// Setup the object, setting the mass to 0 if it should be static
	btScalar mass = (btScalar)(entity.IsEnabled() ? entity.GetMass() : 0);

	// Setup inertia to enable rotations
	btVector3 inertia;
	collisionShape->calculateLocalInertia(mass, inertia);

	rigidBody = new btRigidBody{mass, motionState, collisionShape, inertia};
	rigidBody->setActivationState(DISABLE_DEACTIVATION);

	rigidBody->setFriction(0.5);
	rigidBody->setRestitution(0.5);

	CalculateBoundingBox();
}

/**
 * Update the bounding box to represent the AABB in the global coordinate frame
 */
void CBulletCubeModel::CalculateBoundingBox()
{
	GetBoundingBox().MinCorner.Set(
					GetEmbodiedEntity().GetOriginAnchor().Position.GetX() - (entity->GetSize().GetX() * 0.5),
					GetEmbodiedEntity().GetOriginAnchor().Position.GetY() - (entity->GetSize().GetY() * 0.5),
					GetEmbodiedEntity().GetOriginAnchor().Position.GetZ()
			);
	GetBoundingBox().MaxCorner.Set(
					GetEmbodiedEntity().GetOriginAnchor().Position.GetX() + (entity->GetSize().GetX() * 0.5),
					GetEmbodiedEntity().GetOriginAnchor().Position.GetY() + (entity->GetSize().GetY() * 0.5),
					GetEmbodiedEntity().GetOriginAnchor().Position.GetZ() + entity->GetSize().GetZ()
			);
}

/**
 * Check for ray intersection using ARGoS builtin CBox
 */
bool CBulletCubeModel::CheckIntersectionWithRay(Real &f_t_on_ray, const CRay3 &ray) const
{
	CBox box{entity->GetSize(), GetEmbodiedEntity().GetOriginAnchor().Position, CVector3::Z};
	return box.Intersects(f_t_on_ray, ray);
}

/**
 * Update the bullet model from the ARGoS entity
 */
void CBulletCubeModel::UpdateFromEntityStatus()
{
	position = entity->GetEmbodiedEntity().GetOriginAnchor().Position + rotateARGoSVector(positionOffset, orientation);
	orientation = entity->GetEmbodiedEntity().GetOriginAnchor().Orientation;
	rigidBody->setWorldTransform(bulletTransformFromARGoS(position*engine->worldScale, orientation));
}

/**
 * Update the ARGoS entity as a result of physics simulations
 */
void CBulletCubeModel::UpdateEntityStatus()
{
	bulletTransformToARGoS(rigidBody->getWorldTransform(), position, orientation);
	position *= engine->inverseWorldScale;
	entity->GetEmbodiedEntity().GetOriginAnchor().Position = position - rotateARGoSVector(positionOffset, orientation);
	entity->GetEmbodiedEntity().GetOriginAnchor().Orientation = orientation;
}

REGISTER_BULLET_ENTITY_OPS(CBoxEntity, CBulletCubeModel);
