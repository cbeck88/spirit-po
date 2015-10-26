# spirit-po

**spirit-po** is a header-only C++11 library that you can use for
localization within the GNU gettext system, instead of using `libintl`.

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

N.B.: **spirit-po** is only intended to work with UTF-8 encoded po files.

To use the library, you only need to include the `include` directory
in your C++ source files.

    #include <spirit_po.hpp>

The `src/` folder, and the `Makefile`, are only for building the tests.

To make a catalog, first obtain some po content. Then you can build a catalog
using one of three methods:
  - `spirit_po::catalog` ctor, templated to work with any pair of iterators
  - factory function `spirit_po::catalog<>::from_range` which can take any
    forward range of characters of any type
  - factory function `spirit_po::catalog<>::from_istream` which takes any
    given `istream` and builds a po catalog from it. Spirit reads incrementally,
    so this does not require reading the entire istream into a string first.

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

   - `const char * gettext(const char * msgid)`
   - `const char * ngettext(const char * msgid, const char * msgid_plural, uint plural)`
   - `const char * pgettext(const char * msgctxt, const char * msgid)`
   - `const char * npgettext(const char * msgctxt, const char * msgid, const char * msgid_plural, uint plural)`

These implement, basic message lookup, plural-forms lookup, contextual lookup,
and plural-forms-with-context lookup, respectively. See GNU gettext docs for details.

As in GNU libintl, the string pointers which are returned are generally non-owning pointers.
When a translated form is found the catalog, the returned pointer points to a string owned by
the catalog. When a translated form is not found, the returned pointer is one of the arguments.
This is maximally efficient when using gettext with `_` macros and such, where the input parameters
will be strings with static storage duration. However, in the general case, if the input pointer
becomes invalid, then the output pointer may become invalid also.

We also give equivalent, alternate versions of these which return `std::string`
and take `const std::string &` in place of `const char *` as parameters. In
some scenarios these versions may be more efficient, and their ownership is unambiguous.
They are otherwise equivalent.

   - `std::string gettext_str(const std::string & msgid)`
   - `std::string ngettext_str(const std::string & msgid, const std::string & msgid_plural, uint plural)`
   - `std::string pgettext_str(const std::string & msgctxt, const std::string & msgid)`
   - `std::string npgettext_str(const std::string & msgctxt, const char std::string & msgid, const std::string & msgid_plural, uint plural)`

We do not provide implementations of the `dcgettext` functions, which implement
alternate textdomains.

One of the premises of the library is that you may not want to actually use textdomains as
described by GNU gettext. If you want to have multiple catalogs loaded into the
program at once, you are recommended to throw together your own book-keeping mechanism for
this -- it is straightforward to have a `std::unordered_map` of catalogs or similar, and
then it is transparent to you without cluttering our catalog interface. On the other hand,
you can also use the `merge` function of catalogs (TODO!) to merge multiple catalogs into one
master catalog.

## Customization points

`spirit_po::catalog` is a template, and you may customize it in two ways.

- Specify an alternate hashmap type. The default is `std::unordered_map`, but
if you like you can experiment with `boost::flat_map` or a flat unordered map,
or one of the Loki hashmaps, etc.
- Specify an alternate plural forms compiler. If instead of the pseudo-C
language for plural-forms functions specified by GNU gettext, you would like
to use a different programming language to specify the plural-forms index function,
you can easily specialize the catalog to use a different plural-forms function
compiler for your desired language.

## Acknowledgements

The author thanks David White and Kristina Simpson for conversations
which informed the creation of this library.
