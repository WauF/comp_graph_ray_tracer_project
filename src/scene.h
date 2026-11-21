#pragma once
#include "camera.h"
#include "vec3.h"
#include <string>
#include <vector>

// Sahnenin XML'den okunmuş hali. Sadece VERİ tutar, hesaplama yapmaz.
//
// Önemli kurallar (parser bunları zaten uyguluyor):
//   - XML'deki indeksler 1'den başlar; burada HEPSİ 0'dan başlar.
//   - XML'deki id'ler (materialid, basemeshid, t1/r1/s1) okurken vektör
//     indeksine çevrilir. Render sırasında id -> nesne araması yapılmaz,
//     doğrudan scene.materials[mesh.material] gibi erişilir.
//   - Renk/ışık değerleri XML'deki gibi ham tutulur (background 0-255,
//     ambient 25 gibi). Ölçekleme shading'in işi.

struct SceneCamera {
  Vec3 position, gaze, up;
  double l = -1, r = 1, b = -1, t = 1; // XML'deki sıra: left right bottom top
  double nearDistance = 1;
  int nx = 0, ny = 0;
};

// SceneCamera -> Camera (u, v, w ve image plane hesaplarını Camera yapar)
inline Camera makeCamera(const SceneCamera &c) {
  return Camera(c.position, c.up, c.gaze, c.l, c.r, c.b, c.t, c.nearDistance,
                c.nx, c.ny);
}

struct PointLight {
  Vec3 position;
  Color intensity;
};

// Yön PDF'teki formülle shading tarafında hesaplanacak:
//   d_light = normalize(cross(v1 - v2, v1 - v3))
struct TriangularLight {
  Vec3 v1, v2, v3;
  Color intensity;
};

struct Material {
  Color ambient, diffuse, specular; // ka, kd, ks  (her biri [0,1]^3)
  Color mirror;                     // km
  double phongExponent = 1;         // p
  double textureFactor = 0;         // t: 0 = sadece shading, 1 = sadece texture
};

struct Transformation {
  enum class Kind { Translation, Scaling, Rotation };
  Kind kind;
  Vec3 v;            // translation: (tx,ty,tz)  scaling: (sx,sy,sz)
                     // rotation: dönme ekseni (rx,ry,rz)
  double angle = 0;  // sadece rotation: derece cinsinden
};

struct TexCoord {
  double u = 0, v = 0;
};

// Bir üçgen yüzü: her köşe için vertex / texcoord / normal indeksleri.
// Hepsi 0'dan başlar. XML'de verilmemişse (ör. "3//5") -1 olur.
struct Face {
  int v[3];
  int vt[3];
  int vn[3];
};

struct Mesh {
  int id = 0;           // XML'deki id (sadece hata mesajları / debug için)
  int material = 0;     // scene.materials içindeki indeks
  std::vector<Face> faces;
};

struct Instance {
  int id = 0;
  int baseMesh = 0;     // scene.meshes içindeki indeks
  int material = 0;     // scene.materials içindeki indeks (XML'de yoksa
                        // base mesh'in materyali)
  // XML'de yazıldığı sırayla ("t1 r1 s1"), scene.transformations indeksleri.
  // PDF kuralı: M = T1 · R1 · S1 -> noktaya SAĞDAN SOLA uygulanır (önce S1).
  std::vector<int> transforms;
};

struct Scene {
  int maxRayTraceDepth = 0;
  Color background;
  SceneCamera camera;

  Color ambientLight;
  std::vector<PointLight> pointLights;
  std::vector<TriangularLight> triangularLights;

  std::vector<Material> materials;
  std::vector<Transformation> transformations;

  std::vector<Vec3> vertices;
  std::vector<Vec3> normals;
  std::vector<TexCoord> texCoords;
  std::string textureImage; // XML dosyasının klasörüne göre çözülmüş yol;
                            // sahnede texture yoksa boş

  std::vector<Mesh> meshes;
  std::vector<Instance> instances;
};

// XML sahne dosyasını okur. Hata olursa std::runtime_error fırlatır
// (mesaj hangi etiketin/indeksin bozuk olduğunu söyler).
Scene loadScene(const std::string &path);

// Debug: okunan sahnenin özetini yazdırır (kaç vertex, kaç yüz, ...).
void printSceneSummary(const Scene &scene);
