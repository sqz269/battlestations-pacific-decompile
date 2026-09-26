# B48AF0 actual declaration count lifetime

The production declaration constructor now starts one `std::atomic<int32_t>`
lifetime at the existing actual owner `+4`. It initializes value1 at the native
count-store point, between the base and derived profile stores. This fixes the
raw-word producer versus atomic companion mismatch without adding another
count, reference credit, wrapper, fallback or interface migration.

The only production changes are `src/native_vertex_declaration_owner.cpp` and
its header contract. The constructor includes `<atomic>/<new>` and asserts
four-byte atomic size/alignment on its existing MSVC Win32 target. Callers must
supply genuinely four-byte-aligned storage for fresh owner construction, with
no live owner or admitted companion. The actual pool's `D4h` slot stride keeps
the count aligned; reuse requires prior owner retirement. No alignment repair
or validation branch is inserted into the native schedule.

## Native and C++ contracts

Complete live Ghidra and installed-PE bytes agree for `B48AF0[120]`,
`[B48AF0,B48B68)`, 36 instructions ending with `RET` at `B48B67`. SHA-256:
`bdb6bd6e8a614d43a0f50a2ddf057ca136bb3063b9b5d934a86d857d510b6efe`.
The original interface receives ECX raw `D0h` storage and returns the same
address in EAX. The reconstructed function exposes a C++ interface; it is not
a binary replacement for that ABI.

| Native instruction | Store |
| --- | --- |
| `B48B0E` | Base profile `CEB130` at `+0` |
| `B48B19` | Count1 at `+4` |
| `B48B1E` | Derived profile `D61D1C` at `+0` |
| `B48B24` | Scalar0 at `+8` |
| `B48B2B/B2E/B31` | Main header zeroes |
| `B48B4A` | Call `BF7CD1` usage-array constructor iterator |
| `B48B53` | Stride0 at `+CC` |

The new placement construction occupies that one count initialization point.
All payload stores and cleanup state changes remain in their existing order;
the trailing pool slab index at `+D0` remains untouched. The unchanged loading
provider's `actual_references()` launders the atomic at that same actual `+4`
when constructing `NativeVertexDeclarationReference`.

Constructor cleanup still reverses any initialized usage prefix, destroys the
main header, then restores the base profile. The concrete header initializer
has no throwing operation on valid storage, and the atomic value constructor
adds no throwing operation. No cleanup, destructor or pool-return implementation
changed. Live/PE compiler code `[CBF710,CBF72D)` is29 bytes: base cleanup at
`CBF710`, main cleanup at `CBF718`, and FH3 handler at `CBF723`. Its hash is
`86304e24d9c05f9a73a9fea62d278070e61ae118c79c8682112a27d2c32cbbd5`.
Those bytes establish the cleanup boundaries; this packet executes no native
FH3/SEH transport or failure case.

## Compiled production provider

The actual CMake object contains a149-byte constructor. Its relevant offsets
are base profile store `+12h`, single `MOV DWORD PTR [ESI+4],1` at `+18h`
(`c7460401000000`), and derived profile store `+2Ch`. The count initialization
adds no increment, locked operation, allocation or call. The constructor's only
call is the existing usage-array initialization helper.

The exact CMake `native_vertex_declaration_owner.obj` bytes equal the member in
the newly built `bsp_core.lib`. The fixture map resolves the constructor to that
member, and its linked executable constructor bytes equal the object bytes
apart from the one ordinary call relocation. Full object/disassembly, extracted
constructor, relocation inventory and linked-byte evidence are archived. This
proves the compiled count store order and one initialized credit; it does not
prove byte-identical native instructions or exception ABI compatibility.

## One focused production-lifetime composition

The separate ignored revision in `local/cc10_vertex_declaration_atomic_probe/`
removes all fixture placement construction. It borrows the constructor-created
atomic directly with `std::launder`, compares its actual address to the canonical
companion count, checks all four count bytes and loads value1. Before construction
it poisons only the `D0h` payload. Afterward all52 DWORDs have the exact expected
profile/count/zero values and the real trailing `+D0` slab index is unchanged.

The remaining case preserves the previously reviewed real D3D9 HAL composition:
genuine physical/logical/declaration pools, a real16 MiB DEFAULT/usage208 vertex
buffer published at renderer `+1974`, three CPU declaration records with stride28,
and `B287C0` count4/flags1000. It observes logical1/physical2/declaration2, releases
the declaration creator before logical retirement, reaches genuine logical and
declaration zero terminals, then retires the physical creator. Registrations,
all pools, allocator list, support and strings drain; COM releases finish0/0.
Secondary `19AC=0`; the optional `19B0` row remains outside the admitted case.

Strict MSVC Win32 build and all three existing CTests passed. The standalone
fixture used `/MD /fp:strict /W4 /WX` and `/MANIFEST:EMBED` with active assertions.
Exactly one focused case passed: parent PID66308 exit0 and child PID70236 exit0.
All14 exact run inputs and28 provider pins were unchanged afterward. The full
process closure reports zero matching processes. No tracked tests were added;
no additional successful-case run was performed.

The archive is `local/cc10_vertex_declaration_atomic_lifetime_evidence.zip`, with
`local/cc10_vertex_declaration_atomic_probe/manifest.json`. It includes exact
source/header, three project libraries, CMake provider object, fixture source/
object/executable/map, compiled/native evidence, receipts and full closure.
The prior composition archive, fixture directory and readiness are preserved
byte-for-byte; the previous archive retains its original source/library inputs.
The old on-disk production inputs intentionally differ after this correction.

This closes the constructor's C++ lifetime gap for the stated fresh-storage
contract. The focused case directly constructs a declaration and exercises its
canonical companion. It does not execute `B317E0` name decoding/cache admission,
the full `B4E470` caller, application/renderer startup, draw or original gameplay.
Those paths remain separate validation work. No production activation, original
game launch, guessed provider or successful cold-provider substitution occurs.
