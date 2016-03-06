//
// Created by richard on 18/08/15.
//

#ifndef ARGOS3_BULLET_BULLETENTITYREGISTRATION_H
#define ARGOS3_BULLET_BULLETENTITYREGISTRATION_H

class CBulletEngine;

#include "CBulletEngine.h"

using namespace argos;

template <typename ACTION>
class BulletOperation : public CEntityOperation<ACTION, CBulletEngine, SOperationOutcome>
{
public:
	virtual ~BulletOperation() {}
};

class BulletOperationAddEntity : public BulletOperation<BulletOperationAddEntity>
{
public:
	virtual ~BulletOperationAddEntity() {}
};

class BulletOperationRemoveEntity : public BulletOperation<BulletOperationRemoveEntity>
{
public:
	virtual ~BulletOperationRemoveEntity() {}
};

#define REGISTER_BULLET_ADD(SpaceEntity, BulletModel)\
	class CBulletOperationAdd##SpaceEntity : public BulletOperationAddEntity\
	{\
	public:\
		CBulletOperationAdd##SpaceEntity() {}\
		virtual ~CBulletOperationAdd##SpaceEntity() {}\
	\
		SOperationOutcome ApplyTo(CBulletEngine& engine, SpaceEntity& entity)\
		{\
			BulletModel* model = new BulletModel(engine, entity);\
			entity.GetComponent<CEmbodiedEntity>("body").AddPhysicsModel(engine.GetId(), *model);\
			engine.AddPhysicsModel(entity.GetId(), *model);\
			return SOperationOutcome(true);\
		}\
	};\
	REGISTER_ENTITY_OPERATION(BulletOperationAddEntity,CBulletEngine,CBulletOperationAdd##SpaceEntity,SOperationOutcome,SpaceEntity)

#define REGISTER_BULLET_REMOVE(SpaceEntity, BulletModel)\
	class CBulletOperationRemove##SpaceEntity : public BulletOperationRemoveEntity\
	{\
	public:\
		CBulletOperationRemove##SpaceEntity() {}\
		virtual ~CBulletOperationRemove##SpaceEntity() {}\
	\
		SOperationOutcome ApplyTo(CBulletEngine& engine, SpaceEntity& entity)\
		{\
			engine.RemovePhysicsModel(entity.GetId());\
			entity.GetComponent<CEmbodiedEntity>("body").RemovePhysicsModel(engine.GetId());\
			return SOperationOutcome(true);\
		}\
	};\
	REGISTER_ENTITY_OPERATION(BulletOperationRemoveEntity,CBulletEngine,CBulletOperationRemove##SpaceEntity,SOperationOutcome,SpaceEntity)

#define REGISTER_BULLET_ENTITY_OPS(SpaceEntity,BulletModel)\
	REGISTER_BULLET_ADD(SpaceEntity,BulletModel)\
	REGISTER_BULLET_REMOVE(SpaceEntity,BulletModel)

#endif //ARGOS3_BULLET_BULLETENTITYREGISTRATION_H
