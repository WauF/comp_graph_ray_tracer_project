#include "triangle.h"
#include "ray.h"
#include "vec3.h"
#include <cmath>

static double det3(const Vec3 &x, const Vec3 &y, const Vec3 &z) {
  return dot(x, cross(y, z));
}

bool intersectTriangle(const Ray &ray, const Vec3 &a, const Vec3 &b,
                       const Vec3 &c, double tmin, double tmax, Hit &hit) {
  // A ne denklem şu : o + dt = a +  beta(b-a) + gamma(c-a)
  //   beta(a-b) + gamma(a-c) + dt = a - o;
  //   beta(a-b)x + gamma(a-c)x + dt.x = (a-e)x
  Vec3 ab = a - b, ac = a - c, ae = a - ray.origin;
  double detA = det3(ab, ac, ray.dir);
  if (std::fabs(detA) < 1e-12)
    return false; // ışın düzleme paralel

  // cramer
  double beta = det3(ae, ac, ray.dir) / detA;
  double gamma = det3(ab, ae, ray.dir) / detA;
  double t = det3(ab, ac, ae) / detA;

  if (beta < 0 || gamma < 0 || beta + gamma > 1)
    return false; // üçgenin dışında
  if (t < tmin || t > tmax)
    return false; // aralık dışında

  hit.beta = beta;
  hit.gamma = gamma;
  hit.t = t;
  return true;
}
