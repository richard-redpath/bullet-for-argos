//
// Created by rar500 on 22/10/15.
//

#ifndef ARGOS3_BULLET_CURDFEntity_H
#define ARGOS3_BULLET_CURDFEntity_H

#include <argos3/core/simulator/entity/composable_entity.h>
#include "MeshInfo.h"
#include "MultibodyDefinitions.h"
#include "MultibodyEntityDatabase.h"
#include "CMultibodyLinkEntity.h"
#include "CMotorActuatorEntity.h"

class CMultibodyLinkEntity;
class CMotorActuatorEntity;

using namespace argos;

/**
 * A multibodied entity which is composed of links and joints
 */
class CMultibodyEntity : public CComposableEntity
{
public:
	ENABLE_VTABLE();

	/**
	 * Default constructor
	 */
	CMultibodyEntity();

	/**
	 * Initialise with an ID and position
	 */
	CMultibodyEntity(const std::string& str_id,
					 const CVector3& position,
					 const CQuaternion& orientation);

	/**
	 * Get a reference to the embodied entity
	 */
	inline CEmbodiedEntity& GetEmbodiedEntity()
	{
		return *embodiedEntity;
	}

	/**
	 * Get a const reference to the embodied entity
	 */
	inline const CEmbodiedEntity& GetEmbodiedEntity() const
	{
		return *embodiedEntity;
	}

	virtual void Init(TConfigurationNode &t_tree);


	virtual std::string GetTypeDescription() const
	{
		return "xml_multibody_entity";
	}

	/**
	 * Get a reference to the current state of this entity
	 */
	MultibodyDefinition& getCurrentState()
	{
		return *currentState;
	}

	virtual void Reset();

	/**
	 * Get a specific link by name
	 */
	CMultibodyLinkEntity* GetLinkEntity(std::string name)
	{
		return (CMultibodyLinkEntity*) &GetComponent(name);
	}

	/**
	 * Get all links
	 */
	std::map<std::string, CMultibodyLinkEntity*>& getLinkEntityMap()
	{
		return links;
	}

	/**
	 * Get a specific joint by name
	 */
	CMotorActuatorEntity* GetJointEntity(std::string name)
	{
		return (CMotorActuatorEntity*) &GetComponent(name);
	}

	/**
	 * Get all joints
	 */
	std::map<std::string, CMotorActuatorEntity*>& getJointEntityMap()
	{
		return joints;
	}

	/**
	 * Delegate update calls to all sub components
	 */
	void UpdateComponents()
	{
		for(auto comp : GetComponentVector())
			if(comp->IsEnabled()) comp->Update();
	}

	void Update();

private:
	CEmbodiedEntity* embodiedEntity;
	CControllableEntity* controllableEntity;
	MultibodyDefinition* initialState;
	MultibodyDefinition* currentState;

	CMultibodyLinkEntity* rootLink;

	std::map<std::string, CMultibodyLinkEntity*> links;
	std::map<std::string, CMotorActuatorEntity*> joints;
};

#endif //ARGOS3_BULLET_CURDFEntity_H
