//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#define ENABLE_NLS

#ifdef SPIRIT_PO_NOEXCEPT
#define BOOST_NO_EXCEPTIONS
#include <iostream>
#include <exception>
namespace boost {
  void throw_exception(const std::exception & e) {
    std::cerr << "Boost threw an exception: " << e.what() << std::endl;
    std::terminate();
  }
}
#endif


#include <spirit_po.hpp>

extern "C" {
#include "gettext.h"
}

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#include <dirent.h>
#include <boost/version.hpp>

/***
 * Utility functions
 */
void warning_message(const std::string & str) {
  std::cerr << "spirit_po: " << str << std::endl;
}

std::string escape_string(const std::string & str) {
  std::string result;
  for (char c : str) {
    if (c == '\n') {
      result += "\\n";
    } else if (c == '\t') {
      result += "\\t";
    } else {
      result += c;
    }
  }
  return result;
}

typedef std::pair<std::string, boost::optional<std::string>> msg_id_pair;

template <typename hashmap_type>
std::set<msg_id_pair> all_singular_keys(const hashmap_type & hashmap ) {
  std::set<msg_id_pair> result;
  for (const auto & p : hashmap) {
    if (!p.second.is_plural()) {
      result.insert(std::make_pair(p.second.id, p.second.context));
    }
  }
  return result;
}

typedef std::tuple<std::string, std::string, boost::optional<std::string>> msg_id_plural_info;

template <typename hashmap_type>
std::set<msg_id_plural_info> all_plural_keys(const hashmap_type & hashmap ) {
  std::set<msg_id_plural_info> result;
  for (const auto & p : hashmap) {
    if (p.second.is_plural()) {
      result.insert(msg_id_plural_info { p.second.id, p.second.id_plural(), p.second.context});
    }
  }
  return result;
}

/***
 * Find po test cases
 */

std::string mo_path;

void find_mo_path() {
  char buff[120];
  const char * ignore = getcwd(buff, 120);
  (void) ignore;

  mo_path = buff;
  mo_path += "/mo";
  std::cerr << "mo_path = '" << mo_path << "'\n";
}

std::string po_path;

void find_po_path() {
  std::ifstream stream("po_path.txt");
  if (stream) {
    po_path = std::string{std::istreambuf_iterator<char>(stream), {}};
    std::cout << "po_path = '" << po_path << "'\n" << std::endl;
  } else {
    std::cerr << "Could not find po_path.txt... are you running the program from the correct directory?\n";
    abort();
  }
}

std::vector<std::string> list_po_files() {
  std::vector<std::string> result;

  DIR *dp;
  dp = opendir (po_path.c_str());

  if (dp != nullptr) {
	struct dirent * ep;
	while ((ep = readdir(dp))) {
		std::string entry_name = std::string(ep->d_name);
		if (entry_name == "." || entry_name == "..") { continue; }
        if (entry_name.size() < 4) { continue; }
        if (entry_name.substr(entry_name.size() - 3) != ".po") { continue; }
        result.emplace_back(entry_name.substr(0, entry_name.size() - 3));
    }
    (void) closedir (dp);
  } else {
    std::cerr << "Error: Could not open po dir '" << po_path << "'" << std::endl;
  }

  return result;
}

/***
 * Test routines
 */
bool check_result(const std::string & cat_result, const char * libintl_result, const spirit_po::catalog<> & cat, const std::string & msgid, const boost::optional<std::string> & msgctxt = boost::none) {
  bool check = libintl_result && (cat_result == libintl_result);
  if (!check) {
    std::cerr << "Error, mismatch on msgid = \"" << msgid << "\" line = ";

    if (!msgctxt) {
      if (auto line = cat.gettext_line_no(msgid)) {
        std::cerr << line;
      } else {
        std::cerr << "<<<unknown>>>";
      }
    } else {
      if (auto line = cat.pgettext_line_no(*msgctxt, msgid)) {
        std::cerr << line;
      } else {
        std::cerr << "<<<unknown>>>";
      }
    }

    std::cerr << "\n(libintl):\n";
    if (libintl_result) {
      if (libintl_result == msgid.c_str()) {
        std::cerr << "<<<untranslated>>>\n";
      } else {
        std::cerr << '"' << libintl_result << "\"\n";
      }
    } else {
      std::cerr << "<<<null string>>>\n";
    }
    std::cerr << " vs. (spirit-po):\n";
    if (cat_result == msgid) {
      std::cerr << "<<<untranslated>>>\n";
    } else {
      std::cerr << '"' << cat_result << "\"\n";
    }
  }
  return check;
}

bool check_libintl_gettext(const spirit_po::catalog<> & cat, const std::string & msgid, const boost::optional<std::string> & msgctxt = boost::none) {
  std::string cat_result = msgctxt ? cat.pgettext_str(*msgctxt, msgid) : cat.gettext_str(msgid);
  const char * libintl_result = msgctxt ? (pgettext_expr(msgctxt->c_str(), msgid.c_str())) : (gettext(msgid.c_str()));
  return check_result(cat_result, libintl_result, cat, msgid, msgctxt);
}

bool check_libintl_ngettext(const spirit_po::catalog<> & cat, const std::string & msgid, const std::string & msgid_plural, uint plural, const boost::optional<std::string> & msgctxt = boost::none) {
  std::string cat_result = msgctxt ? cat.npgettext_str(*msgctxt, msgid, msgid_plural, plural) : cat.ngettext_str(msgid, msgid_plural, plural);
  const char * libintl_result = msgctxt ? (npgettext_expr(msgctxt->c_str(), msgid.c_str(), msgid_plural.c_str(), plural)) : (ngettext(msgid.c_str(), msgid_plural.c_str(), plural));
  return check_result(cat_result, libintl_result, cat, msgid, msgctxt);
}

enum class RESULT { PASS, FAIL, NA };

RESULT do_test(const std::string & po_stem) {
  // Find po file
  std::ifstream po_stream(po_path + "/" + po_stem + ".po");
  if (!po_stream) {
    std::cerr << "Could not find po file: '" << po_stem << ".po'\n";
    return RESULT::NA;
  }
  // Buffer the content in a string
  std::string po_content{std::istreambuf_iterator<char>(po_stream), std::istreambuf_iterator<char>()};

  std::cerr << ".";
  // Find mo file
  {
    bindtextdomain (po_stem.c_str(), mo_path.c_str());
    const char * result = textdomain (po_stem.c_str());
    if (result != po_stem) {
      std::cerr << "Could not find mo file: expected '" << po_stem << "', found '" << result << "'\n";
      return RESULT::NA;
    }
    bind_textdomain_codeset(po_stem.c_str(), "UTF-8");

    // Check if we can read the po header, if not it means we couldn't find the mo file
    const char * header = gettext("");
    if (!strlen(header)) {
      std::cerr << "Could not find the mo file, (empty po header): '" << po_stem << ".mo'\n";
      return RESULT::NA;
    }
  }

  std::cerr << ".";
  // Read po file
  auto cat = spirit_po::catalog<>::from_range(po_content, std::function<void(const std::string &)>{&warning_message});
#ifdef SPIRIT_PO_NOEXCEPT
  if (!cat) {
    std::cerr << "Could not read po file: '" << po_stem << ".po', error:\n" << cat.error() << std::endl;
    return RESULT::FAIL;
  }
#endif

  std::cerr << ".";

  std::cerr << " (" << cat.size() << " messages) ";

  bool all_pass = true;
  all_pass = all_pass && check_libintl_gettext(cat, "foo");
  all_pass = all_pass && check_libintl_gettext(cat, "bar");
  all_pass = all_pass && check_libintl_gettext(cat, "baz");
  all_pass = all_pass && check_libintl_ngettext(cat, "qaz", "qazi", 1);
  all_pass = all_pass && check_libintl_ngettext(cat, "qaz", "qazi", 2);

  for (const auto & id : all_singular_keys(cat.get_hashmap())) {
    if (id.first.size()) {
      bool b = check_libintl_gettext(cat, id.first, id.second);
      all_pass = all_pass && b;
    }
  }

  for (const auto & tag : all_plural_keys(cat.get_hashmap())) {
    if (std::get<0>(tag).size()) {
      for (uint n = 0; n < 9; ++n) {
        bool b = check_libintl_ngettext(cat, std::get<0>(tag), std::get<1>(tag), n, std::get<2>(tag));
        all_pass = all_pass && b;
      }
    }
  }

  return all_pass ? RESULT::PASS : RESULT::FAIL;
}

/***
 * Main
 */

int main() {
  find_mo_path();
  find_po_path();

  std::cout << "Boost version = " << BOOST_LIB_VERSION << std::endl;
  std::cout << "SPIRIT_PO_NOEXCEPT = ";
#ifdef SPIRIT_PO_NOEXCEPT
  std::cout << "1\n";
#else
  std::cout << "0\n";
#endif

  std::cout << "SPIRIT_PO_DEBUG = ";
#ifdef SPIRIT_PO_DEBUG
  std::cout << "1\n";
#else
  std::cout << "0\n";
#endif

  std::cout << std::endl;

  setlocale (LC_ALL, "");
  // Use system locale
  // The system locale must match the locale selected in CMakeLists.txt, or
  // this program won't be able to find the mo files, and all tests will fail.

  std::cout << "Validating spirit_po vs libintl:\n" << std::endl;

  uint passed = 0;
  uint failed = 0;

  for (const std::string & str : list_po_files()) {
    std::cout << str << "... ";
    int padding = 46 - str.size();
    if (padding < 0) { padding = 0; }
    for (uint i = 0; i < static_cast<uint>(padding); ++i) { std::cout << " "; }

    std::cout.flush();

    auto result = do_test(str);

    if (result == RESULT::PASS) {
      ++passed;
      std::cout << " pass";
    } else if (result == RESULT::FAIL) {
      ++failed;
      std::cout << " FAIL";
    } else {
      std::cout << "   invalid";
    }
    std::cout << std::endl;
  }

  std::cout << "\n" << std::endl;
  if (!failed) { std::cout << "All "; }
  std::cout << passed << " / " << (passed + failed) << " tests passed.\n" << std::endl;

  return failed ? 1 : 0;
}
