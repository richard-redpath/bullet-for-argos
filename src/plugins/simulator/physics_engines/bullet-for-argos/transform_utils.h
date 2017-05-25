#ifndef __TRANSFORM_UTILS_H__
#define __TRANSFORM_UTILS_H__

#include "./bullet/src/btBulletDynamicsCommon.h"

btTransform bulletTransformFromARGoS(const CVector3 &vec, const CQuaternion &orientation = CQuaternion(1, 0, 0, 0));

void bulletTransformToARGoS(const btTransform& transform, CVector3& locationOut, CQuaternion& orientationOut);

#endif
