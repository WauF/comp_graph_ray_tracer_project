#include "camera.h"

// Struct dışında tanımladığımız için başına "Camera::" yazıyoruz:
// "bu, Camera'nın constructor'ı" demek.
Camera::Camera(Vec3 e, Vec3 up, Vec3 gaze, double l, double r, double b,
               double t, double d, int nx, int ny)
    : e(e), up(up), gaze(gaze), l(l), r(r), b(b), t(t), d(d), nx(nx), ny(ny) {
  w = -normalize(gaze);
  u = normalize(cross(up, w));
  v = cross(w, u);
  // kameranın kendi koordinat sistemini artık biliyorum
  m = e - d * w;
  q = m + l * u + t * v;
  pixelwidth = ((r - l) / nx) * u;
  pixelheight = ((t - b) / ny) * v;
}
