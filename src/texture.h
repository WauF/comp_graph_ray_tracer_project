#pragma once
#include "vec3.h"
#include <string>
#include <vector>

// Diskten yüklenmiş texture resmi. RGB, her kanal 0-255.
// Satır satır tutulur, ÜST satır önce (resim dosyalarındaki gibi).
struct Texture {
  int width = 0, height = 0;
  std::vector<unsigned char> data; // (j * width + i) * 3 + kanal
};

// Resmi stb_image ile yükler. Başarısızsa false döner (tex boş kalır).
bool loadTexture(const std::string &path, Texture &tex);

// (u, v) noktasındaki rengi döner, 0-255 ölçeğinde (shading ile aynı ölçek).
// uv sisteminde v = 0 ALTTA, resimde 0. satır ÜSTTE -> satır = (1 - v) ile.
// Nearest neighbor: en yakın pikselin rengi, karıştırma yok.
Color sampleTexture(const Texture &tex, double u, double v);
