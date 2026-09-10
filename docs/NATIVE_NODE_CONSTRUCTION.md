# Native node construction over one pool slot

This packet reconstructs `00B6F5A0..00B6F8CE` over the actual supplied `0x1F0`
directional-light pool slot. The function initializes only its `0x174` node
prefix. It neither allocates nor returns the slot and does not implement terminal
node destruction, directional-light construction or runtime type bootstrap.

Native ABI: ECX is the raw owner; the stack holds a `NativeString*`; EAX returns
the same owner; `RET4`. The C++ interface additionally receives the slot extent
and the existing `SizedStoragePool`. Its pointer values can refer to host binding
companions, so the layout checks do not make it a binary-compatible replacement.
Addresses, native-byte hashes and validation status are recorded in
`reports/native_node_construction_audit.json`.

## Canonical storage and shared views

`NativeNodeStorage` has the exact Win32 `0x174` prefix layout, including an atomic
reference word at `+4`, the actual pooled string at `+54/+58`, matrices at
`+60/+B0/+F0`, and the one borrowed point-light pointer/count/capacity array at
`+164/+168/+16C`. It contains no expanded host metadata and no member initializers
for untouched byte ranges. Placement construction deliberately omits parentheses
to avoid value-initializing the raw allocation.

`NativeNodeBinding` is an external, stable companion. Its `CameraTransform`
references the prefix's actual links, flags, notification context and matrices;
its `SceneNodeAttachment.scene` references the same actual `+170` slot.
`pointer_key` is the address of the actual pool slot. Creating the companion
does not write any native field, retain anything or register another list.
The type predicate, scene-attach implementation and notification callback are
explicit actual bindings. A freshly constructed node has no `+A0` attachment,
so its caller explicitly supplies a null notification callback.

The primary integration adds `CameraTransformBacking` and backing constructors
to the existing shared types. Their default construction still owns legacy host
storage. Explicit copy/move construction copies values into a new owned backing;
assignment copies values into the target's existing backing and never rebinds
references. This prevents accidental aliasing of another default-owned transform.
The tracked-source audit found no current by-value transform/state copies, but
the existing value semantics are still preserved. Actual concrete declarations
are in the D3D9 shader/mesh probes; other current consumers use references or
pointers.

## Native write map

The constructor installs `00CEB130`, initializes reference count one and installs
`00D62C88`. Before copying the pooled name it clears hierarchy fields
`+30/+34/+38/+3C/+40`, initial mask `+48`, attachment `+A0`, root `+A4`, `+A8`,
retained owner `+130`, point-light array and generic scene `+170`. It sets `+50`
to raw word `501502F9`. `NativeString` default construction initializes only the
native-written name words before the first potentially throwing call.

After name copy, `+4C` and `+AC` become `3F800000`, valid flags `+5C` become zero,
and three fresh identity matrices are copied through the existing sequential
x87 routine `004134F0`, in native destination order **`+B0`, `+F0`, `+60`**.
Auxiliary flags `+138` become `40`; world sphere `+13C` becomes
`(0,0,0,501502F9)` in raw words; minimum bounds `+14C` use `D01502F9` in all three
components and maximum bounds `+158` use `501502F9`. These are the native
positive/negative `1e10` constants, not infinity. Finally it writes byte `+44=0`,
byte `+134=1` and mask `+48=FFFFF`.

The actual slot preimage survives at `+08..2F`, `+45..47`, `+135..137` and the
entire `+174..1EF` tail. That tail includes all directional-light fields and the
pool's authoritative slab ID at `+1EC`. No constructor fallback values are
invented for those bytes.

## Failure and lifetime boundary

The native exception handler `00CC1A31`, descriptor `00DFA93C`, unwinds the
point-light array (`00B6F3E0`), pooled name (`0041DD20`) and base vtable
(`00AA6E10 ->00BD30F0`). The constructor preserves that cleanup order. Array/name
storage is released without invented field clearing; the raw slot still belongs
to its caller, whose allocation path controls pool return after a failed
construction. The shared string pool is reused rather than creating another
allocator domain.

This is construction and a live view binding. Full direct base destruction,
derived virtual dispatch phases, nonnull retained-owner policies and the final
directional-light factory remain separate packets.

## Verification boundary

The focused Win32 fixture passed under `/W4 /WX /fp:strict`. It compared the
complete 496-byte slot against executed,
verified installed constructor bytes for an empty source name. Its native matrix
copy and string-resize equality fast path also use verified original bytes;
only external call targets and constant addresses are relocated. The unused
allocation/memcpy branches and native exception paths are not execution-tested
by that comparison. A nonempty name uses the real reconstructed string pool in
the host check. Shared-view checks cover bidirectional matrix/flag access, actual
scene-slot retain/release and transform copy/assignment backing behavior.

The native spans were checked byte-for-byte against the configured Ghidra
program and installed PE. Six spans (1,093 bytes) matched. Ten relocations
redirected four calls and six constant operands. No native instruction behavior
was replaced. The primary integrator owns the final `scripts/build.ps1` gate
after adding this source and the shared backing changes to the main target.
The audit records the four shared-source hashes used by the focused check.
Native-byte comparison does not establish full native ABI compatibility,
gameplay or visual parity.
