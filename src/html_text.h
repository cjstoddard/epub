#pragma once
#include <string>

// Extremely small HTML→text converter:
// - Collapses whitespace
// - Newlines on <p>, <br>, <li>, <h1..h6>, <div>, <hr>
// - Decodes a few common entities
std::string html_to_text(const std::string& html);

