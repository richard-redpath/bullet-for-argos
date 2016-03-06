//
// Created by richard on 19/08/15.
//

#ifndef ARGOS3_BULLET_CBULLETCYLINDERMODEL_H
#define ARGOS3_BULLET_CBULLETCYLINDERMODEL_H

#include <argos3/plugins/simulator/entities/cylinder_entity.h>
#include "CBulletModel.h"

/**
 * Cylinder model for the bullet physics plugin
 */
class CBulletCylinderModel : public CBulletModel
{
private:
	CCylinderEntity* entity;			// ARGoS entity
	CBulletEngine* engine;				// Our world
	btCylinderShape* collisionShape;	// And collision object
	btMotionState* motionState;			// How are we moving
	const CVector3 positionOffset;		// And our offset between bullet and ARGoS

public:
	CBulletCylinderModel(CBulletEngine& engine, CCylinderEntity& entity);
	virtual ~CBulletCylinderModel() {}

	// Keep the worlds in sync
	virtual void UpdateFromEntityStatus() override;
	virtual void UpdateEntityStatus() override;

	// Update the world
	virtual void Step() override {}

	virtual void CalculateBoundingBox() override;
	virtual bool CheckIntersectionWithRay(Real& f_t_on_ray, const CRay3& ray) const;

	virtual btRigidBody* GetRigidBody() const { return rigidBody; }
};


#endif //ARGOS3_BULLET_CBULLETCYLINDERMODEL_H
