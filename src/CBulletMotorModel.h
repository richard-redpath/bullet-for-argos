//
// Created by rar500 on 12/11/15.
//

#ifndef ARGOS3_BULLET_CBULLETMOTORMODEL_H
#define ARGOS3_BULLET_CBULLETMOTORMODEL_H


#include "CMotorActuatorEntity.h"
#include "CBulletEngine.h"
#include "MultibodyDefinitions.h"

/**
 * Basic, continuous motor model for the bullet physics engine
 */
class CBulletMotorModel : public CBulletModel
{
public:
	CBulletMotorModel(CBulletEngine& engine, CMotorActuatorEntity& entity);

	virtual void UpdateFromEntityStatus() override;
	virtual void UpdateEntityStatus() override;
	virtual void Step() override {}

	virtual btRigidBody* GetRigidBody() const { return nullptr; }

	virtual void AddToEngine(CBulletEngine& engine);

	CMotorActuatorEntity* GetEntity() { return  entity; }

private:
    btHingeConstraint* motor;
    CBulletEngine* engine;
	CMotorActuatorEntity* entity;

	JointType type;

	void CalculateBoundingBox();
};

#endif //ARGOS3_BULLET_CBULLETMOTORMODEL_H
