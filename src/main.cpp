#include "pugixml.hpp"
#include "stb_image_write.h" // makrosuz! implementation stb_impl.cpp'de
#include "vec3.h"
#include <iostream>
#include <ostream>

int main() {

  Vec3 unit_vector(1, 1, 1);
  std::cout << unit_vector.length() << std::endl;
  unit_vector /= 0;
  std::cout << unit_vector.x() << unit_vector.y() << unit_vector.z()
            << std::endl;

  return 0;
}