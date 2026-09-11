# Native vertex-stream binding and owner discovery

This read-only discovery identifies the remaining lifetime dependency of
full renderer vertex binding `B24840` and reset unbinding `B24BF0`. It adds
no source implementation or runtime claim. The report pins complete fresh
live-Ghidra/installed-PE spans and twelve current provider files.

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
increments. B860E0/B861D0 were checked and belong to a different D63194
object, with +4C acting as a count. This scan is incomplete and establishes
neither an owner type nor absence of other writers. Further investigation
must trace actual writes/constructor callers before defining the base's
supported ownership domain.

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
