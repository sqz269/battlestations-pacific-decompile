# Registered model and instance integration

The installed model now flows through the retained root dispatcher, native
material-clone projection, shared dynamic instance geometry, visibility/grouping
fragment, instance upload and actual queued draw entries. The previous diagnostic
instance allocation has been replaced by the recovered renderer buffer contract.
This remains a controlled D3D9 diagnostic, not a runnable game or native frame
dispatcher.

## Integrated behavior

| Part | Implemented and checked | Remaining boundary |
| --- | --- | --- |
| Root loading | Four ordered root tags, actual registered Mesh/Note/GroupParams, hierarchy and root bounds; explicit concrete begin/end hooks | Native resource object graph, cache and intrusive ABI |
| Material clone | Distinct material projection, retained effect/texture identities, all confirmed copied/reset words, lighting and conditional ownership | Native cache bytes and object/destructor ABI |
| Generated geometry | Selected source stream, indices and combined layout retained; two registered streams share the native 16 MiB dynamic buffer | Native model wrapper and renderer raw-object allocation |
| Grouping | Binding ID lookup, two categories, ordered borrowed source pointers, retained model contexts, native diffuse tint | Native debug strings and atomic frame-entry pool |
| Upload | Real mapped building records, count updates, output-entry construction, actual borrowed queue pointers | Native scene attachment and world-sphere service supplied explicitly |
| Sorting | Complete finite-depth insertion, partition and heap paths, including large-list tie permutation | Native object comparator traversal and exceptional floating-point domain |

The installed file is `models/misc/repulogepdarabok_004.mmod`, SHA-256
`5fab0cfe63220f84693cae4140fc98689670316585993dae71bd4f503bc7a7e6`.
Independent header inspection gives `BoundingSphere`, `BoundingBox`, `Resource`,
`Hierarchy`; the copied root BoundingBox is compared directly with 24 bytes at
offset 73. The existing hierarchy, mesh, lighting, LOD and raw buffer checks remain.
All 1303 bytes are consumed. The two concrete D3D9 root hooks have empty normal
behavior, established from their real vtable and machine-code bodies.

The renderer allocation creates the actual 16 MiB DEFAULT/DYNAMIC/WRITEONLY
vertex buffer. Generated instance streams begin with flags 1000h, tag 80000000h,
count 0 and offset FFFFFFFF. The diagnostic's full-opacity and faded records use
offsets 0 and 144. Both generated geometries retain the same source mesh, index
buffer and combined declaration. Clones preserve the actual compiled effect and
texture identities; source words 104/108 use the ordinary constructor's FFFFFFFF
defaults, and clone construction copies them.

Each source record is written through the recovered 144-byte building generator.
Upload creates two stable output entries and appends their actual addresses to
the respective queues. Both entries are drawn using their geometry and section
ranges. Device queries inside each draw verify vertex buffers, offsets, strides,
frequencies, index buffer and declaration identities. This verifies both offsets
even though the controlled instances overlap in the image.

## Validation

`scripts/build.ps1` passes with MSVC Win32 `/W4 /WX /fp:strict`; both existing
CTest targets pass. The installed reader mode and full D3D9 probe pass, including
the existing normal and wrapped font draws. The two queued mesh draws report
2499 nonblack pixels and 54 colors, and restore the captured device state.

The raw render-target RGB is byte-identical to the prior mesh diagnostic. Its
maximum channels remain 3/7/9: this is the existing raw linear output without the
game's final display pipeline. The BMP alpha bytes differ after the additional
faded instance. No original-game screenshot comparison or final-display parity
is claimed.

An isolated 32-bit native sort fixture executes only ten PE-verified function
ranges, with the established comparator projection. All 90 runs compare 5652
pointer positions exactly, including duplicate keys, duplicate pointers and 30
forced heap paths. The primary independently reran that executable successfully.
This is the recorded finite-domain corpus, not an exhaustive algorithm proof.
No permanent test target was added.

One focused capacity check fills the logical cursor to the shared buffer limit
and verifies rejection before scene attachment, COM locking or queue mutation.
Review also found that the physical lock helper advances its lock counter after
an attempted COM failure. Upload now balances that pair only when the depth
changed; preflight rejection is not unlocked, and no implicit rewind is invented.
The attempted-COM-failure cleanup branch was source-reviewed; a device failure was
not injected. Visibility's nonfixture numerical branches were assembly-reviewed,
not covered by the native sort differential test.

Final artifact hashes, logs, reviewed source hashes and limitations are recorded
in `reports/instance_integration_validation.json`. The independent review record
is `reports/instance_integration_review.json`.

## Saved analysis and next work

The primary independently matched 50 unique audited ranges to the installed PE.
Forty reviewed function names and additive comments are applied to the existing
`bsp` project and `/battlestationspacific.exe`, preserving previous annotations.
Three verified continuations after false no-return free calls were restored at
`00b1c500`, `00b1c660`, `00b51b50`. Concrete root hooks `00b1fe40` and `00b28570`
were defined from their complete 1-byte and 35-byte bodies. All mutations were
serialized by the machine-wide Ghidra lock. The original executable is unchanged.

Next packets in `config/parallel_work.json` cover model world-sphere caching and
scale, real scene attachment and its conditional pointer registry, and native
batch-key preparation with comparator-parametric reuse of the recovered sort.
Named but incomplete command execution, system constants and material pass
selection remain explicit dependencies. In particular, the fixed NORMAL pass
used here does not implement `00b45360` or downstream `00b44750`.
