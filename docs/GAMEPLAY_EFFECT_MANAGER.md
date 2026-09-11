# Gameplay effect manager lifetime

Addresses: 004c1650, 00870370, 008703e0, 0086fe20, 0086ac00, 0086aa60,
0086ee50, 0086b0b0, 00869a20, 004e4000.

Packet `orch4_gameplay_effect_manager_j` reconstructs the manager behind global
`00F87664` and connects its concrete getter to the front-end memory checkpoint.
Names are descriptive hypotheses, not recovered symbols. This manager handles
gameplay Lua `Effects`; it is distinct from the renderer shader-effect cache.

## Native storage and lifetime

`004C1650` allocates 10h bytes. Constructor `00870370` writes vtable `00D0DA64`,
an empty map head at +8 and count zero at +C. The allocator word at +4 is untouched.
The 18h node allocator `0086AC00` initializes three links, color byte +14 and nil
byte +15, leaving key +C and pointer value +10 uninitialized. The constructor
marks its head nil and points all three links back to the head.

The getter uses the existing shared `01090AA0` lifetime domain. It returns a
captured non-null global on the fast path. Otherwise it gets the lifetime manager,
captures section +10, enters that section, increments +18, and checks the global
again. It allocates, constructs and publishes the effect manager; calls the
lifetime getter again; reloads `00F87664` for registration; decrements the captured
section counter and leaves; then reloads the global for its return. The host
projection keeps that order and requires the application's shared domain and
registered-owner destructor dispatcher.

`0086FE20` first restores the derived vtable. It recursively frees map nodes via
`0086AA60`, resets the head/count, invokes an erase-range operation on the already
empty map, then frees the head. The formerly hidden tail zeros head and count,
**unconditionally clears `00F87664`**, and writes base vtable `00CE3818`. There is
no unregister call and no payload release. Thus the map's raw effect pointers
are non-owning. `008703E0` invokes this destructor, frees the owner only when
flags bit 0 is set, and returns the original address even after freeing it.

The canonical C++ representation uses one optional `std::map<int32_t, void*>`.
Its disengaged state represents the destroyed native head/count. It does not
maintain a duplicate native tree or retain/release definitions. The untouched
allocator representation is supplied explicitly at construction. Native object
size and host allocation size remain separate.

## Front-end checkpoint

At `004E405B`, shell entry calls `004C1650`; `004E4062` passes that returned owner
to `0086B0B0` with `"Effects before mainmenu"`. The latter traverses checked map
iterators with `00869A20` but never reads a payload, consumes its label, or emits
output. Creation and lifetime registration still occur before the platform-event
gate that may abandon shell setup. `FrontEndShellHost` now supplies the shared
effect-manager context; the sequence calls the reconstructed getter and probe.

## ABI and library boundaries

| Address | Native inputs and return | Treatment |
| --- | --- | --- |
| 004C1650 | no inputs; EAX owner; RET | reconstructed getter |
| 00870370 | ECX fresh owner; EAX this; RET | reconstructed constructor |
| 0086FE20 | ECX owner; RET at 0086FEBE | reconstructed normal destructor |
| 008703E0 | ECX owner, stack flags; EAX original pointer; RET4 | reconstructed scalar deletion |
| 0086B0B0 | ECX owner, unused stack label; RET4 | reconstructed valid-container probe |
| 0086AC00 | no consumed inputs; EAX 18h node; RET | analyzed stock allocation contract |
| 0086AA60 | ECX map, stack subtree root; RET4 | analyzed stock node erasure |
| 0086EE50 | ECX map, output iterator and two owner/node iterator pairs; EAX output; RET14h | analyzed stock range erase; destructor uses empty range |
| 00869A20 | ECX owner/node iterator; RET | analyzed checked STL increment |

The correct scalar-destructor and existing STL names are retained. Native STL
allocation topology and invalid-iterator callbacks are not implemented by the
standard-container projection; only valid-container behavior is claimed. CRT,
STL and compiler unwind code are library boundaries. This is not a binary ABI
replacement, and native SEH unwind remains outside the reconstruction.

## Evidence and validation

`reports/gameplay_effect_manager.json` records full-span disk/live byte hashes
after verifying `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, and records
the narrower shell call-site span separately. `gameplay_effect_manager_flow.json`
preserves repair evidence. The reachable free-call gaps in `008703E0` and
`0086AA60` are repaired. The 39-byte destructor tail `0086FE98..0086FEBE` was
decoded and its bad call override cleared, but **Ghidra's stored function body
still ends before that tail**. The complete semantics use verified native bytes;
do not treat the shortened export/decompiler as a full body or claim the metadata
problem is fixed. The one-byte alignment gap after `0086EECC` is unreachable.

Win32 Release compilation and both existing CTest targets passed. The existing
front-end case now exercises concrete singleton creation before both shell exits,
registration, lock balance, the getter fast path, and destruction through the
shared lifetime dispatcher with a non-dereferenceable weak map value. No new test
case or fixture suite was added. These checks do not validate original ABI or
gameplay; the native differential target covers its existing math seeds only.

## Follow-up packets

- Effect name index: `00871750`, process globals `00F87670..00F8767C`, and its
  atexit cleanup `00CDEB00`. The first-call guard is set before map allocation;
  an empty index is rebuilt on later lookups. Lua comes from the current game's
  embedded owner +1A0C. Preserve existing NativeString and Lua services.
- Definition acquisition: `00871BA0`, `00871B50`, `008700E0`, `0086B870`. Cache
  hits increment definition +4; fresh definitions start with one reference and
  are inserted as weak map pointers. These routines remain unimplemented here.
- Definition/component lifetime: `00870400`, `00871440`, `00870D00` and the 13
  component constructors/readers. Follow assembly for register inputs and the
  Tracer constructor's secondary pointer +80. Recover actual component loading
  before replacing startup/warning acquisition bindings.
- Repair the stored `0086FE20` function-body extent through a supported Ghidra
  operation, retaining the verified end-exclusive bound `0086FEBF`.

The downstream startup reference-vector append `004D9C00` is another distinct
contract. Nothing in this packet provides placeholder effect definitions or
claims acquisition, configuration population, or gameplay completion.
