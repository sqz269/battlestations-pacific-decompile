# GOAL15B byte-vector resize, insert-fill and checked erase

Complete Source reconstruction, 2026-10-15. Names are hypotheses, not recovered
template symbols. Existing `STL_inst_00420dd0` and `STL_inst_0041d7d0` library names
are retained. The three kernels use concrete allocator/free, byte primitives and
current CRT services. There are no abstract success bindings or installed game
bindings. Source CRT and exception identity remain qualified.

| Native routine | Inclusive body | Native argument/return evidence | Coverage |
| --- | --- | --- | --- |
| `004219A0` | `004219A0-00421A48` | ECX vector; stack count and fill DWORD low byte; `RET 8` at `004219F2` / `00421A46` | complete owned instruction schedule, concrete dependencies |
| `00420DD0` | `00420DD0-00420F72` | ECX vector; stack iterator owner/position, count, fill pointer; `RET 10h` at `00420EE7` / `00420F38` / `00420F70` | complete owned instruction schedule, including all allocation and spare-capacity branches |
| `0041D7D0` | `0041D7D0-0041D81F` | ECX vector; stack result iterator pointer, first owner/position, last owner/position; EAX result pointer; `RET 14h` at `0041D81D` | complete owned instruction schedule, including returning-handler continuation |

All live queries used `bsp.py`, whose bridge verifies the configured
`C:/Users/sqz269/bsp.gpr`, project `bsp`, program `/battlestationspacific.exe`,
language `x86:LE:32:default`, image base `00400000`. Full exported listings of all
three bodies were inspected; live prototypes confirmed their inclusive ranges.
No missing-flow repair or Ghidra mutation was required or performed by this worker.

## Schema and calling context

The public header reuses `Cc13AvoidZoneRawVector`: proxy +00h, first +04h,
last +08h, end +0Ch. `00424680` writes zero to TerrainGridLayer +1Ch/+20h/+24h
before calling its dimension setter, confirming the existing producer schema.
All three resize callers pass layer +18h in ECX and a fill DWORD of zero:

| Call site | Containing routine | Requested size |
| --- | --- | --- |
| `0042399E` | `00423960` SetDimension | low DWORD of signed `n*n`; pushes zero at `00423991` |
| `00423A6D` | `004239E0` Deserialize | low DWORD of signed `n*n`; pushes zero at `00423A65` |
| `00423B67` | `00423AF0` ResetLayers | 100; pushes zero at `00423B48` |

These caller bodies were read around their resize setup. Their later memset or
stream read is outside this family. The insert routine has only resize as a
caller; checked erase also has only resize as a caller. The public Source APIs
use `__fastcall(vector, unused_EDX, remaining native stack payload)`. This makes
the Source padding register explicit while retaining the observed stack shapes;
it does not establish recovered C++ member or production binary identity.

## Preserved behavior and alias schedule

- Resize computes size as zero for null first, otherwise last-first in DWORDs.
  Grow caches the old last and old size before validating order, and forwards
  the address of the low byte of its actual second stack argument. Shrink caches
  old last, reloads first after a returning handler, applies the two subsequent
  range checks in order, and erases through the cached endpoint. Equality is a no-op.
- Insert reads `*fill` at `00420DD9`, before the header and count, even for count
  zero. It saves that byte in the original fill-pointer stack slot. Later fills
  read this saved byte, preserving aliases with bytes/header storage that moves
  or allocation may change. The iterator owner word is not checked or consumed.
- Length checking uses unsigned `count > UINT32_MAX-size`. Reallocation selects
  max(capacity+capacity/2, size+count); overflow in the geometric candidate sets
  that candidate to zero before the minimum-size comparison. There is no added
  dimension limit, null repair, shape admission or alternate capacity policy.
- Reallocation allocates, reloads old first, copies a nonzero prefix, fills,
  reloads old last for the suffix, copies a nonzero suffix, then reloads old
  first/last to compute the final size and free the current old first. Header
  stores remain first, end, last. A handler that returns does not trigger a snapshot
  restore. No new EH rollback frees partially created storage.
- With capacity available, unsigned tail<count chooses forward-copy, fill the
  extension, increment the current header last, then fill the insertion range.
  The other branch copies the last count bytes, stores the returned new last,
  copies the middle backwards, then fills. Forward copy guards only nonzero
  length; backward copy guards signed-positive length. Both ignore secure-copy status.
- Checked erase validates nonzero/equal iterator owners, without requiring that
  owner to equal ECX. It loads the cursor words after the validation call returns.
  Different cursors compute signed-positive tail copying; zero/negative tail
  skips the copy but still writes the computed last. It then stores result.position
  followed by result.owner and returns the result pointer. Header/data/result
  overlap is retained by raw instructions, including output aliasing the header.

Native `00BF6713` can return. The kernels call the existing qualified
`invoke_native_invalid_parameter_00bf6713` and retain every subsequent branch,
cached word and reload. Its actual Source `_invalid_parameter_noinfo` policy
does not reproduce Native encoded handler state, debugger/Watson delivery or
original frame/return/fault addresses. No `noexcept`/must-not-return claim is added.

## Concrete dependencies

| Native dependency | Source implementation and evidence |
| --- | --- |
| `0041F450`, `0041C630`, `0041C650`, `0041E630` | Public concrete `native_byte_range_primitives.hpp/.cpp`, packet `goal15b_byte_range_primitives`, commit `72cb207f9c30d5f39a758378dbcff238173acb32`; full native bodies read, calling conventions matched, per-byte alias reloads and copy guards retained |
| `00418540` -> `00BF681B` | Private consumed allocator calls existing `singleton_lifetime_allocate({object,n,n})`; actual malloc(n)/new-handler(n) retry/bad_alloc, paired with the existing free |
| `00BF65AC` | `singleton_lifetime_free`, actual current `std::free` |
| `004200D0` | Genuine current `std::length_error("vector<T> too long")`; native body constructs length_error then calls CxxThrow, so this boundary genuinely does not return |
| `00BF67A7` | Genuine current SDK `memmove_s`, including current handler/errno behavior; native body was read completely, no fake success callback |
| `00BF6713` | Existing qualified Source entry, with returning continuation preserved |

The old allocator annotation incorrectly said n=0 returned null. Full listing
`00418547 TEST ECX; 00418549 JA positive; XOR ECX; PUSH ECX; CALL 00BF681B`
shows allocation of zero. For every positive DWORD n, UINT32_MAX/n>=1, making
the allocator's later bad_alloc precheck unreachable. The primary owns this
annotation-only correction; this worker does not add or rename that address.
Consumed allocation always passes exactly n to the existing provider, including zero.

Current Source exception objects/type descriptors, CRT state, heap/handler hooks
and provider frames are not original-game identities. Native signedness/template
spelling is not recovered beyond these byte effects. All reached header/data,
iterator/result, stack/control and provider backing must remain valid when reached.
The kernels add no guards that would hide original accesses or returning-handler paths.

## Verification and remaining work

- Strict MSVC Win32 `/std:c++17 /EHsc /MD /W4 /WX /fp:strict /O2 /c` passes.
  The stable sibling primitive header is used until its independent commit is merged.
  Reproduction: ignored `local/goal15b_grid_byte_vector_compile.cmd`.
- Static COFF-vs-live-program comparison passes for all three complete bodies:
  resize 169 bytes / 6 calls, insert 419 bytes / 12 calls, erase 80 bytes / 2 calls.
  Only the 20 CALL rel32 operands were masked; every other byte, branch, cleanup
  immediate, memory/register operation and body size matches. Reproduction:
  ignored `local/goal15b_compare_owned_bytes.py` and the captured live-byte files.
  This is static owned-byte evidence, not Native execution or provider/ABI proof.
- The report records all 20 owned direct calls plus the three resize caller sites
  with correct containing functions. `verify_report_calls.py` passes all 23 rows
  with zero failures.
- One ignored Source-only fixture is prepared and strictly compiled at
  `local/goal15b_grid_byte_vector_probe.cpp/.obj`. It uses real allocator/free and
  public entries, and covers allocation, both spare-tail branches with overwritten
  fill aliases, shrink/erase result, output/header overlap, valid zero-count alias,
  a genuine Source length_error, and an installed/restored genuine current SDK
  returning invalid handler with null owner words but valid cursor/data backing.
  That last case checks one report, continued tail move, and cached null output owner.
  The zero-count read itself is established by
  emitted instructions, not by claiming an unobservable ordinary read was observed.
- Registered full Win32 build/CTest and linking/running that Source fixture are
  primary integration work. Link the probe against the actual Release bsp_core,
  Lua/zlib archives and project system libraries, with `/link /MANIFEST:EMBED`
  and executable name `goal15b_grid_byte_vector_probe.exe`. No new test target or
  framework is added. No Native differential, game/runtime or drop-in ABI claim.
- Only the three leased entry name/function records are added. No shared CMake,
  packet registry or Ghidra write is made. Keep the lease through integration.
