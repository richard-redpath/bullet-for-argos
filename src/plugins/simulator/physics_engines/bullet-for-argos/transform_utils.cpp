#include "transform_utils.h"
#include "CBulletModel.h"

btTransform bulletTransformFromARGoS(const CVector3 &vec, const CQuaternion &orientation)
{
  btTransform t;
  bulletTransformFromARGoS(&t, vec, orientation);
  return t;
}

void bulletTransformToARGoS(const btTransform& transform, CVector3& locationOut, CQuaternion& orientationOut)
{
  bulletTransformToARGoS(&transform, locationOut, orientationOut);
}
