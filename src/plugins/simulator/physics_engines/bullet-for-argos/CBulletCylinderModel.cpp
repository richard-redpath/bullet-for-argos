//
// Created by richard on 19/08/15.
//

#include "CBulletCylinderModel.h"
#include <argos3/core/utility/math/cylinder.h>

/**
 * Constructor for bullet cylinders. Friction and restitution both set to 0.5
 */
CBulletCylinderModel::CBulletCylinderModel(CBulletEngine &engine, CCylinderEntity &entity) : CBulletModel(engine, entity.GetEmbodiedEntity()),
																							 positionOffset(0,0,entity.GetHeight() * 0.5)
{
	// Retain a reference to the engine and associated ARGoS entity
	this->entity = &entity;
	this->engine = &engine;

	// Bullet requires boxes to be initialised with half side lengths
	btVector3 size{(btScalar)entity.GetRadius(), (btScalar)entity.GetRadius(), (btScalar)entity.GetHeight() * 0.5f};
	size *= engine.worldScale;
	collisionShape = new btCylinderShapeZ{size};

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
 * Update the bullet simulation from the ARGoS entity
 */
void CBulletCylinderModel::UpdateFromEntityStatus()
{
	position = entity->GetEmbodiedEntity().GetOriginAnchor().Position + rotateARGoSVector(positionOffset, orientation);
	position *= engine->worldScale;
	orientation = entity->GetEmbodiedEntity().GetOriginAnchor().Orientation;
	rigidBody->setWorldTransform(bulletTransformFromARGoS(position, orientation));
}

/**
 * Update the ARGoS entity from the physics model
 */
void CBulletCylinderModel::UpdateEntityStatus()
{
	position /= engine->worldScale;
	bulletTransformToARGoS(rigidBody->getWorldTransform(), position, orientation);
	entity->GetEmbodiedEntity().GetOriginAnchor().Position = position - rotateARGoSVector(positionOffset, orientation);
	entity->GetEmbodiedEntity().GetOriginAnchor().Orientation = orientation;
}

/**
 * Update the AABB in the global coordinate frame.
 */
void CBulletCylinderModel::CalculateBoundingBox()
{
	GetBoundingBox().MinCorner.Set(
					GetEmbodiedEntity().GetOriginAnchor().Position.GetX() - (entity->GetRadius()),
					GetEmbodiedEntity().GetOriginAnchor().Position.GetY() - (entity->GetRadius()),
					GetEmbodiedEntity().GetOriginAnchor().Position.GetZ()
			);
	GetBoundingBox().MaxCorner.Set(
					GetEmbodiedEntity().GetOriginAnchor().Position.GetX() + (entity->GetRadius()),
					GetEmbodiedEntity().GetOriginAnchor().Position.GetY() + (entity->GetRadius()),
					GetEmbodiedEntity().GetOriginAnchor().Position.GetZ() + entity->GetHeight()
			);
}

/**
 * Check for ray intersection using the ARGoS builting CCylinder
 */
bool CBulletCylinderModel::CheckIntersectionWithRay(Real &f_t_on_ray, const CRay3 &ray) const
{
	CCylinder cyl{entity->GetRadius(),
				  	entity->GetHeight(),
					GetEmbodiedEntity().GetOriginAnchor().Position,
					CVector3::Z};
	return cyl.Intersects(f_t_on_ray, ray);
}

REGISTER_BULLET_ENTITY_OPS(CCylinderEntity, CBulletCylinderModel)
