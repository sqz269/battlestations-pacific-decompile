# Complete ordinary message84 source profile

This packet reconstructs seven complete ordinary native bodies and the five-entry hexadecimal 0x84 landing-message SOURCE profile. The types `NativeLandingSlotMessage84` and `NativeLandingSlotMessage84Profile` distinguish it from the existing decimal-84 (0x54) message. It is conditional on actual borrowed storage, the complete class-registry getter, actual handle-bank aliases and actual release services. The new context-bearing C++ interface/profile is unbound to the executable's ABI and game. Neither the lower/composite land constructor nor the landing queue caller is admitted by this prerequisite.

The implementation is [native_session_message84.hpp](../include/bsp/native_session_message84.hpp) and [native_session_message84.cpp](../src/native_session_message84.cpp). Exact bytes, call sites, compiler/probe hashes and limitations are in [the report](../reports/landing_message84_profile_cc11.json). The primary previously defined/saved the missing scalar, reader and validity bodies in [landing_message_profile_definitions_cc11.json](../reports/landing_message_profile_definitions_cc11.json). Workers made no Ghidra changes.

## Complete native coverage

All bounds below are end-exclusive. The original PE and current live program bytes matched across all 622 bytes/219 instructions. Live queries verified `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`; the disk PE is the configured original Steam executable. The raw type predicate remains a reviewed raw body, rather than a claimed worker-created Ghidra definition.

| Entry | End | Instructions | Original inputs/return |
|---|---|---:|---|
| 006BD520 constructor | 006BD5C4 | 50 | ECX message; stack block,index; EAX original message; RET8 |
| 006BD5D0 type predicate | 006BD5F2 | 11 | stack raw type; EAX 0/1; RET4; receiver unused |
| 006BD600 scalar delete | 006BD61F | 11 | ECX message; stack flags; EAX original identity; RET4 |
| 006BD680 writer | 006BD70C | 54 | ECX message; stack cursor; RET4 |
| 006BD710 reader | 006BD79D | 54 | ECX message; stack read-stream wrapper; RET4 |
| 006BD7A0 validity | 006BD7F4 | 27 | ECX message; AL boolean; RET; upper EAX unspecified |
| 0095B9C0 vehicle-class lookup | 0095B9DC | 12 | ECX descriptor; EAX raw registry result; RET |

`0095B9C0` is a vehicle-class lookup. It does not destroy, retain, unregister or establish any descriptor lifetime. Null ECX returns zero without calling anything. A nonnull original descriptor is captured across the complete actual `00437F50` getter; **after** that call its DWORD+70 is read afresh, then the returned registry's DWORD at `+2010+index*4`. Both address calculations are unchecked modulo32 arithmetic. Required storage/registry validity is not replaced by a source census or copied class table.

## Constructor and storage publications

Actual message size is 38h: the producer's stack message occupies `ESP+2C..63`, before the saved EH cell at+64. The independent receive-factory arm pushes 38h at `00769252`, allocates at `00769254`, then calls `00763380` at `00769266` on the nonnull arm. That receive constructor, its allocator and private EH are not implemented or bound here.

The constructor retains bytes 11..13, 1B, 1D..1F and 36..37. The source statically checks every field offset and the complete 38h extent. Normal order is:

1. Complete existing `0075B430(message,84h,actual game-cell context)`. Its current game, selected-owner index and owner-pointer reads remain its actual ordinary contract.
2. WORD18=0, BYTE1A=0, DWORD04=1, BYTE1C=0, DWORD20=original index.
3. Store the concrete SOURCE profile corresponding to native `00CF8610` (`006BD56A`), then freshly read the original block's slot-array cell+4C (`006BD570`). Capture exactly `base+index*58h`.
4. Copy captured slot DWORD+2C to message+24. Read captured slot descriptor+4 and execute the complete `0095B9C0` body with the required actual getter.
5. Store the class result+28, then fresh DWORD+8->message+2C, DWORD+10->message+30 and pointer+28 from the **same captured slot**. A nonnull squad contributes its actual WORD+174 to message+34; a null squad contributes zero.

The getter may have ordinary observable effects while the original slot and descriptor remain valid. Rebinding block+4C does not change the captured slot; changes to that slot's later fields and to the original descriptor+70 are observed. Destruction, relocation, concurrent writes, structurally reentrant lifetime changes, failed allocation and invalid placement are excluded. Controlled fixture field changes demonstrate SOURCE ordering; they are not observations of native runtime reentry.

## Five executable SOURCE bridges

The live native `00CF8610` words are exactly `006BD600,006BD680,006BD710,006BD5D0,006BD7A0`. The SOURCE profile has five executable Win32 fastcall bridges followed by borrowed bank/release metadata. ECX is the source message, EDX is unused, and an original single stack argument is represented as the third fastcall argument. Its context metadata, function addresses and ordinary C++ interfaces form a new SOURCE ABI, not a drop-in original vtable.

Scalar delete captures the profile's required release context **before** overwriting message profile+0 with the existing distinct root SOURCE profile corresponding to `00CE4974`. Native restamp is at `006BD608`; optional actual `00BF65AC` release is at `006BD611`. Only flags bit0 requests release. Other flag bits still root-stamp without releasing. The original message identity is returned even if release invalidated it. No default heap/free, owned payload teardown or post-release read is introduced. The root profile is not restamped to the concrete message84 profile.

Writer and reader reuse the complete existing extended-header and native cursor helpers. Header fields are type8, sender WORD low12 and relay1. Payload is BYTE1C boolean1; unsigned DWORDs20/24/28/2C/30 at widths6/4/10/6/4; WORD34 presence1 and optional WORD low12. The writer captures the presence predicate before its boolean write, branches on that captured predicate and then reads WORD34 afresh for the optional payload. Reader's false-presence arm explicitly stores WORD34=0. It reads unsigned DWORDs, not sign-extended variants. Padding is retained. Native cursor bounds/carry-byte behavior is preserved; valid disjoint message/stream/backing storage is required, without a new clamp or stream constructor.

The type predicate accepts precisely raw 84h,49h,46h. This does not establish compatible payload semantics for49h or46h; their semantic admission remains outside the caller domain. Validity reads WORD34 once: zero returns true without dereferencing any bank. Nonzero uses the approved complete `object_from_handle_006AD080` against `ObjectHandleTables` references to actual globals. Zero-extended ID is compared signed against fresh F89A10; the selected branch subtracts fresh F89A0C or F89A60 with modulo32 arithmetic, reads the corresponding fresh F89A54/F89AA8 bank and tests pointer at stride10h+Ch. There are no private tables, bounds clamps or always-true substitute. Source bool models the AL result, not unspecified high native EAX bits.

## Verification and remaining adoption boundary

Fresh actual message84, base-message and handle-resolver TUs plus one ignored fixture compiled with MSVC Win32 `/O2 /W4 /WX`. The manifested `local/cc11_message84_probe.exe` passed. It runs constructor -> all five SOURCE profile methods with actual-shaped block/slot/descriptor/squad/registry/game storage, the real existing header/cursor codecs and actual bank-reference API. One controlled getter changes descriptor+70 and later captured-slot fields while rebinding block+4C; the fixture checks the original slot's captured state and fresh later fields. It checks present/absent payload cursor lengths and retained padding, bank rebinding, zero's bank-free true arm, type acceptance, no-release flags and root-before-required-release order. These are SOURCE fixtures, not original-PE differential or game runtime tests.

The three previously pinned support libraries were hashed before and after linking and stayed unchanged. The executable is PE32/x86 (`14Ch`), with one embedded `asInvoker` manifest. Source COFF also preserves profile-store before block+4C read, captured-slot loads after the getter, original descriptor+70 after the getter, and release-context capture before root-stamp before indirect required release. All 22 native direct call rows passed the existing live call verifier. `git diff --check` and staged diff checks passed. Root owns CMake registration, serialized full Win32 build, existing CTests and independent probe/integration.

The complete `009F9CE0` ordinary approach head is now independently reconstructed in `native_bot_approach_head.cpp`. The next connected lower/composite source recovery still needs complete actual `009AFE70`, `009B2E50`, their constructor/state/registry/geometry contracts and primary-owned fixed x87 regions; this packet supplies only the genuine message prerequisite. Complete queue producer `006C0B50` and append `006BF720` still need actual allocations, observer lifetime/lock contexts, slot0C event4/value0 dispatch and complete `0077C7B0` route/sender-copy/peer lifetimes. No opaque whole head/tail phase service is supplied. The producer calls this message constructor at `006C0CC6`, routes it at `006C0CD6` and has no explicit normal scalar call; private unwind behavior remains external.

Cached task+404, fresh original input-plane+9D4 queue receiver, record+4 preappend holder, and fresh post-notify/message block+80 return remain distinct identities. Holder replacement does not free the old holder; its actual destructor/unlink does not clear those borrowed cells. The actual class getter, current game/owner, original profile/arena/release context, native stream/backing, actual bank owners/population and all borrowed object lifetimes remain unbound. No GameUnitsHost projection, Boolean queue/lifecycle binding, whole constructor admission, native ABI compatibility or gameplay proof is claimed.
