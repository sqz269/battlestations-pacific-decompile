# Camera-resource dependency and consumer readiness

The two called dependencies already have complete current Source providers.
The retained application composition shares the counter, root and Scene cells
they need. That does **not** supply a camera-resource owner: `00CD8390` still
has neither an exact nor a containing Ghidra function, and the known pointer
cell `00CE35BC` has no recorded incoming references. The observed 79-byte local
sequence remains a readiness input, not an admitted complete initializer.

This packet is documentation only. Baseline Source is pinned to main
`5ec099c2a99b7547e7dfd529dcc4977e4de27bdf`; the worker inspection revision is
`3c758a9c247c74dc00d6ae2cce8ba10ca2d2cbbc`. Complete files, Git blobs,
current-byte hashes and retained copies accompany the report. No build or
test result is newly claimed.

## Approved metadata and evidence boundary

Three complete raw HTTP responses from `get_flow_metadata` are retained and
accepted by the strict client. The project marker is
`C:\Users\sqz269\bsp.gpr`, program `/battlestationspacific.exe`, language
`x86:LE:32:default`, image base `00400000`, space `ram`. Modification numbers
remain `3` before/after each response and across the batch. The client also
verified program identity before/after the batch.

| Query | Exact / containing function | Complete AddressSet | Flags |
| --- | --- | --- | --- |
| `00B869C0` | Both `00B869C0` | `00B869C0..00B869FD`, one range, 62 bytes | NoReturn false; nonthunk; direct thunk target null |
| `006FAC20` | Both `006FAC20` | `006FAC20..006FACD4`, one range, 181 bytes | NoReturn false; nonthunk; direct thunk target null |
| `00CD8390` | Both explicitly null | No function records | Function flags unavailable, not inferred |

Each queried instruction has override `NONE`, default/effective `FALL_THROUGH`
and no overridden fallthrough. This describes only the three entry
instructions. Empty direct-call arrays at those instructions do not mean the
enclosing functions have no calls. Full body ranges are metadata, not new
byte, instruction-owner, control-flow, ABI or runtime validation.

The bounded GET `get_xrefs_to(00CE35BC, limit=100)` returned
`No references found to address: 00ce35bc`. Its complete CLI output is retained;
it is not represented as a raw HTTP receipt. Missing recorded xrefs do not
exclude an indexed/table-range consumer. No such consumer address was found.

No new Native memory/PE bytes, live disassembly, decompilation, data/table or
handler body was opened. One offline metadata extraction from the pre-existing
mesh-classification report inadvertently displayed its nested, stored
17-instruction `00B869C0` listing. This exposure was reported to Root and is
not used as fresh evidence. The report preserves that scope exception. No
Ghidra mutation, POST/script execution, restart, config change, C++/ledger edit,
build, test or runtime execution occurred. Loaded Java CodeSource remains
unattested, as in the prior audit.

## Existing complete Source dependencies

`NativeMeshResourceTypeIds::initialize_scene_resource_00b869c0` is complete at
`src/native_mesh_subset_loading.cpp:444`. Its constructor borrows the counter,
root bootstrap and storage; it does not initialize them. On a zero
`scene_guard_0109020c`, it publishes guard `1`, writes numeric name address
`00D631E4` to caller target word 2, initializes the same root at `0109DB84`,
copies its current own ID to target word 1, then consumes and increments the
same counter before target word 0. An already-set process guard suppresses
all writes even for a different caller target. The canonical Scene view is
exactly three words at the role named `01090210`: own, root, name.

`TypeIdCounterLifetime::get_006fac20` is complete at
`src/light_type_bootstrap.cpp:22`. Its fast path returns the first current
`0109DB7C` publication load. The cold path retains the original manager's
section, rechecks the publication, allocates an eight-byte owner, sets table
identity and DWORD+4 zero, publishes it, resolves the manager again and
registers the reloaded publication, including null. Section release precedes
the final publication reload. Existing allocation/manager failures and C++
cleanup remain part of this provider; a new caller must not replace it with a
private integer or a second manager/counter domain.

The same file's `initialize_root_00bea780` at line 68 publishes the borrowed
root guard before consuming this counter and storing its numeric name. The
Source declarations require the same retained storage. Existing CP/BB
documentation records Scene's original ECX-target/no-stack-argument/RET ABI
and the counter's no-argument/EAX-result/RET0 ABI. These historical Native
claims were not revalidated against bytes in this packet; the C++ methods
remain Source interfaces, not drop-in binary entry points.

## Retained application cells and present consumers

`GameNativeVfsApplication::Impl` retains `type_counter_0109db7c`,
`NativeVfsOwnerServices`, `GameNativeTypeStorage`, and `LightTypeBootstrap`.
Its constructor passes the existing singleton-host manager publication to
the services and builds the common bootstrap from `owner_services.types()`
and `type_storage.light_types()`. `initialize_core` passes those same two
objects into `initialize_resource_types`.

`GameNativeTypeStorage` owns stable root and Scene guard/descriptor fields;
it cannot be copied or moved. Its mesh view refers to those Scene fields, and
`require_common_bootstrap` checks the identity of the borrowed root/node/light
storage. `initialize_resource_types` constructs the mesh provider using the
existing counter and common bootstrap. The extra animation/bone provider
borrows that same mesh provider and captures both Scene parent IDs before
the descriptor stores. These are complete current Source paths, not a new
camera initializer inferred from the neighboring implementations.

The application cells are host-owned fields named for original roles. Their
names do not imply fixed-address mappings, current original-game values,
original absolute numeric IDs, or original CRT invocation order.

The current `GameNativeTypeStorage` has no camera-resource backing guard at
the role `01090266`, no four-word descriptor at `01090288`, and no call to
`00CD8390`. The existing `CameraTypeBootstrap` instead uses node-family
guard `0108FF9C`, descriptor `0108FFA0`, and initializer `00CD7D80`. It cannot
stand in for the resource descriptor. The `010902A0` camera resource pool is
also a different object.

Current `NativeCameraGroupResourceCalls` accepts a borrowed pointer to three
camera tokens; its predicate forwards to the existing three-token comparison.
`NativeResourcePostprocessContext` likewise borrows the camera own-ID
cell. Those consumers do not create or initialize the missing publication.
Exact address/name searches over `src` and `include` are retained; no current
Source occurrence of `01090266`, `0109028C`, `01090290`, `01090294`,
`00CD8390` or `00CE35BC` supplied an owner or startup route.

## Separately owned fragment readiness

The observed local 79-byte schedule can be expressed as a separately owned,
unbound Source fragment without another Native opening. Its inputs would
explicitly borrow the guard and four store cells with roles `01090266`,
`01090288`, `0109028C`, `01090290` and `01090294`, the actual literal-address
binding for `00D6327C`, and the genuine existing Scene/counter Source
providers in the same domain. A numeric original pointer token is sufficient
for the observed name-word store; it does not admit a string's contents or
extent, and a substitute local string is not equivalent evidence.

That fragment would return on the nonzero guard; otherwise publish guard `1`
and the bound name pointer, initialize the canonical Scene cells, capture
both current parent IDs before either store, call the same counter getter,
capture DWORD+4, increment it modulo `2^32`, and finally publish the old value
to the own-ID cell. Borrowed cell identities and provider lifetimes must be
the caller's explicit contract. It would preserve the observed partial-write
schedule under ordinary Source exceptions without claiming original EH or
hardware-fault behavior. No fragment code is added in this packet.

Such an interface would have its own Source ABI and no bound production
caller. It would not allocate the missing retained camera descriptor, admit
`CD8390` as a whole original function, establish original entry/register ABI,
validate a CRT consumer/order, or extend actual startup composition. Those
remain separate held gates even if this narrowly named fragment is later
implemented and build-tested.

## Existing CRT evidence and held gates

The immutable previous ownership audit established only the original/live
four-byte pointer `00CE35BC -> 00CD8390`, plus the local guard/name/parent/counter
store order. It did not establish a CRT slot, callable ABI or ownership.

The current R30 report also contains a historical receipt for
`[00CE35B4,00CE35F4)`, 64 bytes, kind `crt_entries_extras_through_mesh`, SHA-256
`e5b074c795e9393018f00632233ade4cc7b75c7187e500d04d2ce5549579f359`.
This pass inspected receipt metadata only, not that table's stored bytes.
Its explicit `crt_order` records name only `CE35B4 -> CD82F0`,
`CE35B8 -> CD8340`, and `CE35F0 -> CD8690`. Its table label and interval
membership do not prove `CE35BC`'s consumer or camera invocation contract.
Current Source explicitly composes selected initializers and already warns
that this does not establish every original CRT position.

| Prerequisite | Result / remaining gate |
| --- | --- |
| Scene and shared counter Source providers | Present, complete and pinned against current baseline Git |
| Same retained counter/root/Scene domain | Present in application composition; actual runtime state untested here |
| Dependency Ghidra function ownership | Exact complete ranges present; independent NoReturn/thunk flags retained |
| Camera function entry and full owner | Held: both exact and containing records remain null at `CD8390` |
| Camera descriptor/name/guard contract | Held beyond the previously observed local stores; name contents, canonical owner and global writer/consumer contract unestablished |
| Camera backing storage and initializer Source | Absent from the inspected current type owner; no implementation added |
| Original CRT consumer/order/invocation | Held: one pointer, historical neighboring claims, and a negative incoming-xref query do not supply it |
| Whole camera ABI/control/EH behavior | Held; dependency Source availability does not admit a whole Native body |
| Production routing/ABI/game validation | Held; no build, fixture or runtime validation in this packet |

There is no evidence-backed new CRT-consumer/table window to open from the
negative xref result. No wider interval is selected from alignment or
adjacency. If Root later requires renewed dependency Native ABI verification,
the metadata bounds two exact candidate windows: `00B869C0`, length 62, and
`006FAC20`, length 181. They would recheck the already available dependencies;
they would not resolve camera entry/CRT ownership. Neither was opened here.

Report: [current Source pins and metadata receipts](../reports/cc12_camera_resource_dependency_readiness.json).
