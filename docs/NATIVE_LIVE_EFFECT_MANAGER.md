# Native live-effect manager construction and list storage

Packet `orch3_live_effect_manager_constructor_t` implements four complete native
functions in `src/native_live_effect_manager.cpp`, with actual Win32 storage in
`include/bsp/native_live_effect_manager.hpp`. Descriptive names are hypotheses.
Existing `BSP_EffectManager_Construct` is retained with appended evidence.

| Span (exclusive end) | Original ABI | Behavior |
| --- | --- | --- |
| `004C3200..004C321A` | No consumed inputs; EAX sentinel; RET | Allocate and self-link a 0Ch list sentinel |
| `004C5940..004C5988` | ECX 0Ch list header; RET | Detach/free all nodes, free sentinel, clear head |
| `004B7F50..004B7F61` | ECX actual manager; RET | Clear F8765C and restore base identity CE3818 |
| `004CF700..004CF761` | ECX actual 28h owner; EAX same owner; RET | Construct the live-instance manager |

The manager is published at `00F8765C`. Its +00 native identity is `00CE789C`;
+04 is an untouched allocator word. The list's sentinel is at +08 and its count
at +0C. Two actual pointer-array headers occupy +10 and +1C. This owner
is distinct from the 8h insertion-lock owner at F87650 and the 10h effect-definition
owner at F87664. The header reuses the physical pointer-array storage established
in `docs/LIVE_EFFECT_INSERTION.md`; no replacement list or duplicate count exists.

Subsequent complete destructor evidence in `docs/LIVE_EFFECT_MANAGER_LIFETIME.md`
corrects the earlier ownership interpretation: +10 releases its contained owners,
whereas +1C only frees/clears backing during destruction, preserving count/capacity.
The constructor's zero stores alone did not establish identical ownership policies.

Each 0Ch list node contains next, previous and raw payload pointers. Sentinel
creation allocates 0Ch through the canonical allocator, writes the two self links,
and leaves payload+08 untouched. Native ECX is unused. The independent tests of
the allocation address and address+4 are preserved; a null allocation is not a
successfully constructed sentinel. The normal allocator domain returns storage
or throws. Malformed-address fault behavior is not a supported C++ interface.

The manager constructor publishes CE789C before allocating the sentinel, then
publishes that sentinel, count zero, and the six zero array fields in order.
It preserves the allocator word and does not publish the global singleton.
Its handler `00C65C68..00C65C72` selects `00D8E698`, whose one-entry unwind map
at `00D8E690` is state 0 -> -1 through `00C65C60`. That funclet loads the saved
owner and jumps to `004B7F50`. Consequently allocation failure unconditionally
clears F8765C and restores CE3818, preserving all other preexisting owner bytes.
There is no constructed-list cleanup in this constructor's unwind state.

List destruction captures the original first node, self-links the current head,
and publishes count zero before freeing anything. It captures each next link
before freeing that node, compares against the current head, and finally frees
the current sentinel. It then clears the head field; the allocator word remains
untouched. It does not inspect or destroy payloads. Pending-object dispatch must
be performed by the separate flush operation before whole-manager destruction.
This helper requires a valid finite ring and introduces no lock or recovery path.

## Evidence and validation

Analysis used `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. All four
complete body spans and the constructor handler matched the installed executable.
Their hashes, saved annotation history and fixture details are in
`reports/native_live_effect_manager_constructor.json`.

The incorrect CALL_RETURN override after the list-node free at `004C5963` was
removed under the write lock. `reports/native_live_effect_manager_flow_t.json`
records the restored 11-byte loop continuation. Ghidra's stored function body
still ends at `004C597B`; it excludes the 12-byte tail at `004C597C..004C5988`,
which adjusts ESP, clears the head, pops ESI and returns. The tail was checked
against live and installed bytes and included in reconstruction and the original
fixture. Zero remaining interior call gaps does not mean the stored body includes
this tail. The handler definition/save is recorded separately in
`reports/native_live_effect_manager_definitions_t.json`.

Strict `scripts/build.ps1` and both existing CTests passed. Ignored
`local/live_manager_probe_t.cpp` executes the four original bodies alongside the
reconstruction. It compares all 28h owner bytes with only the allocated sentinel
pointer normalized, self links, unchanged sentinel payload and allocator word,
empty and three-node ring destruction, exact free order, ring detachment/count
zero before each free, final null head, and unchanged payload objects. Original
base cleanup also verifies unconditional global clearing even when it points to
a different owner. No permanent tests were added.

Copied native allocator/free calls use local bridges to canonical allocation/free;
the constructor calls the copied sentinel routine. The base routine's absolute
F8765C operand is rebound to the fixture's actual publication cell. The ignored
fixture build recompiles this source module with allocation/free symbols rebound
to those same bridges. They fill allocated bytes for preservation checks, inspect
state before real frees, and inject one C++ `bad_alloc`. That failure verifies
base identity, cleared global and untouched remaining preimage. Production has
no added allocation seam. Original EH handler immediates are unchanged and
original exception dispatch was not executed.

Subsequent `docs/EFFECT_DELETION_QUEUE.md` and
`docs/LIVE_EFFECT_MANAGER_LIFETIME.md` cover queue operations and complete manager
registration/destructor/scalar composition. Concrete effect virtual dispatch and
whole point-effect construction remain separate work. These new C++ interfaces
are not binary replacements; there is no gameplay validation from this packet.
