# Native renderer resource removal: next complete packet

The smallest ready implementation packet is the **complete native record storage
destructor `00B2F990`, extent `[00B2F990,00B2FA08)`**. Its prerequisites already
have concrete pool, node-free and list-clear behavior in the repository. This
packet removes one real ownership dependency from renderer name removal. It does
not implement record assignment, the container, or the full texture owner.

This read-only audit starts from `d344880`. All 38 selected code/data spans in
`reports/native_render_resource_removal_next.json` match current Ghidra memory
and the installed PE. Each live query verifies `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`. No code, shared metadata or Ghidra state changed;
no build, fixture, injected exception or game validation is claimed.

## Dispatchable packet

Packet ID: `native_render_resource_record_storage`. Own exactly `00B2F990` and
its full 120-byte extent. Proposed descriptive name:
`BSP_RenderResourceRecord_DestroyStorage`; this is a hypothesis, not a recovered
symbol, and has not been applied. Original ABI is ECX = actual 2Ch record,
no EDX input, no stack arguments, `RET 0`, no semantic return value.

Use dedicated `include/bsp/native_render_resource_record.hpp` and
`src/native_render_resource_record.cpp` plus focused evidence artifacts. The
integrator owns build registration and shared ledgers. Reuse the actual shared
string-pool and native lifetime allocation domain. The existing implementation
at `src/particle_clock_lifetime.cpp:29` demonstrates concrete list/name cleanup;
its particle sink interface must not be imposed on this resource record.

| Record offset | Established storage behavior |
| --- | --- |
| +00h, +04h | Owned name length and pooled buffer |
| +08h | Unknown list word; neither this destructor nor assignment changes it |
| +0Ch | Actual linked-list sentinel pointer |
| +10h | Alias count |
| +14h through +24h | Five payload DWORDs; assignment copies them in order |
| +28h | Resource pointer; this destructor never reads or releases it |

Each actual 10h alias node is next, previous, string length, string pointer.
`004D05E0` receives record+8, captures the first node, resets both sentinel
links to the sentinel and sets count to zero before freeing nodes. Each loop
captures next before returning its nonnull string to the sized pool with
length+1, then ordinary-frees the node. The post-free loop compares the captured
next pointer with the current sentinel. There is no replacement container.

`00B2F990` reloads and ordinary-frees the actual sentinel, writes only
record+0Ch = 0 at `00B2F9C9`, then reloads name+04h and returns its nonnull
buffer with current name length+1. Name length/data, +08h and +14h through +28h
remain unchanged. It does not free the 2Ch record storage itself. Calling
`NativeString::release_to` directly would incorrectly clear the name fields.

The saved Ghidra body stops at the false no-return call to `_free` at
`00B2F9C4`. The full post-free tail ends with `RET` at `00B2FA07`.
The existing `004D05E0` saved body also omits its post-free loop tail; this audit
checks the complete bytes through `004D0632`. Implementation annotation must
retain these tails and use the required Ghidra write lock, metadata backup,
refreshed export and project save.

`00B2F990` has one EH state. FuncInfo `00DF6100` and unwind map `00DF60F8`
route state 0 to `00CBD840`, which calls pooled-string destruction `0041DD20`
on the saved original record. That state covers alias-list clear and sentinel
free. State -1 is installed at `00B2F9D8` before normal name-buffer release.
This is static exception-map evidence; typed host cleanup does not establish
binary-compatible native exception dispatch.

Build with `scripts/build.ps1` and use relevant existing ownership checks.
Only add one focused lifetime case if needed to exercise actual node/sentinel
and name ownership while checking untouched payload words. Neither a synthetic
resource callback nor an intrusive sink release belongs in this packet.

## Actual renderer and container route

Primary renderer profile `00D5F0A8` slot +6Ch at `00D5F114` contains
`00B32250`. Its complete 240-byte body is still absent from Ghidra's function
table. ABI: ECX = entry renderer, one stack native-string pointer, `RET 4`.

The entry gate reads `0108D6DC`. If enabled, it captures current renderer
publication `00F8D394` for optional-guard enter `00B33AD0`. It then creates and
lowercases a real temporary name, but passes the **original input string
pointer** to `00B31DC0` on the **entry renderer's** +1A74h container. Normal
cleanup releases the temporary data pointer captured in EDI, using current
temporary length+1. Guard exit rereads the gate and uses the saved guard owner
and enter result. These renderer identities and gate observations must stay
distinct. FuncInfo `00DF6694` gives state 1 temporary-name cleanup through
`00CBDCE8`, then state 0 saved guard cleanup through `00CBDCE0`/`00B21110`.

`00B31DC0`, extent `[00B31DC0,00B31FC8)`, takes ECX = actual container and
one stack native-string pointer, `RET 4`. Container+04h holds the array,
+08h the count and +10h the accounting total. It makes a raw local name copy;
`00BEE780` copies and normalizes a separate value that is immediately destroyed.
The alias comparison still uses the original local copy: equal stored lengths,
then `_stricmp` for nonempty strings. Do not substitute a normalized key.

The scan captures array begin and the count-derived end once, then walks actual
2Ch records and their sentinel lists. On the first match:

1. Retain the matched record address. Load its current resource+28h, current
   vtable and current virtual +0Ch, then call with ECX = resource and no stack
   argument. Subtract EAX from the **current** container accounting word.
2. Read the matched record name after that callback for diagnostics. Reload
   array and count after the diagnostic to compute the final record. If its
   address differs from the retained match, assign it through `00B30510`.
3. Reload array and count **again after assignment**, destroy the now-current
   final record through `00B2F990`, then decrement current count. Preserve the
   array allocation and capacity.

The container's EH state 0 only destroys the local name via
`00CBDC60`/`0041DD20`; it does not roll back accounting, assignment or count.
There is no intrusive resource decrement or COM release in this path.

For the actual 2D texture profile `00D61948`, virtual +0Ch resolves to the
previously unrecognized four-byte getter `[00B3CE30,00B3CE34)`:
`8B 41 24 C3`, `MOV EAX,[ECX+24h]; RET`. It returns the actual stored source-size
word. This getter is independently ready, but cannot stand in for the complete
texture owner or for other resources' current virtual dispatch.

## Why record assignment is a later packet

`00B30510`, `[00B30510,00B305B2)`, takes ECX destination, a stack source,
returns destination in EAX, and `RET 4`. It copies the name, captures source
sentinel and first node before clearing the destination list, invokes
`004D26A0`, then copies payload +14h through +28h. It has no local EH handler.
The existing material `effect_cache.cpp` uses host `std::vector/std::string`
storage; its named destructor `00B2FA10` is not a reusable concrete native
storage implementation.

The complete alias insertion contract includes dependencies absent from the
normal pseudocode:

| Entry | Required behavior still to implement/review as a complete packet |
| --- | --- |
| `0044BCB0` | Native string copy construction: ECX destination, EDX source, zero fields before self-copy check |
| `004CE6F0` | Allocate real 10h node and copy its string; `RET 0Ch`; catch `004CE75C` frees captured raw allocation and rethrows |
| `004CE780` | Check count against `1FFFFFFFh` and increment it; concrete length-error construction/throw, not only its named normal path |
| `004D26A0` | Seven stack DWORDs, `RET 1Ch`; checked owner/node iterators, link insertion and catch rollback |
| `004BE820`, `004BECC0`, `004B9FF0` | Checked inequality, previous and next; preserve validation behavior |
| `004D0990` | Unlink and free an actual pooled-string node, decrement count after free and return the next iterator |

Node allocation and string copying happen **before** the count-growth check;
link writes follow that check. Node construction's catch does not cover a later
count-growth exception. Range insertion FuncInfo `00D8EB30`, try map
`00D8EB1C` and catch descriptor `00D8EAFC` select handler `004D2746`. It compares
saved initial and current source iterators, repeatedly erases the last inserted
destination node while advancing the saved source iterator, then rethrows.
This rolls back completed insertions, not the already-copied record name.

The report retains the exact catch maps, both catch bodies, and post-free erase
tail. The iterator-failure routine `00BF6713`, native exception construction,
copy-normalization helper and concrete current owner bindings remain explicit
prerequisites. A normal-flow-only list, guessed standard-library container, or
no-op renderer notification cannot close full texture-owner lifetime work.
