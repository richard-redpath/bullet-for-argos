//
// Created by richard on 19/08/15.
//

#ifndef ARGOS3_BULLET_CBALLENTITY_H
#define ARGOS3_BULLET_CBALLENTITY_H

#include <argos3/core/simulator/entity/composable_entity.h>

using namespace argos;

/*
 * A simple sphere object
 */
class CSphereEntity : public CComposableEntity
{
public:
	ENABLE_VTABLE();

	CSphereEntity();

	CSphereEntity(const std::string &str_id,
				  const CVector3 &c_position,
				  const CQuaternion &c_orientation,
				  bool b_movable,
				  float f_radius,
				  float f_mass = 1.0f);

	inline CEmbodiedEntity& GetEmbodiedEntity() {
		return *m_pcEmbodiedEntity;
	}

	inline const CEmbodiedEntity& GetEmbodiedEntity() const {
		return *m_pcEmbodiedEntity;
	}

	virtual void Init(TConfigurationNode &t_tree);

	virtual void Reset();

	inline float GetRadius() const
	{
		return m_fRadius;
	}

	inline void SetRadius(float c_radius)
	{
		m_fRadius = c_radius;
	}

	inline float GetMass() const
	{
		return m_fMass;
	}

	inline void SetMass(float f_mass)
	{
		m_fMass = f_mass;
	}

	virtual std::string GetTypeDescription() const
	{
		return "ball";
	}

private:
	CEmbodiedEntity *m_pcEmbodiedEntity;
	float m_fRadius;
	float m_fMass;
};

#endif //ARGOS3_BULLET_CBALLENTITY_H
