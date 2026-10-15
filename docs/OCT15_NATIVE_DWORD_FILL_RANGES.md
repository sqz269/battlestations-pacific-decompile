# Native DWORD fill ranges

`native_dword_fill_ranges.hpp/.cpp` supplies complete actual-storage Source for
two callee-free native entries. The existing correct STL names remain
`STL_Fill_0043f9e0` and `STL_FillN_00441180`; the new C++ names describe their
observed behavior and are not recovered symbols. Historical projections and
fragments are preserved.

| Native entry | New C++ interface | Original physical contract |
| --- | --- | --- |
| `0043F9E0` | `fill_native_dword_range_0043f9e0` | cdecl: stack first, last, value address; plain RET; EAX captured last |
| `00441180` | `fill_native_dword_count_00441180` | stdcall: stack first, unsigned count, value address; RET0Ch; EAX original first plus count times4 modulo2^32 |

The full bodies are `0043F9E0..0043F9FD` (30 bytes,13 instructions) and
`00441180..004411AD` (46 bytes,21 instructions). Read-only tools verified
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`. Complete typed
metadata at modification389 identifies one complete instruction-body range
per entry, neither a thunk nor no-return. All76 installed-PE/live bytes and
all34 instruction starts agree; Capstone decodes every byte. Both bodies
contain zero calls, zero external tail jumps and no local EH frame/action.
The ignored captures and audit are named in the JSON report.

The range form first captures the destination cursor, then the last address.
Equality returns immediately without consuming the value address or pointed
storage. Otherwise it captures the value address once and repeatedly loads
its CURRENT four bytes, stores those four bytes at the current destination,
advances the DWORD cursor by4, then compares with the captured last address.
It does not inspect ordering, alignment or a null destination. A nonempty
range whose cursor cannot reach the captured bound is not repaired.
Termination leaves EAX equal to the captured last address, including on the
empty path.

The count form captures unsigned count before the original first address.
Its TEST/JBE pair skips the loop exactly for count0, without consuming the
value-address argument. A nonzero count captures that address once. Each
iteration performs source DWORD load, destination DWORD store, decrement,
cursor+=4, then the unsigned remaining-count test. The final native LEA uses
the separately retained original first and count, independently of the loop
cursor. Source uses uint32 arithmetic for the same modulo2^32 result, even
when the count's upper bits are discarded by multiplication by4. Incoming
ECX and EDX do not supply semantic inputs to either entry.

The source value is never snapshotted for the whole operation. Its current
bytes are loaded before every store, so partial-byte overlap and forward
aliases retain progressive effects. Four-byte memcpy loads/stores avoid
adding C++ alignment or typed-object requirements. The implementation adds
no null skip, callback, provider, allocation, validation or overlap repair.

The current producer `00441860` snapshots its own input DWORD into a private
word and passes that word's address. It pushes value address, count, first
for the count entry and value address, last, first for the range entry.
That producer policy is not moved into these leaves. Its ECX owner is
unused by these leaves. The first range site explicitly removes0Ch;
the later0x18 removal also removes the three pending backward-copy words.
The complete leaf RET/RET0Ch instructions independently establish cleanup.
The producer's SAR2/scale4 operations and the upstream resize's SAR2/scale4
operations agree with DWORD elements. These incoming transfers are evidence,
not completion of the larger producers:

```text
0044192C CALL 00441180
004419BA CALL 00441180
004419D2 CALL 0043F9E0
00441A05 CALL 0043F9E0
0049225C CALL 00441860
```

`00441860` still needs its allocator/copy/length/free contracts and
`00492210` still needs its erase/memmove_s boundary. No parent implementation
or existing fragment is promoted by this packet.

All reached storage must be admitted. Private argument/stack-frame aliases,
asynchronous mutation and native register spills/fault/SEH identity are
outside these new Source interfaces. MSVC Win32 calling-convention keywords
and explicit stable returns preserve the recovered physical interface words;
they do not establish drop-in binary ABI, runtime, native differential or
gameplay equivalence.

The clean worker fast-forwarded to published `073ac362c92e81a426e401ac8fe2213458f9d93d`
during its FIFO registration turn, appended one bsp_core Source line, then
released that lease before building. No queue jump, Ghidra mutation or new
test was performed. `scripts/build.ps1` compiled Win32 Release, including
this Source, and the three existing suites passed in8.97s. The five JSON
transfer rows and five Markdown claims passed with zero failed/unchecked
claims. These suites do not newly exercise these fill entries against game
code. The preexisting unrelated LNK4006 duplicate-symbol warning remains.
