
#define SPIRIT_PO_DEBUG
#include <spirit_po/spirit_po.hpp>

#include <iostream>

namespace spirit_po {

void inspect_expression(const std::string & str) {

  using namespace default_plural_forms;

  expr e;

  typedef std::string::const_iterator str_it;
  str_it it = str.begin();
  str_it end = str.end();
  op_grammar<str_it> grammar;

  if (qi::phrase_parse(it, end, grammar, qi::space, e) && it == end) {
    stack_machine m(e);
    m.debug_print_instructions();
    std::cerr << "\n";
  } else {
    std::cerr << ("Plural-Forms expression reader: Could not parse expression, stopped parsing at:\n" + string_iterator_context(str, it));
  }
}

} // end namespace spirit_po

int main (int argc, char *argv[]) {

  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " expr\n\n";
    std::cerr << "  where expr is a plural forms expression\n";
  } else {
    spirit_po::inspect_expression(argv[1]);
  }
}
