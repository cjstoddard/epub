#pragma once
#include <string>
#include <vector>
#include <unordered_map>

// Basic EPUB loader: finds the rootfile (OPF), reads manifest & spine,
// and returns spine documents as UTF-8 HTML strings.
struct EpubDoc {
  struct SpineItem { std::string href; std::string mediaType; };
  std::string rootDir;                     // directory of OPF
  std::vector<SpineItem> spine;            // reading order
  std::unordered_map<std::string,std::string> objects; // href->raw file text
};

class Epub {
public:
  explicit Epub(const std::string& path);
  ~Epub();

  // Throws std::runtime_error on failure
  EpubDoc load();

private:
  struct zip* z_ = nullptr;
  static std::string readEntry(struct zip* z, const std::string& name);
  static std::string joinPath(const std::string& dir, const std::string& rel);
};

