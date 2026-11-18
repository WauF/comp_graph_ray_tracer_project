#pragma once
#include "vec3.h"

struct Ray {
  Vec3 origin;
  Vec3 dir;
  Vec3 at(double t) const { return origin + t * dir; }
  Ray() = default;
  Ray(const Vec3 &origin, const Vec3 &dir) : origin(origin), dir(dir) {}
};