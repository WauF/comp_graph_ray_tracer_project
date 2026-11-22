// ADIM 1 TESTİ: her pikselden ışın at, ışının yönünü renge çevir, PNG kaydet.

#include "camera.h"
#include "render.h"
#include "scene.h"
#include "stb_image_write.h"
#include "vec3.h"
#include <algorithm>
#include <limits>
#include <vector>

int main() {
  Scene scene = loadScene("scenes/shadow_test.xml");
  printSceneSummary(scene);

  Camera cam = makeCamera(scene.camera);

  std::vector<unsigned char> image(cam.nx * cam.ny * 3);

  // her piksel için ray üretip fırlatma
  for (int j = 0; j < cam.ny; ++j) {
    for (int i = 0; i < cam.nx; ++i) {
      Ray ray = cam.generateRay(i, j);
      unsigned char *px = &image[(j * cam.nx + i) * 3];

      HitRecord rec;
      Color pixelColor;
      if (closestHit(scene, ray, 1e-6, std::numeric_limits<double>::infinity(),
                     rec)) {
        pixelColor = shade(scene, ray, rec);
      } else {
        pixelColor = scene.background;
      }
      px[0] = (unsigned char)std::min(255.0, pixelColor[0]);
      px[1] = (unsigned char)std::min(255.0, pixelColor[1]);
      px[2] = (unsigned char)std::min(255.0, pixelColor[2]);
    }
  }

  stbi_write_png("shadow_test.png", cam.nx, cam.ny, 3, image.data(),
                 cam.nx * 3);
  return 0;
}
