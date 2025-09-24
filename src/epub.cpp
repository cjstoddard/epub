#include "epub.h"
#include <zip.h>
#include <tinyxml2.h>
#include <stdexcept>
#include <sstream>
#include <algorithm>

using namespace tinyxml2;

Epub::Epub(const std::string& path) {
  int err = 0;
  z_ = zip_open(path.c_str(), ZIP_RDONLY, &err);
  if (!z_) throw std::runtime_error("Failed to open EPUB (zip): " + path);
}
Epub::~Epub() { if (z_) zip_close(z_); }

std::string Epub::readEntry(zip* z, const std::string& name) {
  struct zip_stat st;
  zip_stat_init(&st);
  if (zip_stat(z, name.c_str(), 0, &st) != 0)
    throw std::runtime_error("Missing entry: " + name);
  zip_file* f = zip_fopen(z, name.c_str(), 0);
  if (!f) throw std::runtime_error("zip_fopen failed: " + name);
  std::string out;
  out.resize(st.size);
  zip_fread(f, out.data(), st.size);
  zip_fclose(f);
  return out;
}

static std::string dirnameOf(const std::string& p) {
  auto pos = p.find_last_of("/\\");
  return (pos==std::string::npos) ? std::string() : p.substr(0,pos+1);
}

std::string Epub::joinPath(const std::string& dir, const std::string& rel) {
  if (rel.empty()) return dir;
  if (!dir.empty() && (rel[0]!='/'))
    return dir + rel;
  return rel;
}

EpubDoc Epub::load() {
  // 1) Find rootfile from META-INF/container.xml
  std::string container = readEntry(z_, "META-INF/container.xml");
  XMLDocument dc;
  if (dc.Parse(container.c_str()) != XML_SUCCESS)
    throw std::runtime_error("container.xml parse failed");
  auto* rootfile = dc.RootElement()
                     ->FirstChildElement("rootfiles")
                     ->FirstChildElement("rootfile");
  if (!rootfile) throw std::runtime_error("No rootfile in container.xml");
  const char* opfPath = rootfile->Attribute("full-path");
  if (!opfPath) throw std::runtime_error("rootfile missing full-path");

  // 2) Parse OPF: manifest + spine
  std::string opfData = readEntry(z_, opfPath);
  XMLDocument dopf;
  if (dopf.Parse(opfData.c_str()) != XML_SUCCESS)
    throw std::runtime_error("OPF parse failed");
  auto* pkg = dopf.RootElement();
  if (!pkg) throw std::runtime_error("Bad OPF");

  std::unordered_map<std::string,std::pair<std::string,std::string>> manifest; // id -> (href, mediaType)
  auto* man = pkg->FirstChildElement("manifest");
  for (auto* it = man ? man->FirstChildElement("item") : nullptr; it; it=it->NextSiblingElement("item")) {
    const char* id = it->Attribute("id");
    const char* href = it->Attribute("href");
    const char* mt = it->Attribute("media-type");
    if (id && href && mt) manifest[id] = {href, mt};
  }

  EpubDoc doc;
  doc.rootDir = dirnameOf(opfPath);

  auto* spine = pkg->FirstChildElement("spine");
  for (auto* ref = spine ? spine->FirstChildElement("itemref") : nullptr; ref; ref=ref->NextSiblingElement("itemref")) {
    const char* idref = ref->Attribute("idref");
    if (!idref) continue;
    auto it = manifest.find(idref);
    if (it==manifest.end()) continue;
    EpubDoc::SpineItem si;
    si.href = joinPath(doc.rootDir, it->second.first);
    si.mediaType = it->second.second;
    doc.spine.push_back(si);
  }

  // 3) Load text-y objects we’ll need (XHTML/HTML)
  for (auto& si : doc.spine) {
    // only attempt to read text/* or application/xhtml+xml
    std::string mt = si.mediaType;
    std::transform(mt.begin(), mt.end(), mt.begin(), ::tolower);
    if (mt.find("text/") == 0 || mt.find("xhtml") != std::string::npos || mt.find("html") != std::string::npos) {
      try {
        doc.objects[si.href] = readEntry(z_, si.href);
      } catch (...) {
        // Non-fatal: keep going
      }
    }
  }

  return doc;
}

