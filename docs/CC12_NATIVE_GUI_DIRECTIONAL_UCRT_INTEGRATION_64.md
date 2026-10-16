# Directional lifetime and current UCRT integration 64

Combined Source `f30e0bd9348c7d9273c0639c698a728ccfd4db55` integrates
reviewed packets 60 and 61 with their six exact Source blobs unchanged.
Canonical MSVC Win32 Release build and all three existing CTests passed
(8.80 seconds). No new primary test or normal game runtime was added.

Packet 60 (`f7c26281e`) bridges actual raw names into the existing typed
directional-light graph. It reuses the raw constructor, preserves the same
typed tail's bytes, selects raw node destruction, and retains caller allocation
state across entered failure. Publication installs the existing canonical
reference without adding a retain. Its one focused Source case used the actual
raw-string and directional pools, real counter-derived types, installed image
profiles/constants, same count1/retain2/logical1/terminal0, exact name cleanup
and same physical slot reuse. Full primary review also checked existing scene
and lifetime pointer-vector admission against constructor failure.

Packet 61 (`a339040ea`) supplies explicit stateless current owning-UCRT Text
services. Narrow comparison is the actual `_stricmp` function pointer; wide
comparison makes two real `wchar_t` value copies then calls actual `_wcsicmp`.
Its production-object private-process probe linked real canonical game
dependencies/libraries, used a profile created in C, and observed earlier
callback changes C -> `English_United States.1252` -> C. Wide results changed
-32 -> 0 -> -32, and actual narrow results changed -32 -> 0. This case passed
without skipping and restored its original process locale; system locale and
the game were not changed.

These are Source composition results. Packet 60 adds no new Original body,
binary/FH3/fault equivalence or entered-failure runtime injection. Packet 61's
allocating copies have ordinary C++ failure and stable-input obligations;
owning UCRT receives their pointers, not original u16 pointers. It establishes
neither Original CRT state nor no-allocation/Unicode-version equivalence.
Native Text, Screen/page composition and gameplay remain unvalidated.

The JSON receipt records Source/object/build hashes and a checked archive,
including copies of both uniquely hashed worker probe executables and their
focused evidence. Earlier normal startup evidence remains in packet 55; it
does not activate these newly added routes.
