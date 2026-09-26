# Pooled 2D texture surface-cache header lifetime

The genuine source-owned 2D pool already provides a lifetime creation point for
the trivial 12-byte `NativeTextureSurfaceCacheStorage` at owner+40h. No placement,
constructor, reset, retain, ownership normalization or production change is needed
at B3F930 for that path. This is a source-model conclusion using the documented
compiler/defect contract, not new production-code, fixture, ABI or game proof.

`singleton_lifetime.cpp:56` calls genuine `std::malloc`. The 2D pool then starts
`SlabBytes` with placement new in `initialize_slab_00b3d1e0`, beginning its nested
`std::byte` array lifetime. These operations permit the needed implicit-lifetime
objects within the slab; the raw constructor stores supply their values.
The cache header and 8-byte entry are public trivial aggregates without user
constructors/destructors or default member initialization. Malloc alignment,
54h slot stride and +40h offset satisfy the Win32 header alignment. Its extent
+40..+4B precedes retained source+4C and the untouched pool index+50.

The creation rules come from [WG21 P0593R6](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p0593r6.html).
[WG21 P2131R0, Unlisted papers](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p2131r0.html)
explicitly records P0593R6 as a defect resolution against C++17.
[Microsoft's conformance table](https://learn.microsoft.com/en-us/cpp/overview/visual-cpp-language-conformance?view=msvc-170)
reports support without the C++20-only annotation on neighboring features.
The inspected target is MSVC 14.51.36231, Win32, C++17. Applying those documented
rules to this allocator/aggregate path is an inference, not a runtime experiment.
A cast, typed reference or `std::launder` alone does not start any lifetime.

One isolated compile-only traits object qualifies the separate atomic comparison:
with `_HAS_CXX20=0`, actual `atomic<int32_t>` is not an aggregate and has no trivial
default/copy/move constructor, although its destructor and storage-base default
constructor are trivial. Its outer template default constructor is user-provided.
That installed type is not implicit-lifetime. Deleted copying and a user value
constructor alone would not exclude an eligible trivial default constructor in
another C++17 implementation. Explicit producer placement remains separate.
This refines the original readiness explanation without changing its sealed ZIP.

The complete unchanged B3F930..B3FA80 range is 337 bytes/120 instructions and
matches installed PE/Ghidra bytes. After named B34230 and D61948/+24/+3C stores,
native state0 is set at B3F974. B3F978/+97B/+97E zero pointer+40, count+44 and
capacity+48. B3F981 captures current COM+10, B3F984 clears source+4C, and current
GetLevelCount slot34 is captured before state1/B3F992 call. No header values
are moved earlier. Native ECX/stack/EAX/RET14h differs from the explicit C++ API.

Actual file loading obtains B3F2B0 pool storage before B3F930. Typed header
borrowers are the owner cleanup, B3FD80 surface getter/reserve, B3D9B0 reserve,
B3DA20 resize and B3EC40 storage teardown. The B3F7B0 runtime constructor has
the same pool/header dependency but retains its separate count/COM/failure
contract. Current entry consumers use scalar layout reads/stores; this audit
does not fabricate typed entry/surface ownership or certify every pointer access.

B3EC40 is not a C++ header destructor. The header stays live inside the slab
across pool return/reuse; subsequent construction writes its existing fields.
Entry-array free/reallocation ends that backing's lifetimes, so captured entries
must remain live through reached callback-return stores. Negative capacity can
reach the existing reserve path; unconditional stale-data/capacity behavior is
not claimed. Slab free ends its provided storage lifetimes.

External raw images, mapped/native fixtures and direct callers must separately
prove live aligned header backing and valid reached reads; this pooled conclusion
does not admit them. Named/unnamed +4 atomic producer lifetime is separate.
Flags1 slot return still precedes canonical unbind/Entry retirement, requiring
outer exclusion/quiescence; persistent header life supplies no synchronization.
Existing state0/1/2 cleanup, retained failed backing, source/header/string/COM
obligations and no-replay rules remain unchanged; no private EH proof is added.

Frozen readiness is `local/cc10_texture_2d_cache_header_lifetime_readiness`:
`evidence.zip` SHA256 `7a5ee622af26eeb3106f5b27926a47bfecb77033f6dad440d14fa42c0658b7a5`,
33 indexed rows/34 members; index SHA256
`51b6c7c6b17ca2459f6c2e659c4fe664dda5a400df8fa2cfeab45f3470617317`.
It pins 14 source/config files, the full native range, compiler/primary-source
references and unchanged 48 prior archives/four descriptor build outputs.
No production changes, CMake build, tests, runtime, Ghidra/ledger edits or native
credit occur here. The single traits compile links no executable and runs nothing.
