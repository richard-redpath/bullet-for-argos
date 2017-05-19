//
// Created by richard on 19/08/15.
//

#include <argos3/core/simulator/entity/composable_entity.h>
#include <argos3/plugins/simulator/entities/cylinder_entity.h>
#include "CSphereEntity.h"
#include "CBulletMultibodyLink.h"

CSphereEntity::CSphereEntity()
		: CComposableEntity(NULL), m_pcEmbodiedEntity(nullptr), m_fMass(1.0f)
{
}


/**
 * Builds a sphere with the provided attributes
 */
CSphereEntity::CSphereEntity(const std::string &str_id, const CVector3 &c_position, const CQuaternion &c_orientation,
							 bool b_movable, float f_radius, float f_mass)
		: CComposableEntity(NULL, str_id),
		  m_pcEmbodiedEntity(new CEmbodiedEntity(this,	"body_0", c_position, c_orientation, b_movable)),
		  m_fRadius(f_radius), m_fMass(f_mass)
{
	AddComponent(*m_pcEmbodiedEntity);
}

/**
 * Load the sphere configuration from its XML tag
 */
void CSphereEntity::Init(TConfigurationNode &t_tree)
{
	try
	{
		// Init parent
		CComposableEntity::Init(t_tree);

		// Parse XML to get the radius (required)
		GetNodeAttribute(t_tree, "radius", m_fRadius);

		// Parse XML to get the movable attribute (optional: defaults to true)
		bool bMovable;
		GetNodeAttributeOrDefault(t_tree, "movable", bMovable, true);

		// Get the mass from XML if the sphere is movable
		if (bMovable)
		{
			// Parse XML to get the mass (optional, defaults to 1)
			GetNodeAttributeOrDefault(t_tree, "mass", m_fMass, 1.0f);
		}
		else
		{
			m_fMass = 0.0f;
		}

		// Create embodied entity using parsed data
		m_pcEmbodiedEntity = new CEmbodiedEntity(this);

		m_pcEmbodiedEntity->Init(GetNode(t_tree, "body"));
		m_pcEmbodiedEntity->SetMovable(bMovable);
		AddComponent(*m_pcEmbodiedEntity);

		UpdateComponents();
	}
	catch (CARGoSException &ex)
	{
		THROW_ARGOSEXCEPTION_NESTED("Failed to initialize the ball entity.", ex);
	}
}

/****************************************/
/****************************************/

void CSphereEntity::Reset()
{
	/* Reset all components */
	m_pcEmbodiedEntity->Reset();

	/* Update components */
	UpdateComponents();
}


REGISTER_ENTITY(CSphereEntity,"sphere","Richard Redpath","1.0","A ball","It's a ball...","Usable");

REGISTER_STANDARD_SPACE_OPERATIONS_ON_COMPOSABLE(CSphereEntity);
