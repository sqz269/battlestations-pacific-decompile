# Native shadow-job singleton lifetime

Addresses: `00A8E090`, `00A8DDE0`, `00A8DDB0`.

This module implements the complete 266 native normal bytes of the singleton getter and final registered lifetime. It adds the real D5B56C deleting route to the existing singleton map. The primary D5B570 job execution at A8AE50 remains outside this packet.

| Native entry | Bytes | Original ABI | Coverage |
|---|---:|---|---|
| A8E090..A8E15D | 206 | No public inputs; EAX primary; RET | Complete normal body with explicit C++ guard cleanup |
| A8DDE0..A8DE13 | 52 | ECX primary; public flags DWORD; EAX primary; RET4 | Complete terminal schedule with explicit publication context |
| A8DDB0..A8DDB7 | 8 | ECX secondary; same public flags; SUB ECX,4 and tail JMP | Complete adjustor to the source terminal wrapper |

Native function names are hypotheses. Before annotation A8E090 is FUN_00a8e090, A8DDE0 is CG_scalar_deleting_dtor_00a8dde0, and A8DDB0 is undefined as a function. Existing undefined(void) prototypes do not describe the recovered ABI. No Ghidra annotation or new function definition is performed here.

## Actual context and getter

`NativeShadowJobContext` borrows the same mutable volatile pointer cells used by the application for manager01090AA0 and jobE18AD4. It owns no storage, registry or callback. It must survive any singleton drain that uses it; bind it through the appended `NativeSingletonDeletionBindings::shadow_job` member before registering jobs that may reach drain.

A8E090 first captures E18AD4 at A8E0A5. A nonnull fast path returns that captured pointer, without another publication load. The original installs its FH3 registration before this load. The new C++ interface does not reproduce that frame placement or the original no-input calling convention.

On a miss, A8E0B6 calls the real 415350 raw manager getter and captures its +10 section. The local eight-byte guard contains CE37FC and that section pointer. A nonnull section is entered through Win32 EnterCriticalSection, then its current +18 counter is incremented. State0 becomes protected only after both operations, matching A8E0D9.

A8E0E1 rereads E18AD4. A nonnull second read skips allocation and registration. Otherwise BF681B allocates exactly eight bytes through the existing CRT malloc/new-handler provider; native and host requested sizes are both8. For a nonnull result, stores occur in this order: secondary+4 D5B568, primary+0 D5B570, secondary+4 D5B56C, publication E18AD4. No constructor callback is inserted. The native null-allocation publication branch is retained even though the current allocator normally returns storage or throws.

A8E11D reloads the publication and forms nullable primary+4 using DWORD pointer arithmetic. Capture that secondary argument before the second manager getter at A8E12E, then call the existing BD0C30 registration on the returned manager, including a null argument. Neither the registered secondary nor first manager may be replaced by a host owner or globally cached manager.

Normal release decrements the captured section's current +18 and calls Win32 LeaveCriticalSection. The returned publication is freshly loaded afterward at A8E149. The raw manager+10 load, section counter reads/stores and profile stores use explicit x86 DWORD MOV helpers rather than typed uint32 aliases over pointer/critical-section storage. These source helpers introduce no external callbacks. The existing guard-cleanup provider retains its own actual Win32 section domain.

## Cleanup and final deleting route

Retained EF evidence pins FuncInfo DEC650 (magic19930522, one state), unwind map DEC648, action CB63F0..CB63F7 and handler CB63F8..CB6401. Native state0 -> -1 invokes `LEA ECX,[EBP-14h]; JMP411EE0`. The source catch invokes the actual `destroy_native_singleton_guard_00411ee0` and rethrows. It does not free the job or undo publication/registration. Normal leave remains inside that protected source scope; a throwing source import replacement is outside the proved Win32 contract. The helper/handler are not separately reconstructed or credited here.

The terminal computes nullable primary+4, captures flags bit0, then clears the actual E18AD4 cell unconditionally and stamps CE3818 on that secondary. Only the captured bit controls the subsequent BF65AC/free of the original primary. It returns the original primary, including after free. There is no preceding destructor callback, publication-identity check or job execution. Native null primary still reaches an invalid null store; no successful null fallback is added.

The private naked terminal receives the actual publication-cell address in source EDX. Its explicit TEST precedes clear/stamp/free. The public fastcall wrapper obtains this cell from the context in C++, avoiding reference-member representation assumptions in handwritten assembly. That wrapper may copy/capture flags before entering the private body; original public-stack alias identity and whole52-byte native byte identity are not claimed. The naked secondary shim subtracts4 modulo DWORD and tails the public source wrapper while retaining source EDX context and its flags word. Its generated target must be reviewed as the source symbol, not the original numeric game address.

The existing manager pops an object before reading its current profile and dispatching flags1. The new D5B56C case passes that popped secondary, the same retained context and flags through A8DDB0/A8DDE0. The field is appended/default-null at byte92; total binding size becomes96, with the previous final member still at88. All previous binding member offsets remain unchanged; the C++ aggregate and containing host objects are rebuilt together. This is not external binary ABI compatibility. Missing binding/unknown profile retain the existing source diagnostic and existing manager storage-cleanup behavior. Transient D5B568 and primary D5B570 are not admitted as deleting profiles.

## Calls, producer and caller lifetime

| Call site | Native target | Containing function | Contract |
|---|---|---|---|
| A8E0B6 | 415350 | A8E090 | First current raw manager; capture section+10 |
| A8E0CF | [CE2218] | A8E090 | Enter captured section |
| A8E0EC | BF681B | A8E090 | Allocate actual8 bytes; cdecl ADD ESP,4 |
| A8E12E | 415350 | A8E090 | Second manager, after secondary capture |
| A8E135 | BD0C30 | A8E090 | ECX returned manager; original pushed secondary; RET4 provider |
| A8E143 | [CE2210] | A8E090 | Leave captured section before return-publication load |
| A8DE06 | BF65AC | A8DDE0 | Free original primary; ADD ESP,4 at A8DE0B |
| A8DDB3 | A8DDE0 | A8DDB0 | Adjusted ECX and unchanged flags; tail JMP |
| CB63F3 | 411EE0 | CB63F0 | Existing unwind-only guard destructor tail JMP |
| A8EC5D | A8E090 | A8EA20 | Getter used between captured frame secondary and fresh table dispatch |

The getter itself produces the eight-byte profile layout; table D5B568 points A8DD60, D5B56C points A8DDB0, and D5B570 points A8AE50. The separate base deleting entry and standalone inlined constructor are not credited. No call exposes the transient table between its stores and final publication.

A8EA20's sole getter call follows command queue append at A8EC50 and frame getter at A8EC55. It captures frame primary+4 in EBP before A8EC5D, then loads the current frame table at A8EC62, pushes captured command and returned job, and dispatches current table+4 at A8EC6C. No remaining local string EH state owns those queued objects. Jobs, commands, frame pool and real execution bindings remain borrowed through dispatch; no implicit retain or failed-enqueue recovery is supplied. This module does not reconstruct A8EA20, queue execution or A8BD20.

## Evidence and validation

Admission: retained EF JSON SHA256 `9601e1af7ea4ae114d634fc929dd5888302d8a6fbd5b584e1ae1c8b05c281c37`; complete native/source/provider evidence remains in that retained worktree. All27 provider Git blobs match the base `af212ef31f008889ada610a07b9f29e4f0e9996e`. Fresh EH live206/52/8-byte spans match installed PE SHA256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`. Live queries use verified `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe` through BSP CLI.

The primary repaired the actual A8DE06 flow override from `CALL_RETURN` to `NONE`, preserving the old value. It restored `83 C4 04` at A8DE0B..0D, then recreated the complete 52-byte function after verifying that the initial listing repair still left a function-body membership hole. The final function contains all 17 instructions. The eight-byte A8DDB0 adjustor is defined with both instructions. Definition/repair logs preserve old values and the unavailable pre-definition comment query; all eight report call rows now pass. The original worker failure remains historical evidence.

The single coupled EH+EI Win32 build at `3c584d0b28e40ff1f15ddb7b42ba47dfc1ec97d7` passed both existing math CTests. All 2,639 tracked inputs plus the actual verified seed were unchanged, with a clean checkout and exact HEAD before/after. No separate EH build or new fixture was run. Complete current objects are exact members of the pinned current library. Primary review checked all getter publication/guard/counter/registration/cleanup operations, the 48-byte private deleting body, its 12-byte public binding wrapper, the eight-byte secondary tail, and the entire 1,347-byte current dispatcher. The other 17 singleton code sections retain the prior validated bytes and normalized relocations. This is new-context source behavior, not original FH3 or whole native byte identity.

Ghidra saves `BSP_ShadowJob_Get`, `BSP_ShadowJob_DeletingDestructor`, and `BSP_ShadowJob_Secondary_DeletingDestructor`, with native no-input cdecl getter and native thiscall deleting prototypes. Prior comments are preserved, exports refreshed, and three complete normal-body reconstruction rows record 266 bytes. Standalone handler/action, parent update and job execution receive no credit.

Limits: explicit source C++ ABI and existing valid raw provider domain; no original FH3/SEH/public-stack equivalence, arbitrary original IAT replacement, foreign longjmp, hardware-fault parity, concurrent teardown safety or gameplay/render validation. Existing provider exception/registration/allocator restrictions remain in force. Static reconstruction/build evidence does not prove job execution or a working frame path.

Primary integration saved all six native prototypes and refreshed exports. The existing `BSP_Render_GetOverrideContextWord` name and prior evidence are preserved; the two setter names are `BSP_MaterialEffect_SetByte13C` and `BSP_MaterialEffect_SetIndexedFloatBits`. The coupled batch records six normal bodies / 307 native bytes, with 35 exact emitted setter bytes and explicit source-context qualifications elsewhere.

The frozen exact attempt is `local/ehi-validated-attempt.zip`, SHA256 `2d874bffde82b207e428bee0ad512aec8dc275bd75bb7bac9a933bbc20e8d9bf`. All 2,720 payload hashes were reread. It contains actual tested inputs, four libraries, all three current objects, the prior dispatcher comparison object, test/build logs and executables, admissions, primary source/generated reviews and saved Ghidra evidence. Root Git inputs match the tested revision; 2,630 tracked files also match as raw bytes, and nine LF/CRLF-only variants are separately archived. The ignored seed matches. Export inventory metadata may retain historical names; separate live prototype/comment readback establishes the saved names and bounds. No gameplay, rendering or lifetime/material fixture validation is claimed.
