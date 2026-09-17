# Raw embedded game-state construction

Addresses: 0076ede0, 00786a80, 007868c0, 00778610, 00778590, 004ddb90

## Result

The game constructor now binds `0076EDE0` to its complete normal raw body and four real helpers. This closes the embedded owner at game+1EF0, with accesses through owner+29C, and supplies the existing real tracked-critical-section implementation as the default for both embedded and parent game calls. Other game-constructor services and application ownership remain required.

Five previously unported bodies total1,303 bytes: constructor529, row constructor341, row reset375, pair wrapper8 and pair reset50. The existing game-parent composition adds no unique function. Names such as peer rows/pairs are provisional descriptions of fixed eight-entry storage, not recovered symbols or proof of protocol semantics. The accessed prefix does not establish a complete class or allocation size.

## Storage and ordering

`00786A80` initializes eight1Ch rows at receiver0..DF. It retains the first row's distinct store order and then calls the actual `007868C0` reset. The reset writes modeE0, time bitsE4, zeroE8 and byteE1, followed by all eight rows. Each row has DWORDs at0/4/8/C/14/18 and a byte at10; padding remains untouched. Constants at D7A24C are read separately by construction and reset, while D042F8 supplies the reset time bits.

`00778610` calls `00778590`. The latter clears eight pairs at0..3F and40..7F and one flag at80..87 per iteration, preserving its ordered writes and other storage. Both helpers receive their actual subobjects at parent+8 and parent+190.

`0076EDE0` writes vptrD039CC, initializes those subobjects, and performs sparse scalar/control writes. It initializes the legacy short-string length234=0, capacity238=15 and inline byte224=0 without clearing the rest of that storage. Copies of D7A260 go to278 and294. The arithmetic at76EF08 is SSE `SUBSS`, subtracting D0DE84 from the earlier captured D7A208 bits; source uses that instruction explicitly and stores the result at240. No x87 substitution is used.

After creating the critical section through BD1860, the constructor writes its result to298 and reads the current capacity254. Capacity below200 causes capacity200 publication and BF55BE allocation of800h bytes. After allocation it reloads the current pointer24C and count250. For a non-null old pointer it copies DWORDs with current pointer/count reads, frees the current pointer through BF6989, and then publishes the captured replacement. The tail clears bytes27D,29C and4. A null critical-section result does not bypass buffer initialization.

The concrete calls reuse the existing tracked-section factory and malloc/new-handler/free domain. Native CRT or Windows implementations are not reimplemented here. Source operations retain an interrupted child and parent call/unwind stage, including an unpublished replacement on free failure, and reject replay. Diagnostic retirement acknowledges external cleanup; it does not free resources or reproduce native FH3 rollback.

## Ghidra and verification

The free call at76EFC0 had a spurious CALL_RETURN override. A locked repair restored ADD ESP,4 at76EFC5..C8. The function was recreated with `disassemble_first=false` across its complete529-byte span, preserving prior names/comments. The previous stored body had519 bytes: the missing3-byte reachable tail plus7 alignment bytes explain the difference. No callee no-return flag changed. Saved annotations, readback and refreshed exports accompany the report.

`reports/native_game_embedded_state_r110.json` retains15,379 disk/live-matching bytes, including63 copied bodies (13,367), the read-only score-layout producer (1,930), SDK thunk6 and constants/literals76. All393 direct call rows pass the live Ghidra body/call audit. Fixtures relocate357 copied-body calls and four external JMPs.

The strict MSVC Win32 build and all three existing CTests pass. Twenty-three original/source cases compare1,516 ordered observations and103,452,476 bytes. The existing15 game/profile/score/settings cases now execute the real embedded body wherever reached. Eight added cases cover patterned storage, capacity skip after a critical-section callback, allocator replacement of the current old pointer/count, mutation during free followed by saved replacement publication, zero-count old-buffer free, null critical section, upward rounding, signaling NaN and DAZ/FTZ behavior. Full snapshots include opaque neighbor bytes; SSE cases compare MXCSR as well as output bits.

Source-only checks inject embedded allocation and free failures, verifying game4DDEAB/unwind29 and child76EF79 or76EFC0/unwind2, resource retention and replay rejection. A standalone call through the concrete defaults creates, enters, leaves and deletes a real Windows critical section and frees its real800h buffer. This is a source ownership check, not reconstruction of the embedded destructor.

The report records application artifact comparison and immutable tested/integrated archives. These routines remain unwired in the application, so no new runtime result is claimed. Remaining boundaries include actual SDK behavior, other game services, application lifetime/teardown, native EH/ABI, arbitrary aliases/fault timing, unmasked floating-point exceptions, visuals and gameplay.

## Follow-up work

Recover the real game array-element constructors and remaining collection/configuration/resource/Lua-host/Dyn services, then bind the actual game owner into application startup and teardown. Recover this embedded owner's destructor before treating its lifetime as complete. Keep the existing renderer-device runtime failure separate from proof about these unreached constructor paths.
