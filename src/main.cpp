#define SPIRIT_PO_NOEXCEPT
//#define SPIRIT_PO_DEBUG
#include "spirit_po.hpp"

#include <algorithm>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

/***
 * Utility functions
 */
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
 * For debug output
 */
std::ostream & operator<< (std::ostream & ss, const std::vector<std::string> & vec) {
  ss << "{ ";
  for (uint i = 0; i < vec.size(); ++i) {
    if (i) { ss << ", "; }
    ss << '"'
//       << escape_string(vec[i]) 
       << vec[i]
       << '"';
  }
  ss << " }";
  return ss;
}

std::ostream & operator << (std::ostream & ss, const std::set<std::string> & set) {
  std::vector<std::string> temp{set.begin(), set.end()};
  ss << temp;
  return ss;
}

/***
 * Test functions
 */
bool test_default_expr_grammar(const std::string & prog, const std::vector<std::pair<uint, uint>> & vec) {
  spirit_po::default_plural_forms::compiler c;
  auto obj = c(prog);
  if (!obj) {
    std::cerr << "  Failed to compile!\n";
    std::cerr << "  program: '" << prog << "'\n";
    std::cerr << "  error: '" << obj.error() << "'\n";
    return false;
  }
  for (const auto & p : vec) {
    uint output = obj(p.first);
    if (output != p.second) {
      std::cerr << "  Unexpected result!\n";
      std::cerr << "  program:  '" << prog << "'\n";
      std::cerr << "  input:     " << p.first << "\n";
      std::cerr << "  output:    " << output << "\n";
      std::cerr << "  expected:  " << p.second << "\n";
      return false;
    }
  }
  return true;
}

bool test_catalog_gettext(const std::string & po, const std::vector<std::pair<std::string, std::string>> & vec) {
  auto cat = spirit_po::catalog<>::from_range(po);

#ifdef SPIRIT_PO_NOEXCEPT
  if (!static_cast<bool>(cat)) {
    std::cerr << "Failed to load catalog:\n***\n" << po << "\n***\nError:\n" << cat.error() << std::endl;
    return false;
  }
#endif

  for (const auto & p : vec) {
    auto output = cat.gettext(p.first.c_str());
    if (output != p.second) {
      std::cerr << "  Unexpected result!\n";
      std::cerr << "  po:\n***\n" << po << "\n***\n";
      std::cerr << "  input:     " << p.first << "\n";
      std::cerr << "  output:    " << output << "\n";
      std::cerr << "  expected:  " << p.second << "\n";
      return false;
    }
  }
  return true;
}

struct ngettext_test_case {
  std::string input;
  std::string input_plural;
  uint plural;
  std::string expected;
};

bool test_catalog_ngettext(const std::string & po, const std::vector<ngettext_test_case> & vec) {
  auto cat = spirit_po::catalog<>::from_range(po);

#ifdef SPIRIT_PO_NOEXCEPT
  if (!cat) {
    std::cerr << "Failed to load catalog:\n***\n" << po << "\n***\nError:\n" << cat.error() << std::endl;
    return false;
  }
#endif

  for (const auto & p : vec) {
    auto output = cat.ngettext(p.input.c_str(), p.input_plural.c_str(), p.plural);
    if (output != p.expected) {
      std::cerr << "  Unexpected result!\n";
      std::cerr << "  po:\n***\n" << po << "\n***\n";
      std::cerr << "  input:        " << p.input << "\n";
      std::cerr << "  msgid_plural: " << p.input << "\n";
      std::cerr << "        plural= " << p.plural << "\n";
      std::cerr << "  output:       " << output << "\n";
      std::cerr << "  expected:     " << p.expected << "\n";
      return false;
    }
  }
  return true;
}

struct pgettext_test_case {
  std::string context;
  std::string input;
  std::string expected;
};

bool test_catalog_pgettext(const std::string & po, const std::vector<pgettext_test_case> & vec) {
  auto cat = spirit_po::catalog<>::from_range(po);

#ifdef SPIRIT_PO_NOEXCEPT
  if (!cat) {
    std::cerr << "Failed to load catalog:\n***\n" << po << "\n***\nError:\n" << cat.error() << std::endl;
    return false;
  }
#endif

  for (const auto & p : vec) {
    auto output = cat.pgettext(p.context.c_str(), p.input.c_str());
    if (output != p.expected) {
      std::cerr << "  Unexpected result!\n";
      std::cerr << "  po:\n***\n" << po << "\n***\n";
      std::cerr << "  context:      " << p.context << "\n";
      std::cerr << "  input:        " << p.input << "\n";
      std::cerr << "  output:       " << output << "\n";
      std::cerr << "  expected:     " << p.expected << "\n";
      return false;
    }
  }
  return true;
}

struct npgettext_test_case {
  std::string context;
  std::string input;
  std::string input_plural;
  uint plural;
  std::string expected;
};

bool test_catalog_npgettext(const std::string & po, const std::vector<npgettext_test_case> & vec) {
  auto cat = spirit_po::catalog<>::from_range(po);

#ifdef SPIRIT_PO_NOEXCEPT
  if (!cat) {
    std::cerr << "Failed to load catalog:\n***\n" << po << "\n***\nError:\n" << cat.error() << std::endl;
    return false;
  }
#endif

  for (const auto & p : vec) {
    auto output = cat.npgettext(p.context.c_str(), p.input.c_str(), p.input_plural.c_str(), p.plural);
    if (output != p.expected) {
      std::cerr << "  Unexpected result!\n";
      std::cerr << "  po:\n***\n" << po << "\n***\n";
      std::cerr << "  context:      " << p.context << "\n";
      std::cerr << "  input:        " << p.input << "\n";
      std::cerr << "  input_plural: " << p.input_plural << "\n";
      std::cerr << "  plural:       " << p.plural << "\n";
      std::cerr << "  output:       " << output << "\n";
      std::cerr << "  expected:     " << p.expected << "\n";
      return false;
    }
  }
  return true;
}

/***
 * Test macros
 */

uint test_count = 0;
bool any_failed = false;
bool verbose = false;

void test_condition(bool x, int line) {
  if (x) {
    ++test_count;
    if (verbose) {
      std::cout << "Test " << test_count << " passed.\n";
    }
  } else {
    std::cerr << "TEST " << ++test_count << " FAILED! [" __FILE__ << ":" << line << "]\n";
    any_failed = true;
  }
}

#define TEST(X) \
do { \
  test_condition(X, __LINE__); \
} while(0)

template <typename T, typename U>
void check_eq(const T & a, const U & b, const char * a_str, const char * b_str, int line) {
  bool _check_eq = ((a) == (b));
  if (!_check_eq) {
    std::cerr << "Expected: (a) == (b)\n";
    std::cerr << "       a:" << a_str << "\n";
    std::cerr << "       b:" << b_str << "\n";
    std::cerr << "Found: a=" << (a) << "\n";
    std::cerr << "       b=" << (b) << "\n";
  }
  test_condition(_check_eq, line);
}

#define CHECK_EQ(a, b) \
do { \
  check_eq(a, b, #a, #b, __LINE__); \
} while(0)

#define CHECK_EQ_L(a,b, L) \
do { \
  check_eq(a, b, #a, #b, L); \
} while(0)

#define CHECK_PARSE( GRAM, STR ) \
do { \
  std::string test_string = (STR); \
  auto it = test_string.begin(); \
  auto end = test_string.end(); \
\
  spirit_po::po_grammar<decltype(it)> grammar; \
\
  bool _check_parse = qi::parse(it, end, GRAM); \
  if (!_check_parse) { \
    std::cerr << "When testing po grammar terminal '" << #GRAM << "'\n"; \
    std::cerr << "Stopped parsing at:\n"; \
    std::cerr << spirit_po::string_iterator_context(test_string, it) << std::endl; \
  } \
  TEST(_check_parse); \
} while(0)

#define CHECK_NOT_PARSE( GRAM, STR ) \
do { \
  std::string test_string = (STR); \
  auto it = test_string.begin(); \
  auto end = test_string.end(); \
\
  spirit_po::po_grammar<decltype(it)> grammar; \
\
  TEST(!qi::parse(it, end, GRAM)); \
} while(0)

#define CHECK_PARSE_STRING( GRAM, STR, EXPECTED ) \
do { \
  std::string _test_string = (STR); \
  auto _it = _test_string.begin(); \
  auto _end = _test_string.end(); \
\
  std::string _result_string; \
\
  spirit_po::po_grammar<decltype(_it)> grammar; \
\
  bool _check_parse = qi::parse(_it, _end, GRAM, _result_string); \
  if (!_check_parse) { \
    std::cerr << "When testing po grammar terminal '" << #GRAM << "'\n"; \
    std::cerr << "Stopped parsing at:\n"; \
    std::cerr << spirit_po::string_iterator_context(_test_string, _it) << std::endl; \
  } \
\
  TEST(_check_parse); \
  CHECK_EQ( _result_string, EXPECTED ); \
} while(0)

void check_catalog_keys(const std::string & po, const std::set<std::string> & expected, int line) {
  auto cat = spirit_po::catalog<>::from_range( po );
  CHECK_EQ_L(cat.size(), expected.size(), line);
  auto _result = all_keys(cat.get_hashmap());
  if (_result != expected) {
    std::cout << "Difference of result & expected:\n";
    std::vector<std::string> _diff;
    std::set_symmetric_difference(_result.begin(), _result.end(), expected.begin(), expected.end(), std::back_inserter(_diff));
    std::cout << _diff << std::endl;
  }
  CHECK_EQ_L(_result, expected, line);
}

void check_not_parse(const std::string & test_string, int line) {
  try {
    auto cat = spirit_po::catalog<>::from_range(test_string);
#ifdef SPIRIT_PO_NOEXCEPT
    test_condition(!cat, line);
#else
    test_condition(false, line);
#endif
  } catch(...) {}
}

int main() {
  std::cout << "Testing plural forms expression grammar..." << std::endl;
  TEST(test_default_expr_grammar("n != 1", { {0, 1}, {1, 0}, {2, 1}, {3, 1}, {4, 1}}));
  TEST(test_default_expr_grammar("n == 1", { {0, 0}, {1, 1}, {2, 0}, {3, 0}, {4, 0}}));
  TEST(test_default_expr_grammar("n < 5",  { {0, 1}, {1, 1}, {2, 1}, {3, 1}, {4, 1}, {5, 0}, {6, 0}, {7, 0}, {8, 0}, {9, 0}, {10, 0}})); 
  TEST(test_default_expr_grammar("n <= 5", { {0, 1}, {1, 1}, {2, 1}, {3, 1}, {4, 1}, {5, 1}, {6, 0}, {7, 0}, {8, 0}, {9, 0}, {10, 0}})); 
  TEST(test_default_expr_grammar("n > 5",  { {0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}, {6, 1}, {7, 1}, {8, 1}, {9, 1}, {10, 1}})); 
  TEST(test_default_expr_grammar("n >= 5", { {0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 1}, {6, 1}, {7, 1}, {8, 1}, {9, 1}, {10, 1}})); 
  TEST(test_default_expr_grammar("n == 1 ? 0 : (n == 2 ? 1 : 2)", { { 0, 2}, { 1, 0} , {2, 1}, {3, 2}, {4, 2}, {5, 2} }));
  TEST(test_default_expr_grammar("n % 5 == 0", { {0, 1}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 1}, {6, 0}, {7, 0}, {8, 0}, {9, 0}, {10, 1}, {11, 0}}));
  TEST(test_default_expr_grammar("n % 2 || n % 3", { {0, 0}, {1, 1}, {2, 1}, {3, 1}, {4, 1}, {5, 1}, {6, 0}, {7, 1}, {8, 1}, {9, 1}, {10, 1}, {11, 1}, {12, 0}}));
  TEST(test_default_expr_grammar("n % 2 && n % 3", { {0, 0}, {1, 1}, {2, 0}, {3, 0}, {4, 0}, {5, 1}, {6, 0}, {7, 1}, {8, 0}, {9, 0}, {10, 0}, {11, 1}, {12, 0}})); 
  TEST(test_default_expr_grammar("n % 2 || n < 5", { {0, 1}, {1, 1}, {2, 1}, {3, 1}, {4, 1}, {5, 1}, {6, 0}, {7, 1}, {8, 0}, {9, 1}, {10, 0}, {11, 1}, {12, 0}}));
  TEST(test_default_expr_grammar("(n % 6 == 2) || (n % 6 == 1) || (n % 5 == 1)",
                                 {{0, 0}, {1, 1}, {2, 1}, {3, 0}, {4, 0}, {5, 0}, {6, 1}, {7, 1}, {8, 1}, {9, 0}, {10, 0}, {11, 1}, {12, 0}, {13, 1}, {14, 1}}));
  TEST(test_default_expr_grammar("(n % 6 == 2) || (n % 6 == 1) && (n % 5 == 1)",
                                 {{0, 0}, {1, 1}, {2, 1}, {3, 0}, {4, 0}, {5, 0}, {6, 0}, {7, 0}, {8, 1}, {9, 0}, {10, 0}, {11, 0}, {12, 0}, {13, 0}, {14, 1}, {31, 1}}));
  TEST(test_default_expr_grammar("(n % 6 == 2) && (n % 6 == 1) || (n % 5 == 1)",
                                 {{0, 0}, {1, 1}, {2, 0}, {3, 0}, {4, 0}, {5, 0}, {6, 1}, {7, 0}, {8, 0}, {9, 0}, {10, 0}, {11, 1}, {12, 0}, {13, 0}, {14, 0}}));
  TEST(test_default_expr_grammar("!(n % 6 == 2) && (n % 6 == 1) || (n % 5 == 1)",
                                 {{0, 0}, {1, 1}, {2, 0}, {3, 0}, {4, 0}, {5, 0}, {6, 1}, {7, 1}, {8, 0}, {9, 0}, {10, 0}, {11, 1}, {12, 0}, {13, 1}, {14, 0}}));
  TEST(test_default_expr_grammar("!(n % 6 == 2) && !(n % 6 == 1) || (n % 5 == 1)",
                                 {{0, 1}, {1, 1}, {2, 0}, {3, 1}, {4, 1}, {5, 1}, {6, 1}, {7, 0}, {8, 0}, {9, 1}, {10, 1}, {11, 1}, {12, 1}, {13, 0}, {14, 0}}));

  // Test parts of po grammar
  std::cout << "Testing po grammar elements..." << std::endl;
  {
    CHECK_PARSE( grammar.skipped_line, "       \n");
    CHECK_PARSE( grammar.skipped_line, " \t \t    \n");
    CHECK_PARSE( grammar.comment_line, "# asdfasdfa    \n");
    CHECK_PARSE( grammar.skipped_line, "# asdfasdfa    \n");
    CHECK_PARSE( grammar.skipped_line, "#msgid \"\"       \n");
    CHECK_PARSE( grammar.single_line_string, std::string{"\"\""} + "\n");
    CHECK_PARSE( grammar.message_id, std::string{R"(msgid "")"} + "\n");
    CHECK_PARSE( grammar.message_id, std::string{R"(msgid "foo")"} + "\n");
    CHECK_PARSE( grammar.message_context, std::string{R"(msgctxt "")"} + "\n");
    CHECK_PARSE( grammar.message_str,
      std::string{R"(msgstr "something someting")"} + "\n"
    + std::string{R"("etc. etc.")"} + "\n"
    + std::string{R"("and so on")"} + "\n"
    );
    CHECK_NOT_PARSE ( grammar.message_id, std::string{"msgd \"\"\n"} );
    CHECK_NOT_PARSE ( grammar.message_id, std::string{"msgdi \"asdf\"\n"} );

    CHECK_PARSE( grammar.single_line_string, std::string{"\"\""} );
    CHECK_NOT_PARSE( grammar.single_line_string, std::string{"\""} );
    CHECK_PARSE( grammar.single_line_string, std::string{"\"asdf\""} );
    CHECK_NOT_PARSE( grammar.single_line_string, std::string{"\"asdf"} );
    CHECK_PARSE( grammar.single_line_string, std::string{"\"asdf\"\n"} );
    CHECK_NOT_PARSE( grammar.single_line_string, std::string{"\"asdf\n"} );

    CHECK_PARSE( grammar.multiline_string, std::string{"\"\""} );
    CHECK_NOT_PARSE( grammar.multiline_string, std::string{"\""} );
    CHECK_PARSE( grammar.multiline_string, std::string{"\"asdf\""} );
    CHECK_NOT_PARSE( grammar.multiline_string, std::string{"\"asdf"} );
    CHECK_PARSE( grammar.multiline_string, std::string{"\"asdf\"\n"} );
    CHECK_NOT_PARSE( grammar.multiline_string, std::string{"\"asdf\n"} );

    CHECK_NOT_PARSE ( grammar.message_id, std::string{"msgid \"asdf\n"} );
    CHECK_NOT_PARSE ( grammar.message_id, std::string{"msgid \"asdf"} );
    CHECK_NOT_PARSE ( grammar.message_str, std::string{"msgstr \"asdf"} );
    CHECK_PARSE( grammar.message_id >> grammar.message_str,
      std::string{R"(msgid "")"} + "\n"
    + std::string{R"(msgstr "")"} + "\n"
    );
    CHECK_PARSE( -grammar.message_context >> grammar.message_id >> grammar.message_str,
      std::string{R"(msgid "")"} + "\n"
    + std::string{R"(msgstr "")"} + "\n"
    );
    CHECK_PARSE( grammar.message,
      std::string{R"(msgid "")"} + "\n"
    + std::string{R"(msgstr "")"} + "\n"
    );
    CHECK_PARSE_STRING( grammar.single_line_string, std::string{"\"asdf\""} + "\n" , "asdf");

    CHECK_PARSE_STRING( grammar.skipped_block >> grammar.message_id, R"===(#foo
#bar
#baz
msgid "asdf"
)===",
    "asdf");

    CHECK_PARSE_STRING( grammar.message_str,
      std::string{R"(msgstr "something someting")"} + "\n"
    + std::string{R"("etc. etc.")"} + "\n"
    + std::string{R"("and so on")"} + "\n",
      "something sometingetc. etc.and so on"
    );
  }

  std::cout << "Testing po grammar..." << std::endl;
  {
    std::string s1 =
R"===(msgid "asdf"
msgstr "jkl;"
)===";

    auto it = s1.begin();
    auto end = s1.end();
    spirit_po::po_grammar<decltype(it)> grammar;
    spirit_po::po_message_helper msg_h;

    msg_h = {};
    CHECK_EQ(true, qi::parse(it, end, grammar.message, msg_h));
    spirit_po::po_message msg = spirit_po::convert_from_helper_type(std::move(msg_h));
//    std::cerr << "msg = " << debug_string(msg) << std::endl;
    CHECK_EQ(msg.id, "asdf");
    CHECK_EQ(msg.strings.size(), 1);
    CHECK_EQ(msg.strings[0], "jkl;");
  }

  {
    std::string s2 =
R"===(#foo
#bar
#baz
msgid "asdf"
msgstr "jkl;"
)===";

    auto it = s2.begin();
    auto end = s2.end();
    spirit_po::po_grammar<decltype(it)> grammar;
    spirit_po::po_message_helper msg_h;

    CHECK_EQ(true, qi::parse(it, end, grammar, msg_h));
    spirit_po::po_message msg = spirit_po::convert_from_helper_type(std::move(msg_h));
//    std::cerr << "msg = " << debug_string(msg) << std::endl;
    CHECK_EQ(msg.id, "asdf");
    CHECK_EQ(msg.strings.size(), 1);
    CHECK_EQ(msg.strings[0], "jkl;");
  }

  // Test po header code
  {
    std::string test_header =
"Language: Testlang\n"
"Language-Team: Test team\n"
"Project-Id-Version: Test 0.0\n"
"Plural-Forms: nplurals=3; plural=n%10==1 && n%100!=11 ? 0 : n%10>=2 && n"
"%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2;\n";

    // Test parse_header function
    std::cout << "Testing po header handling..." << std::endl;
    {
      spirit_po::catalog_metadata m;
      std::string maybe_err = m.parse_header(test_header);
      if (maybe_err.size()) { std::cerr << maybe_err << "\n"; }

      CHECK_EQ(m.language, "Testlang");
      CHECK_EQ(m.language_team, "Test team");
      CHECK_EQ(m.project_id, "Test 0.0");
      CHECK_EQ(m.last_translator, "");
      CHECK_EQ(m.num_plural_forms, 3);
      CHECK_EQ(m.plural_forms_function_string, "n%10==1 && n%100!=11 ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2;");
    }

    // Test loading a header-only po file
    std::cout << "Testing po parser header step..." << std::endl;
    {
      // escape some characters...
      std::string escaped;
      for (const char c : test_header) {
        if (c == '\n') {
          escaped += "\\n\"\n\"";
        } else {
          escaped += c;
        }
      }

      std::string test_po =
"msgid \"\"\n"
"msgstr \"" + escaped + "\"\n";

      TEST(test_catalog_gettext( test_po, {} ));
    }
  }

  std::cout << "Testing test1.po..." << std::endl;
  {
    std::string test1po =
#include "test1.po"
;
    check_catalog_keys( test1po, { "", "asdf", "foo", "tmnt", "a man\na plan\na canal" }, __LINE__ );
    TEST(test_catalog_gettext( test1po, {{"asdf", "jkl;"}, { "foo", "bar" }, {"tmnt", "teenagemutantninjaturtles"}, {"a man\na plan\na canal", "panama"}}));
  }

  std::cout << "Testing test2.po..." << std::endl;
  {
    std::string test2po =
#include "test2.po"
;
    check_catalog_keys( test2po, { "", "he said \"she said.\"", "say what?" }, __LINE__ );
    TEST(test_catalog_gettext( test2po, {{"he said \"she said.\"", "by the \"sea shore\"?"}, {"say what?", "come again?"}}));
  }

  std::cout << "Testing test3.po..." << std::endl;
  {
    std::string test3po =
#include "test3.po"
;
    check_catalog_keys( test3po, { "", "veni vidi vici", "the sound of a tree falls" }, __LINE__ );
    TEST(test_catalog_gettext( test3po, {{"veni vidi vici", "i came, i saw, i conquered"}, {"a tree falls", "a tree falls"}, {"the sound of a tree falls", "\t"}}));
  }

  std::cout << "Testing test4.po..." << std::endl;
  {
    std::string test4po =
#include "test4.po"
;
    check_catalog_keys( test4po, { "", "note", "goat" }, __LINE__ );
    TEST(test_catalog_ngettext( test4po, {{"note", "notes", 1, "nota"}, {"note", "notes", 2, "notas"}, {"note", "notes", 0, "notas"}, {"goat", "goats", 1, "cabra"}, {"goat", "goats", 2, "cabras"}, {"photo", "photos", 1, "photo"}, {"photo", "photos", 2, "photos"}}));
  }

  std::cout << "Testing test5.po..." << std::endl;
  {
    std::string test5po =
#include "test5.po"
;
    CHECK_EQ(4, spirit_po::catalog<>::from_range(test5po).size());
    TEST(test_catalog_gettext( test5po, {{"note", "nota"}, {"goat", "cabro"}}));
    TEST(test_catalog_ngettext( test5po, {{"note", "notes", 1, "nota"}, {"note", "notes", 2, "notas"}, {"goat", "goats", 1, "cabro"}, {"goat", "goats", 2, "cabros"}}));
    TEST(test_catalog_pgettext( test5po, {{"female", "goat", "cabra"}, {"female", "note", "note"}, {"nice", "photo", "photo"}}));
    TEST(test_catalog_npgettext( test5po, {{"female", "note", "notes", 1, "note"}, {"female", "note", "notes", 2, "notes"}, {"female", "goat", "goats", 0, "cabras"}, {"female", "goat", "goats", 1, "cabra"}, {"female", "goat", "goats", 2, "cabras"}, {"male", "goat", "goats", 1, "goat"}, {"male", "goat", "goats", 2, "goats"}}));
  }


  std::cout << "Testing test_fail1.po..." << std::endl;
  {
    std::string test_fail1po =
#include "test_fail1.po"
;
    check_not_parse(test_fail1po, __LINE__);
  }

  std::cout << "Testing test_fail2.po..." << std::endl;
  {
    std::string test_fail2po =
#include "test_fail2.po"
;
    check_not_parse(test_fail2po, __LINE__);
  }

  std::cout << "Testing test_fail3.po..." << std::endl;
  {
    std::string test_fail3po =
#include "test_fail3.po"
;
    check_not_parse(test_fail3po, __LINE__);
  }

  std::cout << "Testing test_fail4.po..." << std::endl;
  {
    std::string test_fail4po =
#include "test_fail4.po"
;
    check_not_parse(test_fail4po, __LINE__);
  }

  std::cout << "Testing test_fail5.po..." << std::endl;
  {
    std::string test_fail5po =
#include "test_fail5.po"
;
    check_not_parse(test_fail5po, __LINE__);
  }

  std::cout << "Testing test_fail6.po..." << std::endl;
  {
    std::string test_fail6po =
#include "test_fail6.po"
;
    check_not_parse(test_fail6po, __LINE__);
  }

  if (!any_failed) {
    std::cout << "All tests passed." << std::endl;
  }
}
