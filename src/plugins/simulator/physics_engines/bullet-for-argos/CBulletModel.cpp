//
// Created by richard on 17/08/15.
//

#include "CBulletModel.h"

#include "./bullet/src/btBulletDynamicsCommon.h"
#include <argos3/core/simulator/entity/embodied_entity.h>
#include <argos3/core/simulator/simulator.h>
#include <argos3/core/simulator/space/space.h>
#include <argos3/core/utility/datatypes/datatypes.h>

/**
 * Create a btTransform from an ARGoS position vector and (optionally) quaternion. If no quaternion is provided then
 * no rotation is assumed.
 */
void bulletTransformFromARGoS(btTransform* res, const CVector3 &vec, const CQuaternion &orientation)
{
	*res = btTransform {btQuaternion{(btScalar)orientation.GetX(), (btScalar)orientation.GetY(), (btScalar)orientation.GetZ(), (btScalar)orientation.GetW()},
						btVector3{(btScalar)vec.GetX(), (btScalar)vec.GetY(), (btScalar)vec.GetZ()}};
}

/**
 * Populate an ARGoS location vector and quaternion from a btTransform
 */
void bulletTransformToARGoS(const btTransform* transform, CVector3& locationOut, CQuaternion& orientationOut)
{
	btVector3 loc = transform->getOrigin();
	btQuaternion rot = transform->getRotation();
	locationOut.Set(loc.getX(), loc.getY(), loc.getZ());
	orientationOut = CQuaternion(rot.getW(), rot.getX(), rot.getY(), rot.getZ());
}


CBulletModel::~CBulletModel()
{

}

/**
 * Base class for all bullet physics models
 */
CBulletModel::CBulletModel(CBulletEngine& engine, CEmbodiedEntity& entity)
		: CPhysicsModel(engine, entity), engine(&engine), rigidBody(nullptr)
{
	// If the associated embodied entity has no origin anchor then there is nothing more we can do
	if(!&GetEmbodiedEntity().GetOriginAnchor())
		return;

	// Extract initial position and orientation
	initPosition = GetEmbodiedEntity().GetOriginAnchor().Position*engine.worldScale;
	initOrientation = GetEmbodiedEntity().GetOriginAnchor().Orientation;
	position = initPosition;
	orientation = initOrientation;

	// Register the method to update the anchor information
	RegisterAnchorMethod(GetEmbodiedEntity().GetOriginAnchor(), &CBulletModel::UpdateOriginAnchor);
}

/**
 * Update the anchor position from the physics model position
 */
void CBulletModel::UpdateOriginAnchor(SAnchor &anchor)
{
  anchor.Position = position/engine->worldScale;
  anchor.Orientation = orientation;
}

/**
 * Generic method to check for collisions between all objects
 */
bool CBulletModel::IsCollidingWithSomething() const
{
	// Get all entities our engine is responsible for
	std::vector<CBulletModel*> allEntities = engine->GetPhysicsModels();

	// Get our location and position
	CQuaternion q = GetEmbodiedEntity().GetOriginAnchor().Orientation;
	CVector3 loc = GetEmbodiedEntity().GetOriginAnchor().Position;
	SBoundingBox bb = GetBoundingBox();

	// Extract all 8 corners of the bounding box as separate points in the global coordinate frame
	CVector3 corners[8];
	for(char i = 0; i < 8; ++i)
	{
		// Get the appropriate corner values
		corners[i].SetX((i&0x1 ? bb.MinCorner : bb.MaxCorner).GetX());
		corners[i].SetY((i&0x2 ? bb.MinCorner : bb.MaxCorner).GetY());
		corners[i].SetZ((i&0x4 ? bb.MinCorner : bb.MaxCorner).GetZ());

		// Rotate to the global frame
		corners[i].Rotate(q.Inverse());

		// Offset by the body's position
		corners[i] += loc;
	}

	// Extract all 12 edges as rays from one corner to another
	CRay3 rays[12];

	// These 4 corners are used as the start points
	uint8_t startCorners[]{0x0, 0x3, 0x5, 0x6};

	// Set each ray to extend from the current start corner to 1 of the 3 connected edges
	for(int i = 0; i < 12; i+=3)
	{
		rays[ i ].Set(corners[startCorners[i]], corners[startCorners[i] ^ 0x1]);	// Corner indices have been chosen
		rays[i+1].Set(corners[startCorners[i]], corners[startCorners[i] ^ 0x2]);	// to make this work as efficiently
		rays[i+2].Set(corners[startCorners[i]], corners[startCorners[i] ^ 0x4]);	// as possible.
	}

	// Check for collisions with all entities
	for(auto e : allEntities)
	{
		// Skip checking for collision with this object
		if(e != this)
		{
			// Never actually read but required by CheckIntersectionWithRay
			Real r {0};

			// Check every edge for intersection
			for(CRay3& ray : rays)
			{
				if(e->CheckIntersectionWithRay(r, ray))
					return true;
			}
		}
	}

	return false;
}

/**
 * Put the object back to where it started and clear its movement values
 */
void CBulletModel::Reset()
{
	position = initPosition;
	orientation = initOrientation;
	velocity = CVector3{0,0,0};
	acceleration = CVector3{0,0,0};

	// Clear the bullet values too
	GetRigidBody()->setAngularVelocity(btVector3{0,0,0});
	GetRigidBody()->setLinearVelocity(btVector3{0,0,0});

	// Recalculate where we are
	CalculateBoundingBox();
}

/**
 * Change the location of the object
 */
void CBulletModel::MoveTo(const CVector3& position, const CQuaternion& orientation)
{
	this->position = position;
	UpdateEntityStatus();
}

/**
 * Some things will not have a rigid body so it does not make sense for them to be added as a rigid
 * body. We provide a default implementation here for rigid bodies and allow other types of entity to add themselves.
 */
void CBulletModel::AddToEngine(CBulletEngine &engine)
{
	if(!rigidBody)
		return;

	// And add it to our world
	bool isStatic = rigidBody->isStaticObject();
	engine.GetBulletWorld()->addRigidBody(rigidBody, CBulletEngine::GetObjectGroup(isStatic), CBulletEngine::GetObjectCollisionFlags(isStatic));
}
