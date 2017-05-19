//
// Created by rar500 on 13/11/15.
//

#include <argos3/core/wrappers/lua/lua_utility.h>
#include "CLinearActuatorEntity.h"
#include "LUA_CClosure_Helpers.h"

/**
 * A simple linear actuator entity. Currently has no physics implementation
 * @TODO: Add physics implementation
 */
CLinearActuatorEntity::CLinearActuatorEntity(CComposableEntity* parent, CComposableEntity* child, std::string iId,
											 float iVelocityMaxForward, float iEffortMax, float iVelocityMaxReverse,
											 float iDynamicsDamping, float iDynamicsFriction, float iInputMin, float iInputMax,
											 float iLimitMin, float iLimitMax)
		: CSimulatedActuator(),
		  id(iId),
		  limitMin(iLimitMin), limitMax(iLimitMax),
		  velocityMaxForward(iVelocityMaxForward),
		  velocityMaxReverse(std::isnan(iVelocityMaxReverse) ? -iVelocityMaxForward : makeNegative(iVelocityMaxReverse)),
		  inputMin(iInputMin), inputMax(iInputMax),
		  dynamicsDamping(iDynamicsDamping), dynamicsFriction(iDynamicsFriction),
		  effortMax(iEffortMax),
		  velocityTarget(0), velocityCurrent(0),
		  positionTarget(NaN), positionCurrent(0),
		  parent(parent), child(child)
{

}

/**
 * Set the target position of this actuator
 */
void CLinearActuatorEntity::setPositionTarget(float input)
{
	clamp(input, inputMin, inputMax);
	input -= inputMin;                    // Make zero based
	input /= (inputMax - inputMin);    // Normalise between 0 and 1

	positionTarget = limitMin + input * (limitMax - limitMin);
	//std::cout << "New target = " << positionTarget << std::endl;
}

/**
 * Update which robot the actuator belongs to
 */
void CLinearActuatorEntity::SetRobot(CComposableEntity &c_entity)
{
	parent = &c_entity;
}

/**
 * The update method does nothing. All updating is done by the physics engine
 */
void CLinearActuatorEntity::Update()
{
}

#ifdef ARGOS_WITH_LUA

/**
 * Set the position of the calling entity
 */
int LUA_setTargetPosition(lua_State *state)
{
	// Check for the correct number of arguments
	if(lua_gettop(state) != 1)
		return luaL_error(state, "setTargetPosition expects 1 argument");

	// Ensure the argument is a number
	luaL_checktype(state, 1, LUA_TNUMBER);
	float target = (float) lua_tonumber(state, 1);

	// Get the calling instance
	CLinearActuatorEntity* m = LUA_GetCallingInstance<CLinearActuatorEntity>(state);

	// And set the position
	m->setPositionTarget(target);

	// We haven't added any return values to the stack so return 0
	return 0;
}

/**
 * Get the position of the calling instance
 */
int LUA_getCurrentPosition(lua_State *state)
{
	// Ensure we have been given no arguments
	if(lua_gettop(state) != 0)
		return luaL_error(state, "getCurrentPosition expects 0 arguments");

	// Get the calling instance
	CLinearActuatorEntity* m = LUA_GetCallingInstance<CLinearActuatorEntity>(state);

	// Push the return value to the stack
	lua_pushnumber(state, m->getCurrentPosition());

	// We have added 1 return argument so return 0
	return 1;
}

/**
 * Create the LUA state for this entity
 */
void CLinearActuatorEntity::CreateLuaState(lua_State *state)
{
	// Open the state table
	CLuaUtility::OpenRobotStateTable(state, id);

	// Push this object to the state table
	CLuaUtility::AddToTable(state, "_instance", this);

	// Push closures to the state table which include this object to make it accessible
	// from the callbacks
	LUA_PushCClosure(state, "setTargetPosition", &LUA_setTargetPosition, this);
	LUA_PushCClosure(state, "getCurrentPosition", &LUA_getCurrentPosition, this);

	CLuaUtility::CloseRobotStateTable(state);
}

#endif

//REGISTER_ACTUATOR(CLinearActuatorEntity, "linear_actuator", "default", "Richard Redpath", "0.1", "A linear actuator", "See brief", "Beta");
