// ADIM 1 TESTİ: her pikselden ışın at, ışının yönünü renge çevir, PNG kaydet.

#include "camera.h"
#include "stb_image_write.h"
#include "triangle.h"
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

  Vec3 a(-1, -1, 0), b(1, -1, 0), c(0, 1, 0);
  Hit hit;

  for (int j = 0; j < cam.ny; ++j) {
    for (int i = 0; i < cam.nx; ++i) {
      Ray ray = cam.generateRay(i, j);
      unsigned char *px = &image[(j * cam.nx + i) * 3];
      if (intersectTriangle(ray, a, b, c, 0.000001, 100, hit)) {
        px[0] = 255.0;
        px[1] = 255.0;
        px[2] = 255.0;
      } else {
        px[0] = 0.0;
        px[1] = 0.0;
        px[2] = 0.0;
      }
    }
  }

  stbi_write_png("step1.png", cam.nx, cam.ny, 3, image.data(), cam.nx * 3);
  return 0;
}
