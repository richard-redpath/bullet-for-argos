//
// Created by richard on 19/08/15.
//

#ifndef ARGOS3_BULLET_CBALLMODEL_H
#define ARGOS3_BULLET_CBALLMODEL_H

#include <argos3/plugins/simulator/entities/cylinder_entity.h>
#include "CBulletModel.h"
#include "CSphereEntity.h"

/**
 * Simple sphere with fixed radius
 */
class CBulletSphereModel : public CBulletModel
{
private:
	CSphereEntity * entity;
	CBulletEngine* engine;
	btSphereShape* collisionShape;
	btMotionState* motionState;
	const CVector3 positionOffset;

public:
	CBulletSphereModel(CBulletEngine& engine, CSphereEntity & entity);
	virtual ~CBulletSphereModel() {}

	virtual void UpdateFromEntityStatus() override;
	virtual void UpdateEntityStatus() override;
	virtual void Step() override {}

	virtual void CalculateBoundingBox() override;

	virtual bool CheckIntersectionWithRay(Real& f_t_on_ray, const CRay3& ray) const;

	virtual btRigidBody* GetRigidBody() const { return rigidBody; }
};


#endif //ARGOS3_BULLET_CBALLMODEL_H
