//  (C) Copyright Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#ifndef BOOST_SPIRIT_USE_PHOENIX_V3
#define BOOST_SPIRIT_USE_PHOENIX_V3
#endif

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/fusion/include/define_struct.hpp>

#include <spirit_po/po_message_adapted.hpp>

#include <boost/optional/optional.hpp>
#include <string>
#include <utility>
#include <vector>

namespace spirit_po {

typedef unsigned int uint;
namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

template <typename Iterator>
struct po_grammar : qi::grammar<Iterator, po_message()> {
  qi::rule<Iterator> white_line;
  qi::rule<Iterator> comment_line;
  qi::rule<Iterator> skipped_line;
  qi::rule<Iterator> skipped_block;

  qi::rule<Iterator, char()> escaped_character;
  qi::rule<Iterator, std::string()> single_line_string;
  qi::rule<Iterator, std::string()> multiline_string;

  qi::rule<Iterator, std::string()> message_id;
  qi::rule<Iterator, std::string()> message_id_plural;
  qi::rule<Iterator, std::string()> message_context;
  qi::rule<Iterator, std::string()> message_str;
  qi::rule<Iterator, std::string(uint)> message_str_plural;

  qi::rule<Iterator, std::vector<std::string>()> message_single_str;
  qi::rule<Iterator, std::vector<std::string>(uint)> message_strs;

  qi::rule<Iterator, plural_and_strings_type()> message_singular;
  qi::rule<Iterator, plural_and_strings_type()> message_plural;

  qi::rule<Iterator, po_message()> message;

  po_grammar() : po_grammar::base_type(message) {
    using qi::attr;
    using qi::char_;
    using qi::eoi;
    using qi::lit;
    using qi::omit;
    using qi::uint_;

    white_line = *char_(" \t\r");
    comment_line = char_('#') >> *(char_ - '\n');
    skipped_line = (comment_line | white_line);
    skipped_block = skipped_line >> (eoi | (lit('\n') >> -skipped_block));

    // TODO: Do we need to handle other escaped characters?
    escaped_character = lit('\\') >> (char_("\'\"\\") | (lit('n') >> attr('\n')) | (lit('t') >> attr('\t')));
    single_line_string = lit('"') >> *(escaped_character | (char_ - '\\' - '"')) >> lit('"') >> white_line >> (eoi | (lit('\n') >> -skipped_block));
    multiline_string = single_line_string >> (multiline_string | attr(""));

    message_context = lit("msgctxt ") >> multiline_string;
    message_id = lit("msgid ") >> multiline_string;
    message_str = lit("msgstr ") >> multiline_string;
    message_id_plural = lit("msgid_plural ") >> multiline_string;
    message_str_plural = lit("msgstr[") >> omit[ uint_(qi::_r1) ] >> lit("] ") >> multiline_string;
    //                                           ^ the index in the po file must match what we expect

    // qi::repeat converts it from a std::string, to a singleton vector, as required
    message_single_str = qi::repeat(1)[message_str];
    message_strs = message_str_plural(qi::_r1) >> -message_strs(qi::_r1 + 1);
    //                                                          ^ enforces that indices must count up

    // Detect whether we should read multiple messages or a single message by presence of `msgid_plural`
    message_plural = message_id_plural >> message_strs(0); // first line should be msgstr[0]
    message_singular = attr("") >> message_single_str;
    message = -skipped_block >> -message_context >> message_id  >> (message_plural | message_singular);
  }
};

} // end namespace spirit_po
