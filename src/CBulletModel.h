//
// Created by richard on 17/08/15.
//

#ifndef ARGOS3_BULLET_CBULLETMODEL_H
#define ARGOS3_BULLET_CBULLETMODEL_H

class CBulletEngine;

#include <argos3/core/simulator/physics_engine/physics_model.h>
#include <argos3/core/simulator/entity/entity.h>
#include "CBulletEngine.h"

using namespace argos;

/**
 * Base class for a bullet entity
 */
class CBulletModel : public CPhysicsModel
{
protected:
	// Basic spacial properties
	CVector3 position;
	CVector3 velocity;
	CVector3 acceleration;
	CQuaternion orientation;

	// Initial positions
	CVector3 initPosition;
	CQuaternion initOrientation;

	// Reference to the engine and our model in it
	CBulletEngine* engine;
	btRigidBody* rigidBody;

public:
	CBulletModel(CBulletEngine& engine, CEmbodiedEntity& entity);
	virtual ~CBulletModel();

	virtual void MoveTo(const CVector3& position, const CQuaternion& orientation);
	virtual void Reset();

	virtual void Step() = 0;
	virtual void UpdateFromEntityStatus() = 0;

	virtual bool IsCollidingWithSomething() const;

	virtual bool CheckIntersectionWithRay(Real& f_t_on_ray, const CRay3& ray) { return false; }

	void UpdateOriginAnchor(SAnchor& anchor);

	virtual btRigidBody* GetRigidBody() const = 0;
	virtual void AddToEngine(CBulletEngine& engine);
};

/**
 * Create a btTransform from an ARGoS position vector and (optionally) quaternion. If no quaternion is provided then
 * no rotation is assumed.
 */
static inline btTransform bulletTransformFromARGoS(const CVector3 &vec, const CQuaternion &orientation = CQuaternion(1, 0, 0, 0))
{
	return btTransform {btQuaternion{(btScalar)orientation.GetX(), (btScalar)orientation.GetY(), (btScalar)orientation.GetZ(), (btScalar)orientation.GetW()},
						btVector3{(btScalar)vec.GetX(), (btScalar)vec.GetY(), (btScalar)vec.GetZ()}};
}

/**
 * Populate an ARGoS location vector and quaternion from a btTransform
 */
static inline void bulletTransformToARGoS(const btTransform& transform, CVector3& locationOut, CQuaternion& orientationOut)
{
	btVector3 loc = transform.getOrigin();
	btQuaternion rot = transform.getRotation();
	locationOut.Set(loc.getX(), loc.getY(), loc.getZ());
	orientationOut = CQuaternion(rot.getW(), rot.getX(), rot.getY(), rot.getZ());
}

/**
 * Return a rotated ARGoS vector
 */
static inline CVector3 rotateARGoSVector(const CVector3& vec, const CQuaternion& rot)
{
	CVector3 t = vec;
	t.Rotate(rot);
	return t;
}

/**
 * Combine 2 quaternions
 */
static inline CQuaternion combineARGoSQuaternions(const CQuaternion& first, const CQuaternion& second)
{
	return second * first;
}

#endif //ARGOS3_BULLET_CBULLETMODEL_H
