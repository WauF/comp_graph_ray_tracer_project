#include "pugixml.hpp"
#include <iostream>

int main() {
  pugi::xml_document doc;
  pugi::xml_parse_result result = doc.load_file("scenes/example_scene.xml");

  if (!result) {
    std::cerr << "XML yükleme hatası: " << result.description() << std::endl;
    return 1;
  }

  pugi::xml_node scene = doc.child("scene");

  // Doğrudan erişim örneği:
  std::cout << "maxraytracedepth: "
            << scene.child("maxraytracedepth").text().as_int() << "\n\n";

  // scene altındaki tüm çocukları ve değerlerini listeleme:
  for (pugi::xml_node child : scene.children()) {
    std::cout << "Etiket: <" << child.name() << ">";

    // Varsa etiket öznitelikleri (örn: id="1")
    for (pugi::xml_attribute attr : child.attributes()) {
      std::cout << " [" << attr.name() << "=\"" << attr.value() << "\"]";
    }

    // Metin içeriği varsa (örn: 6 veya 0 0 0)
    std::string text_val = child.child_value();
    if (!text_val.empty()) {
      std::cout << " -> Metin: " << text_val;
    }

    std::cout << std::endl;
  }

  return 0;
}