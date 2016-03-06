//
// Created by richard on 25/11/15.
//

#ifndef ARGOS3_BULLET_CBULLETURDFLINK_H
#define ARGOS3_BULLET_CBULLETURDFLINK_H

#include "CBulletModel.h"
#include "CMultibodyLinkEntity.h"

/**
 * A single part of a multibodied entity
 */
class CBulletMultibodyLink : public CBulletModel
{
private:
    CMultibodyLinkEntity* entity;				// ARGoS entity
    CBulletEngine* engine;					// Our world engine

    void addBoxToCollisionShape(btCompoundShape *pShape, GeometrySpecification& spec);
    void addCylinderToCollisionShape(btCompoundShape *pShape, GeometrySpecification& spec);
    void addSphereToCollisionShape(btCompoundShape *pShape, GeometrySpecification& spec);
    void addMeshToCollisionShape(btCompoundShape *pShape, GeometrySpecification& spec);

public:
    CBulletMultibodyLink(CBulletEngine& engine, CMultibodyLinkEntity & entity);
    virtual ~CBulletMultibodyLink() {}

    // To keep the worlds in sync
    virtual void UpdateFromEntityStatus() override;
    virtual void UpdateEntityStatus() override;

    // Execute physics
    virtual void Step() override {}

    virtual void CalculateBoundingBox() override;
    virtual bool CheckIntersectionWithRay(Real& f_t_on_ray, const CRay3& ray) const;

    virtual btRigidBody* GetRigidBody() const { return rigidBody; }
};

#endif //ARGOS3_BULLET_CBULLETBOXMODEL_H