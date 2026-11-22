// ADIM 1 TESTİ: her pikselden ışın at, ışının yönünü renge çevir, PNG kaydet.

#include "camera.h"
#include "render.h"
#include "scene.h"
#include "stb_image_write.h"
#include "vec3.h"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <limits>
#include <thread>
#include <vector>

int main() {
  useBVH = true;
  Scene scene = loadScene("scenes/mirror_test.xml");
  for (Mesh &m : scene.meshes) {
    buildBVH(scene, m);
  }
  printSceneSummary(scene);

  Camera cam = makeCamera(scene.camera);

  std::vector<unsigned char> image(cam.nx * cam.ny * 3);

  // her piksel için ray üretip fırlatma
  auto t0 = std::chrono::steady_clock::now();

  // sıradaki çizilecek satır: thread'ler buradan satır kapar
  std::atomic<int> nextRow(0);

  // her thread bu fonksiyonu çalıştırır ([&]: main'deki değişkenleri referansla
  // görür)
  auto worker = [&]() {
    while (true) {
      int j = nextRow++; // atomik: iki thread aynı satırı ALAMAZ
      if (j >= cam.ny)
        break; // satır kalmadı, bu thread bitti

      for (int i = 0; i < cam.nx; ++i) {
        Ray ray = cam.generateRay(i, j);
        unsigned char *px = &image[(j * cam.nx + i) * 3];

        HitRecord rec;
        Color pixelColor;
        if (closestHit(scene, ray, 1e-6,
                       std::numeric_limits<double>::infinity(), rec)) {
          pixelColor = shade(scene, ray, rec);
        } else {
          pixelColor = scene.background;
        }
        px[0] = (unsigned char)std::min(255.0, pixelColor[0]);
        px[1] = (unsigned char)std::min(255.0, pixelColor[1]);
        px[2] = (unsigned char)std::min(255.0, pixelColor[2]);
      }
    }
  };
  // çalışan makinede kaç thread çalışabilir max buna bakar
  int numThreads = std::thread::hardware_concurrency();

  if (numThreads == 0)
    numThreads = 1; // standart: bilinmiyorsa 0 dönebilir

  std::vector<std::thread> threads;
  for (int k = 0; k < numThreads; ++k)
    threads.emplace_back(worker); // oluşturulduğu anda çalışmaya başlar
  for (std::thread &th : threads)
    th.join(); // hepsi bitene kadar bekle

  auto t1 = std::chrono::steady_clock::now();
  printf("render süresi: %.3f s\n",
         std::chrono::duration<double>(t1 - t0).count());

  stbi_write_png("mirror_test.png", cam.nx, cam.ny, 3, image.data(),
                 cam.nx * 3);
  return 0;
}
