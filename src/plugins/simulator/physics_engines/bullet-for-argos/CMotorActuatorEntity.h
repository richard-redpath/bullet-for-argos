//
// Created by rar500 on 10/11/15.
//

#ifndef ARGOS3_BULLET_CMOTORACTUATOR_H
#define ARGOS3_BULLET_CMOTORACTUATOR_H

#include "NumericalHelpers.h"
#include "CMultibodyLinkEntity.h"

#include <argos3/core/simulator/actuator.h>
#include <argos3/core/control_interface/ci_actuator.h>

#include <argos3/core/utility/plugins/vtable.h>
#include <argos3/core/simulator/entity/embodied_entity.h>
#include <argos3/core/simulator/entity/composable_entity.h>

using namespace argos;

class CMultibodyLinkEntity;

/**
 * A continuously rotating motor with selectable input and speed ranges (linearly mapped)
 */
class CMotorActuatorEntity : public CCI_Actuator, public CSimulatedActuator, public CComposableEntity
{
public:
	/**
	 * Default constructor which sets default values. Should probably never be called.
	 */
	CMotorActuatorEntity() : CMotorActuatorEntity("", "", nullptr, nullptr,
												      CVector3{0,0,0}, CQuaternion{CRadians{0}, CVector3::X},
												      CVector3{0,1,0},
                                                      10, 10, 10, 0, 0, -1, 1)
	{ };

	/**
	 * Main constructor which takes all supported properties of the motor and provides reasonable defaults
	 */
	CMotorActuatorEntity(const std::string iId, const std::string iName, CMultibodyLinkEntity* parent, CMultibodyLinkEntity* child,
						 const CVector3 &position, const CQuaternion &orientation,
						 CVector3 axis = CVector3{0, 1, 0},
						 float iVelocityMaxForward = 10, float iEffortMax = 10,
						 float iVelocityMaxReverse = -10, float iDynamicsDamping = 0,
						 float iDynamicsFriction = 0, float iInputMin = -1, float iInputMax = 1,
						 float iLimitMin = nInf, float iLimitMax = pInf);

	void setVelocityTarget(float input);
	float getVelocityTarget(){ return velocityTarget; }
	float getCurrentVelocity();

	void setPositionTarget(float position){}			// TODO: Implement, does it even have a use? - Maybe for servos?

	CMultibodyLinkEntity* getParentEntity() { return linkParent; }
	CMultibodyLinkEntity* getChildEntity() { return child; }

	virtual void SetRobot(CComposableEntity &entity);

	void Update();

	void UpdateChildPosition();

	CVector3 GetPosition() { return position; }
	CQuaternion GetOrientation() { return orientation; }
	CVector3 GetAxis() { return axis; }

	float GetEffortMax() { return effortMax; }

	float GetVelocityTarget() { return velocityTarget; }
	void SetPositionCurrent(float pos) { positionCurrent = pos; }

	CEmbodiedEntity& GetEmbodiedEntity(){ return *embodiedEntity; }

	void SetEnabled(bool enabled) override;

#ifdef ARGOS_WITH_LUA
	virtual void CreateLuaState(lua_State *state);
#endif

private:
	std::string name;
	float limitMin, limitMax;
	float velocityMaxForward, velocityMaxReverse;
	float inputMin, inputMax;
	float dynamicsDamping, dynamicsFriction;
	float effortMax;

	float velocityTarget, velocityCurrent;
	float positionTarget, positionCurrent;

	CMultibodyLinkEntity* linkParent;
	CMultibodyLinkEntity* child;
	CComposableEntity* robot;

	CVector3 position;
	CQuaternion orientation;

	CVector3 axis;

	CEmbodiedEntity* embodiedEntity;
};


#endif //ARGOS3_BULLET_CMOTORACTUATOR_H
