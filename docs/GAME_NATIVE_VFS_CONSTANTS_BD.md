# Verified initial VFS data spans (BD)

`GameNativeVfsConstants` owns two stable source buffers copied from the exact
supported original executable: the 537-byte MPKG XOR key at `00E144F0` and
the single NUL byte at `00E17BF0`. It checks the **whole** file size
12,223,752 and SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`
before copying either span. No key bytes are embedded in tracked source. Its
`mpkg_xor_key_00e144f0()` and `null_pattern_00e17bf0()` pointers remain stable
while the owner lives; callers retain it through consumer and singleton drain.

Both addresses lie in original writable `.data` (`E08000` virtual start,
`A08000` raw start, raw size `10000h`, flags `C0000040`). The verified PE raw
offsets are `A144F0` for `[E144F0,E14709)` and `A17BF0` for
`[E17BF0,E17BF1)`. Ghidra and disk bytes match across each complete span.
The key's SHA-256 is
`26abd3e2995fdbacecb6f352b5c0eb20ff20c9a7ada6312ba619c9d4c6fa6183`.
The one-byte fallback has value zero. It is an **empty C string of length
zero**, not a longer semantic pattern or MPAK-specific string. The extra zero
bytes visible after it are unrelated to the required C-string value.

Direct Ghidra references to the key show one read at `00BB9A8B` in the MPKG
archive decoder, whose index uses modulo `219h`. Direct references to the
empty fallback show address loads at `0043E9AC` and `0043EAC7`, with byte
reads within `0043E9A0` and `0043EA80`. The latter routines substitute it
when a native string's data pointer is null and compare until NUL. Existing
MPAK, sound, particle, and string contexts borrow that fallback through their
explicit pointer parameters. No direct write to either start address was
returned by the current Ghidra xrefs. Because original `.data` is writable,
the provider represents the **verified initial-image values**, not live
mutable global identity; indirect/runtime writes have not been excluded.

This source service copies only 538 selected bytes into ordinary owned arrays.
It does not reserve fixed numeric addresses, widen `GameNativeReadOnlyData`,
or map unrelated mutable globals. Image size/hash constants and CryptoAPI
validation logic are duplicated from that mapper because this packet owns no
shared helper file. The new C++ API is an explicit-pointer source binding, not
an original ABI replacement or game-process validation. See
`reports/game_native_vfs_constants_bd.json` for exact evidence and fixture
scope.
