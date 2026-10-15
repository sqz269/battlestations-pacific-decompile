# Complete authored-path point wrappers

Addresses: `007B10A0`, `007B10C0`.

| Native range, inclusive | Coverage | New raw Source entry | Original physical ABI |
|---|---|---|---|
| `007B10A0..007B10B8`,25B | complete | `get_native_authored_path_point_007b10a0` | ECX owner, stack output/index, RET8, EAX captured output |
| `007B10C0..007B10FF`,64B | complete | `get_last_native_authored_path_point_007b10c0` | ECX owner, stack output, RET4, EAX captured output |

Existing `BSP_AuthoredPathSource_GetPoint` / `BSP_AuthoredPathSource_LastPoint`
descriptive names and earlier semantic/host implementations remain unchanged.
These new naked Win32 entries operate on actual owner/path/output storage and
compose the genuine complete raw scene-point routine, rather than a virtual
host facade, private vector or invented callback. Source EDX is an unused binding
placeholder; the original consumed ECX and stack shapes remain.

## Exact captures, index and return schedules

Get-point reads the physical index argument at entry ESP+8 BEFORE loading the
actual path from owner+8. Save ESI, then capture the physical output argument
from shifted ESP+8. Push captured index and output, call actual scene-point with
ECX captured path. On normal return, force EAX to the saved output, restore ESI,
RET8. The wrapper does not rely on the child's returned EAX or reread either
incoming argument afterward.

Last-point first captures owner+8 into ECX, then path+8 first into EDX. With
null first, save ESI/capture output; retain the redundant XOR EAX,EAX, then MOV
EAX,FFFFFFFF and invoke the real scene-point routine. This path does not load
end. With nonnull first, read path+Ch end, SUB first moduloDWORD, SAR EAX,2;
save ESI/capture output; ADD EAX,-1 moduloDWORD, then invoke the same real child.
Both branches force saved output into EAX, restore ESI and RET4. A nonnull empty
span also computes FFFFFFFF. No empty return, modern count/index repair, range
guard, owner/path null recovery or replaced state is added.

All actual argument cells, branch flags, reads, captures, saves and cleanup remain
literal. Only the three direct CALL operands change. Every reached owner/path/
pose/point/output/stack/control access needs backing; malformed storage is not
made safe by this public raw interface.

## Layout, calls and actual dependency

The complete consumed producer at `007B22A0` allocates0Ch, writes the captured
EDX path at owner+8 (`007B22C3`), refcount1 at+4 and the D0534C profile at+0.
This reconciles the existing authored-owner schema. The wrappers read only +8;
no complete original class or factory reconstruction is claimed by this packet.

All three transfers have the same genuine target:

```text
007B10AE CALL 007AF800
007B10D8 CALL 007AF800
007B10F5 CALL 007AF800
```

The accepted `transform_native_scene_path_point_007af800` consumes ECX actualpath,
the same stacked output/index and RET8/EAX output. Its new Source EDX binding is
also unused, so these callers require no extra adapter or private argument copy.
The complete211B Source kernel and its actual pose/transform/current invalid-policy
providers are already reconstructed and reviewed. Its captured pose, current point
slot reload after returning policy, x87 reciprocal/input-cell spill and late output
load retain their own evidence and Source stack/frame/CRT qualifications.

Live refs and physical E8/E9/literal census agree: no direct calls; one read-only
data pointer each at D05354 and D05364, corresponding to D0534C slots+8/+18h.
That identifies static table references, not all possible indirect callers. These
new functions do not install or replace a production table or original game binding.

## Verification and boundaries

Project/program checked: `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`,
base00400000. Both complete listings/live bounds, physical refs, owner producer,
old Source schema/API and genuine child entry were read. Strict MSVC Win32
`/MD /std:c++17 /EHsc /W4 /WX /permissive- /O2 /fp:strict /FAcs /c` passes.

`local/goal15h_authored_path_points/compare_bytes.py [COFF-object] [live-directory] [PE-path]`
accepts the primary registered object. All25+64B match fresh live/original disk
bytes and the emitted object after ONLY three CALL operands are masked. Each
exact Source scene-point symbol, REL32 kind, E8 opcode and decoded original
7AF800 target is verified. All77 fixed bytes, including all RET tails, match.

One ignored fixture, `local/goal15h_authored_path_points/authored_points_probe.cpp`,
uses actual raw owner/path/clean pose/point slots and genuine scene-point/canonical
providers. Indexed1 and last(count2) return the captured output and actual last
point. With null first/end, last-point passes FFFFFFFF. A temporary actual SDK
thread-local invalid handler repairs actual path begin to slots+1/end toslots+2
and returns; it does not repair the index. The child's capturedFFFFFFFF*4 wraps
to the actual preceding slot0, returning the first point with one handler call.
Both slot cells/points are backed, so this distinguishes retained underflow from
modern index repair. Handler state is restored. Numeric owner profile remains
data; calls use the actual public Source entries directly.

The fixture uses identity math only and owns its isolated FP environment; the
production wrappers change no FP policy. This is bounded Source composition/raw
backing evidence, not a Native FP differential or general guarantee for empty
storage. Run `local/goal15h_authored_path_points/run_probe.cmd [archive] [archive-only]`.
Default overlays this module AND the already accepted scene-point Source on the
primary existing Release archive while registration is pending; archive-only links
the registered archive. Manifest is embedded. The fixture and report/diff checks
pass. Primary owns CMake, full registered build/archive reproduction, saved
annotation/export and integration. No tracked test target, Native runtime/private
CRT/stack-address/SEH equivalence, virtual installation or gameplay validation is
claimed. Older semantic/host API records remain distinct and unmodified.
