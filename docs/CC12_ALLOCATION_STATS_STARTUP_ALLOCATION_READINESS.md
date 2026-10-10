# Allocation statistics startup allocation readiness

The startup prefix now identifies the explicit zeroing of EDI, its exception
registration frame and the physical state/sentinel slot used around the
twelve-byte allocation and derived construction. The post-call interval restores
that slot to minus one. Original dispatcher/frame behavior and allocation-failure
cleanup are separate prerequisites; neither follows merely from these stores.

This packet starts from worker commit
`f1a01bb996b5c4fcd10e40b64dee8b26492894c8`, whose primary acceptance was pending
when this independent packet began. That packet and its immutable evidence bundle
remain unchanged. Only this document and the paired report are owned here.

## Exact ordinary prefix

Fresh typed metadata identifies `BSP_Application_Initialize@0073D410`, non-thunk
and `no_return=false`, with a complete 4,372-byte AddressSet in two ranges.
The metadata prototype is `undefined BSP_Application_Initialize(void)` and is
not ABI proof. The first instruction begins at `73D410`; the instruction
containing `73D45C` begins at `73D458`, is five bytes long and ends exactly at
`73D45D`. The separately authorized `[0073D410,0073D45D)` prefix is therefore
77 bytes. All original/live bytes agree, and its 24 offline-decoded instruction
starts and lengths match current typed listing metadata. No whole startup
listing, pseudocode or enclosing byte range is opened.

The prefix pushes minus one, pushes the actual handler operand `00C86CBA`,
saves the previous FS frame and publishes its registration through `FS:[0]`.
It subtracts `12Ch` from ESP and saves EBX, EBP, ESI and EDI. ECX is captured
in ESI. At `73D433`, `XOR EDI,EDI` explicitly defines zero. The remainder of
the observed caller instructions does not redefine EDI before the retained
allocation comparison. Calls to `BD1780`, `BD17A0`, `00439040` and then the
allocator `BF681B` intervene, so using that zero at the later comparison
requires their ordinary nonvolatile-EDI contracts. No new callee body proof is
substituted for that precondition.

The prefix also performs its actual allocator/release calls with the observed
16-byte arguments and writes application bytes `+18=1`, `+19=0` before module
path capture. These are observed prefix operations, not new complete child
reconstructions. Their data targets and bodies are not opened here.

## Allocation and physical state slot

The preceding packet's retained original/live `[73D45D,73D480)` evidence
establishes the twelve-byte `BF681B` request, saves its returned EAX in
`[ESP+18h]`, compares EAX against EDI and stores EDI into `[ESP+144h]` before
the conditional `BE2900` construction. Equality skips the constructor; the
ordinary nonzero path supplies EAX through ECX. The code then executes
`OR EBX,-1` at `73D47D`.

Let S be startup entry ESP. Its registration starts at `S-0Ch`, and its
quiescent body ESP Q is `S-148h`. The allocation spill at `Q+18h` is therefore
`S-130h`; the store at `Q+144h` is exactly `S-4`, the original pushed sentinel.
Under the ordinary EDI preservation contract, that physical word becomes zero
before the conditional constructor call. This arithmetic identifies actual
storage; it does not prove how the original exception dispatcher interprets it.

Fresh metadata separately closes the two-instruction post-call span
`[0073D480,0073D48D)`, thirteen bytes. Original/live bytes agree, and both
decoded lengths match typed metadata. The actual instructions are
`CMP DWORD[01090AB0],EDI` followed by `MOV [ESP+144h],EBX`. Since the preceding
owned instruction set EBX to minus one and there is no intervening call, the
second instruction restores the physical sentinel to minus one on the ordinary
path after the constructor returns or is skipped. The global comparison occurs
first. The global word itself is not read by this packet, and no hardware-fault
or exceptional ownership conclusion follows from this ordinary sequence.

## Handler boundary

Metadata at the actual `C86CBA` operand identifies an unowned five-byte
instruction with ordinary fallthrough to `C86CBF`. The latter is another
unowned five-byte instruction with a preserved `CALL_RETURN` override over a
default unconditional jump to `BF6B43`. This closes the candidate ten-byte
prefix `[C86CBA,C86CC4)`; no function is created and no flow is changed. Only
after a separate authorization were these ten original/live bytes compared.
They agree and encode `MOV EAX,00DB609C; JMP BF6B43`.

The named prior application-constructor EH readiness packet concerns
`BEA810` and `CC7240`. Its linked ordinary-construction report supplies no
actual `C86CBA` or `73D410` handler/descriptor/action receipt. The indexed
`docs-for C86CBA` query also returns none. Existing registration-layout
conventions and the qualified constructor action mapping remain context,
not proof of this startup handler's cleanup or the original helper's EBP setup.

Exact operand/end metadata next identifies one defined 36-byte DataDB item
`[DB609C,DB60C0)` and a distinct following 392-byte item at `DB60C0`. The
separately authorized original/live descriptor bytes agree. Its raw nine words
are `19930522,31h,DB60C0,0,0,0,0,0,1`. Under the retained 32-bit V3 layout,
this describes 49 eight-byte unwind records; no try/IP/type-list table is
present. EHFlags remains raw value one. The actual Program data-type name and
loaded Java CodeSource are not exposed or attested.

Only state zero's first eight bytes `[DB60C0,DB60C8)` were then separately
admitted, after metadata confirmed their containing item. Original/live bytes
agree and contain `FFFFFFFF,00C86A30`: conditionally state zero to minus one,
action `C86A30`. The other 48 records remain unread. Metadata identifies this
action as non-thunk, returning `Unwind@00c86a30`, with one complete saved
twelve-byte range `[C86A30,C86A3C)`. These table and extent facts alone establish
neither a deallocation nor the action's frame-coordinate interpretation.

The final separately authorized twelve-byte action range matches the original
image and fully decodes into three instructions matching the saved listing and
typed lengths: `MOV EAX,[EBP-130h]; PUSH EAX; CALL BF65AC`. The call's nested
metadata identifies the returning `_free` thunk and its returning `BF9DC8`
target. No callee implementation bytes are reread. If the actual helper supplies
EBP=S, the loaded word is exactly the parent's allocation spill at `Q+18h`.
The action loads that word's current contents; substituting the current
`0109CEFC` publication would change the operand.

The saved action ends **at the call**. `C86A37` retains `CALL_RETURN`, changing
default ordinary call into an effective call terminator; both exposed
fallthrough fields are null. Therefore a complete saved AddressSet is not proof
of complete physical function coverage. The argument cleanup, return epilogue
and any neighboring continuation remain unopened. The captured operations do
not themselves clear publication or unregister; this limited window does not
exclude additional effects in an unobserved continuation. The packet closes
with that physical-closure and original dispatcher/frame qualification intact.

## Current ordinary Source interfaces

`singleton_lifetime_allocate` already exposes raw storage through an explicit
request containing kind, native byte count and host byte count. Its ordinary
implementation uses malloc, the current CRT new-handler retry and bad allocation;
`singleton_lifetime_free` is the matching existing release service. These
available services do not determine this caller's failure ownership.

`GameSingletonHost` exposes the same canonical `01090AA0` publication through
`sound_lifetime()` and a retained `NativeSingletonDeletionBindings` table. Its
shutdown drains that same manager before freeing it. Existing binding APIs
demonstrate installing stable actual publication/context bindings before
registration and keeping them alive through drain. The raw drain pops an owner
before loading its current profile and dispatching flags one. A missing profile
or required binding is rejected. Current Source has no concrete `D685E0` or
`D685F4` deletion case.

The current allocation-statistics startup object remains stack-local and its
semantic constructor writes a null profile, budget `40000000` and zero to the
last word. It does not perform actual base publication/registration. The cache
consumer instead borrows a live `0109CEFC` cell, reloads the current object and
its slot-one target at each metric call, and provides the established `BE2700`
metric mapping. That consumer interface does not allocate or retain its owner.

A later actual owner needs the observed twelve-byte storage, genuine base and
derived lifetime bodies, a stable borrowed `0109CEFC` cell, and both actual
deletion bindings installed before publication or registration. Their context
must survive the same canonical drain. This packet adds none of those objects,
bindings or startup calls, and does not invent publication clearing,
unregistering or freeing on an unresolved failure path.

## Verification and retention

Fresh original/live scope is exactly 156 bytes: 112 instruction bytes across
the two startup windows, handler and saved action, plus 36 descriptor bytes and
the first eight-byte unwind record. All 31 decoded instruction boundaries match
typed metadata. The preceding 35-byte caller window is retained without a new
read; offline joining it with the two fresh startup windows yields a complete
125-byte, 36-instruction scoped sequence. No enclosing 125-byte image read is
performed and the joins do not expand semantic scope.

The packet retains 48 accepted typed responses and 46 other raw GET responses.
Typed batches independently validate epochs 24 and 26; Root warned that its
separate annotations would advance the global epoch. There is no claim that
all captures occurred in one unchanged-program interval. Original PE mapping
and the previous whole-image hash are retained context; this packet performs
neither a new header read nor a new whole-image hash. Exact reads preserve the
original image and check its size and mtime.

The frozen evidence includes 72 complete current inputs, 36 of them Source
files, both working/Git forms where needed, the unchanged preceding bundle,
selected prior receipts, retained layout sources, sixteen bounded Source
excerpts and four complete sorted queries. Full file copies provide provenance;
semantic review is limited to those excerpts and queries. No Source, CMake,
ledger, Ghidra, build, test, probe or runtime change is made, and no original
ABI, exception-runtime or game equivalence is claimed.
