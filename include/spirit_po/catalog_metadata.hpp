#pragma once 

#include "exceptions.hpp"

#ifndef BOOST_SPIRIT_USE_PHOENIX_V3
#define BOOST_SPIRIT_USE_PHOENIX_V3
#endif

#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <string>

namespace qi = boost::spirit::qi;

namespace spirit_po {

typedef unsigned int uint;
typedef std::pair<uint, std::string> num_plurals_info;

struct catalog_metadata {
  std::string project_id;
  std::string language;
  std::string language_team;
  std::string last_translator;

  uint num_plural_forms;
  std::string plural_forms_function_string;

  catalog_metadata()
    : project_id()
    , language()
    , language_team()
    , last_translator()
    , num_plural_forms(0)
    , plural_forms_function_string()
  {}

private:
  std::string find_header_line(const std::string & header, const std::string & label) {
    size_t idx = header.find(label);
    if (idx == std::string::npos) {
      return "";
    }
    auto it = header.begin() + idx + label.size();
    while (it != header.end() && *it == ' ') { ++it; }

    auto e = it;
    while (e != header.end() && *e != '\n') { ++e; }
    return std::string(it, e);
  }

  template <typename Iterator>
  struct num_plurals_grammar : qi::grammar<Iterator, num_plurals_info()> {
    qi::rule<Iterator, num_plurals_info()> main;
    num_plurals_grammar() : num_plurals_grammar::base_type(main) {
      using qi::lit;
      main = qi::skip(' ') [ lit("nplurals=") >> qi::uint_ >> lit(';') >> lit("plural=") ] >> (*qi::char_);
    }
  };

public:
  // nonempty return is an error mesage
  std::string parse_header(const std::string & header) {
    project_id = find_header_line(header, "Project-Id-Version:");
    language = find_header_line(header, "Language:");
    language_team = find_header_line(header, "Language-Team:");
    last_translator = find_header_line(header, "Last-Translator:");

    std::string num_plurals_line = find_header_line(header, "Plural-Forms:");

    if (num_plurals_line.size()) {
      auto it = num_plurals_line.begin();
      auto end = num_plurals_line.end();

      num_plurals_grammar<decltype(it)> gram;
      num_plurals_info info;
      if (qi::parse(it, end, gram, info)) {
        num_plural_forms = info.first;
        plural_forms_function_string = info.second;
      } else {
        num_plural_forms = 0;
        plural_forms_function_string = "";
        return "Failed to parse Plural-Forms entry -- stopped at:\n" + string_iterator_context(num_plurals_line, it);
      }
    } else {
      num_plural_forms = 2;
      plural_forms_function_string = "n != 1";
    }
    return "";
  }

  // check if this metadata is compatible with another metadata (number of plural forms, maybe other criteria)
  // return a nonempty string containing error message if they are not compatible.
  std::string check_compatibility(const catalog_metadata & other) const {
    if (num_plural_forms != other.num_plural_forms) {
      return std::string{"Num plural forms mismatch. this = "} + std::to_string(num_plural_forms) + " other = " + std::to_string(other.num_plural_forms);
    }
    return "";
  }
};

} // end namespace spirit_po
