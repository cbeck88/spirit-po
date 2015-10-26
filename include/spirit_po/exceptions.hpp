#pragma once

#include <string>

namespace spirit_po {

// Show the next 80 characters from some iterator position.
// Intended to be used for parser error messages
template <typename Iterator>
std::string iterator_context(Iterator & it, Iterator & end) {
  std::string result;
  uint count = 80;
  while (it != end && count) { result += *it; ++it; --count; }
  return result;
}

// When the thing being parsed is a short string, we can give
// a better context report
std::string string_iterator_context(const std::string & str,
                                    std::string::const_iterator it) {
  std::string result{str};
  result += "\n";

  for (auto temp = str.begin(); temp != it; ++temp) {
    result += ' ';
  }
  result += "^\n";
  return result;
}

} // end namespace spirit_po


#ifdef SPIRIT_PO_NOEXCEPT

#define SPIRIT_PO_CATALOG_FAIL(Message) \
do { \
  error_message_ = (Message); \
  return ; \
} while(0)

#else

#include <stdexcept>

namespace spirit_po {

struct catalog_exception : std::runtime_error {
  catalog_exception(const char * what) : runtime_error(what) {}
  catalog_exception(const std::string & what) : catalog_exception(what.c_str()) {}
};

} // end namespace spirit_po

#define SPIRIT_PO_CATALOG_FAIL(Message) \
do { \
  throw spirit_po::catalog_exception( Message ); \
} while(0)


#endif // SPIRIT_PO_NOEXCEPT
