#pragma once

#ifndef BOOST_SPIRIT_USE_PHOENIX_V3
#define BOOST_SPIRIT_USE_PHOENIX_V3
#endif

#include "catalog_metadata.hpp"
#include "default_plural_forms.hpp"
#include "default_plural_forms_compiler.hpp"
#include "exceptions.hpp"
#include "po_grammar.hpp"
#include "po_message.hpp"

#include <boost/spirit/include/qi.hpp>
#include <functional>
#include <istream>
#include <string>
#include <unordered_map>
#include <vector>

#ifdef SPIRIT_PO_DEBUG
#include <iostream>
#endif

namespace spirit_po {

namespace qi = boost::spirit::qi;
typedef unsigned int uint;

typedef std::function<void(const std::string &)> warning_channel_type;
typedef std::unordered_map<std::string, po_message> default_hashmap_type;

template <typename hashmap_type = default_hashmap_type, typename pf_compiler = default_plural_forms::compiler>
class catalog {
  catalog_metadata metadata_;

  typename pf_compiler::result_type pf_function_object_;
  uint singular_index_; // cached result of pf_function_object(1)

#ifdef SPIRIT_PO_NOEXCEPT
  boost::optional<std::string> error_message_;
  // if loading failed, error_message_ contains an error
  // (rather than throwing an exception)
#endif
  warning_channel_type warning_channel_;

  hashmap_type hashmap_;

public:
  static constexpr unsigned char EOT = static_cast<unsigned char>(4);
  // ASCII 4 is EOT character
  // Used to separate msg context from msgid in the hashmap, in MO files
  // We use the same formatting system, just for simplicity.
  // c.f. https://www.gnu.org/software/gettext/manual/html_node/MO-Files.html

  static std::string form_context_index(const std::string & msgctxt, const std::string & id) {
    return msgctxt + static_cast<char>(EOT) + id;
  }

  static std::string form_index(const po_message & msg) {
    return msg.context ? form_context_index(*msg.context, msg.id) : msg.id;
  }

private:
  /***
   * Helper for interacting with hashmap results
   */
  const std::string & get(const po_message & msg) const {
    return msg.strings[singular_index_];
  }

  const std::string & get(const po_message & msg, uint plural) const {
    uint idx = (plural == 1 ? singular_index_ : pf_function_object_(plural));
    return msg.strings[idx];
  }

  /***
   * Emplace a message into the hashmap
   */
  void insert_message(po_message && msg) {
#ifdef SPIRIT_PO_DEBUG
    std::cerr << "DEBUG: insert_message( " << debug_string(msg) << " )\n";
#endif

    if (!msg.strings.size()) { return; }
    // don't allow messages with ZERO translations into the catalog, this will cause segfaults later.
    // should perhaps throw an exception here

    if (!msg.strings[0].size()) { return; }
    // if the (first) translated string is "", it is untranslated and message does not enter catalog

    std::string index = form_index(msg);
    // adjust the id based on context if necessary

#ifdef SPIRIT_PO_DEBUG
    std::cerr << "DEBUG: inserting message at index: \"" << index << "\"\n";
#endif

    auto result = hashmap_.emplace(std::move(index), std::move(msg));

    // Issue a warning if emplace failed, rather than silently overwrite.
    if (!result.second) {
      if (warning_channel_) {
        std::string warning = "Overwriting a message: msgid = <<<" + msg.id + ">>>";
        if (msg.context) { warning += " msgctxt = <<<" + *msg.context + ">>>"; }
        warning_channel_(warning);
      }
      result.first->second = std::move(msg);
    }
  }

public:
#ifdef SPIRIT_PO_NOEXCEPT
  /***
   * Error checking (this is done so we don't have to throw exceptions from the ctor.
   */
  explicit operator bool() const {
    return !error_message_;
  }

  std::string error() const {
    return *error_message_; // asserts that there is an error message
  }
#endif

  /***
   * Ctors
   */
  template <typename Iterator>
  catalog(Iterator & it, Iterator & end, warning_channel_type warn_channel = warning_channel_type(), pf_compiler compiler = pf_compiler())
    : metadata_()
    , pf_function_object_()
    , warning_channel_(warn_channel)
    , hashmap_()
  {
    po_grammar<Iterator> grammar;

    po_message_helper msg_helper;

    // Parse header first
    {
      // must be able to parse first message
      if (!qi::parse(it, end, grammar, msg_helper)) {
        SPIRIT_PO_CATALOG_FAIL("Failed to parse po header, stopped at :'" + iterator_context(it, end));
      }

      po_message msg = convert_from_helper_type(std::move(msg_helper));

#ifdef SPIRIT_PO_DEBUG
      std::cerr << "PO HEADER MESSAGE: " << debug_string(msg) << std::endl;
#endif

      // first message must have empty MSGID (po format says so)
      if (msg.id.size()) {
        SPIRIT_PO_CATALOG_FAIL("Failed to parse po header, first msgid must be empty string \"\", found: " + msg.id);
      }

      // Now parse the header string itself
      if (msg.strings.size()) {
        std::string maybe_error = metadata_.parse_header(msg.strings[0]);
        if (maybe_error.size()) {
          SPIRIT_PO_CATALOG_FAIL("Failed to parse po header: " + maybe_error);
        }
      }

      if (!metadata_.num_plural_forms) {
        SPIRIT_PO_CATALOG_FAIL("Invalid metadata in po header, found num_plurals = 0");
      }

      // Try to compile the plural forms function string
      pf_function_object_ = compiler(metadata_.plural_forms_function_string);
      if (!pf_function_object_) {
        SPIRIT_PO_CATALOG_FAIL(("Failed to read plural forms function. Input: '" + metadata_.plural_forms_function_string + "', error message: " + pf_function_object_.error()));
      } 

      // Cache the 'singular' form index
      singular_index_ = pf_function_object_(1);
      if (singular_index_ >= metadata_.num_plural_forms) {
        SPIRIT_PO_CATALOG_FAIL(("Invalid plural forms function. On input n = 1, returned plural = " + std::to_string(singular_index_) + ", while num_plurals = " + std::to_string(metadata_.num_plural_forms)));
      }

      insert_message(std::move(msg)); // for compatibility, need to insert the header message at msgid ""
    }

    while (it != end) {
      msg_helper = po_message_helper{};
      msg_helper.plural_and_strings.second.reserve(metadata_.num_plural_forms); // try to prevent frequent vector reallocations
      if (!qi::parse(it, end, grammar, msg_helper)) {
        SPIRIT_PO_CATALOG_FAIL(("Failed to parse po file, stopped at: " + iterator_context(it, end)));
      }
      // cannot overwrite header
      if (!msg_helper.id.size()) {
        SPIRIT_PO_CATALOG_FAIL(("Malformed po file: Cannot overwrite the header entry later in the po file. Stopped at: " + iterator_context(it, end)));
      }
      insert_message(convert_from_helper_type(std::move(msg_helper)));
    }
  }

  // Construct a catalog from a range using one expression
  template <typename Range>
  static catalog from_range(const Range & range, warning_channel_type w = warning_channel_type()) {
    auto it = boost::begin(range);
    auto end = boost::end(range);
    return catalog(it, end, w);
  }

  static catalog from_istream(std::istream & is, warning_channel_type w = warning_channel_type()) {
    boost::spirit::istream_iterator it(is);
    boost::spirit::istream_iterator end;
    return catalog(it, end, w);
  }

  ///////////////
  // ACCESSORS //
  ///////////////

  /***
   * Lookup strings from the catalog
   *
   * When using string literals as the parameters, these versions are safe and
   * are maximally efficient.
   * (The returned pointer is either the input pointer, having static storage
   * duration, or has lifetime as long as the catalog.)
   *
   * Chosen to behave in the same manner as corresponding gettext functions.
   */
  const char * gettext(const char * msgid) const {
    auto it = hashmap_.find(msgid);
    if (it != hashmap_.end()) {
      return get(it->second).c_str();
    } else {
      return msgid;
    }
  }

  const char * ngettext(const char * msgid, const char * msgid_plural, uint plural) const {
    auto it = hashmap_.find(msgid);
    if (it != hashmap_.end()) {
      return get(it->second, plural).c_str();
    } else {
      return (plural == 1 ? msgid : msgid_plural);
    }
  }

  const char * pgettext(const char * context, const char * msgid) const {
    auto it = hashmap_.find(form_context_index(context, msgid));
    if (it != hashmap_.end()) {
      return get(it->second).c_str();
    } else {
      return msgid;
    }
  }

  const char * npgettext(const char * context, const char * msgid, const char * msgid_plural, uint plural) const {
    auto it = hashmap_.find(form_context_index(context, msgid));
    if (it != hashmap_.end()) {
      return get(it->second, plural).c_str();
    } else {
      return (plural == 1 ? msgid : msgid_plural);
    }
  }

  /***
   * Lookup strings from catalog, return std::string.
   *
   * When, for whatever reason, it is more comfortable to use idiomatic C++.
   */
  std::string gettext(const std::string & msgid) const {
    auto it = hashmap_.find(msgid);
    if (it != hashmap_.end()) {
      return get(it->second);
    } else {
      return msgid;
    }
  }

  std::string ngettext(const std::string & msgid, const std::string & msgid_plural, uint plural) const {
    auto it = hashmap_.find(msgid);
    if (it != hashmap_.end()) {
      return get(it->second, plural);
    } else {
      return (plural == 1 ? msgid : msgid_plural);
    }
  }

  std::string pgettext(const std::string & context, const std::string & msgid) const {
    auto it = hashmap_.find(form_context_index(context, msgid));
    if (it != hashmap_.end()) {
      return get(it->second);
    } else {
      return msgid;
    }
  }

  std::string npgettext(const std::string & context, const std::string & msgid, const std::string & msgid_plural, uint plural) const {
    auto it = hashmap_.find(form_context_index(context, msgid));
    if (it != hashmap_.end()) {
      return get(it->second, plural);
    } else {
      return (plural == 1 ? msgid : msgid_plural);
    }
  }

  /***
   * Access metadata
   */
  const catalog_metadata & get_metadata() const { return metadata_; }

  /***
   * Catalog size
   */
  uint size() const { return hashmap_.size(); }

  /***
   * Debugging output
   */
  const hashmap_type & get_hashmap() const { return hashmap_; }

  /***
   * Set warning channel (for msgid overwrites)
   */
  void set_warning_channel(const warning_channel_type & w) { warning_channel_ = w; }

  /***
   * Merge a different catalog into this one
   */
  template <typename H, typename P>
  void merge(catalog<H, P> && other) {
    std::string maybe_error = metadata_.check_compatibility(other.metadata_);
    if (maybe_error.size()) {
      SPIRIT_PO_CATALOG_FAIL(("Cannot merge catalogs: " + maybe_error));
    }
    for (auto & p : other.hashmap_) {
      if (p.first.size()) { // don't copy over the header, keep our original header
        insert_message(std::move(p.second));
      }
    }
  }
};

} // end namespace spirit_po
