// ADIM 1 TESTİ: her pikselden ışın at, ışının yönünü renge çevir, PNG kaydet.

#include "camera.h"
#include "scene.h"
#include "stb_image_write.h"
#include "triangle.h"
#include <limits.h>
#include <vector>

int main() {
  Scene scene = loadScene("scenes/example_scene.xml");
  printSceneSummary(scene);

  Camera cam = makeCamera(scene.camera);

  std::vector<unsigned char> image(cam.nx * cam.ny * 3);

  Hit hit;

  // her piksel için ray üretip fırlatma
  for (int j = 0; j < cam.ny; ++j) {
    for (int i = 0; i < cam.nx; ++i) {
      Ray ray = cam.generateRay(i, j);
      unsigned char *px = &image[(j * cam.nx + i) * 3];

      double bestT = std::numeric_limits<double>::infinity();
      double tmin = 1e-6;
      bool hitFound = false;
      const Face *bestFace = nullptr;
      // o pikselden çıkan ışının sahnedeki her bir mesh ile çarpışma testi,
      // en yakın olanı alır
      for (const Mesh &mesh : scene.meshes) {
        for (const Face &f : mesh.faces) {
          Vec3 a = scene.vertices[f.v[0]];
          Vec3 b = scene.vertices[f.v[1]];
          Vec3 c = scene.vertices[f.v[2]];

          if (intersectTriangle(ray, a, b, c, tmin, bestT, hit)) {
            bestT = hit.t;
            bestFace = &f;
            hitFound = true;
          }
        }
      }

      // eğer bir meshe çarptıysa bestface ile o piksele o yüzün rengini bas
      if (hitFound) {
        Vec3 na = scene.normals[bestFace->vn[0]];
        Vec3 nb = scene.normals[bestFace->vn[1]]; // bunlar köşe normallerimiz
        Vec3 nc = scene.normals[bestFace->vn[2]];
        double alpha = 1 - hit.beta - hit.gamma;
        Vec3 n = normalize(alpha * na + hit.beta * nb + hit.gamma * nc);
        Vec3 col = n * 0.5 + Vec3(0.5, 0.5, 0.5);
        px[0] = static_cast<unsigned char>(col.x() * 255);
        px[1] = static_cast<unsigned char>(col.y() * 255);
        px[2] = static_cast<unsigned char>(col.z() * 255);
      } else {
        px[0] = px[1] = px[2] = 0;
      }
    }
  }

  stbi_write_png("step1.png", cam.nx, cam.ny, 3, image.data(), cam.nx * 3);
  return 0;
}
