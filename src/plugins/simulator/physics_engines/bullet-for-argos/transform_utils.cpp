#include "transform_utils.h"

inline btTransform bulletTransformFromARGoS(const CVector3 &vec, const CQuaternion &orientation = CQuaternion(1, 0, 0, 0));
{
  btTransform t;
  bulletTransformFromARGoS(&t, vec, orientation);
  return t;
}

void bulletTransformToARGoS(const btTransform& transform, CVector3& locationOut, CQuaternion& orientationOut);
{
  bulletTransformToARGoS(&transform, locationOut, orientationOut);
}
