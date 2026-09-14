# Native physical-provider enumeration (BF)

The current saved Ghidra function at `00BF47E0` owns all 912 bytes through
`00BF4B6F` (267 instructions); `00D69168+14` contains `E0 47 BF 00`.
Original ABI: ECX actual physical provider, stack directory and extension
native 8h headers, flags DWORD, output native 0Ch string vector; `RET10h`,
no semantic result. The saved body is complete, so no function-definition
repair or provisional instruction attribution is needed.

`enumerate_native_physical_names_00bf47e0` borrows the actual provider,
existing raw path-builder context, canonicalizer context and string pool. It
requires current provider slots `+1C=00BF3970` and, when recursing,
`+14=00BF47E0`. The path builder joins the provider's root to the unchanged
query directory; the enumeration appends a backslash if its final byte is not
one, copies the path, appends `*`, and calls `FindFirstFileA`. It processes each
`WIN32_FIND_DATAA` in API order. Dot-prefixed entries are skipped. Ordinary
files require filename length strictly greater than stored extension length,
a case-insensitive suffix match, and exclusion of `MidwayDL_Content`.
Accepted files append a canonical logical path to the caller's actual vector
through existing `004CDC20`. Directories recurse depth-first only if the
flags DWORD's low byte is nonzero; the full flags word is passed to children.
`FindNextFileA` false ends traversal and `FindClose` runs once. No output is
cleared, sorted or deduplicated here, and no `GetLastError` distinction exists.

The companion `join_native_physical_enumerated_name_00bee520` covers the
complete `00BEE520..00BEE683` saved body in the normal source domain. Empty
parent or child takes the direct existing `00BEE390` canonicalization branch.
The two-nonempty branch creates `/`, concatenates parent then child, builds a
canonical temporary, releases the three concatenation temporaries in reverse,
copy-constructs the result through existing `00426060`, then releases the
canonical temporary. It does not use mount-prefix trimming.

Source deviations and evidence limits: an empty built physical path is
rejected explicitly because the native body reads its preceding byte without
a guard. C++ scope cleanup closes an outstanding find handle on exceptions;
the native FH3 unwind actions are not claimed equivalent. `FindFirstFileA`
failure returns with no additions; later `FindNextFileA` failure retains all
prior additions. Current source treats unsupported virtual slots as explicit
errors. Exact register ABI, compiler stack preimages, hardware-fault paths,
arbitrary provider-table mutation and original game execution are not proven.
The one-byte separator/star temporaries use the existing raw C-string
constructor and append rather than the still-unported `00531030/0054AA70`
helper pair; resulting bytes and pool ownership are represented, but its
intermediate getter schedule is not an exact helper-body reconstruction.
The original installed executable is unchanged.

Both new routines passed strict MSVC Win32 translation-unit compilation.
The shared `bsp_core` CMake registration for `src/native_physical_enumeration.cpp`
is deferred to the primary integration pass; the existing checkout build
therefore does not compile this new TU. The structured report records every
saved-owner direct CALL in the two roots plus the indirect provider/Win32
dispatch sites separately. Source names are descriptive hypotheses.
