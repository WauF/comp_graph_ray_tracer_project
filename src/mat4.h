#pragma once
#include "vec3.h"
#include <cmath>

// 4x4 matris: instancing dönüşümleri için (homojen koordinatlar).
// m[satır][sütun]. Noktayı SÜTUN vektör olarak sağdan çarparız: p' = M · p
// Bu yüzden M = T · R · S noktaya sağdan sola uygulanır (önce S).
//
// Hepsi inline ve header'da: küçük, sık çağrılan fonksiyonlar, .cpp'ye gerek
// yok.
struct Mat4 {
  double m[4][4];
};

// Birim matris: köşegen 1, gerisi 0
inline Mat4 identity() {
  Mat4 r{};
  for (int i = 0; i < 4; ++i)
    r.m[i][i] = 1.0;
  return r;
}

// Matris çarpımı:
inline Mat4 operator*(const Mat4 &a, const Mat4 &b) {
  Mat4 r{};
  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 4; ++j)
      for (int k = 0; k < 4; ++k)
        r.m[i][j] += a.m[i][k] * b.m[k][j];
  return r;
}

// Öteleme: 4. sütuna (tx, ty, tz) yazılır
//  | 1 0 0 tx |
//  | 0 1 0 ty |
//  | 0 0 1 tz |
//  | 0 0 0 1  |
inline Mat4 translation(const Vec3 &t) {
  Mat4 r = identity();
  r.m[0][3] = t.x();
  r.m[1][3] = t.y();
  r.m[2][3] = t.z();
  return r;
}

// Ölçekleme: köşegene (sx, sy, sz)
inline Mat4 scaling(const Vec3 &s) {
  Mat4 r = identity();
  r.m[0][0] = s.x();
  r.m[1][1] = s.y();
  r.m[2][2] = s.z();
  return r;
}

// Rastgele eksen etrafında döndürme (Rodrigues). Açı DERECE (XML'deki gibi),
// eksen normalize edilmemiş olabilir, burada normalize ediliyor.
// Sağ el kuralı: başparmak eksen yönünde, parmaklar pozitif dönüş yönünde.
//   R = c·I + s·[u]x + (1-c)·u·uᵀ
inline Mat4 rotation(double degrees, const Vec3 &axis) {
  Vec3 u = normalize(axis);
  const double PI = 3.14159265358979323846;
  double theta = degrees * PI / 180.0;
  double c = std::cos(theta), s = std::sin(theta), k = 1.0 - c;
  double x = u.x(), y = u.y(), z = u.z();

  Mat4 r = identity();
  r.m[0][0] = c + x * x * k;
  r.m[0][1] = x * y * k - z * s;
  r.m[0][2] = x * z * k + y * s;

  r.m[1][0] = y * x * k + z * s;
  r.m[1][1] = c + y * y * k;
  r.m[1][2] = y * z * k - x * s;

  r.m[2][0] = z * x * k - y * s;
  r.m[2][1] = z * y * k + x * s;
  r.m[2][2] = c + z * z * k;
  return r;
}

// Devrik: satırlar sütun olur. Normal dönüşümü için: n' = (M⁻¹)ᵀ · n
inline Mat4 transpose(const Mat4 &a) {
  Mat4 r;
  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 4; ++j)
      r.m[i][j] = a.m[j][i];
  return r;
}

// Nokta dönüşümü: [p, 1] -> öteleme (4. sütun) EKLENİR.
// Bizim matrislerimiz afin (son satır 0 0 0 1), bu yüzden w hep 1 kalır,
// w'ya bölmeye gerek yok.
inline Vec3 transformPoint(const Mat4 &a, const Vec3 &p) {
  return Vec3(
      a.m[0][0] * p.x() + a.m[0][1] * p.y() + a.m[0][2] * p.z() + a.m[0][3],
      a.m[1][0] * p.x() + a.m[1][1] * p.y() + a.m[1][2] * p.z() + a.m[1][3],
      a.m[2][0] * p.x() + a.m[2][1] * p.y() + a.m[2][2] * p.z() + a.m[2][3]);
}

// Yön dönüşümü: [d, 0] -> öteleme YOK SAYILIR (yönün konumu yok).
// Işın yönü ve normal için kullanılır.
inline Vec3 transformDir(const Mat4 &a, const Vec3 &d) {
  return Vec3(a.m[0][0] * d.x() + a.m[0][1] * d.y() + a.m[0][2] * d.z(),
              a.m[1][0] * d.x() + a.m[1][1] * d.y() + a.m[1][2] * d.z(),
              a.m[2][0] * d.x() + a.m[2][1] * d.y() + a.m[2][2] * d.z());
}
