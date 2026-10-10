# DamageableClass first section-field readiness

The requested first-field packet is **held**. Its smallest retained prefix is
`0087CEBB..0087CF34`, a 122-byte extent / 35 historical listing instructions.
It reads `MshCategory`, constructs the eight-byte string at S+18h and destroys
the temporary Lua value. That cleanup leaves parent **state17**, which still
owns the string. The next field begins at `0087CF35`; this prefix therefore
does not close back to state13 before the next field.

Only this document and
`reports/cc12_damageable_class_first_section_field_readiness.json` are added.
There is no Source implementation or proposed Source packet that prematurely
ends this string lifetime.

## Evidence and exact scope

A file-only lease was extended to CEBB..CF34 before focused parsing and Source
matching. Evidence comes solely from the previously retained 858-instruction
parent listing and existing 37-state report. Their retained parent extent is
3,238 bytes and historical SHA-256 is
`00d679920bd1b8f8653e9a0ab1de627f954cfadaf48b8bf0642676fde5162808`.
All 858 historical addresses still match the earlier receipt.

The retained worker receipts do not contain the whole raw parent byte buffer or
raw bytes for this new selected prefix. Root confirmed its Source627 replay
checked the installed parent but did not freeze that buffer. Accordingly, 122
bytes is the extent between retained instruction starts, and 35 is the count of
retained textual instructions. This packet supplies **no fresh Native byte hash
or decoder replay** for CEBB..CF34. Its text-schedule hash is labeled separately.
The old installed-PE verification is historical evidence, not a new byte read.

No current shared export, installed Native bytes, header/data/name, callee,
handler or body window was opened. No live Ghidra query or annotation refresh
was made. Root's later parent annotations do not refresh this historical receipt.
CF35 and the later row fields/cleanup bodies remain outside the leased prefix.

## Retained storage and ordered schedule

S denotes ESP after the parent's E4h locals/four saved registers. The preceding
fragment supplies the actual selected row in ESI and retained actual header/
S+DCh bindings. This prefix neither dereferences nor publishes a field in that
row. Its eventual MshCategory row+4 interpretation is retained parent metadata;
the actual category-index conversion/store is later and excluded here.

The actual iterator value is the live Lua object at S+2Ch, with key at S+58h.
The new field result occupies fresh aligned 14h bytes at S+E0h, disjoint from
S+DCh and existing live slots. The actual string header is eight raw bytes at
S+18h: length at +0 and data pointer at +4. Existing Sections S+80h, Damage S+44h,
Unique S+94h, iterator ownership and the selected row/header identity remain
unchanged. Native EDI is repurposed for the returned C-string pointer, so an
ordinary interface must not imply that EDI still holds the vector header.

| Retained site | Observable schedule |
| --- | --- |
| CEBB..CECC | Use retained key pointer D0E190 (`MshCategory` in existing metadata), actual S+2Ch value and fresh S+E0h; call B67800. |
| CED1..CEDB | Pass the actual returned field object, enter state15, call B662B0 without a type predicate or default. |
| CEE0..CEE6 | Capture the returned pointer in EDI, then zero S+18h/S+1Ch using EBX. |
| CEEA..CEF9 | Bytewise scan through the first NUL and compute unsigned32 length; no null/fallback check. |
| CEFB..CF02 | Call actual-header resize41DD40 with that length and preserve=true. |
| CF07..CF1E | Capture current data in EBX, test it, read current length into EBP even on the null branch; if nonnull call BF7680 with captured data/text and wrapping length+1. |
| CF21..CF30 | Address actual field S+E0h, set state17 before B67700 cleanup. |
| CF35 excluded | The next field begins while the first field's string remains live. |

The existing retained predecessor records establish `XOR EBX,EBX` at CE02.
Its preservation through calls is part of the original nonvolatile-register
contract, not a fresh register-ABI proof. The byte scan does not use Lua's string
length: embedded NUL terminates the copied text. Empty text leaves the zeroed
header unchanged through the equal-length resize path and skips BF7680.

There is no x87 instruction in these 35 retained operations. Native arithmetic
is integer32 and string scanning/copying; later numeric row conversion is
outside this prefix. Lua numeric-to-string conversion is a genuine provider
boundary, with its own format/CRT/locale behavior, not evidence of identical
Native floating-point or exception behavior.

## Lifetime gate

| Retained state | Parent | Cleanup |
| --- | --- | --- |
| 15 | 13 | C9699C releases Lua field S+E0h through B67700. |
| 16 | 15 | C969A4 releases string S+18h through 41DD20. |
| 17 | 13 | C969A4 releases the same string S+18h through 41DD20. |
| 18, next field | 17 | C969AF releases the next field's Lua object. |

Only writes to states15 and17 occur in the selected ordinary schedule. State16
exists in the retained unwind map; this packet does not invent a normal state16
activation or an extra completed-string guard during resize/copy. Before CF28,
the recorded ordinary state is15. CF28 removes the field from the active cleanup
chain before its destructor is called, while retaining string ownership as17.
Failure of that explicit cleanup therefore uses the existing17->13 chain and
must not retry the field merely because its destructor did not return.

The retained ordinary-state metadata's first subsequent write of13 is D183,
after intervening fields. That metadata is enough to reject a first-field
closure before CF35. It is not a new review of the later cleanup body or proof
of a complete widened lifetime endpoint. Ending a C++ string guard at CF35
would release the string too early; keeping state13 there would lose its owner.

## Genuine current Source dependencies

The complete relevant bodies were compared in bounded named files. Whole-file
pins and the full outputs of eight bounded searches accompany the report.

- `native_lua_get_by_name_00b67800` uses the real Lua stack and constructs/tracks
  the actual output object. Its existing protected adapter encloses checkstack,
  key creation and lookup in the same Lua C frame, restores entry height on Lua
  failure and throws the existing `NativeLuaOperationError`. Capacity, stable
  owner/index/slot identities and inherited error-handler placement still apply.
- `native_lua_string_00b662b0` calls actual `lua_tolstring`; its existing protected
  adapter retains an already performed conversion if later GC fails. Current
  Lua5.1.1 `lua_tolstring`/`luaV_tostring` bodies show number-to-string conversion
  of the actual TValue, using configured `%.14g`, allocation/GC and index reload.
  Non-string/non-number values return null. Neither an exact-string predicate
  nor a default would preserve this caller's behavior.
- `destroy_native_lua_object_00b67700` invokes the real tracked release and then
  clears current kind only. Release can remove a stack slot and adjust other
  tracked indices; copied Lua objects or cached indices cannot replace it.
- Raw `resize_native_string_header_0041dd40` and
  `destroy_native_string_header_0041dd20` bodies exist through the real
  `NativeStringRawPoolContext`. The complete getter/allocation/return bodies
  resolve the actual pool/manager publications and return gate. Cleanup leaves
  header bytes untouched. A semantic CRT allocator or `release_to` reset is a
  different contract. Current Lua/string/pool files match their retained Comment
  Source pins; that is source identity, not a new compiled or runtime test.

The existing actual-header string constructor is not a drop-in proof for this
inlined schedule: it uses `std::strlen`, reads length only after a nonnull data
test and suppresses zero-count copy. The retained prefix performs its own byte
scan, unconditionally captures length after testing data and calls BF7680 on
the nonnull branch even if wrapping length+1 is zero. The raw Source constructor
uses `std::memmove` at a qualified CRT boundary. The bounded Source/header search
finds no separately named BF7680 body; this is not a claim about every possible
copy implementation. The older whole-parent receipt likewise labels BF7680 a
CRT memcpy boundary. No Native callee was opened to expand that evidence.

Existing MshCategory Source occurrences include metadata and a different host
script path. They do not establish this actual Lua/header/string-pool receiver
composition. Concrete production bindings and the complete cross-field string
owner lifetime remain held. No fabricated type, wrapper, pool or callback is
introduced to fill them.

## Readiness result

No complete first-field schedule returns to13 before the next field. A future
Source packet is therefore not proposed here. It first needs an explicitly
bounded lifetime contract retaining S+18h across its real consumers and matched
cleanup, plus the appropriate retained byte evidence and actual bindings.
This packet neither opens nor claims those subsequent regions.

Source627's 627 inputs, 84 Core / 3 App objects, 225 definitions and three checks
are pinned Root context only. This packet adds no Source count, C++, CMake,
provider changes, ledger entries, GPR writes, build, test or probe. Native ABI,
FH3/SEH/longjmp/fault identity, whole-parent completion and runtime remain held.
