#pragma once

#include "scene.h"
#include "vec3.h"

extern bool useBVH;

struct HitRecord {
  double t;
  Vec3 point, normal;
  int material;
};

bool closestHit(const Scene &scene, const Ray &ray, double tmin, double tmax,
                HitRecord &rec);

Color shade(const Scene &scene, const Ray &ray, const HitRecord &rec);
bool anyHit(const Scene &scene, const Ray &ray, double tmin, double tmax);
