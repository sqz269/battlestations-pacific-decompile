# Resource-manager construction and parser registration

Addresses: 004c1400, 00b81040, 00b80a50, 00b7f340, 00b7e740, 00b80290,
00b7df40, 00b7e1d0, 00b7e2a0, 00b7e530, 00b7e370, 00b7e460, 00b7e100,
00736dd0, 00736ea0, 00b7eb60, 00b94170, 00b92a20, 00b941a0, 00b8b210,
00b8f8e0, 00b8b050, 00b8b080.

The manager constructs six type parsers, and application initialization adds
two more. Their type names and parser virtual targets are verified; concrete
mesh, camera, animation, and bone decoding remain separate dependencies.
Names below describe observed behavior and are not recovered source symbols.

The companion `reports/resource_manager_registration_audit.json` records
complete PE/live-matched code spans, vtables and type literals, prior function
documentation, and decoded instructions. It covers the existing `bsp` project
and `/battlestationspacific.exe`, SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

## Manager initialization

`004c1400` returns the singleton at `010901c4`. Its initialization locks the
existing singleton lifetime manager, rechecks the pointer, allocates `28h`
bytes, calls `00b81040`, and registers the resulting lifetime object.
No explicit caller argument is consumed; EAX returns the pointer.

`00b81040` receives the manager in ECX and returns it in EAX, plain RET. It
installs vtable `00d63128`, creates the four-byte object at manager `+4`, and
initializes two tree headers. The parser tree is `+8` (sentinel `+C`, count
`+10`); the resource-name cache is `+14` (sentinel `+18`, count `+1C`).
The observed body does not initialize the later current-factory/current-resource
fields `+20/+24`; load code assigns them. Do not infer a zero-initialized object.

Constructor registration order is exactly the first six rows below. The last
two are registered consecutively by application initialization `0073d410`
after obtaining this manager, as shown by its `0073...` call sites and the
matching constructor/registration assembly. The five game registrations at
`00717e80` are documented separately in `GAME_RESOURCE_PARSER_REGISTRATION.md`.

| Phase | Type literal | Singleton getter | Name getter | Parser virtual +8 |
| --- | --- | --- | --- | --- |
| Constructor | `Mesh` | `00b7e1d0` | `00b7eb60` | `00b947a0` |
| Constructor | `SkinedMesh` | `00b7e2a0` | `00b94170` | `00b94850` |
| Constructor | `SkinedMeshAnimation` | `00b7e530` | `00b92a20` | `00b92f20` |
| Constructor | `MatrixIndexedMesh` | `00b7e370` | `00b941a0` | `00b94900` |
| Constructor | `Camera` | `00b7e460` | `00b8b210` | `00b8b240` |
| Constructor | `GroupParams` | `00b7e100` | `00b8f8e0` | `00b8eb50` |
| Application | `AnimationChannels` | `00736dd0` | `00b8b050` | `00b8a910` |
| Application | `Bone` | `00736ea0` | `00b8b080` | `00b8a990` |

The spelling `Skined` is present in the native literals. Each singleton is an
eight-byte object with a primary parser vtable and lifetime subobject at `+4`;
initialization follows the same locked lifetime registration pattern. Each
name getter is exactly 33 bytes: it constructs the literal into the supplied
hidden output string pointer, returns that pointer in EAX, and uses RET4.
The incoming parser ECX is not consumed by those getters.

## Registration contract

`00b80a50` takes ECX manager and one stack parser pointer, returns boolean AL,
and uses RET4. It calls parser virtual `+4` to obtain an owned temporary type
name. It queries the parser map at manager `+8` through `00b7e740` and destroys
that temporary before branching. An existing equivalent name returns false
without replacing its parser. A missing name obtains the type name again,
constructs a name/pointer pair, and inserts it through `00b80290`, then cleans
both temporary strings and returns true. No explicit AddRef or parser virtual
release appears along this path.

`00b7f340` is a pair constructor with output in ECX, parser pointer in EDX,
and a by-value eight-byte native string on the stack. It copies the stored
name bytes, writes the raw parser pointer at pair `+8`, and destroys the input
string. This is raw pointer storage; singleton lifetime remains separate.

`00b7df40` implements lower bound on the native tree. Node key length/data are
`+C/+10`. Length zero sorts as empty without dereferencing a null data pointer;
nonempty keys compare through CRT `stricmp`. `00b7e740` checks that candidate
with the previously audited `00443d00` comparator and returns an iterator with
container/node words. `00b80290` implements unique insertion selection with
the same ordering, rejecting an equivalent key. Allocation, linking, and
balancing in `00b7fd30` are still external; its old generated throw-oriented
name is not evidence that every call throws.

## Limits

This audit establishes registrations and calling contracts, not complete
parser implementations or manager teardown. These eight rows and the five
separately observed game rows do not prove that later or indirect registration
cannot occur. In particular, an installed `CloudSystem` record is not evidence
of a registered parser in these functions. A traversal probe must report
skipping unsupported data separately from decoding a real resource object.

There is no native ABI replacement or gameplay validation in this packet.

## Correction from docs/APP_INIT_VFS_SINGLETONS.md

The two application parsers' lifetime vtables: `AnimationChannels` writes base `00cfea08` then final `00cfea34`; `Bone` writes base `00cfea0c` then final `00cfea44` (stores at `00736e38`/`e3f`/`e45` and `00736f08`/`f0f`/`f15`).
