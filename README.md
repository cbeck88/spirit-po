# spirit-po

**spirit-po** is a header-only C++11 library that you can use instead of
libintl.

**spirit-po** has the advantage that it does not use mo files, so your
program can use the simple, portable po files produced directly by
translators rather than require a non-portable binary format which
requires special tools to create.

**spirit-po** is very easy to use in your C++11 projects.
It is only a few hundred lines of code in total, and is implemented
using `boost::spirit`. Our po grammar itself is only a few dozen lines.
This makes it very simple and transparent, and easy to modify if needed.

By contrast, the `libgettext-po` parser and `libintl` implementation
together span about ten thousand lines of old-style C. Guess which
one I would prefer to use. :)

An in-depth explanation of the rationale for this library as compared to
`libintl` and `boost::locale::gettext`, and specifically, the advantages
of parsing po files rather than mo files at run-time, is provided
[on the wiki](https://github.com/cbeck88/spirit-po/wiki/Rationale).

## Licensing and Distribution

**spirit-po** is free software available under the Boost software license.

## Using it

To use the library, you only need to include the `include` directory
in your C++ source files.

    #include <spirit_po.hpp>

To make a catalog, obtain some po content. Then you can build a catalog
using one of three methods:
  - `spirit_po::catalog` ctor, templated to work with a pair of iterators
  - factory function `spirit_po::catalog<>::from_range` which can take any
    range of characters of any type
  - factory function `spirit_po::catalog<>::from_istream` which reads any
    given `istream` to completion and builds a po catalog from it
    incrementally.

If the po content is malformed, one of two things will happen (configurable):
  - A `spirit_po::catalog_exception` will be thrown. (This is the default.)
  - If a symbol is defined `SPIRIT_PO_NOEXCEPT` before including `spirit_po.hpp`,
    then the catalog constructor will not throw (and none of the other functions
    will either), and instead, the catalog will result with whatever strings it
    managed to load, an `explicit operator bool() const` function will be defined
    which returns false if the constructor would have thrown, and a method
    `error()` is defined which returns the error string in case there was an
    error.

The `spirit_po::catalog` object has 4 methods which are part of the gettext
specification:

   - `const char * gettext(const char *)`
   - `const char * ngettext(const char *, const char *, uint)`
   - `const char * pgettext(const char *, const char *)`
   - `const char * npgettext(const char *, const char *, const char *, uint)`

These implement, basic message lookup, plural-forms lookup, contextual lookup,
and plural-forms-with-context lookup, respectively. See GNU gettext docs.
We also give equivalent, alternate versions of these which return `std::string`
and take `const std::string &` in place of `const char *` as parameters. In
special cases these versions may be often more efficient, this is the reason
that we provide them.

`spirit_po::catalog` is a tempalte, and you may customize it in two ways.

- Specify an alternate hashmap type. The default is `std::unordered_map`, but
if you like you can experiment with `boost::flat_map` or flat unordered map,
or one of the Loki hashmaps, etc.
- Specify an alternate plural forms compiler. If instead of the pseudo-C
language for plural-forms functions specified by GNU gettext, you would like
to use a different programming language to specify the plural-forms function,
you can easily specialize the catalog to use a different plural-forms function
compiler.

The `src/` folder, and the `Makefile`, are only for building the tests.

## Acknowledgements

The author thanks David White and Kristina Simpson for conversations
which informed the creation of this library.
