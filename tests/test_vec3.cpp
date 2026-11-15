// Vec3 için küçük test programı. Test kütüphanesi kullanmıyoruz, kendi CHECK
// makromuz yeterli. Çalıştırma:  ctest --test-dir build --output-on-failure

#include "vec3.h"
#include <cmath>
#include <iostream>

static int failures = 0;

// double'ları == ile karşılaştırmak tehlikeli (0.1 + 0.2 != 0.3), tolerans kullan.
static bool approx(const Vec3 &a, const Vec3 &b, double eps = 1e-9) {
  return std::fabs(a.x() - b.x()) < eps && std::fabs(a.y() - b.y()) < eps &&
         std::fabs(a.z() - b.z()) < eps;
}

#define CHECK(cond)                                                            \
  do {                                                                         \
    if (!(cond)) {                                                             \
      std::cerr << "FAIL " << __FILE__ << ":" << __LINE__ << "  " #cond "\n";  \
      ++failures;                                                              \
    }                                                                          \
  } while (0)

int main() {
  const Vec3 X(1, 0, 0), Y(0, 1, 0), Z(0, 0, 1);

  // Aritmetik
  CHECK(approx(Vec3(1, 2, 3) + Vec3(4, 5, 6), Vec3(5, 7, 9)));
  CHECK(approx(Vec3(4, 5, 6) - Vec3(1, 2, 3), Vec3(3, 3, 3)));
  CHECK(approx(-Vec3(1, -2, 3), Vec3(-1, 2, -3)));
  CHECK(approx(Vec3(1, 2, 3) * 2.0, Vec3(2, 4, 6)));
  CHECK(approx(2.0 * Vec3(1, 2, 3), Vec3(2, 4, 6)));
  CHECK(approx(Vec3(2, 4, 6) / 2.0, Vec3(1, 2, 3)));
  CHECK(approx(Vec3(1, 2, 3) * Vec3(2, 3, 4), Vec3(2, 6, 12)));

  Vec3 v(1, 1, 1);
  v += Vec3(1, 2, 3);
  v -= Vec3(0, 1, 0);
  v *= 2.0;
  CHECK(approx(v, Vec3(4, 4, 8)));

  // dot
  CHECK(dot(Vec3(1, 2, 3), Vec3(4, 5, 6)) == 32.0);
  CHECK(dot(X, Y) == 0.0); // dik vektörler

  // cross: sağ el kuralı. İşaret ters olursa kamera ve normaller ters döner.
  CHECK(approx(cross(X, Y), Z));
  CHECK(approx(cross(Y, Z), X));
  CHECK(approx(cross(Z, X), Y));
  CHECK(approx(cross(Y, X), -Z)); // sıra değişince işaret değişir

  // length / normalize
  CHECK(Vec3(3, 4, 0).length() == 5.0);
  CHECK(approx(normalize(Vec3(3, 4, 0)), Vec3(0.6, 0.8, 0)));
  CHECK(std::fabs(normalize(Vec3(7, -3, 2)).length() - 1.0) < 1e-12);

  // reflect: aşağı-sağa giden ışın, yukarı bakan zeminden yukarı-sağa döner
  CHECK(approx(reflect(Vec3(1, -1, 0), Y), Vec3(1, 1, 0)));

  // min / max (AABB köşeleri)
  CHECK(approx(min(Vec3(1, 5, -2), Vec3(3, 0, 4)), Vec3(1, 0, -2)));
  CHECK(approx(max(Vec3(1, 5, -2), Vec3(3, 0, 4)), Vec3(3, 5, 4)));

  // operator[]
  CHECK(Vec3(7, 8, 9)[1] == 8.0);

  if (failures == 0)
    std::cout << "Vec3: tum testler gecti\n";
  return failures == 0 ? 0 : 1; // 0 dışı çıkış kodu = ctest "failed" sayar
}
