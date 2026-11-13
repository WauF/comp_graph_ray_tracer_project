// stb kütüphaneleri "single-header" çalışır: header hem bildirimleri hem de
// gövdeleri içerir, ama gövdeler sadece *_IMPLEMENTATION makrosu tanımlıyken
// açılır. Bu makroları projede TAM OLARAK BİR .cpp dosyasında tanımlamalıyız:
//   - hiç tanımlanmazsa  -> linker: "undefined symbol stbi_load"
//   - iki yerde tanımlanırsa -> linker: "duplicate symbol stbi_load"
// Diğer dosyalar sadece #include "stb_image.h" yazar (makrosuz).

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
