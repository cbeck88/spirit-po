#pragma once

#include <boost/optional/optional.hpp>
#include <string>
#include <utility>
#include <vector>

namespace spirit_po {

typedef std::pair<std::string, std::vector<std::string>> plural_and_strings_type;

struct po_message {
  boost::optional<std::string> context;
  std::string id;
  plural_and_strings_type plural_and_strings;

  // Get the 'id_plural', 'strings' fields from the pair.
  // It is arranged as a pair here to allow for simpler parsing with spirit attributes.
  std::string & id_plural() { return plural_and_strings.first; }
  const std::string & id_plural() const { return plural_and_strings.first; }

  std::vector<std::string> & strings() { return plural_and_strings.second; }
  const std::vector<std::string> & strings() const { return plural_and_strings.second; }
};

/***
 * Debug printer
 */
#ifdef SPIRIT_PO_DEBUG
std::string debug_string(const po_message & msg) {
  std::string result = "{\n";
  if (msg.context) {
    result += "  context: \"" + *msg.context + "\"\n";
  }
  result += "  id: \"" + msg.id + "\"\n";
  result += "  id_plural: \"" + msg.id_plural() + "\"\n";
  result += "  strings: { ";
  for (uint i = 0; i < msg.strings().size(); ++i) {
    if (i) { result += ", "; }
    result += '"' + msg.strings()[i] + '"';
  }
  result += " }\n";
  result += "}";
  return result;
}
#endif // SPIRIT_PO_DEBUG

} // end namespace spirit_po
