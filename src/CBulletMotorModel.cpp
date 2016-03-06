//
// Created by rar500 on 12/11/15.
//

#include <argos3/core/simulator/simulator.h>
#include "CBulletMotorModel.h"
#include "CBulletEngine.h"

/**
 * A simple, continuous motor which rotates about a fixed axis
 */
CBulletMotorModel::CBulletMotorModel(CBulletEngine &engine, CMotorActuatorEntity &entity) : CBulletModel(engine, entity.GetEmbodiedEntity())
{
	// Retain a reference to the engine and associated ARGoS entity
	this->entity = &entity;
	this->engine = &engine;
}

/**
 * Update the state of the physics simulation from the ARGoS entity
 */
void CBulletMotorModel::UpdateFromEntityStatus()
{
	// @TODO: Improve
	// There must be a better way to do this?
	float targetPosition = entity->getVelocityTarget();		// Rads/sec
	targetPosition *= engine->GetSimulationClockTick();		// Rads/sec * sec/tick = Rads/tick
	targetPosition += motor->getHingeAngle();
	motor->setMotorTarget(targetPosition, engine->GetSimulationClockTick());
}

/**
 * Update the ARGoS entity from the bullet model
 */
void CBulletMotorModel::UpdateEntityStatus()
{
    entity->SetPositionCurrent(motor->getHingeAngle());
}

/**
 * This doesn't make sense for a motor, do nothing
 */
void CBulletMotorModel::CalculateBoundingBox()
{

}

/**
 * Create the joint in bullet, set it up and add it to the simulation
 */
void CBulletMotorModel::AddToEngine(CBulletEngine &engine)
{
	// Get the involved bodies
    CComposableEntity* parentEntity = entity->getParentEntity();
    CComposableEntity* childEntity = entity->getChildEntity();

	// And their corresponding physics entities
    btRigidBody* parentBody = engine.GetPhysicsModel(parentEntity->GetId())->GetRigidBody();
    btRigidBody* childBody = engine.GetPhysicsModel(childEntity->GetId())->GetRigidBody();

	// Get the transform of where/how the motor is "mounted"
    btTransform parentTransform = bulletTransformFromARGoS(entity->GetPosition(), entity->GetOrientation());
    btTransform childTransform = btTransform{btQuaternion{0,0,0}, btVector3{0,0,0}};

	// Create the constraint
    motor = new btHingeConstraint{*parentBody, *childBody, parentTransform, childTransform};

	// Initialise to a speed of 0
	motor->enableAngularMotor(true, 0, entity->GetEffortMax());

	// What axis should it rotate about?
	CVector3 axis = entity->GetAxis();

	// Set the axis
	btVector3 btAxis{(float)axis.GetX(), (float)axis.GetY(), (float)axis.GetZ()};
	motor->setAxis(btAxis);
//	motor->enableMotor(true);

	// Prevent the 2 involved bodies from colliding
	// @BadIdea?
    bool disableCollisionCheckingBetweenParentAndChild = true;

	// Add the motor to the simulation
    engine.GetBulletWorld()->addConstraint(motor, disableCollisionCheckingBetweenParentAndChild);
}

REGISTER_BULLET_ENTITY_OPS(CMotorActuatorEntity, CBulletMotorModel)
