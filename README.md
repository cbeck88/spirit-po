# spirit-po

[![Build Status](https://travis-ci.org/cbeck88/spirit-po.svg?branch=master)](http://travis-ci.org/cbeck88/spirit-po)
[![Appveyor status](https://ci.appveyor.com/api/projects/status/401vqi4ci7g46oli/branch/master?svg=true)](https://ci.appveyor.com/project/cbeck88/spirit_po)
[![Boost licensed](https://img.shields.io/badge/license-Boost-blue.svg)](./LICENSE)

**spirit-po** is a header-only C++11 library that you can use for
localization within the GNU gettext system, instead of using `libintl`.

**spirit-po** has the advantage that it does not use mo files, so your
program can use the simple, portable po files produced directly by
translators rather than require a binary format which
requires special tools to create.

**spirit-po** is very easy to use in your C++11 projects.
According to `sloccount`, it is only 916 lines of code in total (at time of writing),
and is implemented using `boost::spirit`.
Our po grammar itself is only a few dozen lines.
This makes it relatively easy to understand its behavior, and makes the library as a whole easy to modify if needed.  
  
By contrast, the equivalent po-parser facility within the gettext project is
the `libgettext-po` po-manipulation library. `sloccount` counts the entire
`libgettext-po` directory as `ansic:        29382 (98.35%)`, that is, 30kloc of ANSI C.
The parser is only a piece of this, but it is also quite complex and difficult to separate
from the rest of the code. (Which is partly why I made `spirit-po`.)  
  
An in-depth explanation of the rationale for this library as compared to
`libintl` and `boost::locale::gettext`, and specifically, the advantages
of parsing po files rather than mo files at run-time, is provided
[on the wiki](https://github.com/cbeck88/spirit-po/wiki/Rationale).

## Compatibility

`spirit_po` is intended to be a drop-in replacement for the use of GNU `msgfmt` and GNU `libintl`.  
It should parse any well-formed `.po` file that `msgfmt` would read and the interface should produce the
same results.  

It's not guaranteed to reject any po file that `msgfmt` would reject, or to emit warnings
similar to `msgfmt` for common translator errors. Broadly speaking, the parser has been engineered with a fail-fast
mentality, and there are several unit tests that check that major structural problems cause a parse error rather than
silently being accepted. However, for best results you may wish to validate po files by running them through `msgfmt`
just to see if it emits warnings, before deploying them, even if you use `spirit_po` in your application.  

Similarly, there are certain cases that I am aware of in which `msgfmt` will drop a message from the catalog if
it contains invalid C format specifiers. `spirit_po` doesn't do this, which is a minor discrepancy.

If you are aware of any `.po` file which `msgfmt` parses, but `spirit_po` fails to parse, or, our emulation of the `libintl`
interface doesn't yield expected results, please post a report on the issue tracker, with the po file included.

## Quick Start

To begin, first obtain some `.po` files. PO files are created by translators, they contain a dictionary of translated strings.

For examples from various GNU projects, see [our test folder](https://github.com/cbeck88/spirit-po/tree/master/test_libintl/po).

Then, load the file and construct a `spirit_po::catalog` from it.

```c++

#include <spirit_po/spirit_po.hpp>
#include <fstream>
#include <iostream>
#include <string>

using default_catalog = spirit_po::catalog<>;

int main() {
  std::ifstream ifs("test.po");
  std::string po_file{std::istreambuf_iterator<char>{ifs}, std::istreambuf_iterator<char>()};
  
  default_catalog cat{default_catalog::from_range(po_file)};

  std::cout << cat.gettext("Hello world!") << std::endl;
  
  std::cout << cat.pgettext("Pick a number: ", "prompt") << std::endl;
  
  int number = 6;
  std::cin >> number;

  std::cout << std::endl;

  fprintf(cat.ngettext("Did I fire %d shot or was it only %d? Do you feel lucky, punk?",
                       "Did I fire %d shots or was it only %d? Do you feel lucky, punk?",
                       number),
          number,
          number - 1);
}

```

The catalog object serves translation requests using the strings it loaded from the PO file. 

In this line,

```c++
std::cout << cat.gettext("Hello world!") << std::endl;
```

the translated form of `"Hello world!"` will be displayed. The result will be a `const char *` pointing to
a string owned by the catalog. (Or, if the translation misses becaues this string wasn't in the catalog, it will simply return the
english text `"Hello world!"`, the same pointer it was passed in.)

In this line,

```c++
std::cout << cat.pgettext("Pick a number: ", "prompt") << std::endl;
```

a string is translated, and also marked with a context string. Sometimes the same english phrase or sentence appears in your program in multiple places, but should be translated
differently depending on context. The context string allows you to provide a hint to the translator and allows the program to disambiguate the two usages. (This particular example is unfortunately a poor one.)

In this line,

```c++
fprintf(cat.ngettext("Did I fire %d shot or was it only %d? Do you feel lucky, punk?",
                     "Did I fire %d shots or was it only %d? Do you feel lucky, punk?",
                     number),
        number,
        number - 1);
```

the catalog object will look up the C-format string in the catalog, and search for the plural form corresponding to `number`. This ensures
that `"shots"` will be pluralized correctly no matter what language is used. (In many languages, there are more than two plural forms and language-specific logic is needed to determine the appropriate form to use based on the number. The translator provides this logic in the po-file header.) Then we use `fprintf` to substitute the numbers into the string.

These examples are actually all rehash from gettext documentation -- the member functions `gettext, pgettext, ngettext` are all analogous to calls to the C library `libintl`.

If you aren't already familiar with gettext, have a look at their [documentation](https://www.gnu.org/software/gettext/).

## Usage

#### Loading and Construction

When you load translations with `spirit_po` the loading process is entirely in your hands and you can make it work however
you like. A catalog can be constructed using  one of three methods:
  - factory function `spirit_po::catalog<>::from_iterators` which can take
    any pair of iterators which spirit can use.
  - factory function `spirit_po::catalog<>::from_range` which can take any
    forward range of characters which spirit can use (such as a `std::string`).
  - factory function `spirit_po::catalog<>::from_istream` which takes any
    given `istream` and builds a po catalog from it. Spirit reads incrementally,
    so this does not require reading the entire `istream` into a string first.
    (However, in the typical case of reading a po-file, that will usually be
     faster. Recommendation is not to use `from_istream` with a `std::ifstream`
     for best performance.)

(You should use one of these rather than using the ctor directly.)  

If the po content is malformed, one of two things will happen (configurable):
  - A `spirit_po::catalog_exception` will be thrown. (This is the default.)
  - If preprocessor symbol is defined `SPIRIT_PO_NO_EXCEPTIONS` before including `spirit_po.hpp`,
    then the catalog constructor will not throw (and none of the other functions
    will either), and instead, the catalog will result with whatever strings it
    managed to load, an `explicit operator bool() const` function will be defined
    which returns false if the constructor would have thrown, and a method
    `std::string error() const` is defined which returns the error string in case there was an
    error. This option is here to support projects that compile with `-fno-exceptions`.

#### Looking Up Strings (Messages)

The `spirit_po::catalog` object has 4 methods which are part of the gettext
specification:

   - `const char * gettext(const char * msgid)`
   - `const char * ngettext(const char * msgid, const char * msgid_plural, uint plural)`
   - `const char * pgettext(const char * msgctxt, const char * msgid)`
   - `const char * npgettext(const char * msgctxt, const char * msgid, const char * msgid_plural, uint plural)`

These implement, basic message lookup, plural-forms lookup, contextual lookup,
and plural-forms-with-context lookup, respectively. See GNU gettext docs for details.

As in GNU `libintl`, the string pointers which are returned are non-owning pointers.
When a translated form is found the catalog, the returned pointer points to a string owned by
the catalog. When a translated form is not found, the returned pointer is one of the arguments.
This is maximally efficient when using gettext with `_` macros and such, where the input parameters
will be string literals with static storage duration. However, in the general case, if the input pointer
becomes invalid, then the output pointer may become invalid also.

We also give equivalent, alternate versions of these which return `std::string`
and take `const std::string &` in place of `const char *` as parameters. In
some scenarios (i.e. if you must make a copy of the output anyways, and the input string
is already held by a `std::string`) these versions may actually be more efficient, and the
lifetime of the result is unambiguous.

They are otherwise equivalent.

   - `std::string gettext_str(const std::string & msgid)`
   - `std::string ngettext_str(const std::string & msgid, const std::string & msgid_plural, uint plural)`
   - `std::string pgettext_str(const std::string & msgctxt, const std::string & msgid)`
   - `std::string npgettext_str(const std::string & msgctxt, const  std::string & msgid, const std::string & msgid_plural, uint plural)`

#### Managing multiple catalogs

In the simplest setup, you would have one application, and the set of all strings it needs to translate.
If there are `m` languages you support, you might have one `.pot` file (po-template which you give to translators)
and get back `m` different `.po` files. Your application would then load only one of them at a time based on the locale.

In many GNU programs, the situation is more complicated. The application may be broken up into components, and the strings
from different components classified into different "textdomains". Then, there would be one `.pot` file for each textdomain.
If there are `m` textdomains and `n` languages, you would have `n * m` different `.po` files.

In the traditional `libintl` `C`-interface, textdomains are, like locale, handled by a global variable. The `libintl` library
manages the loading of catalogs and text domains, which is accessed by `gettext` and friends. At any time, a global function may be called to
change the current locale or textdomain, which are stored in global variables. So, if for instance your program has a UI module and a UI textdomain,
the UI code would set the text domain when it is entered, and then call `gettext` on each string it needs. Then when you enter another module,
you would bind a different textdomain, and then make `gettext` calls for those strings.

There are various problems that I've experienced when writing programs that use this interface.

- The built-in C file functions do not support UTF-8 paths when compiling with mingw for windows.
  `libintl` does not provide any way to work around this. (Some other libraries like SDL allow you to pass function pointers
  to alternative filesystem functions. Then you can have one cross-platform filesystem implementation, and make everything in your program use it.)
- If you have a multithreaded program and multiple threads that need to talk to `libintl`, you can create a datarace because
  everything is clobbering the same global variables. This is always a problem with libc, and fortunately it's very unlikely to happen with changes to the
  locale, because the locale rarely changes in typical programs. However, it's a big problem with textdomains, which are likely to change frequently.
  It's also quite likely that code in different threads will be using different textdomains.

When using `spirit_po`, you should understand that, `spirit_po` is not attempting to emulate the entire `libintl` interface. A `spirit_po::catalog` is
only a single catalog, corresponding to a single po file loaded into memory. It doesn't have `dcgettext` method like `libintl` does for instance, because
a catalog only represents one textdomain.

If you don't need multiple textdomains, I recommend that you avoid it. (AFAIK the main advantage of having multiple textdomains is that if you have a
large number of strings, you can assign different textdomains to different translators, to divide up the work. Many translators and translation management platforms
already support having multiple textdomains for a project, and giving progress reports for each one. That is, I think this is primarily done for convenience of
translators rather than programmers.)

If you do need multiple textdomains, I recommend that you throw together your own mechanism for this. For instance, if you are already using textdomains and libintl
and want to switch to `spirit_po`, you can have a global `std::map<std::string, spirit_po::catalog>`. You can provide a function like `dcgettext` which dereferences this
map at the correct text domain, or manage the current textdomain yourself. Coding this up is straightforward.

If your program has multiple threads, you can have one such map for each thread, and put it in thread-local storage. Or it may be that your main thread needs multiple
textdomains, but the other threads each use at most one textdomain, which would make things simpler. You could then pass each one a catalog object when it starts up, or whatever.

Basically, I don't want to make such architectural decisions for you. `spirit_po` is focused just on parsing po-files and handling the queries, I don't want to force
you to use global variables or broken filesystem functions. So we don't completely reproduce the `libintl` interface.

In the interest of being flexible, `spirit_po` also allows you to merge compatible catalogs together into one master catalog. This is an alternate approach -- if
the translation team wants to have multiple textdomains for their convenience, it doesn't mean the programmers have to think about multiple textdomains all the time as
well.

   - `void merge(spirit_po::catalog && other)`  
     Check if the metadata of this catalog and given catalog shows they are compatible
     (number of plural forms are equal). If not then signal an error (exception or error
     state). If so, then move all the message entries from the other hashmap to this map.
     May trigger warnings on the warning channel if there are collisions.

   - `void set_warning_channel(const std::function<void(const std::string &)> & w)`  
     Set the warning channel for this catalog. The warning channel is a function which
     will be called with a warning message whenever a string (with context) is clobbered.
     The warning channel object may also be passed to the constructor, if one is concerned
     about duplicated strings within a single po file. By default warnings are ignored.

However, merging catalogs has its own pitfalls. What happens if two catalogs contain the same string? One of them gets discarded, but which one is essentially arbitrary.
You can give the catalog object a "warning channel" where it can send warnings when that happens. But it's hard to actually handle the problem or fix it. Also, merging catalogs
requires dropping the metadata of one of them, and some other features like getting the line number at which a string appeared in the `po` file no longer makes sense, because when we merge
we don't keep track of which file a string came from. So, merging catalogs might be appropriate for some projects, but generally I would recommend avoiding multiple catalogs, or throwing together
your own system for managing multiple distinct catalogs, as appropriate for your project.

#### Other functions

Some less commonly useful accessors

   - `const spirit_po::catalog_metadata & get_metadata() const`  
     Return the metadata structure that was parsed from the po header.
   - `std::size_t gettext_line_no(const std::string & msgid) const`  
     Return the line number at which a given catalog message was read. `0` if it is not found.
   - `std::size_t pgettext_line_no(const std::string & msgctxt, const std::string & msgid) const`  
     Return the line number at which a given catalog message (with context) was read. `0` if it is not found.


## Customization points

`spirit_po::catalog` is a template, and you may customize it in two ways.

- Specify an alternate hashmap type.  
  The default is `std::unordered_map`, but
  if you like you can experiment with `boost::flat_map` or a flat unordered map,
  or one of the Loki hashmaps, etc.
- Specify an alternate plural forms compiler.  
  GNU Gettext specifies a pseudo-C expression language for plural forms functions.
  For example, in Polish there are three plural forms. There is a form for the singular,
  a form used when the number ends in 12, 13 or 14, and a form for all other cases.
  The appropriate logic is typically specified in the po-header like so:

  ```
  Plural-Forms: nplurals=3; \
      plural=n==1 ? 0 : \
             n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2;
  ```

  To implement functions like `ngettext`, `spirit_po` needs to be able to read these pseudo-C expressions and
  evaluate them for different values of `n`. `spirit_po` contains a built-in facility to do this efficiently --
  it converts such expressions to a sequence of op-codes that run on a simple stack machine.

  If for some reason you want to use a different format for this, or a different C++ implementation of the standard format,
  you can pass a custom plural forms "compiler" type as the second template parameter to `spirit_po::catalog`.

  The compiler is a function object that should be default constructible, and should take a string
  (the part that starts after `plural=`) and return
  a function object of signature `unsigned int(unsigned int)`, representing the compiled plural forms function.
  See the default implementation for details.

## Licensing and Distribution

**spirit-po** is open-source software available under the Boost software license.

## Dependencies

- **spirit-po** is only intended to work with UTF-8 encoded po files.
- **spirit-po** has been tested against many boost versions, ranging from 1.48 to 1.65.
- **spirit-po** does not require C++ exceptions to be enabled.  
  The tests run when compiled with `-fno-exceptions`, provided that
  - `SPIRIT_PO_NO_EXCEPTIONS` is defined
  - `BOOST_NO_EXCEPTIONS` is defined
  - Boost (headers-only) version >= 1.55. (Fails below that due to a bug in `boost::variant`.)

## Compiler Support

**spirit-po** has been tested with

- `gcc` versions `4.9, 5.0, 5.4, 6.3`
- `clang` versions `3.5, 3.7. 3.8, 4.0`
- MSVC 2013, 2015, 2017

See `.travis.yml` and `appveyor.yml` for info about our CI.

## Tests

The `test/` folder contains the unit tests, built with a Makefile.

The `test_libintl/` folder contains the validation tests against `libintl`, built
using cmake. To add new validation test cases, just drop new `.po` files in the folder
`test_libintl/po/`.


## Acknowledgements

The author thanks David White, Kristina Simpson, and others for conversations
which informed the creation of this library.
