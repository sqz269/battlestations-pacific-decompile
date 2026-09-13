# Actual material effect program population

This packet composes B45EE0/B46950 over the existing actual 178h effect,
110h descriptor, 88h pass, native strings, secondary-pass helper and canonical
owner domain. It adds executable B5F160 state pruning and 711370 unsigned
string construction. It does **not** supply the complete descriptor parser,
shader compiler, renderer state caches, original SEH, or a working game shader.

The existing B407A0 constructor leaves C8..137 unwritten. B45EE0 first selects
policy from stack byte2 when override3 is nonzero, otherwise from CURRENT
F8BBF0+0E. It allocates/constructs a descriptor, publishes that SAME allocation
at C4, then calls B43B00(name,3). The root descriptor's Priority08 and PipeID04
are copied to effect B0 and AC before any later program failure.

After removing the last dot suffix from a native local filename, fourteen mode
slots are visited in order. Empty root mode names write null to that one primary
slot. Nonempty entries allocate another real descriptor, resolve its native
name, parse it, write mode10C, and compile the key `stem + decimal(mode) + T/F + 3`.
Compiler success publishes its actual returned pass without another retain,
sets borrowed pass14, and directly deletes the temporary descriptor. A native
null result deletes that descriptor and returns zero without writing the
current or later primary slots, or any secondary slot. No preimage is cleared
to manufacture cleanup safety.

B45E00 then initializes the fourteen secondary slots in its established order.
It makes a real secondary pass only for primary0, copying actual state owners
and using the SAME canonical registration. The caller finalizes primary and
secondary at each index, reloading secondary after primary's callback. B17DD0
finalizes primary0 again, writes borrowed14, appends it at effect9C using live
countA8, increments that count, and increments actual pass04.

If root14 is set, two actual shadow descriptors are acquired and parsed in
native order. The literal is the 22-byte `shadow_passtrough.shfx`, and the key
is `stem + sha + T/F + 3`. B3C3A0 receives effect in ECX, the second shadow
descriptor in EDX, and seven stack words; RET1C confirms that register argument
which the earlier pseudocode presentation obscured. Its returned pass is
published to138 even when null. Success finalizes138, sets borrowed14, copies
138 to primary2 without releasing the prior primary2, and increments actual04.
Both shadow descriptors are directly deleted before the local names are freed.

B46950 flag0 returns B45EE0(name,0,0). With global0108D6F0 nonzero, it calls
B45EE0(name,0,3), B41B10, B187A0, B45EE0(name,1,3), and returns1 irrespective
of both AL results. The second operation therefore starts with all pass slots
known cleared by B41B10, including when its compiler later returns null. On a
fresh effect, an early first compiler-null result leaves unreadable slot
preimages; the host reports that unsafe cleanup boundary instead of iterating
them. This deliberate error is not the original native failure behavior.

## Retained operation and integration

`NativeMaterialEffectProgramOperation` contains both load frames, their actual
local string headers, acquired descriptors, callable descriptor companions and
the active required child frame. It is allocated before acquiring the native
creator, invoked once, and retained on an exception. Successful operations move
into the SAME canonical effect record and survive through effect terminal
destruction. Destruction calls descriptor current0 while the companion still
exists; the callback detaches it before B46930 deletes the SAME allocation.
The first root companion remains stable until the between-variant B41B10.
No second descriptor count, material owner or effect registry is introduced.

`complete()` means the wrapper returned; `pass_slots_initialized()` separately
records whether all twenty-eight slots have known native producers. A wrapper
returning native zero can have either state. `active_call_site()`,
`primary_slots_written()` and `active_child()` identify retained failure state.
Canonical terminal admission needs both completion and initialized slots.
Discarding a frame with live bindings, native local strings or a child is an
error; its destructor never performs completed-creator rollback.

The caller must keep the original root name header at a stable address through
an incomplete child. Required services capture argument values and supplied
descriptor/name addresses, not a transient request object or caller stack.
On a normal child return, all completed ownership has transferred into actual
storage or its canonical record, so retiring its child frame is metadata-only.

The context requires the SAME strings, lifetime, pass construction/copy domain,
pass registration, renderer publication, material manager, VFS publication and
variant byte. Renderer+104 must be a callable binding returning actual readable
capability storage. D61BE8 and D62A80 both dispatch current0C to B5F6A0; other
pass profiles are explicit unsupported cases. No numeric address is executed
as a host function.

## Concrete children and remaining dependencies

B5F160 removes render-state groups when the first matching enabling state is
absent or zero: 1B controls13/14/AB; CE controlsCF/D0/D1; F controls19/18;
7 controls17/E; 34 controls36/37/38/39/3A/3B/35; 1C controls23/8C. It then calls
CURRENT renderer104 and uses result3D to remove9A or B5, followed by the thirteen
unconditional states. All removals use existing actual B5EE00 and preserve its
last-row replacement order. B5F6A0 executes this real pruning before the required
B26500/B265C0/B26680 cache calls and actual18/1C/20 publications. It captures
the next state input before publishing the previous cache result, as assembly
requires. Cache providers must implement actual retain/release and canonical
identity; an identity-only return is not an implementation.

711370 uses decimal `to_chars` instead of porting the CRT, but preserves its
two native pooled copies, captured source pointer/length for the second copy,
and captured normal temporary release. B17DD0 preserves the actual counter
and callback ordering. These are independently usable concrete native bodies.

B43B00 remains a required actual parser. Existing raw children are available:
B66BD0/B6A020 Lua owner/bootstrap, B67800 and the tracked Lua object/getter layer,
B579B0 render states, B439C0 combiner tables, B41830 sampler tables and B419B0
field tables. The B43B00 caller and actual B69D40 file execution still need
integration and assembly review: the decompiler removes blocks and confuses
stack aliases. Existing ShaderLuaCode is a semantic projection, not its raw
descriptor producer. B3C3A0/B354D0/B3B3C0/B3A7E0 require the actual compiler
frame, source/COM ownership and returned pass registration; the existing
CompiledMaterialEffect/Pass model is not a substitute. B26500/B265C0/B26680
also need their actual renderer cache arrays and equality/reserve children.
All these requirements are pure interfaces with no default success or failure
provider added solely to make an executable link.

Coverage is normal control-flow composition with explicit retained host child
errors. Native local SEH cleanup is intentionally not claimed: incomplete
programs retain their name arguments/acquisitions instead of unwinding them.
The existing NativeString operations preserve their published contracts, but
arbitrary allocator callbacks that mutate other B45EE0 stack locals are outside
this composition's callback-order claim. Fresh constructor admission is the
supported caller; unclassified B469A0 code also references B46950 at B469FF and
B46A33 after release operations, and its reload/error behavior is not ported.

## Validation

See reports/native_material_effect_programs.json for exact body ranges,
original ABI, call sites, unresolved children and completed build/fixture status.
The focused ignored fixture relocates all 653 original B5F160 bytes, redirects
its 33 B5EE00 calls to the production actual helper, and binds only the renderer
capability observation. It compares actual row bytes/order for both capability
branches, checks unchanged counters, and executes canonical state terminals.
This validates the pruning child; it does not validate successful shader parsing,
compilation, effect loading, drawing, or gameplay.
