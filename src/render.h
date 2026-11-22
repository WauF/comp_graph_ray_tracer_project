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

// depth: bu ışının kaçıncı yansıma olduğu (kameradan çıkan ışın = 0).
// Varsayılan değer sadece burada (bildirimde) yazılır, .cpp'de tekrar yazılmaz.
Color shade(const Scene &scene, const Ray &ray, const HitRecord &rec,
            int depth = 0);
bool anyHit(const Scene &scene, const Ray &ray, double tmin, double tmax);
