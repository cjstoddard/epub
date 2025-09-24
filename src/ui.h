#pragma once
#include <string>
#include <vector>

// Simple curses viewer with search. Press:
//  - Up/Down/PgUp/PgDn to scroll
//  - '/' to search forward
//  - 'n' to repeat search
//  - 'q' to quit
// Returns 0 on normal exit.
int run_ui(const std::vector<std::string>& pages, const std::string& title);

