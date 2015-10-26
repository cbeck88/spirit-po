#pragma once

#include <boost/fusion/adapted/struct/adapt_struct.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/std_pair.hpp>

#include <boost/optional/optional.hpp>
#include <string>
#include <utility>
#include <vector>

#include "po_message.hpp"

BOOST_FUSION_ADAPT_STRUCT(
  spirit_po::po_message,
  (boost::optional<std::string>, context)
  (std::string, id)
  (spirit_po::plural_and_strings_type, plural_and_strings))
