# Native scene-registry pair transfer leaves

`B821C0..B821EB` (44 bytes) and `B82B80..B82BB1` (50 bytes) now have separate
Win32 implementations in `native_scene_registry_storage_leaves.cpp`. They
operate on actual eight-byte iterator pairs. Descriptive names are hypotheses.

`B821C0` receives first/last in ECX/EDX and destination in the first of four
stack words. It returns the advanced destination in EAX and ends with RET10h
at B821E9. Equal endpoints return without reading source storage. Otherwise
each iteration tests the current destination, copies source+0, and only then
reads and copies source+4. A null destination skips both reads. Source and
destination advance by eight with native 32-bit wrapping. Forward overlap is
observable; neither a pair snapshot nor memmove reproduces all aliases.
The remaining three stack words are unused. The caller must provide a finite
range whose endpoint is reachable in eight-byte steps and valid reached words.

`B82B80` receives destination/count/pair-source in three stack words, returns
captured destination plus captured count times eight in EAX, and ends with
RET0Ch at B82BAF. Its incoming ECX is copied into scratch; only the scratch's
low byte is cleared. The wrapper preserves both pair-source argument loads
and the four outgoing words before calling the existing actual `B82570` at
B82BA4. The shared fill provider preserves componentwise overlap and skips
source dereferences for null current destinations. No allocation, native
reference credit, host iterator record or logical registry is introduced.

Both implementations retain the register/stack schedule with MSVC Win32
assembly, matching the existing storage leaves. This is a source interface;
the packet does not claim complete binary integration, arbitrary invalid
ranges, asynchronous mutation, native access-fault recovery or gameplay.

All 94 bytes match the installed executable and live Ghidra. A focused ignored
executable compares the copied original bodies against the built source:
component-overlap buffers and returned endpoints, plus inaccessible source
storage with null destination and empty range/count. The original fill wrapper
shares the genuine existing `B82570` provider through one relocated CALL;
the original copy leaf has no dependency seam. Assertions remain active.
The probe links with `/MD /MANIFEST:EMBED`; the strict Win32 build and all
three existing CTests pass. No tracked tests were added.

This closes two dependencies of `B82FD0`. Boundary-vector insertion/erase,
bucket resize/assign and full scene registration remain separate packets.
Evidence and frozen local hashes are recorded in
`reports/native_scene_registry_pair_transfer_cc10.json`.
