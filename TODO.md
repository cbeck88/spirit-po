TODO
====

- Add more `msgfmt`-style validation? (if one of `msgid`, `msgstr`
  ends with `'\n'` and one does not, give a warning?)
- Add support for parsing po content with no header? Just 
  'pasting in' the po content? Allow an empty `catalog` to be
  constructed from a `catalog_metadata` object?
- Refactor the catalog metadata header parse function into a
  single `qi` grammar?
- Benchmarks?
- Refactor warning channel so as not to rely on `std::function`?
- Test / support BOOST_NO_RTTI builds?
