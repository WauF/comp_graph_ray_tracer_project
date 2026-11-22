#pragma once
#include "ray.h"
#include "vec3.h"
#include <vector>

struct AABB {
  Vec3 min, max;
};

struct BVHNode {
  AABB box;
  int left = -1, right = -1; // iç düğüm: çocukların nodes[] indeksi
  int start = 0, count = 0;  // yaprak: faceOrder[start .. start+count)
}; // count > 0 ise yaprak

struct BVH {
  std::vector<BVHNode> nodes; // nodes[0] = kök
  std::vector<int> faceOrder; // mesh.faces indeksleri, build sırasında karışır
};

bool hitAABB(const AABB &box, const Ray &ray, double tmin, double tmax);

struct Scene; // forward declaration: sadece referans olarak kullanıyoruz,
struct Mesh; // scene.h'yi include etsek döngü olurdu (scene.h de bunu include
             // edecek)
void buildBVH(const Scene &scene, Mesh &mesh);