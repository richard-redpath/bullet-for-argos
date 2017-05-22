//
// Created by richard on 17/08/15.
//

#ifndef ARGOS3_BULLET_CBULLETENGINE_H
#define ARGOS3_BULLET_CBULLETENGINE_H

class CBulletModel;

#include "bullet/src/btBulletDynamicsCommon.h"
#include "CBulletModel.h"
#include "BulletEntityRegistration.h"
#include <argos3/core/simulator/physics_engine/physics_engine.h>

using namespace argos;

/*
 * An implementation of an ARGoS physics engine which uses the bullet engine underneath
 */
class CBulletEngine : public CPhysicsEngine
{
public:
	using TMap = std::map<std::string, CBulletModel*>;				// Type of our object map

private:
	btDefaultCollisionConfiguration* collisionConfiguration;		//
	btCollisionDispatcher* collisionDispatcher;						// All of our required collision
	btBroadphaseInterface* overlappingPairCache;					// detection components
	btSequentialImpulseConstraintSolver* solver;					//

	btDynamicsWorld* dynamicsWorld;							// Our world

	btScalar internalTimeStep;										// The time step used internally between ticks

	std::vector<btCollisionShape*> collisionShapes;					// All possible colliding objects
	TMap entityMap;													// Name accessible ARGoS entities in this engine

	std::vector<CBulletModel*> entities;							// All entities this engine handles

	int maxTicks{50};

public:								// The most subticks we will ever do in one update
	float worldScale;
	float worldScaleSquared;
	float inverseWorldScale;
	float inverseWorldScaleSquared;
	CBulletEngine();
	virtual ~CBulletEngine();

	virtual void Init(TConfigurationNode& t_tree) override;			// Load settings from ARGoS config
	virtual void Update() override;									// Update the world state

	virtual size_t GetNumPhysicsModels() override;					// How many objects are we handling?

	virtual bool AddEntity(CEntity& entity) override;				// Change what objects this engine
	virtual bool RemoveEntity(CEntity& entity) override;			// is handling updates for

	// First object hit by the provided ray
	virtual CEmbodiedEntity* CheckIntersectionWithRay(Real& f_t_on_ray, const CRay3 &c_ray) const;
    void CheckIntersectionWithRay(argos::TEmbodiedEntityIntersectionData& ret, const CRay3 &c_ray) const;

	// Add and remove created models (used by registration macros so may flag as unused by editors)
	void AddPhysicsModel(const std::string& entityId, CBulletModel& model);
	void RemovePhysicsModel(const std::string& entityId);
	CBulletModel* GetPhysicsModel(std::string id);
	std::vector<CBulletModel*>& GetPhysicsModels() { return entities; }

	static short GetObjectGroup(bool isStatic);
	static short GetObjectCollisionFlags(bool isStatic);

	btDynamicsWorld* GetBulletWorld(){ return dynamicsWorld; }

//	virtual bool IsPointContained(const CVector3& vec){return true;}
//	virtual bool IsEntityTransferNeeded() const {return false;}
//	virtual void TransferEntities(){};
};

#endif //ARGOS3_BULLET_CBULLETENGINE_H
