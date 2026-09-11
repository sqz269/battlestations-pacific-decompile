# Native vertex-stream binding and owner discovery

This read-only discovery identifies the remaining lifetime dependency of
full renderer vertex binding `B24840` and reset unbinding `B24BF0`. It adds
no source implementation or runtime claim. The report pins complete fresh
live-Ghidra/installed-PE spans and twelve current provider files. The follow-up
audit retains all original 24 spans / 2,159 bytes and expands the report to
56 spans / 8,331 bytes. All spans were compared afresh with the guarded live
`bsp` program and installed PE; all twelve provider hashes remain unchanged.

## Binding and current profiles

`D5F0A8+134` selects `B24840`. Its complete 504 bytes take ECX renderer,
stack stream index and logical pointer, RET8. Optional guard entry precedes
the current logical pointer at renderer+1774+DWORD(stream*16); the identity
comparison precedes cleanup arming. Identity skips all work.

With two nonnull distinct owners, it calls old/current +2C buffer getter,
then incoming/current +2C, captures cached stride+1778, calls incoming
current +24 declaration getter and reads declaration+CC, captures cached
offset+177C, then calls incoming/current +28 offset getter. These calls and
loads all occur before comparing the three results. Equal buffer/stride/
offset still reassigns the logical reference using full `B23710`, but skips
SetStreamSource and both cache writes. That helper takes ECX destination
pointer cell and EDX incoming pointer cell, returns the destination in EAX,
and uses plain RET. It rereads both cells, publishes and retains incoming,
then decrements and possibly destroys captured old through current slot0.

The other path rereads the current renderer slot and performs the same
publish/retain/release sequence. It then calls incoming current getters
again (buffer, declaration/stride, offset), or selects all zero values for
null. It stores offset before stride, loads current device+1A10/table+190,
calls SetStreamSource with the raw stream index, and increments current
counter+1BB4 only after return. Current mode is read before disarming each
normal exit. Full native guard/FH3 cleanup remains required.

The concrete logical profile has THIRTEEN slots at `D61D6C..D61DA0`:
slot0 BD30E0, +4 B4BF10, +24 B48CE0, +28 B48D10, +2C B48CF0.
The adjacent D61DA0 starts a different profile. Constructor B4BC00 and
alternate constructor B4A9B0 both install D61D6C; destructor B4B5D0 does too.
Existing raw getter implementations are available. Getter B48CF0 follows
logical+58 to the current physical table+1C; its actual physical provider
must accompany a source token adapter, as in the index binding implementation.

## Complete logical destruction

Full `B4B5D0..B4B6E8` (280 bytes) installs D61D6C and sets EH state0 before
optional entry through the CURRENT F8D394 renderer. Its flags+60 read and
comparison precede state1 arming. Dynamic flags satisfy `(flags&F000)==1000`.
They enter the current renderer's inline critical section+19F4, increment
that captured lock's +18 depth, reload logical+58 for B4B3F0 removal, then
reload F8D394 for decrementing +1A0C and LeaveCriticalSection. The inner
tracked lock has no synthesized exception guard. The non-dynamic branch
still performs a discarded physical+4 read; omitting it changes reached
memory accesses.

Next it reloads F8D394 for full B268E0 renderer removal, reloads and
unconditionally decrements declaration+68, then reloads and unconditionally
decrements physical+58. Final-zero calls use each captured object's current
slot0; no null repair or early field clear occurs. It captures the atomic
decrement import before the declaration release and reuses it for physical.
It reads current mode before disarming the optional guard to state0, leaves
with the saved renderer, then changes to state-1 before full B62010.

Its native FuncInfo DF83DC has state1 -> guard funclet CBF928 -> state0,
then state0 -> base funclet CBF920 -> state-1. A normal guard-leave throw
therefore invokes base cleanup only. Complete `B4BF10..B4BF30` (32 bytes)
calls this destructor, then returns the raw slot through full B49570 only
when flags bit0 is set and destruction returned. EAX is the original owner.
Renderer/physical unregistration, vertex-pool return, declaration lifetime,
physical lifetime and actual optional-guard providers are already available.

## Remaining base-owner question

`B62010..B62097` is a full 135-byte base destructor. It installs the distinct
thirteen-slot D62B68 profile and captures +4C before arming state0. If nonnull,
it atomically decrements that retained object's +4, dispatches current slot0
at zero, and clears +4C only after return. It then reloads +50, frees a
nonnull allocation, and clears +50 only after free returns. Ghidra currently
omits the ten-byte continuation B6206D..B62076 because of a false CALL_RETURN
at B62068. Those bytes are `ADD ESP,4; MOV [ESI+50],0`. This discovery does
not repair that annotation. Both normal and exceptional completion use full
BD30F0 base-profile restoration; native CC14A8/DFA2EC and funclet CC14A0
establish the one-state unwind cleanup.

The full base constructor B61E20 initializes +4C and +50 to zero. That fact
does not establish that +4C stays null. The exact retained runtime owner and
its final-zero terminal are still unresolved, so a null-only base destructor
would not close the requested routine. +50 holds owned compressed-format
records: B61F90 lazily allocates element-count*20h bytes and copies eight
DWORDs into a selected record; B61D90 directly stores a raw allocation.

A narrow raw-byte heuristic found five candidate +4C stores near atomic
increments. The follow-up checked all five and expanded the search to
instruction-aligned address-taking assignments. None establishes a writer
of logical-stream+4C. The field's actual nonnull runtime type and final-zero
terminal remain unresolved; the whole binding/reset owner closure is still
not source-ready.

## Constructor and current-profile audit

Fresh live callers identify exactly two direct callers of B61E20:
B4BC00 and B4A9B0. D61D6C references identify these two constructors and
B4B5D0 destruction; D62B68 references identify B61E20 and B62010. These are
the observed native reference sets, not proof against arbitrary indirect
construction or writes through escaped addresses.

Full B4A9B0 is 228 bytes. Its ABI is ECX logical stream; four stack arguments
count, declaration, physical buffer and byte offset; EAX logical stream;
RET10h. After the base constructor it initializes +58/+5C/+68/+6C/+70,
sets count+64 and flags+60=1, retains declaration+68 and physical+58, stores
the supplied offset+5C, then registers through B28A40 on current F8D394.
It contains no nonnull +4C assignment and accepts no parent logical-stream
argument. The ordinary B4BC00 constructor likewise contains no direct
nonnull +4C assignment; its full 758-byte body is pinned.

B4A9B0's sole direct caller AE47E0 contains two creation sites, AE4941 and
AE4A83. Each gets count through the source logical stream's current +20
method and declaration through current +24; B48D00 reads its physical+58.
The byte offset is the caller object's +48 shifted left five. The result is
tagged +54=80000000h and appended to a draw section. Sharing the physical
buffer on this path does not establish a retained parent in logical+4C.
The full 1,084-byte caller and 234-byte primary factory B287C0 are pinned.

The additional current-profile bodies also expose no nonnull +4C store:
B49980 lock, B49A80 unlock, B48D40 offset invalidation, B48CC0 flags getter,
B48CD0 count getter and the three no-op methods. In particular D61D6C+18
points to B48D30, exactly `RET8`; +0C and +30 point to plain `RET` bodies
B4AAA0 and B4AAB0. B48CC0 and B48D30 currently lack Ghidra function records,
so their exact live bytes were pinned without creating functions. B61D90
and B61F90 concern owned allocation+50 only. This direct-body audit does
not close mutations through calls receiving escaped object addresses.

## Rejected assignment candidates and remaining search

The expanded retain-near search yielded nine instruction-aligned candidates:

| Site | Native evidence excluding a logical-stream writer |
| --- | --- |
| 651189 in 650E00 | HUD `PeriscopeDrops` effect handle at screen+4C. |
| 6522DF in 651800 | HUD+4C passed by address; the subsequent retained assignment destination is HUD+50. |
| 7EC633 in 7EC5A0 | Value comes from aircraft-associated source+4A4; the following retained reference is destination+50. |
| A7C4E0 in A7C480 | Constructor installs distinct D5ABB8 and retains its first argument at its own +4C. |
| B10A00 in B107F0 | Stores a B4E020 render-target-holder construction result; that holder installs D61EB8. |
| B2C618 in B2C2D0 | EBP is the B3F930 texture-constructor result; B23640 receives texture+4C. |
| B85F53 in B85EF0 | D63194 copy constructor initializes an entry count at +4C. |
| B86123 in B860E0 | D63194 constructor initializes that count. |
| B862DB in B861D0 | D63194 assignment updates that count. |

The texture candidate is an actual retained-source writer, but its object
origin is a texture constructor. Equal offsets provide no evidence that its
retained memory-stream type can also occupy logical-stream+4C.

A broader offline pass decoded 3,345,748 instructions / 9,293,732 bytes from
snapshot function starts, collecting 2,429 non-ESP memory+4C references or
literal register adjustments by 4C. Its 71 retain-near candidates include
reads, scalar values and stack cleanup. It also records 239 DWORD stores
whose source is not literal zero; many use zero-valued registers. Most of
that adoption-oriented set is not ownership-classified. Linear decoding
can stop at invalid bytes and includes interfunction gaps, so neither this
inventory nor the retain-near filter proves the field permanently null.

## Completed adoption-store follow-up

The four nominated adoption candidates are now excluded with seven more
fresh native spans / 1,680 bytes. All previous 49 span records and twelve
provider records remain unchanged.

AE5550 constructs a separate recursively allocated 58h-byte object. Its
first four fields are floats copied from an input rectangle, so the
destination is not a logical-stream vtable/refcount layout. AE5870 pushes
58h at AE5C83, allocates at AE5CE2, and passes the returned destination in
ECX to AE5550 at AE5D0F. Recursive child calls also allocate 58h. Stores
AE55CC and AE5835 copy the source object's current virtual+38 return to
this node's +4C. The full 793-byte constructor and root-allocation fragment
are pinned. The constructor takes ECX destination and stack source/bounds
pointer, returns the destination in EAX and uses RET8. The descriptive node
name and exact meaning of its +4C value remain hypotheses; object origin
alone excludes it from the logical-stream writer set.

B0B990 stores into a fresh 80h-byte allocation, with D0D4A4 installed at
B0B98A. EDI was cleared at B0B801 and supplies zero to this object's +4C,
+50 and +54. The object is subsequently stored into the caller destination's
+34. Adjacent B0B971 writes a floating-point value into that caller
destination's +4C, a different address from the fresh allocation's +4C.
The pinned fragment retains the zero-register origin and allocation path.

The apparent B2F630/B2F800 stores were misattributed by the offline scan's
nearest snapshot function head. The actual leaf bodies start at B2F670
and B2F840. Both save ECX in EAX and explicitly clear ECX before storing
it at EAX+4C. B2F670 is 23 bytes and zeroes six fields; B2F840 is 96 bytes
and initializes further scalar fields. Both return the original object
with plain RET. Neither has a current Ghidra function record or reported
xrefs; no function was created.

Fresh native bytes also show complete 52-byte predecessor destructors at
B2F630 and B2F800. Their current Ghidra bodies stop at the free calls ending
B2F654 and B2F824, but execution continues through stack cleanup, zeroing
object+4/+8, register restoration and RET, ending at B2F664 and B2F834.
INT3 padding separates them from the two initialization leaves. The guarded
`ghidra flow` query reports zero gaps for both despite these continuations;
its result does not establish complete native body coverage. No Ghidra
annotations or saved analysis were changed.

This closes four nominated function/gap groups containing five MOV +4C
sites. It does not classify the rest of the 239-store inventory or prove
that logical-stream+4C stays null. The separate alias investigation starts
from escaped constructor outputs through B28A40 registration,
B93800/B93E60 payload loading, B85B80 draw-section append and B73BB0
geometry assignment. The nonnull owner profile and final-zero terminal
remain unresolved; whole binding/reset owner closure remains unready.

## Reset consequence

`B24BF0..B24DB5` (453 bytes) uses current renderer virtual methods: twenty
texture unbinds through +130, then FOUR calls to +134 with literal stream
ZERO and null input, followed by +138 null index/base0. It does not loop
over vertex streams0..3. It separately clears pixel and vertex shader
caches under their native guard scopes, uses +E0 to clear the logical
layout, unbinds color surfaces0..3, and performs an independently guarded
SetDepthStencilSurface(NULL). Current concrete table entries select the
already implemented texture, index and layout routines; complete B24840
and its owner closure remain prerequisites for porting the whole reset path.
