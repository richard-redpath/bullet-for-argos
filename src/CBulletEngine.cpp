//
// Created by richard on 17/08/15.
//

#include "CBulletEngine.h"
#include "NumericalHelpers.h"
#include "StringFuncs.h"

#include "BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h"

/**
 * Setup a bullet world
 */
CBulletEngine::CBulletEngine()
{
	// Basic collision handling
	collisionConfiguration = new btDefaultCollisionConfiguration;
	collisionDispatcher = new btCollisionDispatcher {collisionConfiguration};
	btGImpactCollisionAlgorithm::registerAlgorithm(collisionDispatcher);

	// Dynamics solver setup
	overlappingPairCache = new btDbvtBroadphase;
	solver = new btSequentialImpulseConstraintSolver;
	dynamicsWorld = new btDiscreteDynamicsWorld {collisionDispatcher, overlappingPairCache, solver,
												 collisionConfiguration};

	btStaticPlaneShape* groundShape = new btStaticPlaneShape{btVector3{0, 0, 1}, 0};
	btRigidBody* groundBody = new btRigidBody{0, new btDefaultMotionState, groundShape};
	groundBody->setActivationState(DISABLE_DEACTIVATION);
	groundBody->setFriction(0.8);
	groundBody->setRestitution(0.8);

	dynamicsWorld->addRigidBody(groundBody, GetObjectGroup(true), GetObjectCollisionFlags(true));
	
	// Gravity in Z
	dynamicsWorld->setGravity(btVector3{0, 0, -9.81f});
}

/**
 * Perform physics tick and sync bullet and ARGoS
 */
void CBulletEngine::Update()
{
	// Update physics model from ARGoS entity
	for(auto it = entityMap.begin(); it != entityMap.end(); ++it)
		it->second->UpdateFromEntityStatus();

	// Simulate the physics
	dynamicsWorld->stepSimulation((float) GetSimulationClockTick(), maxTicks, internalTimeStep);

	// Update entityMap from physics model
	for(auto it = entityMap.begin(); it != entityMap.end(); ++it)
		it->second->UpdateEntityStatus();
}

/**
 * Get internal tick time from XML if provided
 */
void CBulletEngine::Init(TConfigurationNode &t_tree)
{
	CPhysicsEngine::Init(t_tree);

	maxTicks = GetIterations();
	internalTimeStep = GetSimulationClockTick()/maxTicks;

	extractFromString(t_tree.GetAttributeOrDefault("world_scale", "1"), worldScale);
	worldScaleSquared = worldScale*worldScale;
	inverseWorldScale = 1/worldScale;
	inverseWorldScaleSquared = 1/worldScaleSquared;
	dynamicsWorld->setGravity(btVector3{0, 0, -9.81}*worldScale);

	std::cout<<"World scale = "<<worldScale<<"  Squared = "<<worldScaleSquared<<std::endl;
}

/**
 * Bullet engine destructor.
 */
CBulletEngine::~CBulletEngine()
{
	// Delete all entities from the world
	for(int i = dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; --i)
	{
		btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);
		if(body && body->getMotionState())
			delete body->getMotionState();
		dynamicsWorld->removeCollisionObject(obj);
		delete obj;
	}

	// Then all collision shapes
	for(int i = 0; i < collisionShapes.size(); ++i)
	{
		btCollisionShape* shape = collisionShapes[i];
		collisionShapes[i] = nullptr;
		delete shape;
	}

	// And any auxiliary objects
	delete dynamicsWorld;
	delete solver;
	delete overlappingPairCache;
	delete collisionDispatcher;
	delete collisionConfiguration;
}

/**
 * How many objects are in the world?
 */
size_t CBulletEngine::GetNumPhysicsModels()
{
	return entityMap.size();
}

/**
 * Add a new ARGoS entity to the world by delegating to the add operation
 */
bool CBulletEngine::AddEntity(CEntity &entity)
{
	return CallEntityOperation<BulletOperationAddEntity, CBulletEngine, SOperationOutcome>(*this, entity).Value;
}

/**
 * Remove an ARGoS entity by delegating to the remove operation
 */
bool CBulletEngine::RemoveEntity(CEntity &entity)
{
	return CallEntityOperation<BulletOperationRemoveEntity, CBulletEngine, SOperationOutcome>(*this, entity).Value;
}

/**
 * Register the entity, setup collision and add the object to the world
 */
void CBulletEngine::AddPhysicsModel(const std::string &entityId, CBulletModel &model)
{
	// Add the entity to our map
	entityMap[entityId] = &model;
	entities.push_back(&model);

	// And let the object add itself
	model.AddToEngine(*this);
}

/**
 * Get the model of a particular object by name
 */
CBulletModel* CBulletEngine::GetPhysicsModel(std::string id)
{
	auto it = entityMap.find(id);

	if(it != entityMap.end())
		return it->second;

	return nullptr;
}

/**
 * Find the object, remove it from the world and clear it from our catalogue
 */
void CBulletEngine::RemovePhysicsModel(const std::string &entityId)
{
	// Find our entity
	auto it = entityMap.find(entityId);
	if(it == entityMap.end())
		return;


	CBulletModel* model = it->second;
	for(auto vecIt = entities.begin(); vecIt < entities.end(); ++vecIt)
	{
		if(*vecIt == model)
		{
			entities.erase(vecIt);
			break;
		}
	}

	// Not all objects have rigid bodies (actuators for example)
	if(model->GetRigidBody())
		dynamicsWorld->removeRigidBody(model->GetRigidBody());

	delete it->second;
	entityMap.erase(entityId);
}

/**
 * Iterate over all objects checking for collisions and return the first object hit, if any
 */
void CBulletEngine::CheckIntersectionWithRay(argos::TEmbodiedEntityIntersectionData& ret, const CRay3 &c_ray) const
{
	CEmbodiedEntity* closest = nullptr;
	Real fTOnRay;

	for(auto& it : entityMap)
	{
		if(it.second->CheckIntersectionWithRay(fTOnRay, c_ray))
			ret.push_back(SEmbodiedEntityIntersectionItem{&it.second->GetEmbodiedEntity(), fTOnRay});
	}
}

/**
 * Iterate over all objects checking for collisions and return the first object hit, if any
 */
CEmbodiedEntity* CBulletEngine::CheckIntersectionWithRay(Real &f_t_on_ray, const CRay3 &c_ray) const
{
	CEmbodiedEntity* closest = nullptr;
	Real fTOnRay;

	f_t_on_ray = pInf;

	for(auto& it : entityMap)
	{
		if(it.second->CheckIntersectionWithRay(fTOnRay, c_ray) && fTOnRay < f_t_on_ray)
		{
			closest = &it.second->GetEmbodiedEntity();
			f_t_on_ray = fTOnRay;
		}
	}

	return closest;
}

/**
 * Setup collision checks, static objects are group 0x10 and dynamic are 0x20
 */
static short static_group = 0x10, dynamic_group = 0x20;

/**
 * Static objects should only be tested for collision with dynamic objects (static v static can't happen)
 * while dynamic objects can collide with anything
 */
static short static_flags = 0x20, dynamic_flags = 0x30;

/**
 * Return the appropriate collision flags for the provided flag
 */
short CBulletEngine::GetObjectCollisionFlags(bool isStatic)
{
	return isStatic ? static_flags : dynamic_flags;
}

/**
 * Return the appropriate collision group for the provided flag
 */
short CBulletEngine::GetObjectGroup(bool isStatic)
{
	return isStatic ? static_group : dynamic_group;
}

REGISTER_PHYSICS_ENGINE(CBulletEngine, "bullet", "Richard Redpath", "0.01a", "Bullet physics engine",
						"Bullet physics engine", "In Dev")
