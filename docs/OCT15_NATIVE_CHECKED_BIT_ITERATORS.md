# Complete actual checked bit iterators

This packet implements the whole ordinary entries `0048D3B0`, `0048F290` and
`0048F2E0` over their actual storage. Descriptive names remain hypotheses.
The older bounded keyboard interfaces and their fixture evidence remain
separate. General packed-bit insertion/erasure and resize are not completed
by these helpers.

| Entry | Original interface | Coverage |
| --- | --- | --- |
| 0048D3B0 | ECX iterator, EDX unused, stacked signed distance, RET4/EAX iterator | 203B/82 instructions |
| 0048F290 | ECX vector, EDX unused, stacked output, RET4/EAX output | 71B/27 instructions |
| 0048F2E0 | ECX source, EDX unused, stacked output/distance, RET8/EAX output | 100B/33 instructions |

Iterator storage is three DWORDs: owner0, word4, bit8. The vector fields
consumed here are count0, backing begin8/endC. Complete typed bodies, all374
installed/live bytes and142 cached export instruction starts agree. All142
current instruction queries return those starts and matching mnemonics;
the eight spelling differences are the exact JZ/JE, JNZ/JNE and JNC/JAE
synonyms. Current CALL strings and installed rel32 targets agree at all ten
sites. The optional flow-property backend refuses script execution; it was
not enabled and no flow-override-query claim is made. No listing repair or
other analysis mutation was needed.

Advance returns immediately for zero distance without reading pointed
storage. Otherwise it reads owner first, then word only if owner is nonzero,
and reports either null condition. After a returning report it reloads owner,
captures that owner's backing begin and the iterator bit, then checks bounds.
The negative branch checks end against captured begin, reloads current word,
and compares the unsigned magnitude against `(SAR2(word-begin)<<5)+bit`.
The positive branch checks the captured owner's end, then reads current word,
CURRENT owner, and finally that current owner's bit count for its unsigned
bound comparison. Reports do not replace subsequent work or reloads.

The mutation phase reads CURRENT bit after the bounds report. For negative
distance with bit below unsigned magnitude, it wraps `sum=bit+distance` and
adds `-4-(((0xFFFFFFFF-sum)>>5)*4)` to CURRENT word. This is the native
logical-shift borrow calculation, including `INT_MIN` magnitude. Otherwise
the common path reloads CURRENT bit, wraps the sum, and adds `(sum>>5)*4`.
The memory ADD precedes publishing `sum&31`. All arithmetic is DWORD or an
explicit x86 SAR2; no signed-overflow C++ expression or division replacement
is used.

End captures backing begin before reporting begin>end. It writes output
owner0=0, captured word and bit0, reports null receiver, publishes owner,
then reads CURRENT receiver count and optionally advances the actual output.
This order matters for receiver/output aliases. The receiver fields are
already read before its null test; no new early null guard is introduced.

Copy-advance tests source owner, captures word/bit before the null report,
then reloads CURRENT owner after a returning report into a private12B
iterator. It advances that storage, captures owner/word/bit, writes output
0/word/bit, reports captured owner null, then publishes captured owner.
Private cells do not establish native frame placement or private-frame aliases.

All direct call claims are recorded in the packet report:

```text
0048D3CB CALL 00BF6713
0048D3E6 CALL 00BF6713
0048D407 CALL 00BF6713
0048D421 CALL 00BF6713
0048F29D CALL 00BF6713
0048F2BA CALL 00BF6713
0048F2CA CALL 0048D3B0
0048F2FA CALL 00BF6713
0048F30E CALL 0048D3B0
0048F333 CALL 00BF6713
```

The diagnostics call the accepted current SDK noinfo wrapper directly. It
can return or throw and does not reproduce the original native encoded
handler/debugger/Watson policy. No handler installation, callback, exception
conversion, noreturn promise, catch, rollback, snapshot or extra guard is
introduced. None of the complete bodies has a local FH3/EH frame.

All reached actual read/write targets must remain valid through the native
schedule, including after reports. These new Source interfaces and current
CRT/C++/private-cell conventions do not prove original register residue,
stack/private-frame alias, CRT, SEH, instruction fault, exception or gameplay
identity. Existing native math differential tests do not execute these new
entries. The report records the Win32 build, existing checks and independent
fourteen-check Source review separately from native byte/listing evidence.
