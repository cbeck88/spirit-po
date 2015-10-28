//  (C) Copyright Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <libintl.h>
#include <spirit_po.hpp>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#include <dirent.h>

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

template <typename hashmap_type>
std::set<std::string> all_keys(const hashmap_type & hashmap ) {
  std::set<std::string> result;
  for (const auto & p : hashmap) {
    result.insert(p.first);
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

bool check_libintl_gettext(const spirit_po::catalog<> & cat, const std::string & msgid) {
  std::string cat_result = cat.gettext_str(msgid);
  const char * libintl_result = gettext(msgid.c_str());
  bool check = libintl_result && (cat_result == libintl_result);
  if (!check) {
    std::cerr << "Error, mismatch on msgid = \"" << msgid << "\" (libintl):\n";
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

enum class RESULT { PASS, FAIL, NA };

RESULT do_test(const std::string & po_stem) {
  // Find po file
  std::ifstream po_stream(po_path + "/" + po_stem + ".po");
  if (!po_stream) {
    std::cerr << "Could not find po file: '" << po_stem << ".po'\n";
    return RESULT::NA;
  }
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

  // Read po file
  //auto cat = spirit_po::catalog<>::from_istream(po_stream);
  auto temp = std::string{std::istreambuf_iterator<char>(po_stream), {}};
  auto cat = spirit_po::catalog<>::from_range(temp, std::function<void(const std::string &)>{&warning_message});
#ifdef SPIRIT_PO_NOEXCEPT
  if (!cat) {
    std::cerr << "When reading po:\n***\n" << temp << "\n***\n";
    std::cerr << "Could not read po file: '" << po_stem << ".po', error:\n" << cat.error() << std::endl;
    return RESULT::FAIL;
  }
#endif

  bool all_pass = true;
  all_pass = all_pass && check_libintl_gettext(cat, "foo");
  all_pass = all_pass && check_libintl_gettext(cat, "bar");
  all_pass = all_pass && check_libintl_gettext(cat, "baz");

  for (const auto & id : all_keys(cat.get_hashmap())) {
    if (id.size()) {
      bool b = check_libintl_gettext(cat, id);
      all_pass = all_pass && b;
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
    auto result = do_test(str);

    std::cout << str << "... ";
    int padding = 50 - str.size();
    if (padding < 0) { padding = 0; }
    for (uint i = 0; i < static_cast<uint>(padding); ++i) { std::cout << " "; }

    if (result == RESULT::PASS) {
      ++passed;
      std::cout << "pass";
    } else if (result == RESULT::FAIL) {
      ++failed;
      std::cout << "FAIL";
    } else {
      std::cout << "  invalid";
    }
    std::cout << std::endl;
  }

  std::cout << "\n" << std::endl;
  if (!failed) { std::cout << "All "; }
  std::cout << passed << " / " << (passed + failed) << " tests passed.\n" << std::endl;
}
