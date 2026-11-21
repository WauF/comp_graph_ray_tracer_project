// XML sahne dosyasını Scene yapısına çevirir (pugixml kullanarak).
// Ray tracing mantığı burada YOK; sadece metin -> sayı -> struct.

#include "scene.h"
#include "pugixml.hpp"

#include <cstdio>
#include <filesystem>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

namespace {

// --- Küçük yardımcılar ------------------------------------------------------

[[noreturn]] void fail(const std::string &msg) {
  throw std::runtime_error("sahne hatasi: " + msg);
}

// Metindeki tüm sayıları sırayla okur: "1.0 -1.0  2.5" -> {1, -1, 2.5}
std::vector<double> readNumbers(const char *text, const std::string &where) {
  std::istringstream in(text);
  std::vector<double> out;
  double x;
  while (in >> x)
    out.push_back(x);
  if (!in.eof()) // sayı olmayan bir şeye takıldıysa
    fail("<" + where + "> icinde sayi olmayan deger var");
  return out;
}

// Tam olarak n sayı bekler.
std::vector<double> readExactly(const char *text, size_t n,
                                const std::string &where) {
  std::vector<double> nums = readNumbers(text, where);
  if (nums.size() != n)
    fail("<" + where + "> " + std::to_string(n) + " sayi bekliyordu, " +
         std::to_string(nums.size()) + " buldu");
  return nums;
}

pugi::xml_node requireChild(pugi::xml_node parent, const char *name) {
  pugi::xml_node n = parent.child(name);
  if (!n)
    fail(std::string("<") + parent.name() + "> icinde <" + name +
         "> eksik");
  return n;
}

Vec3 readVec3(pugi::xml_node node, const std::string &where) {
  std::vector<double> v = readExactly(node.child_value(), 3, where);
  return Vec3(v[0], v[1], v[2]);
}

Vec3 childVec3(pugi::xml_node parent, const char *name) {
  return readVec3(requireChild(parent, name), name);
}

double childDouble(pugi::xml_node parent, const char *name) {
  return readExactly(requireChild(parent, name).child_value(), 1, name)[0];
}

int requireId(pugi::xml_node node) {
  pugi::xml_attribute a = node.attribute("id");
  if (!a)
    fail(std::string("<") + node.name() + "> etiketinde id yok");
  return a.as_int();
}

// Düz bir sayı listesini k'lı gruplara böler (vertexdata: 3'lü, texturedata: 2'li).
std::vector<double> readGrouped(pugi::xml_node node, size_t k,
                                const char *name) {
  std::vector<double> nums = readNumbers(node.child_value(), name);
  if (nums.size() % k != 0)
    fail(std::string("<") + name + "> eleman sayisi " + std::to_string(k) +
         "'nin kati degil");
  return nums;
}

// "v/vt/vn" tokenını okur. "v", "v/vt", "v//vn" biçimleri de kabul edilir.
// XML 1-indeksli -> burada 1 çıkarıp 0-indeksliye çeviriyoruz.
// Eksik alan -1 olur.
void parseFaceToken(const std::string &tok, int &v, int &vt, int &vn) {
  int parts[3] = {0, 0, 0};
  int field = 0;
  std::string cur;
  for (size_t i = 0; i <= tok.size(); ++i) {
    if (i == tok.size() || tok[i] == '/') {
      if (field > 2)
        fail("yuz tokeni cok fazla '/' iceriyor: " + tok);
      if (!cur.empty()) {
        try {
          parts[field] = std::stoi(cur);
        } catch (...) {
          fail("yuz tokeni okunamadi: " + tok);
        }
      }
      cur.clear();
      ++field;
    } else {
      cur += tok[i];
    }
  }
  if (parts[0] <= 0)
    fail("yuz tokeninde vertex indeksi yok veya 1'den kucuk: " + tok);
  v = parts[0] - 1;
  vt = parts[1] - 1; // verilmemişse 0 - 1 = -1
  vn = parts[2] - 1;
}

void checkIndex(int idx, size_t size, const char *what, int meshId) {
  if (idx < -1 || idx >= static_cast<int>(size))
    fail("mesh " + std::to_string(meshId) + ": " + what + " indeksi " +
         std::to_string(idx + 1) + " gecersiz (toplam " +
         std::to_string(size) + ")");
}

// XML'deki id -> vektör indeksi
int lookup(const std::unordered_map<int, int> &ids, int id,
           const std::string &what) {
  auto it = ids.find(id);
  if (it == ids.end())
    fail(what + " id=" + std::to_string(id) + " bulunamadi");
  return it->second;
}

} // namespace

// --- Asıl parser --------------------------------------------------------------

Scene loadScene(const std::string &path) {
  pugi::xml_document doc;
  pugi::xml_parse_result res = doc.load_file(path.c_str());
  if (!res)
    fail("'" + path + "' acilamadi: " + res.description());

  pugi::xml_node root = doc.child("scene");
  if (!root)
    fail("<scene> kok etiketi yok");

  Scene s;

  // --- Genel ---
  if (pugi::xml_node n = root.child("maxraytracedepth"))
    s.maxRayTraceDepth =
        static_cast<int>(readExactly(n.child_value(), 1, "maxraytracedepth")[0]);
  if (pugi::xml_node n = root.child("background"))
    s.background = readVec3(n, "background");

  // --- Kamera ---
  {
    pugi::xml_node cam = requireChild(root, "camera");
    s.camera.position = childVec3(cam, "position");
    s.camera.gaze = childVec3(cam, "gaze");
    s.camera.up = childVec3(cam, "up");

    // DİKKAT: XML sırası left right BOTTOM TOP (slaytlardaki l r t b değil!)
    std::vector<double> np =
        readExactly(requireChild(cam, "nearplane").child_value(), 4, "nearplane");
    s.camera.l = np[0];
    s.camera.r = np[1];
    s.camera.b = np[2];
    s.camera.t = np[3];

    s.camera.nearDistance = childDouble(cam, "neardistance");

    std::vector<double> res2 = readExactly(
        requireChild(cam, "imageresolution").child_value(), 2, "imageresolution");
    s.camera.nx = static_cast<int>(res2[0]);
    s.camera.ny = static_cast<int>(res2[1]);
    if (s.camera.nx <= 0 || s.camera.ny <= 0)
      fail("imageresolution pozitif olmali");
  }

  // --- Işıklar ---
  // PDF'te ışıklar bazen <lights> altında, bazen doğrudan <scene> altında
  // gösteriliyor; ikisini de kabul ediyoruz.
  pugi::xml_node lights = root.child("lights");
  if (!lights)
    lights = root;

  if (pugi::xml_node n = lights.child("ambientlight"))
    s.ambientLight = readVec3(n, "ambientlight");

  for (pugi::xml_node n : lights.children("pointlight")) {
    PointLight pl;
    pl.position = childVec3(n, "position");
    pl.intensity = childVec3(n, "intensity");
    s.pointLights.push_back(pl);
  }

  for (pugi::xml_node n : lights.children("triangularlight")) {
    TriangularLight tl;
    tl.v1 = childVec3(n, "vertex1");
    tl.v2 = childVec3(n, "vertex2");
    tl.v3 = childVec3(n, "vertex3");
    tl.intensity = childVec3(n, "intensity");
    s.triangularLights.push_back(tl);
  }

  // --- Materyaller ---
  std::unordered_map<int, int> materialIds;
  for (pugi::xml_node n : root.child("materials").children("material")) {
    Material m;
    m.ambient = childVec3(n, "ambient");
    m.diffuse = childVec3(n, "diffuse");
    m.specular = childVec3(n, "specular");
    m.phongExponent = childDouble(n, "phongexponent");
    // Bu ikisi verilmemişse 0 kabul ediyoruz (ayna yok, texture yok).
    if (pugi::xml_node mr = n.child("mirrorreflectance"))
      m.mirror = readVec3(mr, "mirrorreflectance");
    if (pugi::xml_node tf = n.child("texturefactor"))
      m.textureFactor = readExactly(tf.child_value(), 1, "texturefactor")[0];

    materialIds[requireId(n)] = static_cast<int>(s.materials.size());
    s.materials.push_back(m);
  }

  // --- Dönüşümler ---
  // id'ler her tür için ayrı: "t1" ile "r1" farklı şeyler.
  std::map<std::pair<char, int>, int> transformIds;
  for (pugi::xml_node n : root.child("transformations").children()) {
    if (n.type() != pugi::node_element)
      continue;
    std::string name = n.name();
    Transformation tr;
    char key;
    if (name == "translation") {
      tr.kind = Transformation::Kind::Translation;
      tr.v = readVec3(n, name);
      key = 't';
    } else if (name == "scaling") {
      tr.kind = Transformation::Kind::Scaling;
      tr.v = readVec3(n, name);
      key = 's';
    } else if (name == "rotation") {
      std::vector<double> r = readExactly(n.child_value(), 4, name);
      tr.kind = Transformation::Kind::Rotation;
      tr.angle = r[0];
      tr.v = Vec3(r[1], r[2], r[3]);
      key = 'r';
    } else {
      fail("bilinmeyen donusum etiketi <" + name + ">");
    }
    transformIds[{key, requireId(n)}] =
        static_cast<int>(s.transformations.size());
    s.transformations.push_back(tr);
  }

  // --- Geometri verisi ---
  if (pugi::xml_node n = root.child("vertexdata")) {
    std::vector<double> d = readGrouped(n, 3, "vertexdata");
    for (size_t i = 0; i < d.size(); i += 3)
      s.vertices.emplace_back(d[i], d[i + 1], d[i + 2]);
  }
  if (pugi::xml_node n = root.child("normaldata")) {
    std::vector<double> d = readGrouped(n, 3, "normaldata");
    for (size_t i = 0; i < d.size(); i += 3)
      s.normals.emplace_back(d[i], d[i + 1], d[i + 2]);
  }
  if (pugi::xml_node n = root.child("texturedata")) {
    std::vector<double> d = readGrouped(n, 2, "texturedata");
    for (size_t i = 0; i < d.size(); i += 2)
      s.texCoords.push_back(TexCoord{d[i], d[i + 1]});
  }

  // Texture yolu XML dosyasının klasörüne göre çözülür, programı nereden
  // çalıştırdığına göre değil: "scenes/a.xml" + "tex.png" -> "scenes/tex.png"
  if (pugi::xml_node n = root.child("textureimage")) {
    std::string name = n.child_value();
    if (!name.empty())
      s.textureImage =
          (std::filesystem::path(path).parent_path() / name).string();
  }

  // --- Nesneler ---
  pugi::xml_node objects = requireChild(root, "objects");

  std::unordered_map<int, int> meshIds;
  for (pugi::xml_node n : objects.children("mesh")) {
    Mesh m;
    m.id = requireId(n);
    m.material = lookup(materialIds,
                        static_cast<int>(childDouble(n, "materialid")),
                        "mesh " + std::to_string(m.id) + ": materyal");

    std::istringstream in(requireChild(n, "faces").child_value());
    std::vector<std::string> tokens;
    std::string tok;
    while (in >> tok)
      tokens.push_back(tok);
    if (tokens.size() % 3 != 0)
      fail("mesh " + std::to_string(m.id) +
           ": yuz tokeni sayisi 3'un kati degil (her ucgen 3 kose)");

    for (size_t i = 0; i < tokens.size(); i += 3) {
      Face f;
      for (int k = 0; k < 3; ++k) {
        parseFaceToken(tokens[i + k], f.v[k], f.vt[k], f.vn[k]);
        checkIndex(f.v[k], s.vertices.size(), "vertex", m.id);
        checkIndex(f.vt[k], s.texCoords.size(), "texture", m.id);
        checkIndex(f.vn[k], s.normals.size(), "normal", m.id);
        if (f.v[k] < 0)
          fail("mesh " + std::to_string(m.id) + ": vertex indeksi eksik");
      }
      m.faces.push_back(f);
    }

    meshIds[m.id] = static_cast<int>(s.meshes.size());
    s.meshes.push_back(std::move(m));
  }

  for (pugi::xml_node n : objects.children("instance")) {
    Instance inst;
    inst.id = requireId(n);
    std::string who = "instance " + std::to_string(inst.id);

    inst.baseMesh = lookup(meshIds,
                           static_cast<int>(childDouble(n, "basemeshid")),
                           who + ": base mesh");

    // Materyal verilmemişse base mesh'inkini kullan.
    if (pugi::xml_node mn = n.child("materialid"))
      inst.material = lookup(
          materialIds,
          static_cast<int>(readExactly(mn.child_value(), 1, "materialid")[0]),
          who + ": materyal");
    else
      inst.material = s.meshes[inst.baseMesh].material;

    // "t1 r1 s1" -> dönüşüm indeksleri (XML'deki sırayla)
    if (pugi::xml_node tn = n.child("transformations")) {
      std::istringstream in(tn.child_value());
      std::string ref;
      while (in >> ref) {
        char kind = ref.empty() ? '?' : ref[0];
        int id = 0;
        if ((kind != 't' && kind != 's' && kind != 'r') ||
            std::sscanf(ref.c_str() + 1, "%d", &id) != 1)
          fail(who + ": gecersiz donusum referansi '" + ref +
               "' (t1, r2, s3 gibi olmali)");
        auto it = transformIds.find({kind, id});
        if (it == transformIds.end())
          fail(who + ": donusum '" + ref + "' tanimli degil");
        inst.transforms.push_back(it->second);
      }
    }

    s.instances.push_back(std::move(inst));
  }

  return s;
}

void printSceneSummary(const Scene &s) {
  size_t faceCount = 0;
  for (const Mesh &m : s.meshes)
    faceCount += m.faces.size();

  const SceneCamera &c = s.camera;
  std::cout << "--- sahne ozeti ---\n"
            << "kamera: pos " << c.position << " gaze " << c.gaze << " up "
            << c.up << "\n"
            << "        l r b t = " << c.l << " " << c.r << " " << c.b << " "
            << c.t << ", d = " << c.nearDistance << ", " << c.nx << "x"
            << c.ny << "\n"
            << "maxraytracedepth: " << s.maxRayTraceDepth
            << ", background: " << s.background << "\n"
            << "isiklar: ambient " << s.ambientLight << ", "
            << s.pointLights.size() << " point, " << s.triangularLights.size()
            << " triangular\n"
            << "materyal: " << s.materials.size()
            << ", donusum: " << s.transformations.size() << "\n"
            << "vertex: " << s.vertices.size()
            << ", normal: " << s.normals.size()
            << ", texcoord: " << s.texCoords.size() << "\n"
            << "mesh: " << s.meshes.size() << " (toplam " << faceCount
            << " ucgen), instance: " << s.instances.size() << "\n"
            << "texture: "
            << (s.textureImage.empty() ? "(yok)" : s.textureImage) << "\n";
}
