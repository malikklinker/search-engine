# Usenet search engine

An in-memory search engine in C with an inverted index, hash-based lookups, binary search tree sets, stop-word filtering, and Boolean AND/OR queries.

## Build and run

Requires Clang or GCC, POSIX libraries, and GNU Make. Run from this directory:

```sh
make -j4
./build/debug/searchengine
```

The default indexes three messages in `examples/`. Try `or orbit fractals`, `and velocity energy`, `show 1`, and `quit`.

Pass a quoted glob to index other messages:

```sh
./build/debug/searchengine '/path/to/20_newsgroups/*/*'
```

Files need a `Message-ID:` header and a blank line before the body. Duplicate message IDs share metadata. Stop words are loaded from `english_stopwords`. The full corpus is not bundled. Sets use unbalanced trees, so large or sorted inputs can be slow.

Use `make test` for local checks, `make release` for an optimized build, `make sanitize` for AddressSanitizer/UBSan, and `make clean` to remove build files. Binaries are stored in `build/<mode>/`.
