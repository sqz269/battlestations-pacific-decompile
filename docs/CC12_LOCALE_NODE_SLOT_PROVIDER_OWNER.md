# Locale node-slot provider and owner readiness

`00736090` is the matching raw locale node-slot return provider. The complete
`0073C220` wrapper proves that its actual fixed owner base is `00E1768C`.
The provider and wrapper contracts are closed at the Native-body level; actual
owner initialization/lifetime and a canonical Source binding remain open.
**Raw-map composition C++ GO and application GO remain false.** No C++ is added.

Fresh read-only evidence covers complete105-byte `00736090..007360F8` (36
instructions), complete32-byte `0073C220..0073C23F` (11 instructions), and56 bytes
of initial image data at00E1768C:193 fresh raw bytes, comprising137 code and56
data bytes. The previously accepted map/node/EH409 bytes are reused separately.
Two owner-reference sites receive size/prototype, typed listing/function and
single-instruction metadata only. There are17 `bsp.py ghidra` query commands,
with no Ghidra edits, body/listing/prototype/flow repair or third owner site.

The existing `C:/Users/sqz269/bsp.gpr` and `/battlestationspacific.exe` are verified
by the typed client. Complete retained typed captures have modification67 before
and after. Ghidra ranges match the original installed executable, whose hash is
unchanged: SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
A read-only copy of that12,223,752-byte image is included for independent hash
and disk-range replay. Copying it does not expand analyzed function scope.

Both stored prototypes are `undefined ...(void)` and remain untouched. Assembly
establishes the register and stack contracts:

| Body | Actual inputs | Return and preservation |
| --- | --- | --- |
|00736090 |ECX=actual pool; one stacked DWORD slot pointer at incoming ESP+4 |RET4; preserves EBX/EBP/ESI/EDI. No meaningful EAX result established. |
|0073C220 |ECX=original node; one stacked DWORD flags at incoming ESP+4; tests bit0 of its low byte |RET4; preserves ESI; EAX=original node address, including after slot return. |

The provider captures ECX pool in ESI and computes section=pool+0Ch in EDI. It
calls the current EnterCriticalSection import at00CE2218, increments the current
depth DWORD at section+18h, and only then reads the stacked slot pointer, current
slot+18h block index, current pool+28h table pointer and current table entry.
None of those slot/table values is cached before Enter.

It subtracts block address from slot address modulo2^32 and applies the signed
92492493 IMUL/high-plus-input/SAR4/sign correction: signed division by28 with
truncation toward zero. It stores the low16 quotient through the current block's
WORD free-index stack at7000h using the captured unsigned16 free count at7800h.
It then increments the **current** count WORD at7800h, after the index write.
JNC/JAE compares the saved block index against the current unsigned minimum at
pool+34h and lowers that minimum only when smaller. Finally it decrements current
depth and calls the current LeaveCriticalSection import at00CE2210.

There is no payload cleanup, link rewrite, direct CRT free, null/capacity/range
check or local FS/EH/unlock guard in this provider. Slot return is a descriptive
interpretation of the evidenced free-index publication, not a recovered symbol.

`0073C220` first retains the original node in ESI and calls00737CE0. Only after
payload cleanup returns normally does it test byte[ESP+8],1; the saved ESI explains
the offset from incoming flags at ESP+4. A clear bit skips node-slot return.
A set bit pushes the original node, executes `MOV ECX,00E1768C` at0073C230 and
calls00736090. Other flag bits are ignored. It returns the original node address
bits, which are not a guarantee of a still-live dereferenceable object. An
escaping payload failure precedes the flag test and slot return; an escaping
provider failure prevents the final return path. No wrapper-local rollback is
installed. Scalar deleting-destructor shape is descriptive, not a new owning
C++ type or proof of direct heap deallocation.

The literal base and relative provider accesses establish these exact roles:

| Role | Base-relative offset | Actual address |
| --- | --- | --- |
| Critical section |+0Ch |00E17698 |
| Current depth DWORD |+24h, also section+18h |00E176B0 |
| Current table-publication DWORD pointer |+28h |00E176B4 |
| Current minimum DWORD, unsigned comparison |+34h |00E176C0 |

The last access requires at least38h bytes; it does not prove the complete owner
size or the role/initialization of intervening fields. The selected block uses
1Ch slots, block identity at slot+18h, WORD free indices at block+7000h and a WORD
count at+7800h. Return alone does not prove allocation extent, initial capacity,
table count/capacity fields or constructor-produced slot identities. The inspected
56-byte prefix is zero in file-backed `.data` and Ghidra. These are initial-image
bytes, not an initialized critical section or a usable zeroed allocator.

With the now-proved00E1768C binding, the provider matches the accepted map's
inline73C27A..73C2DC data schedule: same section, current depth, post-Enter node
index/table/block reloads, signed quotient, low16 index, current count increment,
unsigned minimum and final depth/Leave. The map separately saves next before
payload cleanup, returns the slot unconditionally after normal payload return,
and performs its final map clears; it does not need the scalar wrapper's flags.

There is one material import-identity qualification. The map caches Enter's
00CE2218 target once at map entry, whereas00736090 reads that IAT on each call.
Both reload Leave's00CE2210 target per node return. Factoring through the provider
therefore requires stable imports; arbitrary IAT rebinding/hooking is outside
equivalence. The provider also has its own ECX-pool/stacked-slot frame, rather
than the map's register-held node. A normalized interface does not establish
stack aliasing, register, native fault-boundary or binary ABI parity.

Fixed-cell xrefs show the map's section literals, depth RMWs, table read and
minimum compare/store. Relative provider accesses need not have fixed-cell
xrefs. Base00E1768C has references at007373F1,0073C230,00A9FCDD,00CC8D30 and
00CD9410. The unrelated wrappers/insertion body are not expanded here.

The two bounded owner-reference candidates are unresolved:

| Site | Exact current instruction | Typed function/body state |
| --- | --- | --- |
|00CC8D30 |5-byte MOV ECX,00E1768C; ordinary fallthrough00CC8D35 |No function at or containing query; no function body ranges. |
|00CD9410 |5-byte MOV ECX,00E1768C; ordinary fallthrough00CD9415 |No function at or containing query; no function body ranges. |

Their size/prototype queries return no function/signature. Complete typed metadata
confirms no flow override, but does not establish a routine extent or subsequent
target. Startup/teardown roles are provisional leads from their locations. No
constructor, destructor, atexit registration, critical-section initialization,
allocator-list/profile registration or publication lifetime is inferred from the
MOV instructions. No body expansion or repair is performed after this boundary.

A usable return requires the same actual initialized owner, alive section/cells,
current table/selected block, and a valid unique aligned allocated1Ch slot with
coherent block identity and available free-index capacity. Preserve DWORD wraps,
signed displacement interpretation and WORD truncation/increment; use unsigned
address arithmetic instead of subtracting unrelated C++ pointers. Do not invent
range guards or replace current-count RMW with cached-count+1 under possible
aliasing. Admit valid extents and nonoverlapping metadata; exclude unsafe reentry
or concurrent ownership mutation. The node section protects return metadata, not
payload callbacks or map traversal. A failure after Enter can leave a held section,
changed depth or partial metadata. No noexcept, retry, rollback or SEH recovery
promise is justified.

Payload layout stays as corrected: narrow length+0/pointer+4 and wide
length+8/pointer+Ch relative to the node. This provider does not read those fields.
The accepted00737CE0 helper and existing canonical string context remain separate
from the node-slot owner. A private pool, foreign allocator or zeroed storage
cannot supply the missing actual owner lifecycle.

The current Source/Git freeze is `1232117fcb9ba549faa23a7bdb809297a8ea3a31`.
All4215 retained current inputs match committed Git bytes;4208 are tracked files
under `include/` and `src/`. The3354 selected inputs differ from the prior worker
build in exactly four canonical-game-free files;3350 remain byte-exact. The old
worker artifacts are unchanged but do not compile those four current changes.
No new build or test is run. The finite Source search identifies no exact736090/
73C220 or E1768C-authority implementation; it is not a universal semantic-absence
proof. Corrected readiness `a2dd1792d`, its erroneous historical `5cd08cb76`
version, and the accepted map/node primary dependencies remain separately pinned.

The next exact owner frontier is bounded entry/body-target readiness for00CC8D30
and00CD9410, followed only by separately admitted initialization/destruction
providers. Preserve their unresolved body state until sufficient evidence is
available; any repair requires separate authorization. A future dormant return
helper can use this105-byte contract with an explicit already-initialized owner
precondition. That would still not establish canonical binding, node allocation,
owner startup/teardown, map ownership or application activation.

The finite proof includes the complete selected Native bytes/assembly/typed
metadata, original-image hash and disk ranges, current and committed Source,
actual committed dependency bytes and the old/new cohort distinctions. Strict
ZIP-only read-only review and physical-copy replay both pass, including deliberate
denial of an external read. There is no reconstructed or Native runtime execution.

Evidence: `reports/cc12_locale_node_slot_provider_owner.json` and
`local/cc12_locale_node_slot_provider_owner/evidence.zip` (19,095,714 bytes,
SHA-256 `8826ac783963cd349f802c0ee9e561aef0f04e4cf052c75d2ccff53a0a5c00a0`).
