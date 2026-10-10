# DamageableClass MshCategory retained readiness

An independent ordinary Source adapter is ready to implement under an explicit
caller-retained owner contract. Its scope is the MshCategory prefix
`[0087CEBB,0087CF35)` plus the matched normal string release
`[0087D181,0087D1A0)`: **122 + 31 bytes, 35 + 10 instructions**. These are two
disjoint fragments, totaling 153 bytes / 45 instructions. The intervening fields
remain excluded; this does not make the complete 741-byte field body ready.

The owner must survive the excluded continuation. It retains the data and length
captured at CF07/CF0D for normal release, while exceptional cleanup uses the
current actual string header. No implementation, registration, build, test or
runtime execution is included in this readiness packet.

## Retained evidence and exact boundary

Only the already accepted `[CEBB,D1A0)` field body was used. The existing full
parent file was hashed as an input artifact; only the two selected regions were
decoded and semantically inspected here. Their raw hashes are:

| Region, exclusive end | Bytes / instructions | SHA-256 |
| --- | --- | --- |
| CEBB..CF35 prefix | 122 / 35 | `72fd3a254217182df31cffba09c199636a87df779326b75c2b18c33195652be6` |
| D181..D1A0 normal release | 31 / 10 | `e1db6873f0e3018c5f041bddbbaa964522188cfa3393f7c40cdf23e19536ecd7` |

The copied 741-byte input has hash
`e32d87a567bcc8ec8d8bc689823b9155674d1ac04a4ca5548f6f586794bd2c76`.
The parent 3,238-byte hash and older 858-entry index remain provenance. The
accepted epoch10 typed receipt is historical: later parent annotation changes
were not queried. No installed PE, new physical Native bytes, live Ghidra,
callee, name/data/table or handler window was opened. In particular, 870CD0 was
not inspected; the other worker owns that wrapper audit.

The `MshCategory` spelling and D0E190 key identity come from the accepted field
metadata. No fresh key-byte hash or name-data verification is claimed.
The retained 37-state map supplies cleanup metadata, not a fresh handler-body
review. States15 and17 are also present as actual stores in the selected prefix;
state16 is present only in the map and must not be invented as an active guard.

## Actual storage and prefix schedule

S is ESP after the parent's E4h locals and four saves. The lookup uses the actual
iterator **value S+2Ch**, producing a fresh tracked Lua object at S+E0h. The raw
eight-byte MshCategory header is S+18h: length at +0, data at +4. Key S+58h,
Sections S+80h, Damage S+44h and Unique S+94h remain with their existing owners.
The selected row stays with the caller as the same ESI-derived address. Neither
selected fragment reads or writes that row, its vector header or S+DCh cursor.

The preceding row temporary's accepted state14 cleanup must already have
completed normally to13, as must the back-row selection. The Msh owner neither
rearms14 nor borrows the dead temporary. The successful iterator setup owner
and its same actual scratch remain live throughout Msh ownership.

The exact prefix schedule is:

1. CEBB..CECC: look up the retained name on actual value S+2Ch into S+E0h.
   Only after successful lookup does CED3 enter15.
2. CEDB: obtain the string through the genuine converting B662B0 getter. Capture
   the returned C-string address as the EDI-equivalent value.
3. CEE2/CEE6: clear length and data in that order. Native EBX is the established
   zero from the accepted predecessor; an ordinary Source body can express those
   zero stores without assuming a new register ABI. Stale header bytes are
   permitted before this point and are not initialized before the getter.
4. CEEA..CEF9: perform the actual bytewise NUL scan, including its wrapping DWORD
   address/count arithmetic. Do not replace it with the generic constructor's
   `strlen` schedule. Embedded NUL ends the string regardless of Lua's length.
5. CF02: resize the same raw header through genuine 41DD40 with preserve=true.
6. CF07: capture current data once. CF0B tests it; CF0D captures current length
   **unconditionally**, even when the saved pointer is null.
7. Only a nonnull captured pointer reaches BF7680, with captured data, the
   retained Lua text and wrapping captured-length+1. Preserve this call even when
   that count is zero; do not add the generic constructor's zero-count guard.
8. CF28 enters17 **before** CF30 destroys the actual S+E0h Lua object. Return from
   the proposed prefix only after that cleanup returns, keeping the string owner
   alive through the excluded CF35..D180 continuation.

There is no exact-STRING predicate or empty-string fallback here. Current Lua
`lua_tolstring` also converts a NUMBER through `luaV_tostring` and its actual
`%.14g` CRT formatting. Other types return null; the Native prefix then clears
the header and attempts its byte access. No eager guard or substitute empty
string is proposed. Ordinary positive behavior requires the real returned text
and copy ranges to be valid; Native fault/SEH identity stays held.

Use the genuine same-frame protected lookup and string adapters for the ordinary
C++ reader boundary. A converting-getter failure restores stack height but can
retain a TValue already changed to STRING by a conversion followed by failing
GC/finalizer work. Cleanup must operate on the actual field's current tracked
owner/index. It must not restore the old TValue or a copied Lua object/index.

## A fixed owner across the excluded continuation

The minimal future Source shape is a fixed, noncopyable/nonmovable owner created
before its one permitted prefix call, inside the successful iterator owner's
lifetime. It borrows actual S+E0h, raw S+18h, the same actual iterator scratch and
the genuine raw string-pool context. It retains the captured data/length as
stable Source values, exposing the captured data to later category work without
rereading the header. Those are explicit Source interface values, not a new
Native string type or a copied Lua/container owner.

No ordinary C++ `NativeString` object need be started over the raw eight bytes;
the existing actual-header APIs accept raw storage. No generic string
constructor/destructor, auto-closing successful local, continuation callback or
new iterator API is needed. The fixed owner survives all later inner field and
handle guards. Only after those have normally returned to17 does the caller
invoke its matching normal release. Reopening, repeated prefix calls and retry
after a partially completed operation are outside the contract.

| Owner state or operation | Required cleanup behavior |
| --- | --- |
| initial13 / lookup failure before15 | No Msh field or string cleanup; existing outer owners handle propagation. |
| active15 / getter, scan, resize or copy failure | Lower ordinary ownership to13 and destroy only actual Lua S+E0h. Do not introduce16 or release the raw string, even after partial allocation/header writes. |
| active17 / field-destruction or later inner failure | Lower ordinary ownership to13 and call genuine raw41DD20 on **current S+18h**. The field already released at CF30 is not retried. |
| matching normal release | Test **captured data**, lower17->13 at the D183-equivalent point, then use the captured data/length path below. |
| normal release getter failure after lowering | Do not retry current-header or captured-data release. Existing outer owners handle propagation. |

State15->13 and17->13 exceptional eligibility comes from the retained map and
existing Source cleanup bodies. The map does not establish original handler
instruction timing. An ordinary noexcept owner destructor has the existing
secondary-failure termination boundary; Native FH3/SEH/longjmp and double-
exception behavior remain unclaimed.

Normal D181..D19F first tests captured EBX, then lowers17->13. Null skips every
pool operation, even if later work changed the actual header to nonnull. For
nonnull data, capture the wrapping EBP+1 release size before the getter, call
current `native_string_pool_get_or_create_00419cc0`, and pass its returned owner
to `return_native_string_pool_00bd1510` with the saved block/size and actual
01090AA4 reference. The Native PUSH1 is BD1510's retained unused argument; it is
not permission to replace the live return gate with literal1. Resolve the getter
even for a large block or disabled small returns. Leave the actual header and
all callback changes untouched. No release is retried after the state drops.

Exceptional17 cleanup differs: raw41DD20 reads the **current** header data and,
only when nonnull, its current length. It then resolves the current getter.
Thus a captured nonnull buffer with a later null header is skipped exceptionally,
while normal release still uses the saved buffer. These two methods cannot be
collapsed into one `close` routine called by both normal and exceptional paths.

## Services, alias obligations and remaining limits

All selected direct Source services exist: genuine protected named lookup,
protected converting string getter, actual Lua destruction, raw41DD40,
raw41DD20, current raw419CC0 and actual BD1510. The current raw pool composes
BD1120/BD12A0, the canonical raw manager getter/registration and real allocation,
critical-section and CRT services. Their current Source and dependency frontier
are frozen; this packet does not re-admit every transitive helper or any Native
callee. Existing provider bodies remain unchanged.

Use `NativeStringRawPoolContext` with the application's actual AA8 pool
publication, AA4 mutable return gate and AA0 raw-manager publication. Each
operation must observe their current values, with genuine raw pool/manager
construction and the required shutdown deletion binding. No fresh private
publication cells, host semantic pool, callback facade, default allocator or
cached pool substitutes for that domain. This packet does not prove a concrete
DamageableClass application binding or install a shutdown binding.

The `NativeStringStorage::release` interface is noexcept. Its actual-pool bridge
can terminate if lazy getter recreation throws, so it covers a stronger domain
than the raw-context path. It must not silently replace the latter or erase the
normal-release failure schedule. Existing Comment composition demonstrates the
raw-context interface; its exact-string/default behavior is not this field's
converting-getter behavior.

Require stable, disjoint actual frame slots and Source owner/captured storage,
with proper alignment and valid extents. Keep Source private capture/state
storage separate from provider arguments, publication cells and callback-visible
actual buffers. Callback writes to the actual string header remain observable;
they cannot rewrite the saved EBX/EBP-equivalent values. The retained Lua text
must stay valid through scan/resize/copy. Valid source/destination overlap is
permitted by the existing memmove boundary. Keep the existing Lua capacity,
tracking-address, current-index and inherited error-handler placement contracts,
including through field removal and surrounding cleanup. No alias repair,
copied object, row reselection, early release or rollback is added.

BF7680 remains the already qualified host-CRT/memmove boundary. No address-matched
Native body is newly supplied. Existing resize retains its documented zero-byte
preserve-copy omission; the new caller must still preserve its own nonnull
zero-count copy call. Current CRT formatting, byte-scan faults, complete Native
ABI/FH3, concurrency, invalid storage, production composition and runtime proof
remain separate limits.

The concrete next step is a separately authorized four-file Source packet for
this 153-byte disjoint owner contract. It should use actual `/Oy-` Win32 build
options and inspect the complete candidate/provider objects: getter-before-zero,
pointer-test-before-unconditional-length, copy-call eligibility, state17 before
Lua cleanup, saved-data test before state13 and current-getter release must all
survive compilation. It must preserve the distinct current-header unwind path.
No new Native window is needed to implement that bounded ordinary proposal.

The 741-byte field implementation and 870CD0/effect binding work remain outside
this packet. Root Source636's 636 selected inputs, 89 Core / 3 App objects,
233 selected positive definitions and three existing checks are pinned context
only, not a full-project count or new execution evidence. No C++ or shared
metadata changed, and no build, test, probe or runtime check was run.
