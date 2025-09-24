// src/ui.cpp
#include "ui.h"
#include <ncursesw/curses.h>
#include <string>
#include <vector>
#include <algorithm>
#include <cwctype>   // for std::iswspace

static void draw_status(const std::wstring& status) {
  int y,x; getmaxyx(stdscr,y,x);
  mvhline(y-1,0,' ',x);
  mvaddwstr(y-1,0,status.c_str());
}

static void paint(const std::vector<std::wstring>& lines, int topLine) {
  int rows, cols; getmaxyx(stdscr, rows, cols);
  int usable = rows-1;
  for (int r=0; r<usable; ++r) {
    move(r,0);
    clrtoeol();
    int idx = topLine + r;
    if (idx<(int)lines.size()) {
      const auto& L = lines[idx];
      // clip to cols
      for (int c=0;c<std::min<int>(cols,(int)L.size());++c) {
        addnwstr(&L[c], 1);
      }
    }
  }
}

static std::vector<std::wstring> wrapText(const std::wstring& text, int width) {
  std::vector<std::wstring> out;
  size_t i=0, n=text.size();
  while (i<n) {
    if (text[i]==L'\n') { out.emplace_back(); ++i; continue; }
    size_t lineStart=i, lastSpace=i;
    int count=0;
    while (i<n && text[i]!=L'\n' && count<width) {
      if (std::iswspace(static_cast<wint_t>(text[i]))) lastSpace=i;
      ++i; ++count;
    }
    size_t end = (i<n && count==width && lastSpace>lineStart)? lastSpace : i;
    out.emplace_back(text.begin()+lineStart, text.begin()+end);
    i = (end==i)? i : end+1; // skip space if we broke at whitespace
  }
  return out;
}

int run_ui(const std::vector<std::string>& pages, const std::string& title) {
  // join pages with blank line between
  std::string joined;
  for (size_t i=0;i<pages.size();++i) {
    joined += pages[i];
    if (i+1<pages.size()) joined += "\n\n";
  }
  // convert to wide
  std::wstring w;
  w.reserve(joined.size());
  for (unsigned char ch : joined) w.push_back((wchar_t)ch);

  initscr(); cbreak(); noecho(); keypad(stdscr, TRUE);
  int rows, cols; getmaxyx(stdscr, rows, cols);

  auto lines = wrapText(w, cols);
  int top=0; size_t lastFindPos=0; std::string query;

  while (true) {
    paint(lines, top);
    std::wstring status = L" " + std::wstring(title.begin(), title.end())
                        + L"   ↑/↓ PgUp/PgDn  '/' search  n next  q quit";
    draw_status(status);
    int ch = getch();
    if (ch=='q') break;
    else if (ch==KEY_UP) { if (top>0) --top; }
    else if (ch==KEY_DOWN) { if (top+rows-1<(int)lines.size()) ++top; }
    else if (ch==KEY_NPAGE) { top = std::min<int>(top + rows-2, (int)lines.size()-1); }
    else if (ch==KEY_PPAGE) { top = std::max(0, top - (rows-2)); }
    else if (ch=='/') {
      echo(); curs_set(1);
      mvprintw(rows-1,0,"Search: "); char buf[256]={0};
      getnstr(buf, 255);
      noecho(); curs_set(0);
      query = buf; lastFindPos=0;
      std::wstring wq(query.begin(), query.end());
      for (size_t i=0;i<lines.size();++i) {
        if (lines[i].find(wq) != std::wstring::npos) { top = (int)i; break; }
      }
    } else if (ch=='n' && !query.empty()) {
      std::wstring wq(query.begin(), query.end());
      for (size_t i=top+1;i<lines.size();++i) {
        if (lines[i].find(wq) != std::wstring::npos) { top=(int)i; break; }
      }
    } else if (ch==KEY_RESIZE) {
      getmaxyx(stdscr, rows, cols);
      lines = wrapText(w, cols);
      top = std::min<int>(top,(int)lines.size()-1);
    }
  }
  endwin();
  return 0;
}

