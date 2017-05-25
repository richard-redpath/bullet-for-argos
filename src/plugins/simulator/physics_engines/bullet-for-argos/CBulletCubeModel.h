//
// Created by richard on 17/08/15.
//

#ifndef ARGOS3_BULLET_CBULLETBOXMODEL_H
#define ARGOS3_BULLET_CBULLETBOXMODEL_H

class btBoxShape;

#include <argos3/plugins/simulator/entities/box_entity.h>
#include "CBulletModel.h"

/**
 * A simple cube model to insert boxes into the world
 */
class CBulletCubeModel : public CBulletModel
{
private:
	CBoxEntity* entity;						// ARGoS entity
	CBulletEngine* engine;					// Our world engine
	btBoxShape* collisionShape;				// Bullet body
	btMotionState* motionState;				// Motion info
	const CVector3 positionOffset;			// Offset between reference point in ARGoS and bullet

public:
	CBulletCubeModel(CBulletEngine& engine, CBoxEntity& entity);
	virtual ~CBulletCubeModel() {}

	// To keep the worlds in sync
	virtual void UpdateFromEntityStatus() override;
	virtual void UpdateEntityStatus() override;

	// Execute physics
	virtual void Step() override {}

	virtual void CalculateBoundingBox() override;
	virtual bool CheckIntersectionWithRay(Real& f_t_on_ray, const CRay3& ray) const;

	virtual btRigidBody* GetRigidBody() const { return rigidBody; }
};

#endif //ARGOS3_BULLET_CBULLETBOXMODEL_H
