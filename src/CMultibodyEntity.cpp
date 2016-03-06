//
// Created by rar500 on 22/10/15.
//

#include <argos3/plugins/simulator/entities/box_entity.h>
#include <argos3/plugins/simulator/entities/cylinder_entity.h>
#include <argos3/core/wrappers/lua/lua_utility.h>
#include "CMultibodyEntity.h"
#include "CMultibodyLinkEntity.h"
#include "CMotorActuatorEntity.h"
#include "CMultibodyLinkEntity.h"
#include "LUA_CClosure_Helpers.h"
#include "CBulletModel.h"

#ifdef ARGOS_WITH_LUA
#include <argos3/core/wrappers/lua/lua_controller.h>
#endif

using namespace ticpp;

/**
 * Default constructor
 */
CMultibodyEntity::CMultibodyEntity() : CComposableEntity(nullptr), embodiedEntity(nullptr),
									   controllableEntity(nullptr), rootLink(nullptr)
{
}

/**
 * Construct with the provided name and position
 */
CMultibodyEntity::CMultibodyEntity(const std::string& str_id,
								   const CVector3& position,
								   const CQuaternion& orientation)
		: CComposableEntity(nullptr, str_id),
		  embodiedEntity(new CEmbodiedEntity(this, str_id, position, orientation, true)),
		  rootLink(nullptr)
{
	AddComponent(*embodiedEntity);
}

/*
 * Parse the ARGoS XML file to get the URDF file details
 */
void CMultibodyEntity::Init(TConfigurationNode &t_tree)
{
	// Initialise the super class
	CComposableEntity::Init(t_tree);

	// Definition file name
	std::string fileName = t_tree.GetAttribute("definition_file");

	// Get the initial state of this entity
	initialState = MultibodyEntityDatabase::getInstance().getModel(fileName);
	currentState = new MultibodyDefinition;
	*currentState = *initialState;

	// Create a controllable entity
	controllableEntity = new CControllableEntity(this);
	AddComponent(*controllableEntity);
	controllableEntity->Enable();

	// And initialise it with the controller specified
	controllableEntity->Init(GetNode(t_tree, "controller"));

	// Create embodied entity using parsed data
	embodiedEntity = new CEmbodiedEntity(this);

	embodiedEntity->Init(GetNode(t_tree, "body"));
	embodiedEntity->SetMovable(true);
	AddComponent(*embodiedEntity);

	// Create all necessary links
	for(auto& linkPair : initialState->getLinkMap())
	{
		auto link = new CMultibodyLinkEntity {this, linkPair.second};
		link->SetEnabled(true);
		links[linkPair.first] = link;

		// This is our root link
		if(linkPair.second.parent != nullptr)
		{
			continue;
		}
		else
		{
			rootLink = link;
		}
	}

	// And all joints
    for(auto& mot : initialState->getJointMap())
    {
		// Get the joint position
		const JointDefinition& def = mot.second;
		CVector3 position = CVector3{def.originX, def.originY, def.originZ};
		CQuaternion orientation = quaternionFromDegrees(def.originRoll, def.originPitch, def.originYaw);

		// And its effective axis
		CVector3 axis = CVector3{def.axisX, def.axisY, def.axisZ};

		// Get the entities it effects
		CMultibodyLinkEntity* parent = links[def.parent];
		CMultibodyLinkEntity* child = links[def.child];

		auto name = mot.first;

        auto motor = new CMotorActuatorEntity{name, parent, child, position, orientation, axis, def.limitVelocity, def.limitEffort, -def.limitVelocity};
		motor->SetEnabled(true);
		joints[name] = motor;

		parent->ConnectMotor(motor);
		controllableEntity->GetController().AddActuator(motor->GetID(), motor);
    }

#ifdef ARGOS_WITH_LUA
	// Check if our controller is a lua controller
	CLuaController* luaController = dynamic_cast<CLuaController*>(&controllableEntity->GetController());

	// If it is then initialise the LUA state
	if(luaController)
		luaController->CreateLuaState();
#endif

	Reset();
	UpdateComponents();
}

/**
 * Reset the state of the entity to its initial state
 */
void CMultibodyEntity::Reset()
{
	CComposableEntity::Reset();
	*currentState = *initialState;

	embodiedEntity->Reset();
	controllableEntity->Reset();

	for(auto& pair : links)
		pair.second->Reset();

	UpdateComponents();

	if(rootLink)
	{
		auto& rootAnchor = rootLink->GetEmbodiedEntity().GetOriginAnchor();
		rootAnchor.Position += GetEmbodiedEntity().GetOriginAnchor().Position;
		rootAnchor.Orientation = combineARGoSQuaternions(GetEmbodiedEntity().GetOriginAnchor().Orientation, rootAnchor.Orientation);
		rootLink->UpdateChildPosition();
	}
}

void CMultibodyEntity::Update()
{
	if(rootLink)
		rootLink->UpdateChildPosition();
}

//#ifdef ARGOS_WITH_LUA
//void CMultibodyEntity::CreateLuaState(lua_State *state)
//{
//	CLuaUtility::OpenRobotStateTable(state, GetID());
//	CLuaUtility::AddToTable(state, "_instance", this);
//
//	for(auto actuator : this->getJointEntityMap())
//		actuator.second->CreateLuaState(state);
//
//	CLuaUtility::CloseRobotStateTable(state);
//}

//#endif

REGISTER_ENTITY(CMultibodyEntity, "xml_entity", "Richard Redpath", "0.1a", "An entity defined in URDF",
				"A generic entity which can have its shape defined in URDF format and have actuators and sensors adjusted from LUA. More details on the URDF format can be found at http://wiki.ros.org/urdf",
				"In Development")

REGISTER_STANDARD_SPACE_OPERATIONS_ON_COMPOSABLE(CMultibodyEntity)
