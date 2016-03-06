//
// Created by richard on 26/11/15.
//

#include "CBulletMultibodyEntity.h"

/**
 * A bullet physics model for a multi-bodied entity.
 *
 * This is a conceptual entity and does not actually contain any physical components itself.
 * This entity simply creates a series of sub-entities and collects them in one place
 */
CBulletMultibodyEntity::CBulletMultibodyEntity(CBulletEngine &engine, CMultibodyEntity &entity)
		: CBulletModel(engine, entity.GetEmbodiedEntity())
{
	// This entity does not have its own body
	rigidBody = nullptr;

	// Create all links
	for (auto pair : entity.getLinkEntityMap())
	{
		auto bulletLink = new CBulletMultibodyLink {engine, *pair.second};
		bulletLinks[pair.first] = bulletLink;
	}

	// And all joints
	for (auto pair : entity.getJointEntityMap())
	{
		auto bulletJoint = new CBulletMotorModel{engine, *pair.second};
		bulletJoints[pair.first] = bulletJoint;
	}
}

REGISTER_BULLET_ENTITY_OPS(CMultibodyEntity, CBulletMultibodyEntity)