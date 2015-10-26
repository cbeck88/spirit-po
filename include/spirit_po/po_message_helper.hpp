#pragma once

#include <boost/fusion/include/define_struct.hpp>
#include <boost/fusion/include/std_pair.hpp>

#include <boost/optional/optional.hpp>
#include <string>
#include <utility>
#include <vector>

namespace spirit_po { typedef std::pair<std::string, std::vector<std::string>> plural_and_strings_type; }

BOOST_FUSION_DEFINE_STRUCT(
 (spirit_po),
 po_message_helper,
 (boost::optional<std::string>, context)
 (std::string, id)
 (spirit_po::plural_and_strings_type, plural_and_strings))

