//
// Created by rar500 on 13/11/15.
//

#ifndef ARGOS3_BULLET_CLINEARACTUATOR_H
#define ARGOS3_BULLET_CLINEARACTUATOR_H

#include "NumericalHelpers.h"

#include <argos3/core/simulator/actuator.h>
#include <argos3/core/control_interface/ci_actuator.h>

#include <argos3/core/utility/plugins/vtable.h>
#include <argos3/core/simulator/entity/embodied_entity.h>
#include <argos3/core/simulator/entity/composable_entity.h>

using namespace argos;

/**
 * A linear actuator entity
 */
class CLinearActuatorEntity : public CCI_Actuator, public CSimulatedActuator
{
public:
	CLinearActuatorEntity()
			: CLinearActuatorEntity(nullptr, nullptr, "", 10, 10, 10, 0, 0, -1, 1)
	{

	}

	CLinearActuatorEntity(CComposableEntity* parent, CComposableEntity* child, std::string iId,
						  float iVelocityMaxForward, float iEffortMax, float iVelocityMaxReverse = NaN,
						  float iDynamicsDamping = 0, float iDynamicsFriction = 0,
						  float iInputMin = 0, float iInputMax = 1,
						  float iLimitMin = 0, float iLimitMax = 10);


	void setVelocityTarget(float input){}				// TODO: Implement
	void setPositionTarget(float position);

	float getCurrentVelocity() { return velocityCurrent; }
	float getCurrentPosition() { return positionCurrent; }

	virtual void SetRobot(CComposableEntity &entity);

	void Update();

#ifdef ARGOS_WITH_LUA

	void CreateLuaState(lua_State *state);

#endif

private:
	std::string id;
	float limitMin, limitMax;
	float velocityMaxForward, velocityMaxReverse;
	float inputMin, inputMax;
	float dynamicsDamping, dynamicsFriction;
	float effortMax;

	float velocityTarget, velocityCurrent;
	float positionTarget, positionCurrent;

	CComposableEntity* parent;
	CComposableEntity* child;

	CVector3 parentTranslate;
	CVector3 childTranslate;

	CQuaternion parentRotate;
	CQuaternion childRotate;
};


#endif //ARGOS3_BULLET_CLINEARACTUATOR_H
