# Registered root-resource traversal

Addresses: `00b7f430`, `00b7f100`, `00b1fe40`, `00b28570`.

`dispatch_root_00b7f430` retains supported resource payloads, hierarchy records,
and root bounds in `StructuredModel`. It uses the existing registered parser
map and actual Mesh, Note, and GroupParams implementations supplied by its
caller. Unsupported resource types retain an explicit `nullopt` payload and
their serialized position; their bytes are skipped. This preserves hierarchy
resource indices without inventing a native fallback item or a parsed payload.

The C++ model owns decoded wire values. It does not implement the native
`0x74`-byte GameResource, its intrusive reference count, virtual classification
lists, factory, or cache entry. Those boundaries remain described by
[STRUCTURED_RESOURCE_DISPATCH.md](STRUCTURED_RESOURCE_DISPATCH.md) and
[RESOURCE_ITEM_OWNERSHIP.md](RESOURCE_ITEM_OWNERSHIP.md).

## Evidence and original calling conventions

[registered_root_resource_audit.json](../reports/registered_root_resource_audit.json)
records complete current PE/live code matches, previous Ghidra names,
signatures and full plate comments, and separate validation states. Every live
query used `bsp.py ghidra`, which verifies `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, x86 language, and image base `00400000` before
querying. The installed executable SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

| Function | Complete matched span | Original ABI |
| --- | --- | --- |
| Root dispatch | `00b7f430-00b7f5ab`, 380 bytes | ECX manager; stack pointer to parent reader-handle wrapper; RET4 |
| Hierarchy container | `00b7f100-00b7f1a3`, 164 bytes | ECX manager; stack pointer to Hierarchy reader-handle wrapper; RET4 |
| Concrete begin hook | `00b1fe40`, 1 byte | ECX renderer unused; no stack arguments; RET |
| Concrete end hook | `00b28570-00b28592`, 35 bytes | ECX renderer unused; no stack arguments; RET |

The current saved signatures omit inputs. Full assembly establishes the
manager register and stack handle, direct callee arguments, normal completion
and RET4. No native return status is inferred from the host `bool` result.
The two concrete hooks had no Ghidra function definitions, so their bounded
current live bytes were decoded with Capstone and matched to disk through RET
followed by CC padding. Their unavailable names/signatures/comments are recorded
explicitly for the primary's function creation. This worker did not mutate
Ghidra or the address/name ledgers.

## Root order and retained destination

At `00b7f457`, root dispatch invokes renderer global `00f8d394` virtual `+50`,
with ECX renderer and no stack arguments. Only then does it read the root
control word through `00be9a40`. It iterates child handles using `00715bf0`
and `00bea680`, preserving the serialized order and comparing tags through
CRT case-insensitive C-string comparisons:

| Root child | Retained behavior |
| --- | --- |
| Resource | Dispatch through the actual registry and append records to `resources` |
| Hierarchy | Append recognized Item payloads to `hierarchy`; skip unknown child tags |
| BoundingBox | Read six float32 words and replace `bounding_box`; set the host presence bit |
| Other, including BoundingSphere | Explicitly skip its serialized payload |

`root_order` retains the original counted tags, including bytes after an
embedded NUL. Dispatch applies C-string comparison to their prefixes, matching
the native comparison. It imposes no MMOD tag restriction or control-version
check because neither appears in the root function. Callers can impose their
own format expectations after dispatch.

The root function does not clear its resource. Repeated Resource and Hierarchy
containers append, including when the host model already contains records.
Each subsequent BoundingBox replaces all six words. No sorting, clamping,
normalization, or derived sphere is introduced. The native base constructor
initializes these bounds to zero; the host model follows that default and adds
`has_bounding_box` to distinguish an encountered record. If no BoundingBox is
encountered, the destination's prior bounds and presence bit remain unchanged.

The BoundingBox branch reloads native current resource from manager `+24` at
`00b7f51c`, then uses six MOVSS stores at resource `+28..+3C`. Root dispatch
does not assign or restore manager `+24`. The host API binds one caller-supplied
model for the traversal; it does not emulate parser reentrancy changing that
native pointer. Native item dispatch's current-resource virtual append and
hierarchy's owning append are represented by actual owning wire vectors,
without claiming identical native object behavior.

## Hierarchy container and handle lifetime

`parse_hierarchy_00b7f100` recognizes Item with the same CRT comparison and
calls existing `parse_hierarchy_item_00b7eb90`. It retains the complete decoded
name, parent, resource DWORDs, optional matrix, flags, sphere and box. It does
not resolve graph links, substitute identity for a missing matrix, or classify
resource objects. Other child tags explicitly skip and are recorded in
`skipped_hierarchy_tags`.

Every successfully handled child follows the native release path. Recognized
records close without an implicit seek over unread payload; unsupported
records explicitly skip first. Hierarchy and root input containers remain
attached for their callers to close. Resource and hierarchy records append
before their child handle is released.

## Renderer hooks and host failure policy

The API requires explicit `StructuredModelDispatchHooks`; it supplies no default
renderer. `begin_root()` represents the call before the control word.
`end_root()` represents `00b7f597`, which reloads the global renderer and calls
virtual `+54` after normal traversal. An empty root still receives both calls.
The host implementation uses the same borrowed hook object throughout; native
global replacement during traversal is outside that fixed-binding domain.

The native normal path does not establish a finally-style end call on parser
or reader failure. Consequently a host read, parser, or allocation failure
returns false without synthesizing `end_root()`. Consumed bytes and completed
output remain; no rollback is promised. Hook exceptions propagate to the caller.
The initial ready check, sticky reader errors, bounds read commit, allocation
checks and diagnostic vectors are explicit host behavior, not native exception
or short-read equivalence.

The observed concrete D3D9 renderer vtable `00d5f0a8` contains `00b1fe40` at
`+50` and `00b28570` at `+54`; both table words were matched to the installed
PE. Constructor `00b32410` installs this table at `00b3243b`, as independently
observed by the geometry packet. The application startup path calls that
constructor at `0073da88` (see [D3D9_STARTUP.md](D3D9_STARTUP.md)). The base
constructor `00b283f0` forms renderer `+0C` and passes that subobject to
`00b25f40`; the latter stores subobject minus `0C` in global `00f8d394` at
`00b25f9b`. This establishes that the global points to the primary renderer,
whose vtable the concrete constructor replaces. Both complete supporting
constructor spans were also matched to PE/live bytes; only this binding
provenance is used here, without implementing their lifetime manager services.
The primary base renderer table `00d5e628` contains `00bf698e` at both offsets;
`00d5e610` belongs to the singleton subobject at `+0C`. The
concrete result must not be generalized to every renderer implementation.

`00b1fe40` is a single RET. `00b28570` installs an SEH frame, immediately restores
`FS:[0]` and its stack, then returns. Neither body reads or writes renderer
fields, invokes another service, or produces a meaningful return value.
`RendererRootHooks_00d5f0a8` therefore implements their actual empty normal
behavior. The caller must select it explicitly; arbitrary renderer hooks do
not silently receive this implementation. This closes the concrete normal
hook path, while native SEH/async-exception behavior remains outside the host
API.

## Validation boundary

All four complete implementation function spans, both supporting constructor
spans and both hook table pairs were matched to current installed PE bytes
and the live saved Ghidra image. Root/hierarchy
pseudocode and full assembly were inspected; the absent hook definitions were
decoded from complete matched bytes.
The implementation adds no tests; the primary integrator owns the existing
installed-model probe migration, CMake integration and MSVC Win32 build.
Build, installed-file fixture validation, native differential validation,
binary ABI compatibility, and game validation remain separately reported.
