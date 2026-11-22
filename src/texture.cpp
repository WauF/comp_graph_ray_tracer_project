#include "texture.h"
#include "stb_image.h" // sadece bildirimler; gövdeler stb_impl.cpp'de
#include <algorithm>
#include <cmath>

bool loadTexture(const std::string &path, Texture &tex) {
  int w, h, channels;
  // son parametre 3: dosya gri/RGBA olsa bile bize RGB (3 kanal) ver
  unsigned char *pixels = stbi_load(path.c_str(), &w, &h, &channels, 3);
  if (pixels == nullptr)
    return false;

  tex.width = w;
  tex.height = h;
  tex.data.assign(pixels, pixels + w * h * 3); // kendi vektörümüze kopyala
  stbi_image_free(pixels); // stb'nin ayırdığı belleği geri ver
  return true;
}

Color sampleTexture(const Texture &tex, double u, double v) {
  // uv [0,1] dışına taşarsa (kayan nokta hatası) kenara sabitle
  u = std::clamp(u, 0.0, 1.0);
  v = std::clamp(v, 0.0, 1.0);

  int i = (int)std::lround(u * (tex.width - 1));         // sütun
  int j = (int)std::lround((1.0 - v) * (tex.height - 1)); // satır: v ters!

  const unsigned char *px = &tex.data[(j * tex.width + i) * 3];
  return Color(px[0], px[1], px[2]);
}
