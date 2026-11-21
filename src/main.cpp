// ADIM 1 TESTİ: her pikselden ışın at, ışının yönünü renge çevir, PNG kaydet.

#include "camera.h"
#include "stb_image_write.h"
#include <vector>

int main() {
  // PA1 örnek kamerası
  Camera cam(Vec3(0, 0, 4),  // position
             Vec3(0, 1, 0),  // up
             Vec3(0, 0, -1), // gaze
             -1, 1, -1, 1,   // l r b t
             1,              // neardistance
             400, 400);      // nx ny

  std::vector<unsigned char> image(cam.nx * cam.ny * 3);

  for (int j = 0; j < cam.ny; ++j) {
    for (int i = 0; i < cam.nx; ++i) {
      Ray ray = cam.generateRay(i, j);

      // yön [-1, 1] -> renk [0, 1] -> [0, 255]
      Vec3 c = normalize(ray.dir) * 0.5 + Vec3(0.5, 0.5, 0.5);

      unsigned char *px = &image[(j * cam.nx + i) * 3];
      px[0] = static_cast<unsigned char>(c.x() * 255.0);
      px[1] = static_cast<unsigned char>(c.y() * 255.0);
      px[2] = static_cast<unsigned char>(c.z() * 255.0);
    }
  }

  stbi_write_png("step1.png", cam.nx, cam.ny, 3, image.data(), cam.nx * 3);
  return 0;
}
