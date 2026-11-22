#include "bvh.h"
#include "scene.h"
#include <algorithm>
#include <limits>

// ADIM 3'te yazılacak (slab method).
bool hitAABB(const AABB &box, const Ray &ray, double tmin, double tmax) {
  for (int a = 0; a < 3; ++a) { // a = 0:x  1:y  2:z

    // t0 = (box.min.x - o.x) / d.x, t1 = (box.max.x - o.x) / d.x
    double invD = 1.0 / ray.dir[a];
    // kutunun bu axisteki sınırlarına ışının ilk girdiği an
    double t0 = (box.min[a] - ray.origin[a]) * invD;
    // ışının kutudan çıktığı ans
    double t1 = (box.max[a] - ray.origin[a]) * invD;

    // ışın bu eksende negatif yöne gidiyorsa önce max duvarına çarpar
    if (invD < 0)
      std::swap(t0, t1);

    // aralığı daralt: giriş = en geç giriş, çıkış = en erken çıkış
    tmin = t0 > tmin ? t0 : tmin;
    tmax = t1 < tmax ? t1 : tmax;

    if (tmax < tmin)
      return false; // aralıklar ayrıştı → ıskaladı
  }
  return true;
}

// i numaralı yüzün ağırlık merkezi (3 köşenin ortalaması)
static Vec3 centroid(const Scene &scene, const Mesh &mesh, int i) {
  const Face &f = mesh.faces[i];
  return (scene.vertices[f.v[0]] + scene.vertices[f.v[1]] +
          scene.vertices[f.v[2]]) /
         3.0;
}

// faceOrder[start..end) aralığı için bir düğüm kurar, düğümün indeksini döner
static int build(const Scene &scene, const Mesh &mesh, BVH &bvh, int start,
                 int end) {
  int idx = bvh.nodes.size();
  bvh.nodes.push_back(BVHNode());

  // 1) kutu: boş kutudan başla, aralıktaki her üçgenin 3 köşesiyle genişlet
  const double inf = std::numeric_limits<double>::infinity();
  AABB box;
  box.min = Vec3(inf, inf, inf);
  box.max = Vec3(-inf, -inf, -inf);
  for (int k = start; k < end; ++k) {
    const Face &f = mesh.faces[bvh.faceOrder[k]];
    for (int c = 0; c < 3; ++c) {
      Vec3 p = scene.vertices[f.v[c]];
      box.min = min(box.min, p);
      box.max = max(box.max, p);
    }
  }
  bvh.nodes[idx].box = box;

  // 2) az üçgen kaldıysa yaprak: üçgenleri burada tut, bölme
  if (end - start <= 4) {
    bvh.nodes[idx].start = start;
    bvh.nodes[idx].count = end - start;
    return idx;
  }

  // 3) kutunun en uzun ekseni
  Vec3 d = box.max - box.min;
  int axis = 0;
  if (d[1] > d[axis])
    axis = 1;
  if (d[2] > d[axis])
    axis = 2;

  // 4) medyana göre böl: mid'deki eleman yerine oturur, solundakilerin
  //    merkezi daha küçük, sağındakilerin daha büyük olur
  int mid = (start + end) / 2;
  std::nth_element(bvh.faceOrder.begin() + start, bvh.faceOrder.begin() + mid,
                   bvh.faceOrder.begin() + end, [&](int a, int b) {
                     return centroid(scene, mesh, a)[axis] <
                            centroid(scene, mesh, b)[axis];
                   });

  // 5) çocukları kur. DİKKAT: build içinde push_back var, vektör taşınabilir;
  //    o yüzden önceden referans tutmuyoruz, bvh.nodes[idx] ile yeniden
  //    erişiyoruz
  int l = build(scene, mesh, bvh, start, mid);
  int r = build(scene, mesh, bvh, mid, end);
  bvh.nodes[idx].left = l;
  bvh.nodes[idx].right = r;
  return idx;
}

void buildBVH(const Scene &scene, Mesh &mesh) {
  BVH &bvh = mesh.bvh;
  bvh.nodes.clear();
  bvh.faceOrder.resize(mesh.faces.size());
  for (int i = 0; i < (int)mesh.faces.size(); ++i)
    bvh.faceOrder[i] = i; // başta sıra: 0, 1, 2, ...

  if (mesh.faces.empty())
    return;
  build(scene, mesh, bvh, 0, mesh.faces.size());
}
