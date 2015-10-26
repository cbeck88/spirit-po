#pragma once

#include "po_message_helper.hpp"

#include <boost/fusion/include/define_struct.hpp>

#include <boost/optional/optional.hpp>
#include <string>
#include <vector>

BOOST_FUSION_DEFINE_STRUCT(
 (spirit_po),
 po_message,
 (boost::optional<std::string>, context)
 (std::string, id)
 (std::string, id_plural)
 (std::vector<std::string>, strings))

namespace spirit_po {

/***
 * Function to convert the grammar's helper type to the desired type
 */
inline static po_message convert_from_helper_type(po_message_helper && h) {
  return { std::move(h.context), std::move(h.id), std::move(h.plural_and_strings.first), std::move(h.plural_and_strings.second) };
}

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
  result += "  id_plural: \"" + msg.id_plural + "\"\n";
  result += "  strings: { ";
  for (uint i = 0; i < msg.strings.size(); ++i) {
    if (i) { result += ", "; }
    result += '"' + msg.strings[i] + '"';
  }
  result += " }\n";
  result += "}";
  return result;
}
#endif // SPIRIT_PO_DEBUG

} // end namespace spirit_po
