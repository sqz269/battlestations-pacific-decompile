# Native CRT SSE2 pow and error-support readiness

This is read-only discovery, not reconstructed source. The next coherent source packet is the complete 304-byte `C04EFB` / `BFBB61` / `C05920` prerequisite, followed by the actual pointer decoder, libm error service, and SSE2 wrapper/core. A host `pow`, forced zero dispatch flag, or synthetic math-error callback would not close the original route.

The companion [report](../reports/native_crt_sse2_pow_support_discovery.json) contains every captured native span, all 59 core constant-read sites, seven finite indexed-table bounds, stack/return-flag inventories, source pins, and explicit readiness limits. Ignored `local/sse2_pow_support_handoff/manifest.json` seals the exact report, document, capture scripts, raw bytes, listings and input copies after primary review. No source, metadata, Ghidra, build, test, game or runtime changes were made.

## Target and complete bodies

The worker starts at `8bc18703` in the isolated `agent/native-crt-sse2-pow-support-discovery` worktree. Each live capture uses the guarded BSP client, which verifies `C:/Users/sqz269/bsp.gpr` and `/battlestationspacific.exe` before querying. Disk-backed spans match the installed executable at `I:/SteamLibrary/steamapps/common/Battlestations Pacific/battlestationspacific.exe`, SHA-256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

| Entry | Half-open complete extent | Bytes | Instructions | Original contract |
|---|---|---:|---:|---|
| `C19260` | `C19260..C19279` | 25 | 10 | x87 two-input wrapper; aligned stack double arguments; x87 result |
| `C19279` | `C19279..C19DB1` | 2,872 | 645 | SSE2 double core; two stack qwords; result in ST0 |
| `C0F0E4` `___libm_error_support` | `C0F0E4..C0F35E` | 634 | 176 | cdecl three qword pointers and signed selector; output through third pointer |

All 645 core instructions and all 176 error-service instructions are statically CFG-reachable from their entries. This establishes connected instruction coverage, not runtime feasibility of every branch. The core has 15 RETs, all restoring its entry ESP; its last physical instruction is `C19DAC JMP C19CFC`. The only external direct call is `C19AA3 -> C0F0E4`. Fifteen following padding bytes end at the saved next `__d_inttype@C19DC0`; they are not part of the core. The error service ends in `C0F35D RET`; its following two bytes are padding. Its separate `C0F360[52]` DWORD jump table has 13 in-body targets: wrapped `selector-1000`, unsigned comparison with 12, then index times four. The address after that table belongs to a saved discontiguous CRT body; it is not claimed as a newly established function.

The finite evidence comprises 14 code spans / 4,597 bytes and 37 disk-backed data, padding, name and writer spans / 14,942 bytes: **51 disk-backed spans / 19,539 bytes**. Five separate saved-image zero-fill regions total 32 bytes. Zero fill is not an on-disk DWORD or evidence of current runtime values. Only the three requested analysis addresses and two output files are owned; dependency spans are read-only evidence.

## Wrapper, registers, FP environment and aliasing

`C19260` saves EBP, reserves 10h stack bytes, aligns ESP down to 16, exchanges ST0/ST1, and performs two ordered `FSTP qword` stores. Incoming ST1 is base x; incoming ST0 is exponent y. Those stores round under the current x87 environment before calling the core. The wrapper consumes two x87 values and receives one, with deeper caller x87 entries retained on a normal return. `LEAVE; RET` restores its original stack and EBP. No control-word or MXCSR read/reset occurs in these two bodies. The parent's separately verified `BFEB10` mutable `109EEA0` / current MXCSR `1F80` / x87 control `007F` dispatch is an upstream dependency, not proof generated here.

At core entry E, `[E+4]` is x, `[E+C]` is y, and `[E+10]` is y's high DWORD. Its own operations clobber EAX, ECX, EDX, XMM0..7 and integer flags; EBX/EBP remain unchanged, and the slow scaling branch saves/restores ESI/EDI. External calls additionally have their actual callee ABI effects. The report records the exact last integer flag writers for each RET; most scratch paths finish with `ADD ESP,10h/1Ch`, while other special returns retain their path's comparison flags. `LEAVE` does not normalize them. The smallest private ESP delta is -28, excluding callee internals. The original wrapper makes core-entry ESP congruent to 12 modulo 16; core stack memory accesses are qword operations rather than aligned 16-byte loads.

Preserve the complete legacy SSE2 instruction sequence and packed lane behavior. Scalar register and memory moves have different upper-lane effects; incoming upper XMM lanes must not be silently zeroed. Packed operations must not be replaced with scalar arithmetic. Arithmetic, `CVTSD2SI`, NaN propagation and deliberately generated FP exceptions use current MXCSR; final `FLD` uses current x87 controls. Neither body clears/restores FP status. ST0 is the result contract, not a universal XMM0 return. No mathematical host-library equivalence or binary replacement ABI is claimed.

The common error path at `C19A80` reserves 1Ch bytes, stores XMM0 into private result qword `[E-C]`, stores EDX as the selector, and passes pointers to original x `[E+4]`, original y `[E+C]`, and private result. After the service, it reloads the current result with `FLD` and restores ESP. This caller's output is distinct scratch storage; the general error service itself allows pointer aliasing. Core error selectors are 24, 25, 26, 27, 28, 29 and 1006.

Several paths deliberately produce FP status before selecting the final constant: `D74408 = 7FF0000000000001` is multiplied by itself; minimum normal `D74438 = 0010000000000000` is squared; large `D74430 = 7FE0000000000000` is squared or combined with negative `D74428 = FFE0000000000000`; other paths divide one by zero. Preserve these operations even when the subsequent returned bits are known.

## Exact core data footprint

All 59 absolute/indexed constant-read sites use eight or sixteen bytes in the read-only `.rdata` section, flags `40000040`. All reached sixteen-byte operands are 16-byte aligned. The report associates each instruction, operand width and effective index expression with a fully captured physical region. It does not infer table lengths from nearby symbols.

| Table base | Entries | Stride | Read width | Required bytes | Exclusive end |
|---|---:|---:|---:|---:|---|
| `D70B10` | 129 | 8 | 8 | 1,032 | `D70F18` |
| `D70F20` | 129 | 16 | 16 | 2,064 | `D71730` |
| `D71730` | 129 | 8 | 8 | 1,032 | `D71B38` |
| `D71B40` | 129 | 16 | 16 | 2,064 | `D72350` |
| `D72350` | 257 | 8 | 8 | 2,056 | `D72B58` |
| `D72B60` | 257 | 16 | 16 | 4,112 | `D73B70` |
| `D73B90` | 128 | 16 | 16 | 2,048 | `D74390` |

The first two stages extract a word, mask its low eight bits, add one, and mask with `1FE`: exactly the even indices 0..256, not 0..510. Scalar tables multiply that value by four; paired tables double it before the same scale. The third stage masks low nine bits, adds one, then ANDs `3FE`: even values 0..512, scaled by four/eight. Every exponent-table route ANDs `7F`, then doubles four times, giving offsets 0..2032 in steps of 16. These bounds were exhaustively enumerated over the finite masked integer domains without executing native code.

Additional required data is `D73B70[32]`, `D74390[8]`, and `D743A0[A8h]` through `D74448`. Together with the seven tables this is **14,616 bytes**. Four eight-byte alignment holes are not read and are excluded. All original bytes, including coefficient pairs, remain in the report; no rounded decimal transcription is needed. Notable raw cells are duplicate `000FFFFFFFFFFFFF` mantissa masks at A0/A8, duplicate `3FF0000000000000` at B0/B8, duplicate `7FFFFFFFFFFFFFFF` at C0/C8, packed coefficients at D0/E0, `3FE62E42FEFA39EF` at F0, `FFFFFFF800000000` at F8, and `BFF7154740000000` at `D74400`. `D74410/18/20/40` hold positive infinity, negative infinity, negative zero, and `FFF8000000000000` respectively. Names for mathematical meanings remain hypotheses; exact bytes and instructions are the contract.

## Full libm error-service schedule

`C0F0E4` first compares current DWORD `109E1B8` with zero, then captures the three argument pointers while preserving that comparison's flags. It creates positive-zero scratch by eight individual byte stores. The adjacent 32-byte exception record is otherwise initially uninitialized. When the captured flag is zero, callback identity is literal `C28545` (`XOR EAX,EAX; RET`). Otherwise it reads current encoded `109ED80` and calls complete `C04FDE`. Callback selection precedes selector dispatch even for unknown selectors and selector 26. There is no decoded-null fallback or callback validation.

The existing `CameraAxesCrtException` layout matches: DWORD type at 0, name pointer at 4, first double at 8, second double at 10h, result double at 18h. This service populates both arguments for callback routes, including unary names. The cdecl callback receives the actual mutable record and returns an integer. The branch selects its errno policy before the callback; callback changes to `record.type` do not select a new policy. Its result qword is reloaded after the callback and, where applicable, after the owning errno accessor/store. No general EAX result, FP-control restoration or EH guard is supplied by the service.

| Selector(s) | Actual name | Type | Initial result schedule | Callback-zero errno |
|---|---|---:|---|---:|
| 2 / 3 | `log` | 2 / 1 | current output | 34 / 33 |
| 8 / 9 | `log10` | 2 / 1 | current output | 34 / 33 |
| 14 / 15 | `exp` | 3 / 4 | current output | 34 / none |
| 24 / 25 | `pow` | 3 / 4 | current output | 34 / none |
| 26 | none | none | write x87 `FLD1` | no callback |
| 27 / 28 | `pow` | 2 / 1 | current output | 34 / 33 |
| 29 | `pow` | 1 | copy first to output before argument captures | 33 |
| 58 / 61 | `acos` / `asin` | 1 | current output | 33 |
| 166 | `exp10` | 3 | current output | 34 |
| 1000..1005 | `log`, `log10`, `exp`, `atan`, `ceil`, `floor` | 1 | first-to-output before argument captures | 33 |
| 1006 | `pow` | 1 | current output | 33 |
| 1007 | `modf` | 1 | first-to-output before argument captures | 33 |
| 1008 / 1009 | `acos` / `asin` | 1 | current output | 33 |
| 1010..1012 | `sin`, `cos`, `tan` | 1 | first times local positive zero; see below | 33 |

The actual strings are byte-pinned; selector 166 is `exp10`, 1005 is `floor`, and 58/61 are `acos`/`asin`. Conventional library expectations are not a replacement for these addresses. Unknown selectors perform no pointee reads/writes, callback invocation or errno write after the already executed callback-selection/local-zero schedule. Selector 26 needs only its output pointer after that same selection schedule.

For selector 29 and the listed early-copy table entries, `FLD first; FSTP output` precedes reloading first/second into the record. Aliased outputs can therefore change later argument reads. For 1010..1012, `FLD first; FMUL local+0; FST output` deliberately retains the product on the x87 stack, reloads first and second into the record, then pops the retained product into record.result. Do not replace this with two independent rounded double operations or reorder aliased captures. Final `FLD record.result; FSTP output` occurs after any errno call. A valid callback must obey its actual cdecl/nonvolatile and x87 stack conventions; unsupported callback lifetime/ABI is not repaired by this body.

The complete default `_matherr@C28545` is only three bytes. The separate existing `legacy_crt_87except_00c27489` source uses `E16BD0` and its own default-matherr policy; it is not this error service. Its existing `LegacyCrtMathRuntime::errno_location_00bffb8b` is a reusable explicit owning-CRT service, but its private atomic binding does not expose a public getter. A new API should borrow the actual runtime/reference explicitly, subject to review, rather than invent a second errno store or use the wrong bypass global.

## Decoder, actual TLS storage and remaining callback uncertainty

Complete `C04FDE[110]` is cdecl with one pointer-sized argument word. After saving ESI, it pushes current `E15B00`, captures the imported `TlsGetValue` pointer from `CE20BC` into ESI, and always calls it. Only a nonnull result triggers a current `E15AFC != FFFFFFFF` test. It captures that FLS/TLS index, calls the same captured imported getter with reread `E15B00`, then calls the newly returned raw getter with the captured index. A nonnull returned PTD supplies the current decoder pointer at `+1FC`. A null decoder in an existing PTD returns the original argument without falling back.

If the initial getter value is null, the FLS/TLS index is -1, or the selected getter returns a null PTD, fallback captures `GetModuleHandleA("KERNEL32.DLL")`, requires nonzero, calls complete `C04EFB`, then uses `GetProcAddress(captured module,"DecodePointer")` when the gate returns nonzero. A nonnull selected decoder is called stdcall with the current argument. **The decoded EAX is written back to the caller's actual argument slot at `[ESP+8]` after the ESI save, then that slot is reloaded into EAX.** A return-value-only implementation omits an observable native write. Identity paths do not write the argument. There is no decoder cache update, forced fallback, added null check, or LastError restoration.

The raw TLS indices `E15AFC/E15B00` are writable disk-backed DWORDs initially -1, not fixed runtime selectors. Complete `__mtinit@C053DC[388]` is captured only to ground storage: it resolves the four FLS functions, falls back to local allocation/TLS imports when needed, allocates `E15B00`, and stores the selected **raw** getter there before encoding the global function words. It allocates the FLS/TLS index in `E15AFC`, allocates an actual 214h-byte PTD, publishes it, then calls `C050F8[182]`. That initializer resolves `EncodePointer` into PTD+1F8 and `DecodePointer` into PTD+1FC through the same gate. The decoder requires a readable 200h-byte PTD prefix; the captured initialization route allocates 214h, not a universal object-size contract. These bodies and their startup/allocation/locale/EH providers remain source-incomplete.

Selected pointer domains are therefore actual `FlsGetValue`/`TlsGetValue` and `DecodePointer` on caller-owned initialized TLS/PTD storage. The current TLS indices, both getter reads, the current PTD header and current decoder slot must remain observable. A private TLS allocator, copied PTD, or generic resolver callback would lose this contract. The report lists the initializer's named-but-incomplete direct dependencies without claiming their implementation.

Fresh direct cross-references to `109E1B8` and `109ED80` establish their reads but do not establish their writer/population or lifetime. Absence of direct write xrefs does not prove they remain zero or null. A full error-service source packet therefore still needs an explicit admitted domain for the actual registered decoded cdecl callback and canonical name pointers. Discovery establishes the exact dispatcher, not all callback bodies or current runtime registrations. No fake callback or all-default policy is proposed.

## Smallest ready prerequisite: module gate, OS getter and strcmp

`C04EFB[108]` starts local major at zero and local result at one, calls full `__get_winmajor@BFBB61`, and ignores its return code. A **signed** local-major comparison greater than five returns one. Otherwise `GetModuleHandleA(nullptr)` selects the current main module. The body reads DWORD `e_lfanew` at module+3Ch, the section-count WORD at NT+6, and optional-header-size WORD at NT+14h, then captures first section address NT+18h+size. Even the zero-count path reads the optional-header size and computes that address before its conditional branch. Zero sections return one. Each section compares actual literal `.mixcrt` with its name using complete `C05920`. Equality makes result zero. A mismatch reloads the current section-count WORD **before** incrementing the DWORD loop index and advancing the section pointer by 28h, then compares the index unsigned against that captured count. No MZ/PE signature, optional-magic, module-null, section-range or terminator validation occurs.

`__get_winmajor@BFBB61[60]` captures its output pointer before saving ESI. A null output or current DWORD `109DD84 == 0` calls owning errno accessor `BFFB8B`, pushes five zero invalid-parameter arguments, writes errno 22, invokes `BF66EF`, and, if the handler returns, returns 22 with output untouched. Success reads current `109DD90`, writes the caller DWORD, and returns zero. `109DD84` is the CRT OS-platform word, not a generic initialized flag: the pinned `BFD146[92]` writer slice follows imported `GetVersionExA` and stores `OSVERSIONINFO.dwPlatformId` there and `dwMajorVersion` at `109DD90`. This slice is not a full startup-function claim. The gate's nonnull local pointer does not remove the platform-zero/returning-handler branch.

`_strcmp@C05920[136]` is a complete cdecl raw comparator. It aligns the first pointer using a byte and, if required, word load, then uses aligned DWORD first-string loads and byte reads of the second string. Each wider first load occurs before subsequent zero/mismatch decisions. Bytes compare unsigned; mismatch normalization returns exactly -1 or +1, equality zero. EAX/ECX/EDX and flags change; EBX/ESI/EDI/EBP remain unchanged. No host CRT substitution may erase the load-width/order contract. Here the first `.mixcrt` literal is aligned and eight bytes including NUL, so at most two DWORD reads of that literal are needed. The general body is not a bounded-eight-byte comparator.

The established returning invalid-parameter boundary is available through `SingletonLifetimeCallbacks`; the new packet should use its current actual service rather than reconstruct a synthetic handler. Full original `BF66EF[36]` decodes mutable `109DD64`, tail-dispatches a nonnull handler, otherwise calls `C04EF3[8]` to clear `109EEA8` and tail-jumps to `__invoke_watson@BF65BB`. Thus claiming the whole original CRT invalid-handler runtime complete would be false. `BFFB8B[19]` likewise calls incomplete PTD getter `C051B7`, returning PTD+8 or fallback `E159E8`; its existing source contract is explicitly the owning runtime's errno service. These two existing service boundaries avoid making the 304-byte prerequisite recursively depend on the decoder while preserving the real call and returning-handler behavior.

## Proposed source sequence and explicit limits

1. **`native_crt_pointer_decode_support`: 304 bytes, three full entries.** New `src/native_crt_pointer_decode_support.cpp`, `include/bsp/native_crt_pointer_decode_support.hpp`, `docs/NATIVE_CRT_POINTER_DECODE_SUPPORT.md`, and `reports/native_crt_pointer_decode_support_audit.json`. Implement full `C04EFB`, `BFBB61`, `C05920` using borrowed actual current OS-platform/major DWORDs, actual main-module/Win32 access, original `.mixcrt` bytes, the existing owning-CRT errno service and returning invalid-parameter service. Preserve raw readable-storage and current-module preconditions. No allocator or semantic comparator callback is needed. Original register/flags access contracts must be distinguished from any qualified new C++ API.
2. **Actual pointer decode: 110 bytes at `C04FDE`.** Depends on step 1 and explicit current TLS/FLS indices, actual getter/PTD/decoder profiles, Win32 imports and raw argument-slot writeback. Establish an admitted current-storage domain without claiming startup construction or replacing unknown current provider words.
3. **Libm error service: 637 bytes at `C0F0E4` plus `C28545`.** Depends on step 2, existing 32-byte record and actual errno service, canonical raw name identities, current callback-present/encoded-pointer state, and reviewed actual callback ABI/lifetime domain. Registration/population remains unresolved. Implement all selector, alias, x87 and callback schedules, including unknown/no-callback routes.
4. **SSE2 wrapper/core: 2,897 bytes at `C19260/C19279`.** Depends on step 3 and exact 14,616-byte data footprint. Preserve the concrete SSE2/x87 instruction algorithm, lanes, current controls/status, stack shape and selector schedule. This proposal establishes evidence and a bounded implementation target, not current source completeness or host-pow equivalence. The parent owns integration with `BFEB10` dispatch.

Current source/header pins are unchanged relative to observed main `d5e85f879f50b12050f8fea15ac434a615a294a5` after CRLF normalization; `reports/native_crt_x87_error_dispatch_audit.json` has a separately recorded main-version drift. Both blob identities are retained. Existing names are preserved, including correct CRT names; there are no annotation requests needed to fix the verified extents. All reconstruction/build/fixture/ABI/game-validation flags for this discovery remain false.


## Primary review

Primary verified all 105 immutable worker pins, 13 original input pins and current source/header parity, then reread all 51 disk-backed spans (19,539 bytes) and five separate virtual-zero regions (32 bytes) through guarded BSP queries. The installed PE and each raw/virtual section extent agree. The existing error-dispatch audit drift is recorded separately from unchanged source inputs. Full native contracts, finite table bounds and prerequisite sequence are reviewed. No source, annotation, build or runtime validation is claimed by this discovery. Primary evidence: `local/sse2_pow_support_primary/`.
