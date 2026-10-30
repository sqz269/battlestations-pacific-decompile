# Actual raw low-byte construction

The new raw-context `construct_narrow_from_wide_00436630` supplies the complete32-instruction normal constructor through an ordinary Win32 Source interface. Original ECX=raw8 destination, stacked raw wide pointer, EAX=SAME destination and RET4 are normalized; no binary/FH3/SEH/hardware timing claim follows.

Capture both identities, then store destination LENGTH0 before DATA0 without releasing old backing. Scan actual full WORDs through WORD0, advancing a wrapping DWORD cursor by2. Wrapping cursor-(source+2) is shifted with explicit unsigned SAR1 sign-extension bits, not unsigned division or implementation-defined signed shift. Pass that exact request/preserve1 to genuine RAW41DD40 using SAME AA8/AA4/AA0 cells.

Only after resize normal return capture CURRENT destination DATA once. Null skips copying. Otherwise reset the source cursor and read ONLY source BYTEs at offsets0,2,4...; write successive destination BYTEs, advance destination1/source2 and test the captured BYTE. A zero low byte stops even when the measured full WORD was nonzero. The actual resize/header length remains and the allocated tail is untouched. No Unicode/locale conversion, full-WORD second scan, count bound, strlen, terminator repair or fake empty result is substituted.

Header/WORD reads and writes are fenced byte copies; actual BYTE accesses are volatile/fenced. There is no primitive/wchar_t/NativeString object adoption, Source callback, NativeStringStorage adapter, Frame, input guard, old-buffer release, catch, rollback, retry or local output cleanup. Existing xlive typed and all raw providers stay unchanged.

Caller proves real raw8/source/output extents and lifetime through all reached accesses, valid streaming overlap and DWORD cursor advances, no invalidating writer, genuine raw pool/cell/code graph and retention of abandoned/partial allocation debt. Real raw resize retains its own current provider/overlap/zero-byte-copy/error qualifications. Empty input still follows actual clear/full-WORD scan/resize/current-DATA behavior. Bytes alone never prove constructor return.

Source/all32ASM review and the separately recorded single strict Win32 object are distinct from Root's Core registration/common build. No test, probe, entry/game, Ghidra mutation, resolver/VM/loading/startup/ABI/runtime or new Native/vendor-body credit belongs to this packet.
