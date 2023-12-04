#include "helpers.hpp"

std::string UTF8toISO8859_1(const char *in) {
  std::string out;
  if (in == NULL)
    return out;

  unsigned int codepoint;
  while (*in != 0) {
    unsigned char ch = static_cast<unsigned char>(*in);
    if (ch <= 0x7f)
      codepoint = ch;
    else if (ch <= 0xbf)
      codepoint = (codepoint << 6) | (ch & 0x3f);
    else if (ch <= 0xdf)
      codepoint = ch & 0x1f;
    else if (ch <= 0xef)
      codepoint = ch & 0x0f;
    else
      codepoint = ch & 0x07;
    ++in;
    if (((*in & 0xc0) != 0x80) && (codepoint <= 0x10ffff)) {
      if (codepoint <= 255) {
        out.append(1, static_cast<char>(codepoint));
      } else {
        // do whatever you want for out-of-bounds characters
      }
    }
  }
  return out;
}
