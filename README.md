# ENET - revamped
[![CMake](https://github.com/anvouk/enet-revamp/actions/workflows/cmake.yml/badge.svg)](https://github.com/anvouk/enet-revamp/actions/workflows/cmake.yml)

A cleaned-up version of enet + some features and better documentation.

This fork of enet tries to be as API compatible with the original as possible.

Changes so far:
- Repo cleaned-up
- `clang-format` and `clang-tidy` integration and code reformatted
- Drop autotools support in favor of full cmake
- Add examples
- Improve doxygen generation and publish docs on GitHub Pages with CI
- Expose all doxygen comments in public header (enet.h)
- Expose all param names in in public header (enet.h)

New Features:
- Switched to `epoll` as default where present (Linux only, BSD is untested)

Enet is cool, but it's overly difficult to learn the common patterns.
This fork aims at improving the user experience plus minor improvements.

`master` branch should be a 1 to 1 drop in replacement. `epoll` is now the
default on supported Linux versions, but it is expected to behave exactly the
same.

> **Note**: I'm not an expert with ENET and I started this because I got
> flustered by the lack of docs/examples/difficulty to read source code. The
> examples may very well have subtleties not yet known to me. Please, if you
> have had extensive experience with ENET, don't esitate to point out the
> errors.

## Notes on epoll

I tried to benchmark epoll vs poll inside enet but with little success. With
improvised tests they seem to perform both the same.

So far I didn't have any problems with epoll, but it'd be nice to test it out
under much heavier load than what I can currently muster.
