//
// Created by richard on 26/11/15.
//

#ifndef ARGOS3_BULLET_CURDFLINKENTITY_H
#define ARGOS3_BULLET_CURDFLINKENTITY_H

#include <argos3/core/simulator/entity/composable_entity.h>
#include "NumericalHelpers.h"
#include "MultibodyDefinitions.h"
#include "CMultibodyEntity.h"
#include "CMotorActuatorEntity.h"

using namespace argos;

class CMultibodyEntity;

/**
 * A component of a multibodied entity. Represents a single link in a more complex entity.
 */
class CMultibodyLinkEntity : public CComposableEntity
{
public:
	ENABLE_VTABLE();

	CMultibodyLinkEntity(): CComposableEntity(nullptr), defaultState(Link{}), currentState(Link{}){}
	CMultibodyLinkEntity(CMultibodyEntity* parent, const Link &linkDef);

	/**
	 * Get a reference to the embodied entity
	 */
	inline CEmbodiedEntity& GetEmbodiedEntity()
	{
		return *embodiedEntity;
	}

	/**
	 * Get a constant reference to the embodied entity
	 */
	inline const CEmbodiedEntity& GetEmbodiedEntity() const
	{
		return *embodiedEntity;
	}

	/**
	 * Get a descriptive string for the type of this entity
	 */
	virtual std::string GetTypeDescription() const
	{
		return "xml_multibody_link_entity";
	}

	/**
	 * Reset to this link's initial configuration
	 */
	virtual void Reset();

	/**
	 * Get the current state of this entity
	 */
	const Link& getCurrentState()
	{
		return currentState;
	}

	void UpdateChildPosition();

	void ConnectMotor(CMotorActuatorEntity* motor)
	{
		connectedMotors.push_back(motor);
	}

private:
	CEmbodiedEntity* embodiedEntity;
	Link currentState;
	const Link defaultState;

	std::vector<CMotorActuatorEntity*> connectedMotors;
};


#endif //ARGOS3_BULLET_CURDFLINKENTITY_H
