# Actual hidden-entity map rotations

Addresses: `00467620`, `00467670`.

The two new ordinary C++ bodies operate directly on existing actual tree/node
storage. There are no callees, host/provider dependencies, callbacks, copied
owners, node overlays or STL substitutes. The current index and source search
had no reconstruction for either address before this packet.

| Native library name, preserved | Body | Coverage | Source interpretation |
| --- | --- | --- | --- |
| `STL_Tree_Rotate_00467620` | `00467620..0046766D`, 78 bytes | complete, qualified actual-storage Source | left rotation |
| `STL_Tree_Rotate_00467670` | `00467670..004676C1`, 82 bytes | complete, qualified actual-storage Source | right rotation |

Left/right are descriptive hypotheses derived from the body and established
producer layout, not recovered original symbols. Native inputs are ECX actual
map and one stacked pivot pointer, all paths `RET 4`; there is no semantic
result. Native EAX happens to retain the promoted node but all nine known call
sites ignore it. The Source APIs retain the physical slots with explicitly
unused EDX through `__fastcall`:

```cpp
void __fastcall rotate_left_native_scene_record_map_00467620(
    void* actual_map, std::uint32_t unused_edx, void* actual_pivot);
void __fastcall rotate_right_native_scene_record_map_00467670(
    void* actual_map, std::uint32_t unused_edx, void* actual_pivot);
```

The layout authority was already inspected in the preceding lookup packet and
remains `docs/SCENE_RECORD_MAP.md` plus the actual producer. The constructor
listing `00469F50..00469FBF` writes node links +0/+4/+8, places key length/data
at +0Ch/+10h and writes node+19h=0. The insertion producer at `0046AD70`
publishes root at head+4 and links children through parent+0 or parent+8.
The lookup walk `00468140` and insertion descent `0046B340` agree on left+0,
parent+4, right+8, map+4 head and node+19h sentinel. This packet adds no node
structure, conflicting field declarations or record layout. Color, keys,
mapped value, proxy and map size are not read or written by these bodies.

The left-rotation schedule is preserved exactly:

1. Capture promoted from pivot+8. Read promoted+0, store that pointer into
   pivot+8, then **reread** promoted+0. Aliasing may make those values differ.
2. Test the reread child's +19h byte. Only when it is zero, write child+4=pivot.
   A sentinel child does not receive a parent write.
3. Read CURRENT pivot+4 and write it into promoted+4. Only then load CURRENT
   map+4 head and read head+4 root. Earlier writes may alias these cells.
4. If pivot equals that current root, write the captured head+4=promoted.
   Otherwise reread CURRENT pivot+4, test its left+0 against pivot, and write
   parent+0=promoted on equality or parent+8=promoted otherwise. There is no
   extra check that the latter field had pointed to pivot.
5. Write promoted+0=pivot, then pivot+4=promoted, in this order on every path.

The right-rotation schedule mirrors the native fields, rather than rebuilding
a conventional tree result: promote pivot+0; transfer and reread promoted+8;
test that reread child sentinel; repair its parent only when zero; read current
pivot parent and write promoted parent before head/root lookup. Its nonroot
branch tests parent's current right+8 and chooses right+8 on equality or left+0
otherwise. The final stores are promoted+8=pivot, then pivot+4=promoted.

Every external DWORD read/store and sentinel-byte read uses volatile raw
access. Captures and reloads follow the assembly. In particular, Source does
not cache the transferred child across the first write, cache pivot parent
across promoted-parent publication, or load map head before those writes.
Common final stores preserve branch-specific write order. No validation,
cycle check, sentinel-parent normalization, color repair or rollback is added.
There is no local exception handler, implicit destruction or provider dispatch.

All live callers were reviewed before assigning the interpretations:

| Caller body | Left-call sites | Right-call sites |
| --- | --- | --- |
| `0046A470` | `0046A612`, `0046A661`, `0046A6BF` | `0046A645`, `0046A676`, `0046A6D9` |
| `0046AD70` | `0046AE85` | `0046AEA3`, `0046AED1` |

Each is an ordinary CALL with ECX map and one pushed pivot, cleaned by child
`RET 4`. For the first caller, ECX comes from the saved original receiver at
current ESP+10h and is reloaded after earlier rotations; its entry saves that
receiver through EBX before the later EBP/EDI pushes. Insertion's ECX comes
from the saved receiver in EDI. The relevant full listing pages and exact
instruction contexts establish receiver/pivot setup; the containing functions
were checked live. Post-call instructions overwrite EAX or ignore it until a
later overwrite. Caller behavior, colors and balancing are not reconstructed
by these helpers, and the caller files are not edited here.

The report retains live original bytes, per-body SHA256, all nine exact call
rows, previous native names/comments and verification state. All live clients
verify project `bsp`, program `/battlestationspacific.exe`, x86 language and
image base; configured project file `C:/Users/sqz269/bsp.gpr`. Locked Ghidra
annotation preserves established names/comments, records prior values, saves
the project and refreshes both exports.

The registered source (`312322c24`, registration `f3fb169f4`) passed
`scripts/build.ps1` Release Win32: the new translation unit compiled, all targets
linked, and both enabled existing CTests passed (`reconstructed_math`,
`tool_tests`; 7.87 seconds). The log is
`local/goal_oct15_scene_map_rotations_build.log`. Native math differential is
not enabled in this worker tree. All nine report CALL rows and staged
whitespace checks passed. No new tests were added.

Compilation and existing tests are recorded separately in the companion
report. Physical slots and static complete
coverage do not establish original mangling/binary ABI, native private-frame
or instruction-fault identity, direct native rotation execution or game
validation. Source requires valid backing for every taken DWORD/byte access;
the native private argument/frame storage is not exposed by its value inputs.
