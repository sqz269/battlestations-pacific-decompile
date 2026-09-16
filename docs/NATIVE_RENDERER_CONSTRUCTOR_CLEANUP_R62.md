# Native renderer constructor cleanup (R62)

The B32410 renderer constructor needs genuine cleanup for partially constructed
members. This packet restores its array, capability, embedded critical-section
and effect-registry providers. It does not yet invoke the full renderer constructor.

## Source and contracts

Reviewed parent-member source comes from `aa836cddc`; effect-registry source
comes from `0b3a8cac7`. Only the four owned source/header files were recovered.
The accompanying report records full commit IDs and original/current hashes.

| Address | Original entry and behavior |
| --- | --- |
| 008D4E60 | ECX resolution-pair header; signed capacity/count handling and ordinary free; pointer/capacity remain stale |
| 0086AE00 | ECX DWORD header; full 86A430 resize0 then free current data |
| 00B2F690 / 00B2F700 | ECX capability member; nested arrays at +50 then DWORD array at +44, with the original one-state unwind schedule; second entry is a jump |
| 00402F70 | ECX embedded tracked Win32 section; decrement positive depth before each Leave, then Delete; never free embedded storage |
| 00B31750 | ECX effect registry; current reverse traversal, virtual size accounting, actual release and record destruction |
| 00B317C0 | ECX record header; full B30410 resize0 then free current base |
| 00B32010 | Stack actual effect, RET4; atomic +4 decrement, canonical terminal only at zero |
| 00B320F0 / 00B32200 | ECX registry; base profile stamp, guarded clear, array destruction; second entry is a jump |
| 00B2FFE0 / 00B30410 | Existing full reserve/resize bodies shared by loading and storage-only overloads |

`NativeEffectRecordStorageContext` borrows the application's same actual string
pool, validation and array allocator. Cleanup can therefore run before the full
effect-loading graph exists. Both existing loader overloads keep their complete
behavior. The new context does not allocate a companion owner or replace growth
with a special resize-to-zero implementation.

Array headers retain native stale pointer/capacity words after destruction.
Registry traversal rereads current count/base and current profiles at the native
points. Capability cleanup arms +44 destruction while +50 is being released.
Registry destruction stamps D5F04C before arming record-array cleanup and does
not free the registry allocation. Source cleanup exceptions during unwinding
terminate; original native exception machinery remains unproved.

## Evidence and validation

- Fresh Ghidra/installed-PE parity: 25 spans, 1136 bytes, including all 12 complete
  function bodies, profile tables, imports and the two new unwind maps.
- B3003D is three bytes of unreferenced alignment skipped by the preceding jump.
  It was preserved; no listing repair was needed for this packet.
- Strict MSVC Win32 `/MD /W4 /WX /fp:strict` build and all three existing CTests pass.
- Twenty focused native/source checks cover whole scalar-array headers, empty
  and populated nested capability arrays, real Win32 section depths 0/2/-1,
  record growth/reserve/shrink and empty/populated registry destruction.
- The probe uses the real raw pool, manager, allocators, locks and atomic release.
  Copied complete native callers use documented ABI bridges to genuine compiled
  helpers; all seven physically resolved process modules are I386.
- Linked-map receipts preserve emitted source intervals and reached edges,
  including inlining and the zero-accounting leaf's ICF alias with `__matherr`.

See `reports/native_renderer_constructor_cleanup_r62.json` for contracts,
call-site checks, prior annotations, provenance and immutable local archives.
No permanent test cases were added.

## Remaining limits

These are new C++ interfaces, not drop-in binary replacements. Original FH3,
SEH, private stack/register behavior, failure injection and concurrent mutation
were not exercised. Effect fixtures use valid nonterminal reference prefixes
(2 to 1), not complete constructed effects; terminal effect deletion is open.
The full B32410 constructor, application renderer ownership, active rendering,
visual parity and gameplay remain unvalidated.
