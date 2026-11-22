#include "render.h"
#include "scene.h"
#include "triangle.h"
#include "vec3.h"
#include <algorithm>
#include <cmath>
#include <limits>

bool closestHit(const Scene &scene, const Ray &ray, double tmin, double tmax,
                HitRecord &rec) {
  Hit hit, bestHit;
  double bestT = tmax;
  const Face *bestFace = nullptr;
  int bestMaterial = 0;

  // sahnedeki her üçgenle test, en yakını tut (tmax yerine bestT veriyoruz)
  for (const Mesh &mesh : scene.meshes) {
    for (const Face &f : mesh.faces) {
      Vec3 a = scene.vertices[f.v[0]];
      Vec3 b = scene.vertices[f.v[1]];
      Vec3 c = scene.vertices[f.v[2]];

      if (intersectTriangle(ray, a, b, c, tmin, bestT, hit)) {
        bestT = hit.t;
        bestHit = hit;
        bestFace = &f;
        bestMaterial = mesh.material;
      }
    }
  }

  if (bestFace == nullptr)
    return false;

  // köşe normallerini barycentric ağırlıklarla karıştır
  Vec3 na = scene.normals[bestFace->vn[0]];
  Vec3 nb = scene.normals[bestFace->vn[1]];
  Vec3 nc = scene.normals[bestFace->vn[2]];
  double alpha = 1 - bestHit.beta - bestHit.gamma;

  rec.t = bestT;
  rec.point = ray.at(bestT);
  rec.normal = normalize(alpha * na + bestHit.beta * nb + bestHit.gamma * nc);
  rec.material = bestMaterial;
  return true;
}

bool anyHit(const Scene &scene, const Ray &ray, double tmin, double tmax) {
  Hit hit;

  // sahnedeki her üçgenle test, herhangi bir intersect olduğu anda true
  for (const Mesh &mesh : scene.meshes) {
    for (const Face &f : mesh.faces) {
      Vec3 a = scene.vertices[f.v[0]];
      Vec3 b = scene.vertices[f.v[1]];
      Vec3 c = scene.vertices[f.v[2]];

      if (intersectTriangle(ray, a, b, c, tmin, tmax, hit)) {
        return true;
      }
    }
  }
  return false;
}

Color shade(const Scene &scene, const Ray &ray, const HitRecord &rec) {
  const Material &mat = scene.materials[rec.material];

  // ambient = AMB * ambLight
  Color ambient = mat.ambient * scene.ambientLight;

  // diffuse = foreachlight(DIFF * E * (n*l))
  Color diffuse(0, 0, 0);
  // specular = SPEC * (n*h)^p
  Color specular(0, 0, 0);
  Vec3 wo = normalize(-ray.dir); // göze gelen ışın

  for (const PointLight &pl : scene.pointLights) {

    Vec3 toLightVector = pl.position - rec.point;
    double r2 = dot(toLightVector, toLightVector); // ışık şiddeti için
    Vec3 wi = normalize(toLightVector);
    double angle_between_wi_n = dot(wi, rec.normal);

    if (angle_between_wi_n <= 0) // açı kontrolü
      continue;

    Vec3 shadowOrigin = rec.point + 1e-4 * rec.normal; // x' = x + εn
    Ray shadowRay(shadowOrigin,
                  pl.position - shadowOrigin); // normalize etmedik
    if (anyHit(scene, shadowRay, 0, 1))
      continue; // arada engel → gölge

    Color E = pl.intensity / r2;
    diffuse += mat.diffuse * E * angle_between_wi_n;

    Vec3 h = normalize(wi + wo);                         // half vector
    double cosAlpha = std::max(0.0, dot(rec.normal, h)); // anglebetween n and h
    specular += mat.specular * E * std::pow(cosAlpha, mat.phongExponent);
  }

  for (const TriangularLight &tl : scene.triangularLights) {
    Vec3 dLight =
        normalize(cross(tl.v1 - tl.v2, tl.v1 - tl.v3)); // ışığın yayılma yönü
    Vec3 wi = -dLight;                                  // x'ten ışığa
    double angle_between_wi_n = dot(wi, rec.normal);
    if (angle_between_wi_n <= 0)
      continue;

    Vec3 shadowOrigin = rec.point + 1e-4 * rec.normal;
    Ray shadowRay(shadowOrigin, wi);
    if (anyHit(scene, shadowRay, 0, std::numeric_limits<double>::infinity()))
      continue;

    Color E = tl.intensity; // mesafeyle azalmaz
    diffuse += mat.diffuse * E * angle_between_wi_n;

    Vec3 h = normalize(wi + wo);
    double cosAlpha = std::max(0.0, dot(rec.normal, h));
    specular += mat.specular * E * std::pow(cosAlpha, mat.phongExponent);
  }

  return ambient + diffuse + specular;
}
