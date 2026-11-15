#pragma once
#include <cassert>
#include <cmath>
#include <ostream>

struct Vec3 {

  double e[3];
  Vec3() : e{0, 0, 0} {}
  Vec3(double e0, double e1, double e2) : e{e0, e1, e2} {}

  double x() const { return e[0]; }
  double y() const { return e[1]; }
  double z() const { return e[2]; }

  Vec3 operator-() const { return Vec3(-e[0], -e[1], -e[2]); }
  double operator[](int i) const { return e[i]; }
  double &operator[](int i) { return e[i]; }

  Vec3 &operator+=(const Vec3 &v) {
    e[0] += v.e[0];
    e[1] += v.e[1];
    e[2] += v.e[2];
    return *this;
  }

  Vec3 &operator-=(const Vec3 &v) {
    e[0] -= v.e[0];
    e[1] -= v.e[1];
    e[2] -= v.e[2];
    return *this;
  }

  Vec3 &operator*=(double t) {
    e[0] *= t;
    e[1] *= t;
    e[2] *= t;
    return *this;
  }

  Vec3 &operator/=(double t) { return *this *= 1 / t; }

  double length_squared() const {
    return e[0] * e[0] + e[1] * e[1] + e[2] * e[2];
  }

  double length() const { return std::sqrt(length_squared()); }
};

// Renkler de 3 bileşenli (r, g, b), aynı aritmetiği kullanıyor.
// Ayrı isim sadece okunabilirlik için: Color görünce "bu bir renk" anlaşılır.
using Color = Vec3;

// ---------------------------------------------------------------------------
// Serbest (struct dışı) operatörler
//
// Neden struct dışında? `2.0 * v` yazabilmek için. Member operatörde sol taraf
// her zaman Vec3 olmak zorunda; serbest fonksiyonda sol taraf double olabilir.
// Hepsi `inline` ve header'da: derleyici her çağrıyı yerinde açabilsin
// (ray tracer'da bunlar yüz milyonlarca kez çağrılıyor).
// ---------------------------------------------------------------------------

inline Vec3 operator+(const Vec3 &a, const Vec3 &b) {
  return Vec3(a.e[0] + b.e[0], a.e[1] + b.e[1], a.e[2] + b.e[2]);
}

inline Vec3 operator-(const Vec3 &a, const Vec3 &b) {
  return Vec3(a.e[0] - b.e[0], a.e[1] - b.e[1], a.e[2] - b.e[2]);
}

// Vektör * skaler ve skaler * vektör: C++ bunları otomatik eşlemez, ikisi de
// lazım.
inline Vec3 operator*(const Vec3 &v, double t) {
  return Vec3(v.e[0] * t, v.e[1] * t, v.e[2] * t);
}

inline Vec3 operator*(double t, const Vec3 &v) { return v * t; }

// Bir bölme + üç çarpma, üç bölmeden daha ucuz (bölme işlemcide çarpmadan
// yavaş).
inline Vec3 operator/(const Vec3 &v, double t) { return v * (1.0 / t); }

// Eleman bazında çarpım (Hadamard). Geometride anlamı yok ama renkte çok lazım:
// diffuse katsayısı * ışık şiddeti  ->  kd * I  =  (kd.r*I.r, kd.g*I.g,
// kd.b*I.b)
inline Vec3 operator*(const Vec3 &a, const Vec3 &b) {
  return Vec3(a.e[0] * b.e[0], a.e[1] * b.e[1], a.e[2] * b.e[2]);
}

// ---------------------------------------------------------------------------
// Geometri
// ---------------------------------------------------------------------------

// a·b = |a||b|cos(θ). İki birim vektör için doğrudan aradaki açının kosinüsü:
// Blinn-Phong'daki (n·l) ve (n·h) terimleri tam olarak bu.
inline double dot(const Vec3 &a, const Vec3 &b) {
  return a.e[0] * b.e[0] + a.e[1] * b.e[1] + a.e[2] * b.e[2];
}

// a×b: hem a'ya hem b'ye dik vektör, sağ el kuralı. Sıra önemli: a×b = -(b×a).
// Kullanıldığı yerler: kamera tabanı (u = gaze × up), üçgen normali,
// triangular light yönü, Möller–Trumbore kesişimi.
inline Vec3 cross(const Vec3 &a, const Vec3 &b) {
  return Vec3(a.e[1] * b.e[2] - a.e[2] * b.e[1],
              a.e[2] * b.e[0] - a.e[0] * b.e[2],
              a.e[0] * b.e[1] - a.e[1] * b.e[0]);
}

// Aynı yönde, uzunluğu 1 olan vektör.
// DİKKAT: sıfır vektörü normalize edilirse 0/0 = NaN olur ve sessizce yayılır
// (ekranda açıklanamayan siyah pikseller). Debug build'de assert yakalar;
// Release'te (-DNDEBUG) assert kapalıdır.
inline Vec3 normalize(const Vec3 &v) {
  double len = v.length();
  assert(len > 0.0 && "sifir vektor normalize edildi");
  return v / len;
}

// Yansıma: r = d - 2(d·n)n   (ödev PDF'teki formül)
// d: gelen yön, n: birim yüzey normali. n birim değilse sonuç yanlış olur.
inline Vec3 reflect(const Vec3 &d, const Vec3 &n) {
  return d - 2.0 * dot(d, n) * n;
}

// Eleman bazında min / max. AABB (bounding box) hesaplamak için:
// kutunun alt köşesi = tüm noktaların min'i, üst köşesi = max'ı.
inline Vec3 min(const Vec3 &a, const Vec3 &b) {
  return Vec3(std::fmin(a.e[0], b.e[0]), std::fmin(a.e[1], b.e[1]),
              std::fmin(a.e[2], b.e[2]));
}

inline Vec3 max(const Vec3 &a, const Vec3 &b) {
  return Vec3(std::fmax(a.e[0], b.e[0]), std::fmax(a.e[1], b.e[1]),
              std::fmax(a.e[2], b.e[2]));
}

// Debug için: std::cout << v;  ->  (1, 2, 3)
inline std::ostream &operator<<(std::ostream &os, const Vec3 &v) {
  return os << '(' << v.e[0] << ", " << v.e[1] << ", " << v.e[2] << ')';
}