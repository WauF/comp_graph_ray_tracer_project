// ADIM 1 TESTİ: her pikselden ışın at, ışının yönünü renge çevir, PNG kaydet.

#include "camera.h"
#include "render.h"
#include "scene.h"
#include "stb_image_write.h"
#include "vec3.h"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <limits>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

// Kullanım: ./raytracer <sahne.xml> <çıktı.png> [--no-bvh]
int main(int argc, char **argv) {
  // 3. argüman isteğe bağlı: sadece "--no-bvh" olabilir
  bool badArgs = argc < 3 || argc > 4 ||
                 (argc == 4 && std::string(argv[3]) != "--no-bvh");
  if (badArgs) {
    fprintf(stderr, "Usage: %s <scene.xml> <output.png> [--no-bvh]\n",
            argv[0]);
    return 1;
  }
  const char *scenePath = argv[1]; // argv[0] programın kendi adı
  const char *outputPath = argv[2];

  useBVH = (argc != 4); // "--no-bvh" verildiyse brute force
  printf("BVH: %s\n", useBVH ? "on" : "off");
  Scene scene;
  try {
    scene = loadScene(scenePath);
  } catch (const std::runtime_error &e) { // parser hata mesajıyla fırlatır
    fprintf(stderr, "Error: %s\n", e.what());
    return 1;
  }
  for (Mesh &m : scene.meshes) {
    buildBVH(scene, m);
  }

  // texture resmi: sahnede varsa bir kez yükle (render sırasında sadece okunur)
  if (!scene.textureImage.empty() &&
      !loadTexture(scene.textureImage, scene.texture))
    printf("UYARI: texture yüklenemedi: %s (texture'sız devam)\n",
           scene.textureImage.c_str());

  // her instance için M ve M⁻¹ (XML sırası: "t1 r1 s1" -> M = T1 · R1 · S1)
  for (Instance &inst : scene.instances) {
    inst.M = identity();
    inst.Minv = identity();
    for (int id : inst.transforms) {
      const Transformation &tr = scene.transformations[id];
      Mat4 A, Ainv; // bu dönüşüm ve tersi
      if (tr.kind == Transformation::Kind::Translation) {
        A = translation(tr.v);
        Ainv = translation(-tr.v);
      } else if (tr.kind == Transformation::Kind::Scaling) {
        A = scaling(tr.v);
        Ainv = scaling(Vec3(1 / tr.v.x(), 1 / tr.v.y(), 1 / tr.v.z()));
      } else {
        A = rotation(tr.angle, tr.v);
        Ainv = rotation(-tr.angle, tr.v);
      }
      inst.M = inst.M * A;          // sağa ekle: T, T·R, T·R·S
      inst.Minv = Ainv * inst.Minv; // sola ekle: T⁻¹, R⁻¹T⁻¹, S⁻¹R⁻¹T⁻¹
    }
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

  // stbi_write_png başarısızsa 0 döner (ör. klasör yoksa)
  if (!stbi_write_png(outputPath, cam.nx, cam.ny, 3, image.data(),
                      cam.nx * 3)) {
    fprintf(stderr, "Error: cannot write %s\n", outputPath);
    return 1;
  }
  printf("image saved: %s\n", outputPath);
  return 0;
}
