# Plain GUI page root construction

Addresses: `00b8f5e0` (constructed), `00b8f450` (allocator route analyzed).
`00aa5840` uses the engine's cGroup as a plain page root when no page model is
loaded. cGroup is an observed native type string; descriptive C++ names are hypotheses.

`construct_native_group_00b8f5e0` receives the actual aligned18Ch allocation and
calls the existing complete174h node constructor. Its188h object installs table
`00d634f8`, clears the178h/17Ch/180h attachment-array descriptor, sets auxiliary
bit2, copies global `00d7a24c` to184h, writes byte175h=1, sets pose08h/0Ch/10h
to positive zero and copies `00ce4970` to14h. Bytes174h,176h/177h, all other
base-constructor-unwritten bytes, and the pool index188h remain allocation preimages.
The caller retains responsibility for storage on constructor failure.

Assembly confirms ECX actual slot, stack NativeString pointer, EAX same slot,
RET4. Exact function interval is `[00b8f5e0,00b8f64a)`. The allocator thunk
`00b8f450` loads canonical pool address `010902f4` into ECX then jumps to
`00b8f310`. The latter uses18Ch slots,32 slots per31C4h slab, and free-index
metadata at3180h/31C0h. Its pseudocode has a false return after free; that pool
has only been analyzed and is not silently replaced with the model pool.

Validation: Win32 build and both existing math CTests pass. A focused ignored
fixture executed the106-byte original derived constructor, redirecting its base
call to the same reconstructed node constructor used by the C++ path and its two
constant loads to supplied words. Four entire396-byte allocation images matched,
including dirty preimages and the trailing pool word. This verifies the derived
stores; it does not independently validate the native base or actual pool lifetime.

Allocation, group type bootstrap, group-specific current virtual dispatch,
logical release/deletion and root binding remain required. This file provides a
complete raw-slot constructor, not a fabricated live page root. No GUI render or
gameplay claim is made. See `NATIVE_NODE_PARENTING.md` for recovered reparenting.
