# Landing Follow parameter-copy kernel

The complete recovered `009BE150` body now has an instruction-exact MSVC Win32
source bridge. Native ECX holds the existing destination, the source is on the
stack, EAX returns the destination, and RET4 removes the source. The declared
fastcall interface reserves unused EDX; the body clobbers EDX exactly as the
original byte copies do. This is a routine-level proof, not a game replacement.

The 87 instructions occupy `009BE150..009BE28B` (exclusive), 315 bytes, SHA256
`48dfafc30042b9594b4ee2dbcee1ab2252b8039676f27a6134b1fabd9601cf1a`.
Primary reviewed the complete assembly and verified installed-PE bytes against
the supported live Ghidra query on bsp.gpr / battlestationspacific.exe. There
are no calls, absolute operands or relocations in this body.

Forty binary32 lanes execute ordered FLD/FSTP pairs: 00h through 64h, then 6Ch
through A0h, stepping by four. Byte68h and byte69h are copied between those
ranges. Holes6Ah/6Bh are untouched. This cannot become memcpy or a self-copy
early return: signaling NaNs can quiet and x87 exception status can change.

Both blocks expose live A4h-byte spans. Exact self alias and disjoint spans are
admitted; partial overlap remains outside the validated domain. Exceptions are
masked, no pending unmasked exception exists, and at least one x87 stack slot
is free. The body preserves the control word and existing stack values; its
status follows the actual load/store instructions. Invalid storage, unmasked
exceptions, faults, private unwind behavior and game lifetimes are unproved.

One ignored manifested Win32 differential fixture independently extracts the
original body from the installed PE into an RX page and compares it with the
freshly compiled actual production translation unit. All 315 instruction bytes
match. It passes 144 cases: disjoint/self-alias blocks, mixed IEEE binary32
payloads including positive and negative signaling NaNs, three precision and
four rounding settings, zero/one occupied stack values, and an existing masked
denormal sticky flag. Returns, every block byte, holes, NaN quieting, CW/SW/TOP,
tags and 80-bit register payloads match. FIP/FDP differ by executable/data
address and reserved register padding is excluded. All 144 cases preserve the
holes and quiet signaling NaNs; there are zero failures. No tracked tests were
added. The full main build and connected caller receipt remain pending.

The real Follow entry calls this operation on its existing state+6C block and
fresh singleton+380h source. Constructor009C2980 stores singleton+380h as that
existing pointer rather than allocating a block; normal unchanged publication
therefore means exact self alias. A changed singleton requires both old and new
blocks to remain valid. No default tuning, allocation, pointer replacement,
singleton or state lifetime is supplied by this kernel. The existing worker
will connect its opt-in abstract entry adapter to this fixed complete contract.

Reproduce with `python local/cc11_land_parameter_x87_extract.py` and
`cmd /c local\cc11_land_parameter_x87_probe.cmd`. The JSON report records the
ignored artifacts and hashes. Actual game binding and game validation remain
unclaimed.
