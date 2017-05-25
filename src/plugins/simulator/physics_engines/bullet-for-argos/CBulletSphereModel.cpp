//
// Created by richard on 19/08/15.
//

#include "./bullet/src/btBulletDynamicsCommon.h"
#include "CBulletSphereModel.h"

#include "transform_utils.h"
/**
 * A simple sphere model for the bullet engine - Friction, rolling friction and restitution all set to 0.5
 */
CBulletSphereModel::CBulletSphereModel(CBulletEngine &engine, CSphereEntity &entity) : CBulletModel(engine, entity.GetEmbodiedEntity()),
																					   positionOffset(0,0,0)
{
	// Retain a reference to the engine and associated ARGoS entity
	this->entity = &entity;
	this->engine = &engine;

	// Bullet requires boxes to be initialised with half side lengths
	collisionShape = new btSphereShape{(btScalar)entity.GetRadius()};

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
	rigidBody->setRollingFriction(0.5);

	CalculateBoundingBox();
}

/**
 * Update physics model state from ARGoS entity
 */
void CBulletSphereModel::UpdateFromEntityStatus()
{
	position = entity->GetEmbodiedEntity().GetOriginAnchor().Position + rotateARGoSVector(positionOffset, orientation);
	orientation = entity->GetEmbodiedEntity().GetOriginAnchor().Orientation;
	rigidBody->setWorldTransform(bulletTransformFromARGoS(position, orientation));
}

/**
 * Update ARGoS entity from physics model
 */
void CBulletSphereModel::UpdateEntityStatus()
{
	bulletTransformToARGoS(rigidBody->getWorldTransform(), position, orientation);
	entity->GetEmbodiedEntity().GetOriginAnchor().Position = position - rotateARGoSVector(positionOffset, orientation);
	entity->GetEmbodiedEntity().GetOriginAnchor().Orientation = orientation;
}

/**
 * Calculate the AABB in the global coordinate frame
 */
void CBulletSphereModel::CalculateBoundingBox()
{
	GetBoundingBox().MinCorner.Set(
					GetEmbodiedEntity().GetOriginAnchor().Position.GetX() - (entity->GetRadius()),
					GetEmbodiedEntity().GetOriginAnchor().Position.GetY() - (entity->GetRadius()),
					GetEmbodiedEntity().GetOriginAnchor().Position.GetZ()
			);
	GetBoundingBox().MaxCorner.Set(
					GetEmbodiedEntity().GetOriginAnchor().Position.GetX() + (entity->GetRadius()),
					GetEmbodiedEntity().GetOriginAnchor().Position.GetY() + (entity->GetRadius()),
					GetEmbodiedEntity().GetOriginAnchor().Position.GetZ() + entity->GetRadius()
			);
}

/**
 * Check for ray intersection
 */
bool CBulletSphereModel::CheckIntersectionWithRay(Real &f_t_on_ray, const CRay3 &ray) const
{
	CVector3 rayOrigin = ray.GetStart();
	CVector3 rayDirection;
	ray.GetDirection(rayDirection);

	CVector3 sourceToOrigin = rayOrigin - position;
	double sourceToOriginLength = sourceToOrigin.Length();

	double lineDotSourceToOrigin = rayDirection.DotProduct(sourceToOrigin);

	double solutionCheck = pow(lineDotSourceToOrigin, 2);
	solutionCheck -= pow(sourceToOriginLength, 2);
	solutionCheck += pow(entity->GetRadius(), 2);

	if(solutionCheck < 0)
		return false;

	f_t_on_ray = -lineDotSourceToOrigin - sqrt(solutionCheck);
	return true;
}

REGISTER_BULLET_ENTITY_OPS(CSphereEntity, CBulletSphereModel)
