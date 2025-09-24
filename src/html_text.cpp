#include "html_text.h"
#include <string>
#include <unordered_map>
#include <cctype>

static std::string decodeEntities(const std::string& s) {
  static const std::unordered_map<std::string,std::string> ents = {
    {"amp","&"},{"lt","<"},{"gt",">"},{"quot","\""},{"apos","'"},
    {"nbsp"," "}
  };
  std::string out; out.reserve(s.size());
  for (size_t i=0;i<s.size();++i) {
    if (s[i]=='&') {
      size_t j = s.find(';', i+1);
      if (j!=std::string::npos && j-i<=10) {
        auto name = s.substr(i+1, j-i-1);
        if (!name.empty() && name[0]=='#') {
          // numeric
          char32_t cp=0;
          if (name[1]=='x' || name[1]=='X') cp = (char32_t)strtoul(name.c_str()+2,nullptr,16);
          else cp = (char32_t)strtoul(name.c_str()+1,nullptr,10);
          if (cp>0 && cp<128) out.push_back((char)cp);
          else out.push_back('?');
          i = j; continue;
        } else {
          auto it = ents.find(name);
          if (it!=ents.end()) { out += it->second; i=j; continue; }
        }
      }
    }
    out.push_back(s[i]);
  }
  return out;
}

std::string html_to_text(const std::string& html) {
  std::string out;
  out.reserve(html.size());
  bool in_tag=false;
  bool last_space=false;
  auto newline = [&](int n=1){
    while (n-->0) { if (!out.empty() && out.back()!='\n') out.push_back('\n'); }
    last_space=false;
  };

  for (size_t i=0;i<html.size();++i) {
    char c = html[i];
    if (!in_tag) {
      if (c=='<') {
        // lookahead cheaply for block tags
        size_t k = html.find('>', i+1);
        std::string tag = (k!=std::string::npos)? html.substr(i+1, k-i-1) : "";
        for (auto& ch: tag) ch = std::tolower((unsigned char)ch);
        // normalize tag name
        auto spacePos = tag.find_first_of(" \t/");
        std::string t = (spacePos==std::string::npos)? tag : tag.substr(0, spacePos);
        if (t=="br") newline();
        else if (t=="p"||t=="div"||t=="li"||t=="ul"||t=="ol") newline(2);
        else if (t=="h1"||t=="h2"||t=="h3"||t=="h4"||t=="h5"||t=="h6") newline(2);
        else if (t=="hr") { newline(); out.append("――――――"); newline(); }
        in_tag = true;
      } else {
        // text
        if (std::isspace((unsigned char)c)) {
          if (!last_space) { out.push_back(' '); last_space=true; }
        } else {
          out.push_back(c);
          last_space=false;
        }
      }
    } else {
      if (c=='>') in_tag=false;
    }
  }
  return decodeEntities(out);
}

