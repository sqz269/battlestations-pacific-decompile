# Canonical typed atlas registry import supplier

Complete ignored Source candidate119 against Main68ba833ccf775c7a871e56984d0392278e635564.
Five source postimages and exact preimages are pinned. The finite LF patch passes
read-only git apply --check. No tracked source edit, build/test, Native call,
Game, activation, Ghidra mutation, commit or push occurred. Root owns admission,
independent review, Win32 build and default-startup regression.

## Recovered import evidence and exact source types

The installed executable SHA256 and four narrow PE import records are pinned in
ORIGINAL_IMPORT_EVIDENCE.json. All four descriptors name KERNEL32.dll:

| Original IAT cell | Function | Original thunk RVA / hint-name VA |
|---|---|---|
| 00CE2210 | LeaveCriticalSection | 00A05688 / 00E05688 |
| 00CE2218 | EnterCriticalSection | 00A056B8 / 00E056B8 |
| 00CE221C | InterlockedIncrement | 00A056D0 / 00E056D0 |
| 00CE2220 | InterlockedDecrement | 00A056E8 / 00E056E8 |

Original IAT DWORDs are import-name RVAs, not callable host-code addresses.
The saved/live Ghidra bytes agree with these exact slots and name records.
The live Ghidra launch command selected C:/Users/sqz269/bsp.gpr and the guarded
bsp status query verified /battlestationspacific.exe. No program mutation occurred.
Installed Windows SDK10.0.26100.0 synchapi.h declares VOID WINAPI Enter/Leave
CriticalSection(LPCRITICAL_SECTION); winnt.h supplies the global _RTL_CRITICAL_SECTION
tag. The headers forward-declare that actual global tag rather than introducing
another section type or requiring Windows.h in the public source headers.

GameNativeRendererSectionImport and NativeShadowTargetSectionCall become
void(__stdcall*)(::_RTL_CRITICAL_SECTION*), exactly the existing atlas registry
section-call type. Existing GameNativeRendererAtomicImport is already exactly
long(__stdcall*)(volatile long*), matching NativeTextureAtlasRegistryDecrement.
Consequently the future actual atlas caller can bind these expressions directly:

    NativeTextureAtlasRegistrySectionCall const volatile& enter = process.enter_iat_00ce2218();
    NativeTextureAtlasRegistrySectionCall const volatile& leave = process.leave_iat_00ce2210();
    NativeTextureAtlasRegistryDecrement const volatile& decrement = process.decrement_iat_00ce2220();

These bindings add const qualification to the same existing volatile pointer
objects. They need no reference reinterpretation, callable-pointer copy or new
shadow import cell. Type identity is established by the exact alias declarations
and the unchanged direct production aggregate binding; compilation remains a
separate Root check. No extra tests or generic import framework are added.

## Finite implementation and preserved epochs

A finite source search finds one direct production consumer of the canonical
section cells: src/game_native_renderer_shadow.inc:12 in the base. It still
passes those same process accessors into NativeShadowDepthTargetBindings, whose
two fields remain const volatile references. That fragment has a comment-only
hunk documenting the borrow. The operation's two captured callable fields retain
the same alias, and every existing capture/reload/current-cell epoch is preserved.
Only the actual two invocation arguments in native_shadow_depth_target_owner.cpp
change: an ordinary static_cast of the existing void* captured_section to the
global critical-section pointer. The same physical address is passed; the enter
import capture precedes its call and +18 increment, while the +18 decrement still
precedes the leave-import capture and call. Removing those two casts reproduces
the entire old cpp exactly as normalized Source. No registration, publication,
lock/acquisition, destructor, fault or retry schedule is changed by this typing.
No existing tests reference the changed aliases or direct cell accessors.

The actual scalar process gains one private appended volatile callable pointer
and an accessor returning that exact member by reference. Its dedicated narrow
resolver uses GetProcAddress(GetModuleHandleW(L"kernel32.dll"),"InterlockedDecrement")
and the existing checked FARPROC-to-typed-pointer memcpy convention. This copies
the resolved function representation into the canonical cell; it does not pun a
reference or invoke an Original numeric address. The getter performs no decrement.
The old increment resolver, section resolver and process factory are exact text.
The constructor keeps existing increment, enter, leave resolution in that order,
then resolves the new decrement last. No earlier logical field is reordered.

## Source layout, initialization and remaining qualification

The new member follows the previous last instance-generator serial. With current
MSVC Win32 default layout, its appended function pointer is expected to increase
this ordinary Source class from0x38 to0x3C; old member offsets/cell identities are
expected to remain unchanged. This is a source-layout calculation, not a compiler
receipt or Original object-layout/VA claim. Both section aliases remain Win32
function pointers and the shadow binding/capture field declarations are unchanged.
The later Root build/emitted review must confirm layout and typed call transport.

A newly required export lookup occurs on the first process construction even
when atlas activation is still disabled. Missing InterlockedDecrement now throws
before the process constructor completes. The existing function-local-static
initialization/failure/retry policy is preserved; no new one-attempt guarantee or
Native cleanup policy is invented. Existing zero scalar preimages, synchronization,
publication and all earlier import initialization remain in their old order.

Root should review/admit these exact five postimages, build with the repository's
MSVC Win32 path, run existing checks, and perform a default-startup regression
because every renderer process initialization now resolves the added export.
Subsequent atlas activation must borrow all three exact canonical cells into the
retained real graph. It still needs SAME role routing, actual scratch/write+NUL
fit, genuine pre-GUI construction/common descriptor-parser-creator completion,
selected Host retention and ready normal retirement. All nine gates remain held.
This candidate establishes ordinary Source suppliers only. It adds no Original
function/vendor body credit, drop-in register ABI, private-stack/FH3/SEH/fault/
unwind/Origin or Native runtime/Game proof. Windows OS implementation bodies and
unrelated import consumers were not audited or reconstructed.
