//
// Created by richard on 26/11/15.
//

#ifndef ARGOS3_BULLET_CBULLETURDFMODEL_H
#define ARGOS3_BULLET_CBULLETURDFMODEL_H

#include "CBulletModel.h"
#include "CMultibodyEntity.h"
#include "CBulletMultibodyLink.h"
#include "CBulletMotorModel.h"

/**
 * A multibodied entity model. This entity does not contain its own rigid body,
 * instead acting simply as a container for other rigid body models and delegating
 * calls to them as appropriate
 */
class CBulletMultibodyEntity : public CBulletModel
{
public:
	CBulletMultibodyEntity(CBulletEngine &engine, CMultibodyEntity &entity);

	virtual ~CBulletMultibodyEntity()
	{ }

	// To keep the worlds in sync
	virtual void UpdateFromEntityStatus()
	{
		for(auto pair : bulletLinks)
			pair.second->UpdateFromEntityStatus();
	}

	virtual void UpdateEntityStatus()
	{
		for (auto pair : bulletLinks)
			pair.second->UpdateEntityStatus();
	}

	virtual void Step() override
	{
		for (auto pair : bulletLinks)
			pair.second->Step();
	}

	virtual void CalculateBoundingBox() override
	{ };

	// @TODO: Implement
	virtual bool CheckIntersectionWithRay(Real &f_t_on_ray, const CRay3 &ray) const
	{
		return false;
	};

	virtual btRigidBody *GetRigidBody() const
	{
		return rigidBody;
	}

	/**
	 * Adds each of the entities parts to the engine
	 */
	void AddToEngine(CBulletEngine &engine)
	{
		for (auto pair : bulletLinks) engine.AddPhysicsModel(pair.second->GetEmbodiedEntity().GetId(), *pair.second);
		for (auto pair : bulletJoints) engine.AddPhysicsModel(pair.second->GetEntity()->GetId(), *pair.second);
	}

private:
	std::map<std::string, CBulletMultibodyLink*> bulletLinks;
	std::map<std::string, CBulletMotorModel *> bulletJoints;
};


#endif //ARGOS3_BULLET_CBULLETURDFMODEL_H
