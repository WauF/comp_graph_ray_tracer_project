#pragma once
#include "ray.h"

struct Hit {
  double t;
  double beta, gamma;
  // ileride: int faceIndex, materialId, Vec3 normal ...
};

bool intersectTriangle(const Ray &ray, const Vec3 &a, const Vec3 &b,
                       const Vec3 &c, double tmin, double tmax, Hit &hit);