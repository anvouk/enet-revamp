# ENET - revamped

A cleaned-up version of enet + some features and better documentation.

This fork of enet tries to be as API compatible with the original as possible.

Changes so far:
- Repo cleaned-up
- `clang-format` and `clang-tidy` integration and code reformatted
- Drop autotools support in favor of full cmake
- Add examples
- Improve doxygen generation
- Expose all doxygen comments in public header (enet.h)
- Expose all param names in in public header (enet.h)

ENET is cool but it's overly difficult to work with. This fork aims at improving
the user experience.

`master` branch should be a 1 to 1 drop in replacement. I'm experimenting on
epoll in another branch (in a backward compatible way?).

> **Note**: I'm not an expert with ENET and I started this because I got
> flustered by the lack of docs/examples/difficulty to read source code. The
> examples may very well have subtleties not yet known to me. Please, if you
> have had extensive experience with ENET, don't esitate to point out the
> errors.
