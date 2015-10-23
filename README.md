# spirit-po

**spirit-po** is a header-only C++ library that you can use instead of
libintl.

**spirit-po** has the advantage that it does not use mo files, so your
program can use simple, portable po files produced directly by
translators rather than a non-portable binary format which requires
special tools to create.

**spirit-po** is also very easy to use in your C++11 projects.
It is only a few hundred lines of code in total, and is implemented
using boost::spirit.

An in-depth explanation of the rationale is provided [on the wiki](Rationale).

## Licensing and Distribution

**spirit-po** is free software available under the Boost software license.

## Using it

To use the library, you only need to include the `include` directory
in your C++ source files.

The src/ folder is only for building the tests.

## Acknowledgements

The author thanks David White and Kristina Simpson for conversations
which led to the creation of this library.
