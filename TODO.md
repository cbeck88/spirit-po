TODO
====

- Add support for parsing po content with no header? Just 
  'pasting in' the po content? Allow an empty `catalog` to be
  constructed from a `catalog_metadata` object?
- More testing for contexts / plural forms?
- Add a bytecode for the plural forms function expressions?
- Refactor the catalog metadata header parse function into a
  single qi grammar?
- Add more msgfmt style validation? (if one of msgid, msgstr
  ends with '\n' and one does not, give a warning?)
- Allow to detect and omit fuzzy strings?
- Benchmarks?
