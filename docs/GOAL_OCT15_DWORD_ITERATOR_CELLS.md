# Checked DWORD iterator operations

Complete ordinary Source for 00467A80 (54 bytes, 22 instructions) and
00467D50 (39 bytes, 18 instructions), using the actual current-CRT diagnostic
provider. Both take ECX iterator8 {owner,current}, one stacked DWORD and RET4.
No live incoming references establish semantic prototypes or collection types.
New descriptive Ghidra names are hypotheses replacing generic STL placeholders.

00467A80 first checks owner for zero and reports through 00BF6713. Only after
that returns does it read the live delta parameter, current position, and current
owner. Two unsigned DWORD additions scale delta by four modulo 2^32. It compares
the proposed current against owner+8 first, skipping the owner+4 lower-bound read
if above the upper bound. A returning bounds diagnostic is followed by a fresh
current-position read and addition of the captured scaled delta. It returns the
original iterator pointer. There are no clamps, replacement values, or rechecks.

00467D50 captures left.owner before reading its live other-pointer parameter.
It captures that pointer before any diagnostic; zero owner skips other.owner.
After the owner diagnostic returns, it reads current left.position then current
captured-other.position. SETNZ CL followed by MOV AL,CL preserves the upper 24
bits of the left position in EAX. Source exposes those raw bits, with inequality
in AL, without inventing a wider semantic return type.

Native transfers are `00467A89 CALL 00BF6713`, `00467AA7 CALL 00BF6713`,
and `00467D62 CALL 00BF6713`. Both handlers may return. Explicit raw argument
backing retains late argument reads and valid described-cell aliases; binding
pointers remain stable. The Source APIs require every reached access to remain
valid, including continuation after diagnostics. Physical private-stack identity,
register/CRT ABI, new-body differential behavior, application adoption and game
validation remain open. No new tests are added for this direct reconstruction.

Final combined default MSVC Release Win32 build passed all three existing CTests in 8.27 seconds. Complete independent listing review and three typed transfers passed; detailed receipt: reports/goal_oct15_integer_range_primary_review.json.
