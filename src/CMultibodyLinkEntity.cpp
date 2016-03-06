//
// Created by richard on 26/11/15.
//

#include "CMultibodyLinkEntity.h"
#include "CMultibodyEntity.h"

CMultibodyLinkEntity::CMultibodyLinkEntity(CMultibodyEntity* parent, const Link &linkDef)
	: CComposableEntity(parent, linkDef.name), defaultState(linkDef), currentState(linkDef)
{
	// Get our orientation relative to our parent
	CRadians roll{linkDef.roll};
	CRadians pitch{linkDef.pitch};
	CRadians yaw{linkDef.yaw};

	// And wrap it in a quaternion
	CQuaternion orientation;
	orientation.FromEulerAngles(yaw, pitch, roll);

	// Get our position relative to our parent
	CVector3 pos{linkDef.originX, linkDef.originY, linkDef.originZ};

	// Create our embodied entity
	embodiedEntity = new CEmbodiedEntity{this, linkDef.name, pos, orientation};
	embodiedEntity->SetEnabled(true);
	embodiedEntity->SetMovable(true);

	// And add the embodied component
	AddComponent(*embodiedEntity);

	UpdateComponents();
}

/**
 * Reset this link back to its original state
 */
void CMultibodyLinkEntity::Reset()
{
	// Call through to our super reset method
	CComposableEntity::Reset();

	// Restor our state
	currentState = defaultState;

	// Get our reset position (relative to our parent)
	CVector3 pos{currentState.originX, currentState.originY, currentState.originZ};

	// Get our orientation (relative to our parent)
	CRadians roll{currentState.roll};
	CRadians pitch{currentState.pitch};
	CRadians yaw{currentState.yaw};

	// And wrap it in a quaternion
	CQuaternion orientation;
	orientation.FromEulerAngles(yaw, pitch, roll);

	// Set our position
	embodiedEntity->GetOriginAnchor().Position = pos;
	embodiedEntity->GetOriginAnchor().Orientation = orientation;

	// And reset
	embodiedEntity->Reset();
	UpdateComponents();
}

void CMultibodyLinkEntity::UpdateChildPosition()
{
	for(auto motor : connectedMotors)
		motor->UpdateChildPosition();
}

REGISTER_STANDARD_SPACE_OPERATIONS_ON_COMPOSABLE(CMultibodyLinkEntity);
