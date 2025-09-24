#include "epub.h"
#include "html_text.h"
#include "ui.h"
#include <iostream>

int main(int argc, char** argv) {
  if (argc<2) {
    std::cerr << "Usage: saraswati <file.epub>\n";
    return 1;
  }
  try {
    Epub epub(argv[1]);
    auto doc = epub.load();

    std::vector<std::string> pages;
    pages.reserve(doc.spine.size());
    for (auto& si : doc.spine) {
      auto it = doc.objects.find(si.href);
      if (it==doc.objects.end()) continue;
      pages.push_back(html_to_text(it->second));
    }

    return run_ui(pages, argv[1]);
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 2;
  }
}

