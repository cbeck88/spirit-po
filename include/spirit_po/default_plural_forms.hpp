#pragma once

/***
 * The namespace default_plural_forms contains all the details to implement
 * the subset of the C grammar used by standard GNU gettext po headers.
 *
 * Boolean expressions return uint 0 or 1.
 *
 * The 'compiler' is a spirit grammar which parses a string into an expression
 * object.
 */

#ifndef BOOST_SPIRIT_USE_PHOENIX_V3
#define BOOST_SPIRIT_USE_PHOENIX_V3
#endif

#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/variant/variant.hpp>
#include <boost/variant/recursive_wrapper.hpp>

namespace spirit_po {

namespace qi = boost::spirit::qi;
typedef unsigned int uint;

namespace default_plural_forms {

struct constant { uint value; };
struct n_var { n_var() = default; n_var(char) {}};
struct not_op;
struct and_op;
struct or_op;
struct eq_op;
struct neq_op;
struct ge_op;
struct le_op;
struct gt_op;
struct lt_op;
struct mod_op;
struct ternary_op;

template <typename T>
using rw = boost::recursive_wrapper<T>;

typedef boost::variant<constant, n_var, rw<not_op>, rw<and_op>, rw<or_op>, rw<eq_op>, rw<neq_op>, rw<ge_op>, rw<le_op>, rw<gt_op>, rw<lt_op>, rw<mod_op>, rw<ternary_op>> expr;

struct not_op { expr e1; };

#define MAKE_BINARY_OP(name, op) \
struct name { \
  expr e1; \
  expr e2; \
};

MAKE_BINARY_OP(and_op, &&)
MAKE_BINARY_OP(or_op, ||)
MAKE_BINARY_OP(eq_op, ==)
MAKE_BINARY_OP(neq_op, !=)
MAKE_BINARY_OP(ge_op, >=)
MAKE_BINARY_OP(le_op, <=)
MAKE_BINARY_OP(gt_op, >)
MAKE_BINARY_OP(lt_op, <)
MAKE_BINARY_OP(mod_op, %)

#undef MAKE_BINARY_OP

struct ternary_op {
  expr e1;
  expr e2;
  expr e3;
};

struct evaluator : public boost::static_visitor<uint> {
  uint n_value_;
  evaluator(uint n) : n_value_(n) {}

  uint operator()(const constant & c) const { return c.value; }
  uint operator()(n_var) const { return n_value_; }
  uint operator()(const not_op & op) const { return !boost::apply_visitor(*this, op.e1); }
#define MAKE_BINARY_OP(name, OPERATOR) \
  uint operator()(const name & op) const { return (boost::apply_visitor(*this, op.e1)) OPERATOR (boost::apply_visitor(*this, op.e2)); } \

MAKE_BINARY_OP(and_op, &&)
MAKE_BINARY_OP(or_op, ||)
MAKE_BINARY_OP(eq_op, ==)
MAKE_BINARY_OP(neq_op, !=)
MAKE_BINARY_OP(ge_op, >=)
MAKE_BINARY_OP(le_op, <=)
MAKE_BINARY_OP(gt_op, >)
MAKE_BINARY_OP(lt_op, <)
MAKE_BINARY_OP(mod_op, %)

#undef MAKE_BINARY_OP

  uint operator()(const ternary_op & op) const { return boost::apply_visitor(*this, op.e1) ? boost::apply_visitor(*this, op.e2) : boost::apply_visitor(*this, op.e3); }
};

} // end namespace default_plural_forms

} // end namespace spirit_po

/***
 * Adapt structs
 */

BOOST_FUSION_ADAPT_STRUCT(spirit_po::default_plural_forms::constant,
  (uint, value))
BOOST_FUSION_ADAPT_STRUCT(spirit_po::default_plural_forms::not_op,
  (spirit_po::default_plural_forms::expr, e1))
BOOST_FUSION_ADAPT_STRUCT(spirit_po::default_plural_forms::and_op,
  (spirit_po::default_plural_forms::expr, e1)
  (spirit_po::default_plural_forms::expr, e2))
BOOST_FUSION_ADAPT_STRUCT(spirit_po::default_plural_forms::or_op,
  (spirit_po::default_plural_forms::expr, e1)
  (spirit_po::default_plural_forms::expr, e2))
BOOST_FUSION_ADAPT_STRUCT(spirit_po::default_plural_forms::eq_op,
  (spirit_po::default_plural_forms::expr, e1)
  (spirit_po::default_plural_forms::expr, e2))
BOOST_FUSION_ADAPT_STRUCT(spirit_po::default_plural_forms::neq_op,
  (spirit_po::default_plural_forms::expr, e1)
  (spirit_po::default_plural_forms::expr, e2))
BOOST_FUSION_ADAPT_STRUCT(spirit_po::default_plural_forms::ge_op,
  (spirit_po::default_plural_forms::expr, e1)
  (spirit_po::default_plural_forms::expr, e2))
BOOST_FUSION_ADAPT_STRUCT(spirit_po::default_plural_forms::le_op,
  (spirit_po::default_plural_forms::expr, e1)
  (spirit_po::default_plural_forms::expr, e2))
BOOST_FUSION_ADAPT_STRUCT(spirit_po::default_plural_forms::gt_op,
  (spirit_po::default_plural_forms::expr, e1)
  (spirit_po::default_plural_forms::expr, e2))
BOOST_FUSION_ADAPT_STRUCT(spirit_po::default_plural_forms::lt_op,
  (spirit_po::default_plural_forms::expr, e1)
  (spirit_po::default_plural_forms::expr, e2))
BOOST_FUSION_ADAPT_STRUCT(spirit_po::default_plural_forms::mod_op,
  (spirit_po::default_plural_forms::expr, e1)
  (spirit_po::default_plural_forms::expr, e2))
BOOST_FUSION_ADAPT_STRUCT(spirit_po::default_plural_forms::ternary_op,
  (spirit_po::default_plural_forms::expr, e1)
  (spirit_po::default_plural_forms::expr, e2)
  (spirit_po::default_plural_forms::expr, e3))

namespace spirit_po {

namespace default_plural_forms {

/***
 * Pseudo-C Grammar
 */

template <typename Iterator>
struct op_grammar : qi::grammar<Iterator, expr(), qi::space_type> {
  qi::rule<Iterator, constant(), qi::space_type> constant_;
  qi::rule<Iterator, n_var(), qi::space_type> n_;
  qi::rule<Iterator, not_op(), qi::space_type> not_;
  qi::rule<Iterator, and_op(), qi::space_type> and_;
  qi::rule<Iterator, or_op(), qi::space_type> or_;
  qi::rule<Iterator, eq_op(), qi::space_type> eq_;
  qi::rule<Iterator, neq_op(), qi::space_type> neq_;
  qi::rule<Iterator, ge_op(), qi::space_type> ge_;
  qi::rule<Iterator, le_op(), qi::space_type> le_;
  qi::rule<Iterator, gt_op(), qi::space_type> gt_;
  qi::rule<Iterator, lt_op(), qi::space_type> lt_;
  qi::rule<Iterator, mod_op(), qi::space_type> mod_;
  qi::rule<Iterator, ternary_op(), qi::space_type> ternary_;
  qi::rule<Iterator, expr(), qi::space_type> paren_expr_;

  // expression precedence levels
  qi::rule<Iterator, expr(), qi::space_type> ternary_level_;
  qi::rule<Iterator, expr(), qi::space_type> or_level_;
  qi::rule<Iterator, expr(), qi::space_type> and_level_;
  qi::rule<Iterator, expr(), qi::space_type> eq_level_;
  qi::rule<Iterator, expr(), qi::space_type> rel_level_;
  qi::rule<Iterator, expr(), qi::space_type> mod_level_;
  qi::rule<Iterator, expr(), qi::space_type> atom_level_;
  qi::rule<Iterator, expr(), qi::space_type> expr_;

  // handle optional ';' at end
  qi::rule<Iterator, expr(), qi::space_type> main_;

  op_grammar() : op_grammar::base_type(main_) {
    using qi::lit;

    constant_ = qi::uint_;
    n_ = qi::char_('n');
    paren_expr_ = lit('(') >> expr_ >> lit(')');
    not_ = lit('!') >> atom_level_;
    atom_level_ = paren_expr_ | not_ | n_ | constant_;

    mod_ = atom_level_ >> lit('%') >> atom_level_; 
    mod_level_ = mod_ | atom_level_;

    ge_ = mod_level_ >> lit(">=") >> mod_level_;
    le_ = mod_level_ >> lit("<=") >> mod_level_;
    gt_ = mod_level_ >> lit(">") >> mod_level_;
    lt_ = mod_level_ >> lit("<") >> mod_level_;
    rel_level_ = ge_ | le_ | gt_ | lt_ | mod_level_;

    eq_ = rel_level_ >> lit("==") >> rel_level_;
    neq_ = rel_level_ >> lit("!=") >> rel_level_;
    eq_level_ = eq_ | neq_ | rel_level_;

    and_ = eq_level_ >> lit("&&") >> and_level_;
    and_level_ = and_ | eq_level_;

    or_ = and_level_ >> lit("||") >> or_level_;
    or_level_ = or_ | and_level_;

    ternary_ = or_level_ >> lit('?') >> ternary_level_ >> lit(':') >> ternary_level_;
    ternary_level_ = ternary_ | or_level_;

    expr_ = ternary_level_;

    main_ = expr_ >> -lit(';');
  }
};

} // end namespace default plural forms

} // end namespace spirit_po
