#include "render.h"
#include "bvh.h"
#include "scene.h"
#include "triangle.h"
#include "vec3.h"
#include <algorithm>
#include <cmath>
#include <limits>

bool useBVH = true;

// BVH'de en yakın çarpışmayı arar. bestT / bestHit / bestFace referansla
// geliyor: daha yakın bir üçgen bulursa onları günceller.
static bool traverseClosest(const Scene &scene, const Mesh &mesh, int nodeIdx,
                            const Ray &ray, double tmin, double &bestT,
                            Hit &bestHit, const Face *&bestFace) {
  const BVHNode &node = mesh.bvh.nodes[nodeIdx];

  // kutuyu ıskaladıysa (ya da kutu bestT'den uzaktaysa) içine girme
  if (!hitAABB(node.box, ray, tmin, bestT))
    return false;

  // yaprak: içindeki birkaç üçgeni eski usul tek tek test et
  if (node.count > 0) {
    bool found = false;
    Hit hit;
    for (int k = node.start; k < node.start + node.count; ++k) {
      const Face &f = mesh.faces[mesh.bvh.faceOrder[k]];
      Vec3 a = scene.vertices[f.v[0]];
      Vec3 b = scene.vertices[f.v[1]];
      Vec3 c = scene.vertices[f.v[2]];

      if (intersectTriangle(ray, a, b, c, tmin, bestT, hit)) {
        bestT = hit.t;
        bestHit = hit;
        bestFace = &f;
        found = true;
      }
    }
    return found;
  }

  // iç düğüm: İKİ çocuğa da in (|| ile birleştirme, kısa devre sağı atlar!)
  bool l = traverseClosest(scene, mesh, node.left, ray, tmin, bestT, bestHit,
                           bestFace);
  bool r = traverseClosest(scene, mesh, node.right, ray, tmin, bestT, bestHit,
                           bestFace);
  return l || r;
}

// Gölge: herhangi bir çarpışma yeter, ilkinde dön.
static bool traverseAny(const Scene &scene, const Mesh &mesh, int nodeIdx,
                        const Ray &ray, double tmin, double tmax) {
  const BVHNode &node = mesh.bvh.nodes[nodeIdx];
  if (!hitAABB(node.box, ray, tmin, tmax))
    return false;

  if (node.count > 0) {
    Hit hit;
    for (int k = node.start; k < node.start + node.count; ++k) {
      const Face &f = mesh.faces[mesh.bvh.faceOrder[k]];
      if (intersectTriangle(ray, scene.vertices[f.v[0]], scene.vertices[f.v[1]],
                            scene.vertices[f.v[2]], tmin, tmax, hit))
        return true;
    }
    return false;
  }

  // burada kısa devre İSTİYORUZ: solda engel varsa sağa bakmaya gerek yok
  if (traverseAny(scene, mesh, node.left, ray, tmin, tmax))
    return true;
  return traverseAny(scene, mesh, node.right, ray, tmin, tmax);
}

bool closestHit(const Scene &scene, const Ray &ray, double tmin, double tmax,
                HitRecord &rec) {
  Hit hit, bestHit;
  double bestT = tmax;
  const Face *bestFace = nullptr;
  int bestMaterial = 0;
  // en yakın çarpışma bir instance'tan mı geldi? (nullptr = düz mesh)
  // normali dönüştürmek için lazım
  const Instance *bestInstance = nullptr;

  // sahnedeki her üçgenle test, en yakını tut (tmax yerine bestT veriyoruz)
  for (const Mesh &mesh : scene.meshes) {
    if (useBVH) {
      if (traverseClosest(scene, mesh, 0, ray, tmin, bestT, bestHit,
                          bestFace)) {
        bestMaterial = mesh.material;
        bestInstance = nullptr; // daha yakın bir düz mesh bulundu
      }
    } else {
      for (const Face &f : mesh.faces) {
        Vec3 a = scene.vertices[f.v[0]];
        Vec3 b = scene.vertices[f.v[1]];
        Vec3 c = scene.vertices[f.v[2]];

        if (intersectTriangle(ray, a, b, c, tmin, bestT, hit)) {
          bestT = hit.t;
          bestHit = hit;
          bestFace = &f;
          bestMaterial = mesh.material;
          bestInstance = nullptr;
        }
      }
    }
  }

  // instance'lar: ışını M⁻¹ ile base mesh'in yerel uzayına taşı,
  // base mesh'in KENDİ BVH'sinde ara. Yön normalize EDİLMEZ, böylece yerel t
  // dünyadaki t ile aynı kalır ve bestT doğrudan karşılaştırılabilir.
  for (const Instance &inst : scene.instances) {
    Ray localRay(transformPoint(inst.Minv, ray.origin),  // [o, 1]
                 transformDir(inst.Minv, ray.dir));      // [d, 0]
    const Mesh &mesh = scene.meshes[inst.baseMesh];

    if (useBVH) {
      if (traverseClosest(scene, mesh, 0, localRay, tmin, bestT, bestHit,
                          bestFace)) {
        bestMaterial = inst.material; // mesh'in değil, instance'ın materyali
        bestInstance = &inst;
      }
    } else {
      for (const Face &f : mesh.faces) {
        Vec3 a = scene.vertices[f.v[0]];
        Vec3 b = scene.vertices[f.v[1]];
        Vec3 c = scene.vertices[f.v[2]];

        if (intersectTriangle(localRay, a, b, c, tmin, bestT, hit)) {
          bestT = hit.t;
          bestHit = hit;
          bestFace = &f;
          bestMaterial = inst.material;
          bestInstance = &inst;
        }
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
  Vec3 n = alpha * na + bestHit.beta * nb + bestHit.gamma * nc; // yerel normal

  // instance ise normali dünyaya taşı: n_world = (M⁻¹)ᵀ · n_local
  // (M · n değil: eşit olmayan ölçeklemede yüzeye dik kalmaz)
  if (bestInstance != nullptr)
    n = transformDir(transpose(bestInstance->Minv), n);

  rec.t = bestT;
  rec.point = ray.at(bestT); // dünya ışını + aynı t -> dünyadaki nokta
  rec.normal = normalize(n);
  rec.material = bestMaterial;
  return true;
}

bool anyHit(const Scene &scene, const Ray &ray, double tmin, double tmax) {
  Hit hit;

  // sahnedeki her üçgenle test, herhangi bir intersect olduğu anda true
  for (const Mesh &mesh : scene.meshes) {
    if (useBVH) {
      if (traverseAny(scene, mesh, 0, ray, tmin, tmax))
        return true;
    } else {
      for (const Face &f : mesh.faces) {
        Vec3 a = scene.vertices[f.v[0]];
        Vec3 b = scene.vertices[f.v[1]];
        Vec3 c = scene.vertices[f.v[2]];

        if (intersectTriangle(ray, a, b, c, tmin, tmax, hit)) {
          return true;
        }
      }
    }
  }

  // instance'lar: yerel ışınla aynı test (gölge için normal gerekmez)
  for (const Instance &inst : scene.instances) {
    Ray localRay(transformPoint(inst.Minv, ray.origin),
                 transformDir(inst.Minv, ray.dir));
    const Mesh &mesh = scene.meshes[inst.baseMesh];

    if (useBVH) {
      if (traverseAny(scene, mesh, 0, localRay, tmin, tmax))
        return true;
    } else {
      for (const Face &f : mesh.faces) {
        if (intersectTriangle(localRay, scene.vertices[f.v[0]],
                              scene.vertices[f.v[1]], scene.vertices[f.v[2]],
                              tmin, tmax, hit))
          return true;
      }
    }
  }
  return false;
}

Color shade(const Scene &scene, const Ray &ray, const HitRecord &rec,
            int depth) {
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

  // mirror = km * (yansıyan ışının getirdiği renk)
  Color mirror(0, 0, 0);
  bool isMirror = mat.mirror[0] > 0 || mat.mirror[1] > 0 || mat.mirror[2] > 0;
  if (isMirror && depth < scene.maxRayTraceDepth) { // sonsuz özyineleme olmasın
    Vec3 wr = -wo + 2 * dot(rec.normal, wo) * rec.normal; // wr = -wo + 2n(n.wo)
    Ray reflectRay(rec.point + 1e-4 * rec.normal, wr);   // gölgedeki gibi εn
    HitRecord reflectRec;
    if (closestHit(scene, reflectRay, 0, std::numeric_limits<double>::infinity(),
                   reflectRec)) {
      // çarptığı noktanın rengi = o noktada yeniden shade (bir derin)
      mirror = mat.mirror * shade(scene, reflectRay, reflectRec, depth + 1);
    } // hiçbir şeye çarpmazsa katkı yok
  }

  return ambient + diffuse + specular + mirror;
}
