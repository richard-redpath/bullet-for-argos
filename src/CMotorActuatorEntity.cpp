//
// Created by rar500 on 10/11/15.
//

#include "CMotorActuatorEntity.h"
#include "LUA_CClosure_Helpers.h"
#include "CMultibodyLinkEntity.h"
#include "CBulletModel.h"
#include <argos3/core/simulator/entity/composable_entity.h>

/**
 * Constructor which takes all possible parameters and simply initialises the motor descriptor
 */
CMotorActuatorEntity::CMotorActuatorEntity(const std::string iId, CMultibodyLinkEntity* parent,
										   CMultibodyLinkEntity* child,
										   const CVector3 &position, const CQuaternion &orientation, CVector3 axis,
										   float iVelocityMaxForward, float iEffortMax, float iVelocityMaxReverse,
										   float iDynamicsDamping, float iDynamicsFriction, float iInputMin,
										   float iInputMax,
										   float iLimitMin, float iLimitMax)
		: CSimulatedActuator(),
		  CComposableEntity(parent),
		  id(iId),
		  limitMin(iLimitMin), limitMax(iLimitMax),
		  velocityMaxForward(iVelocityMaxForward),
		  velocityMaxReverse(isnan(iVelocityMaxReverse) ? -iVelocityMaxForward : makeNegative(iVelocityMaxReverse)),
		  inputMin(iInputMin), inputMax(iInputMax),
		  dynamicsDamping(iDynamicsDamping), dynamicsFriction(iDynamicsFriction),
		  effortMax(iEffortMax),
		  velocityTarget(0), velocityCurrent(0),
		  positionTarget(NaN), positionCurrent(0),
		  parent(parent), child(child),
		  embodiedEntity(new CEmbodiedEntity(this)),
		  position(position), orientation(orientation),
		  axis(axis)
{
}

/**
 * Calculate the speed that the motor should be rotating at based on the
 * input range and speed ranges specified on construction
 */
void CMotorActuatorEntity::setVelocityTarget(float input)
{
	// Clamp the input
	clamp(input, inputMin, inputMax);
	input -= inputMin;					// Make zero based
	input /= (inputMax - inputMin);		// Normalise between 0 and 1

	// Calculate the appropriate speed
	velocityTarget = velocityMaxReverse + input * (velocityMaxForward - velocityMaxReverse);
}

/**
 * Returns the rate the motor should be turning at
 */
float CMotorActuatorEntity::getCurrentVelocity()
{
	return velocityCurrent;
}

/**
 * Set which robot this motor belongs to
 */
void CMotorActuatorEntity::SetRobot(CComposableEntity &c_entity)
{
	CSimulatedActuator::SetRobot(c_entity);
}

void CMotorActuatorEntity::Update()
{

}

void CMotorActuatorEntity::UpdateChildPosition()
{
	// Get parent transform in global space
	CVector3 parentPosition = parent->GetEmbodiedEntity().GetOriginAnchor().Position;
	CQuaternion parentOrientation = parent->GetEmbodiedEntity().GetOriginAnchor().Orientation;

	// Get our position in global space
	CVector3 globalPosition = parentPosition + rotateARGoSVector(position, parentOrientation);
	CQuaternion globalOrientation = combineARGoSQuaternions(parentOrientation, orientation);

	// Where will the child be as a result of the rotating motor?
	CQuaternion childOrientation = CQuaternion{CRadians{positionCurrent}, axis};

	// Set the child location
	auto& childOriginAnchor = child->GetEmbodiedEntity().GetOriginAnchor();
	childOriginAnchor.Position = globalPosition;
	childOriginAnchor.Orientation = globalOrientation;

	// Tell the child to update any of its children
	child->UpdateChildPosition();
}

/**
 * Get the id of this robot
 */
std::string CMotorActuatorEntity::GetID()
{
    return id;
}

/**
 * Enabled this motor and setup the anchors of the associated links
 */
void CMotorActuatorEntity::SetEnabled(bool enabled)
{
	CComposableEntity::SetEnabled(enabled);
}


#ifdef ARGOS_WITH_LUA

/**
 * LUA closure to set the velocity of a motor
 */
int LUA_setTargetVelocity(lua_State *state)
{
	// Check the parameter count is correct
	if(lua_gettop(state) != 1)
		return luaL_error(state, "setTargetVelocity expects 1 argument");

	// Check the parameter is a number
	luaL_checktype(state, 1, LUA_TNUMBER);
	float target = (float) lua_tonumber(state, 1);

	// Get the motor to change
	CMotorActuatorEntity* m = LUA_GetCallingInstance<CMotorActuatorEntity>(state);

	// Call the set velocity method
	m->setVelocityTarget(target);

	// We are not returning anything to LUA
	return 0;
}

/**
 * LUA closure to query the velocity of a motor
 */
int LUA_getCurrentVelocity(lua_State *state)
{
	// Check the parameter count is correct
	if(lua_gettop(state) != 0)
		return luaL_error(state, "getTargetVelocity expects 0 arguments");

	// Get the motor we are interested in
	CMotorActuatorEntity* m = LUA_GetCallingInstance<CMotorActuatorEntity>(state);

	// Push the number to the call stack
	lua_pushnumber(state, m->getCurrentVelocity());

	// And inform LUA we have 1 return value
	return 1;
}

/**
 * Create the LUA state for this motor
 */
void CMotorActuatorEntity::CreateLuaState(lua_State *state)
{
	// Add this motor to the robot state table
	CLuaUtility::OpenRobotStateTable(state, id);

	// Add a reference to this object
	CLuaUtility::AddToTable(state, "_instance", this);

	// Push the required functions with references to the motor
	LUA_PushCClosure(state, "setTargetVelocity", &LUA_setTargetVelocity, this);
	LUA_PushCClosure(state, "getCurrentVelocity", &LUA_getCurrentVelocity, this);

	// Finish the state setup for this motor
	CLuaUtility::CloseRobotStateTable(state);
}

#endif

REGISTER_ACTUATOR(CMotorActuatorEntity, "motor", "default", "Richard Redpath", "0.1", "A motor", "See brief", "Beta");
