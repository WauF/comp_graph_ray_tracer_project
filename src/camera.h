#pragma once
#include "ray.h"
#include "vec3.h"

// Kamera: konum (e) + yönelim (u, v, w) + image plane (l, r, b, t, d, nx, ny).
// Notasyon hocanın Ray Tracing (1) slaytları 4-7 ve 14 ile aynı.
//
// Header / .cpp ayrımı:
//   - struct tanımı burada, çünkü Camera'yı kullanan her .cpp onun içini
//     (üyelerini, boyutunu) görmek zorunda.
//   - Constructor camera.cpp'de: sahne başına BİR kez çağrılıyor, inline
//     edilmesinin faydası yok.
//   - generateRay burada (struct içinde gövdesiyle -> otomatik inline): her
//     piksel için çağrılıyor, derleyici çağrı yerine gömebilsin.
struct Camera {
  Vec3 e;
  Vec3 up, gaze;
  double l, r, b, t, d;
  int nx, ny;

  Vec3 u, v, w;
  Vec3 pixelwidth, pixelheight, q, m;

  Camera(Vec3 e, Vec3 up, Vec3 gaze, double l, double r, double b, double t,
         double d, int nx, int ny);

  // (i, j) pikselinin merkezinden geçen birincil ışın.
  // i: sütun (0 = sol), j: satır (0 = üst). Yön normalize EDİLMEZ:
  // t = 1 tam image plane üzerine denk gelir.
  Ray generateRay(int i, int j) const {
    Vec3 su = pixelwidth * (i + 0.5);
    Vec3 sv = pixelheight * (j + 0.5);
    Vec3 s = q + su - sv;
    return Ray(e, s - e);
  }
};
