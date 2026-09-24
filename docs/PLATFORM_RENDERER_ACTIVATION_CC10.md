# Renderer activation (cc10)

This packet reconstructs the remaining renderer control paths called by
`WM_ACTIVATE`: `B24FB0` and `B0D1E0`. It provides concrete registry scans, the
known renderer virtual `+120` operation, and raw render-service invalidation.
The effect reload, texture reload and material texture refresh leaves remain
required bindings. This is a bounded provider, not complete production renderer
activation. No window-message ordering or application owner was added here.

Evidence was rechecked read-only in `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, using the BSP wrappers. Nine bodies, 744 bytes,
matched live Ghidra bytes and the installed PE. The report records their hashes,
exact exclusive ends, call sites, coverage and remaining dependencies. Existing
descriptive Ghidra names were preserved; no Ghidra state was changed.

| Native range (exclusive end) | Source coverage | Native input/cleanup |
|---|---|---|
| `B24FB0..B24FE1` | Whole control body for current slot `+120 = B24DD0` | ECX renderer; RET0/tail B21F70 |
| `B22030..B220CE` | Whole effect registry scan; required reload leaf | ECX registry; RET0 |
| `B21F70..B2200E` | Whole texture registry scan; required reload leaf | ECX registry; RET0 |
| `B24DD0..B24E11` | Whole material scan; required B19000 leaf | ECX renderer; RET0 |
| `B0D1E0..B0D210` | Complete normal body | ECX service; RET0/tail B4ECC0 |
| `B50010..B50018` | Complete | ECX batch; RET0 |
| `B4ECC0..B4ECF2` | Complete normal body, fixed null-root call | ECX owner; RET0 |
| `B72220..B7224C` | Complete raw unlink | ECX root, stacked node; RET4 |
| `B6D890..B6D934` | Only requested-root-null path | ECX node, stacked root=0; RET4 |

All ranges include the complete final instruction. The null-root projection
includes `B6D890..B6D8C6` and `B6D916..B6D934`; it excludes the nonzero-root
registration, lighting and virtual calls at `B6D8C6..B6D916`. Existing
`propagate_native_node_root_00b6d890` and `unlink_render_root_node_00b72220`
remain the typed host-view providers. Their `RenderNodeRootList` stores references
and is not an actual native root object with its head at `+0C`. The new raw
projection is confined to the actual render-service domain and creates no
parallel hierarchy or owner.

The original register bodies were inspected rather than adopting Ghidra's
zero-argument prototypes. B24FB0 and B0D1E0 preserve ESI around their tail calls;
B22030/B21F70 preserve EBX/EBP/ESI/EDI and consume no caller stack arguments;
B24DD0 preserves ESI/EDI; B4ECC0 and B72220 preserve ESI; the null B6D890 path
preserves EBX/ESI and recursively pushes the same zero root. BDD340 receives two
stack arguments and cleans eight bytes. Scratch EAX/ECX/EDX and flags have no
exposed source return contract. These C++ entry points are not ABI replacements
and do not claim native exception/unwind equivalence.

## Captures and mutation ordering

`B24FB0` samples canonical byte `108D4BB` once and captures its ECX receiver in
ESI. When enabled it scans renderer `+1A98`, reloads that receiver's current table
and virtual `+120`, then tail-scans renderer `+1A74`. It does not reread global
`F8D394` between these steps or recheck the gate. The actual table cell
`D5F0A8+120 = D5F1C8` contains `B24DD0`. Source dispatch verifies that current
numeric target and explicitly rejects unsupported targets; it never silently
substitutes the known method for an unknown table.

Both date scans capture registry `+8` count before `+4` data and form one
wrapping DWORD end using 0x2C-byte rows. They reload current `109CEEC` for each
BDD340 call. Existing `NativeRenderResourceRecord` supplies the exact native
layout: the name header starts at row zero, five date words are at `+14..+24`,
and the resource is at `+28`. Comparison is lexicographic and unsigned across
all five words. On a strictly newer date all five stores precede rereading the
row resource and its current table's `+8` target. Callback changes to registry
count/data do not recompute this scan's end or restart it.

In contrast, `B24DD0` captures the original row cursor at renderer `+1A9C`, calls
B19000 on each current row's resource, then reloads renderer count `+1AA0`
before data `+1A9C` to recompute the end. The original cursor advances by 0x2C.
If a callback relocates the array, its old cursor must remain readable exactly
as in the native caller. No extra owner retention, null repair, date reset or
rescan was introduced.

`B0D1E0` captures the service, checks byte `+1C4` and current nonnull `+20`, and
calls B50010 to set child byte `+24C`. It then rechecks service `+1C4` and current
nonnull `+30` before its B4ECC0 tail. B4ECC0 captures owner `+3C` before setting
owner byte `+250`, tests the captured root's `+0C` head, and reloads current
owner `+3C` and head for each null-root propagation. It finishes by setting
owner byte `+251`. B72220 patches neighboring links or root head without clearing
the removed node's own links or releasing it. Null B6D890 preserves the native
early return when root is already null and parent is nonnull; that case does
not recurse into children. Otherwise it unlinks a top-level registered node,
clears `+A4`, and recursively walks children `+34`, reloading each child's next
`+3C` after recursion.

## Production binding requirements

The primary application must borrow the current domains already owned by
`GameNativeRendererApplication`; none are constructed by this module:

* `game_native_shader_process().modes().reload_resources_0108d4bb` is the same
  process mode cell used by startup parsing. Do not copy or force it false.
* The argument to B24FB0 is the renderer captured by the actual call site
  `BED4B9`. Preserve that receiver across all three operations.
* The context borrows the real volatile `0109CEEC` publication by reference.
  `NativeRendererActivationVfsBindings` implements the actual BDD340 provider
  using the existing `NativeVfsDateRouteContext` and `NativeStringRawPoolContext`.
  Bind the same VFS date and raw pool cells used by renderer loading; this
  adapter does not fabricate dates or mount state.
* Supply two required leaf methods: current resource virtual `+8` dispatch,
  and B19000 material texture refresh. Numeric table `D61A00+8` is B469A0;
  `D61948+8` is B3FA90. Dispatch must use the passed current target and actual
  receiver, preserving the established compiler/cache/owner domains.
* The B0D1E0 argument is the current nonnull `00F8D39C` service selected after
  the focus calls and fullscreen check at `BED4EE`. Use
  `RenderResourcesGraph.base` and its actual-service publication. Do not cast
  `RenderNodeRootList` or synthesize a second service/root graph.

B14A10 initializes service `+1C4`, `+20` and `+30` to zero. Its production
resource graph remains deferred pending B107F0; observing these zero fields does
not establish initialized fullscreen resources. B107F0 call sites B10832,
B12FF6 and B1300B were inspected as bounded producer-use evidence only, not
recovered as a complete producer. No fabricated layout or default owner fills
this gap.

The historical raw focus module in `84c6fe880` was inspected and its relevant
logic recovered into this owned module after current body/layout checks. The
historical B469A0 implementation in `769bdc3e3` was also inspected. At this packet's
base, it was not directly usable: it requires
`NativeMaterialEffectDestructionAccess.actual_descriptor` and a three-argument
`release_native_material_effect_owners_00b41b10` overload absent from the current
owner API. Its numeric-descriptor guard must not be removed to make it compile.
The primary assigned that adaptation to a separate worker. B19000 (462 bytes)
and B3FA90 (747 bytes) remain outside this packet; their bounded bodies show
actual texture-cache/retained-owner work and texture/VFS/device recreation,
respectively. A callback interface alone does not complete those operations.

## Verification and limits

`scripts/build.ps1` passed in this isolated worktree, including both existing
CTest tests. The call-row verifier passed. A local, uncommitted x86 `/MD` probe
linked the freshly built `bsp_core.lib` with `/MANIFEST:EMBED` and compared the
source against the exact native bodies, relocating only fixture input cells and
explicit external leaf calls. It exercised disabled B24FB0 with a null receiver;
enabled activation with four date calls, two reloads and two material refreshes;
unsigned high-bit and fifth-word date comparison; mutation of the current VFS,
resource, gate and receiver table; fixed date extent and expanding material
extent; and four B0D1E0 branch combinations with a real raw root/child topology.
Full raw snapshots and call events matched. A separate source case verified
explicit rejection of an unsupported renderer slot.

The fixture's date, resource reload and B19000 leaves are observers, not native
resource implementations. It does not establish VFS date correctness, real
shader/texture recompilation, initialized B107F0 resources, exception behavior,
ABI compatibility, normal application activation, graphics or game parity.
Gate-false behavior alone is not complete application validation.
