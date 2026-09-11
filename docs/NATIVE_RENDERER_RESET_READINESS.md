# Native renderer reset readiness

This packet reconstructs four complete original entries, **653 code bytes**:

| Entry | Bytes | Original behavior and ABI |
| --- | ---: | --- |
| B1FD90 | 70 | Restore current dynamic wrappers; ECX renderer, RET |
| B20C50 | 31 | Active platform has current thread focus; ECX platform, EAX 0/1, RET |
| B492B0 | 276 | Recreate physical vertex buffer; ECX wrapper, stacked device, RET 4 |
| B49180 | 276 | Recreate physical index buffer; ECX wrapper, stacked device, RET 4 |

The two recreation bodies also use complete 16-byte jump tables at B493C4 and
B49294. Descriptive names remain reconstruction hypotheses. Fresh guarded
queries verified `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, and
16 live/installed-PE spans totaling 1,218 bytes. The installed binary SHA256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Exact spans, original ABI notes and artifact pins are in the accompanying audit.

## Source boundary and concrete producers

The renderer source accepts actual raw storage and an 8-byte
`NativeRendererResetReadinessProfiles` reference in EDX. That context borrows
all nine immutable original DWORDs of pooled index profile D61E58 and pooled
vertex profile D61E7C. Raw wrapper word +00 contains the original numeric
profile identity. These addresses identify original data; the source resolves
their captured +20 selectors to the two complete new raw providers. No host
callback table, semantic wrapper cast, arbitrary allocator or success policy
is introduced.

This is an explicit **two-profile source domain**. Native B1FD90 itself performs
unrestricted virtual dispatch. Unsupported identities/selectors are outside
the new interface's contract; the source's `__assume(0)` defaults are not native
validation or recovery. Context pointers must designate the immutable original
profiles and remain valid for the call. The ready/lost fast paths do not read
them. The source does not require wrapper +1974 to remain a vertex wrapper or
+1978 to remain an index wrapper: it resolves each current proven profile.

The restriction has concrete producer evidence. Full pooled constructors
B4BB60[51] and B4BBB0[51] write D61E58 and D61E7C respectively. The bounded
B2AEB0 initialization span B2B067..B2B11A allocates 2Ch wrapper storage, calls
B4BBB0 and publishes its result at renderer+1974, then calls B4BB60 and publishes
its result at +1978. This producer span is evidence, not reconstruction of the
whole B2AEB0 parent. The pooled profile +20 slots contain B49180 and B492B0.
Private profiles D61E10 and D61E34 instead contain B4B810 and B4B9C0; those
different implementations remain outside this packet.

Existing `D3D9BufferBinding` and the helpers in `d3d9_buffers.cpp` are semantic
interfaces with incompatible storage and failure behavior. In particular, they
reject unsupported pools and conditionally publish successful results. Their
prior readiness labels did not establish a raw provider contract. This packet
adds separate APIs and preserves those existing APIs unchanged.

## Ordered raw behavior

B1FD90 tests raw ready byte +1D8C and then lost byte +1D8A, returning for either
nonzero value. It loads current wrapper +1974, loads current device +1A10,
publishes ready=1, then reads that wrapper's current profile and its +20 target.
After the first call returns, it loads current wrapper +1978 and its current
profile, reloads device +1A10, and only then reads that profile's +20 selector.
It does not reread the gates, roll ready back, check a returned HRESULT, or
establish a guard. The compiled source preserves this storage-access order.
The original incidental EAX is not a declared semantic result of the parent;
the new C++ parent does not promise its value on fast paths.

Each recreation API takes an explicit unused EDX argument so that the actual
device remains the third, stacked argument. Its **own by-value callee argument
word**, at entry ESP+4, is also passed as CreateBuffer's output address. It is
not initialized to null. This is not a pointer to a renderer device field or
a source-side snapshot. The complete original instruction sequence is retained,
including its stack cleanup and incidental EAX.

Raw wrapper fields are flags +14, capacity +18 and owned COM pointer +28.
Pool nibble values 0..3 select those same pool integers. Pool 0 additionally
sets local flag 10000h. A nibble greater than 3 falls through with the current
device argument's **pointer bits as the pool value**. Usage is assembled in
the original order:

| Raw flags | Usage contribution |
| --- | ---: |
| bit 10h | 1 |
| F00h group: 100h / 200h / 300h / 400h / 500h | 2 / 4000h / 40h / 100h / 80h |
| F000h group equals 1000h | 200h |
| F0000h group equals 10000h | 8 |

The vertex call uses actual device slot +68 and FVF 0; the index call uses
slot +6C and format 65h. Both pass a null shared-handle pointer, read current
capacity at the original outgoing-argument position, and ignore HRESULT.

After Create returns, the body captures current old COM +28 and compares it
with the current callee output word. If different, it publishes the output
to +28 before AddRef, AddRefs a nonnull new output, reloads the output word
after that call, Releases the captured old COM, then reloads the output word
again. The final nonnull current output is Released as the temporary reference.
When old==output it skips publication/AddRef/old-release but still performs
the final temporary Release. Failed/null results retain this same behavior:
there is no success branch, null fallback or preservation policy. Exceptions
do not gain synthetic cleanup. No wrapper allocation, intrusive retain,
wrapper destructor or pool return is reached by these four bodies; COM
AddRef/Release owns the reached terminal lifetime behavior.

B20C50 tests raw platform byte +41. If active, it calls the real GetFocus import
at original IAT cell CE2310, then compares EAX with the **current** HWND DWORD
at +30. Equality returns exactly 1; the inactive/mismatched paths return 0.
The original byte is not interpreted as a canonical C++ bool before its test.
No focus-changing API or injected focus context is part of the source.

## Validation and practical limits

The ignored source hook compiled this file into the actual MSVC Win32
`bsp_core.lib` with strict floating-point settings. The standard build, both
existing CTests and all eight native seed checks passed. No permanent tests,
shared CMake changes, ledger edits, Ghidra changes or installed-game changes
belong to this worker packet.

Whole COFF proof matches both 276-byte recreation bodies and the 31-byte focus
body to the originals, normalizing only one DIR32 address operand per body.
Both complete pool tables contain the corresponding relocated case addresses;
there is no runtime initializer. The full 144-byte compiled parent is also
verified from COFF through linked and runtime bytes, with its original storage
read/write ordering checked separately. It is not byte-identical to B1FD90.

The focused ignored fixture executes all four complete original bodies at
their original addresses in a dedicated fixture-owned PE section. Original
code, jump tables and profiles remain unchanged. Only the original GetFocus
IAT cell is bound to the actual imported OS function. The original parent
therefore invokes the complete original recreation children; the source parent
invokes the full built raw library providers. Setup uses the existing complete
raw pooled constructors and cleanup uses the complete raw COM reset-release
providers, linked from the same frozen library. Wrapper storage is borrowed
2Ch storage with native flags/capacity and genuine nonempty COM ownership;
no whole-wrapper allocator/pool/destructor proof is claimed.

Four paired cases cover inactive/equal/mismatched real focus and both gate
returns; a first creation callback replacing the second wrapper/profile and
current device while changing both gates; two callback mutations of the exact
callee output word after real AddRef and old Release; and an actual invalid-pool
CreateVertexBuffer failure. The observed failure was 8876086Ch with a real null
output, followed by the native old binding release. No HRESULT or output was
substituted to manufacture that failure.

The fixture used two real HAL devices on the existing desktop HWND. It called
no CreateWindow, SetFocus, SetForegroundWindow, ShowWindow, DestroyWindow or
presentation method. Actual Create/AddRef/Release methods were forwarded by
observers that recorded their original DLL targets and call sites. It compared
224,152 bytes of paired state/trace, captured 58 raw renderer/wrapper states,
recorded 80 real COM calls, and observed 28 buffer terminal-zero releases.
Only the actual renderer device field and wrapper COM fields are normalized
by recorded object identities; all raw captures remain available. Complete
buffer vtables remain unchanged during each observed lifetime except the two
declared observer entries, which forward the real methods.

The sealed bundle pins the actual archive, four exact extracted library
members, fixture object, source/header inputs, 328 whole mapped COFF sections,
1,633 relocations, 184 mapped functions including inline fixture functions,
22 complete runtime spans across nine postimage phases, and actual imported
module identities/bytes. See `local/reset_readiness/sealed.json` in this worker
worktree and the tracked audit for immutable hashes and replay commands.

This is build, object-byte and focused original/full-library fixture evidence.
It does not establish every callback/exception path, native SEH compatibility,
arbitrary wrapper profile support, whole reset-parent reconstruction, a general
drop-in binary replacement, or installed-game validation. The source performs
unchecked raw 32-bit accesses; invalid addresses and unhandled COM behavior
retain the stated native or explicit source-domain limits.
