# Native input action listener ownership

Addresses: 00A92B70, 00A92150. Names are source hypotheses, not recovered symbols.
The additive fragment operates on the actual 30h action record and its retained
24h listener. It supplies a finite D5B610 deletion binding for the existing
`NativeInputActionRecordCalls` interface; it creates no second owner or vector.

| Entry | Native ABI and coverage |
| --- | --- |
| A92B70 | ECX actual action, EDX unused, no stack arguments, void, RET at A92BFE. Complete normal replacement schedule and the native old-release exception boundary. |
| A92150 | ECX actual listener, one stack DWORD whose lowbyte bit0 is tested, EAX original address, RET4 at A92171. Complete scalar schedule through A92173. |

These are new C++ interfaces, not binary replacements. The caller/provider and
valid writable native allocation domain remain required. Original FH3/SEH and
arbitrary listener profiles are not supplied.

A92B70's sole direct caller is A93C80 at A93CB3. That caller obtains an actual
30h row from owner+4 after resizing when needed, sets row byte0, and conditionally
calls with ECX=row when its third stack argument is nonzero. A93940 independently
confirms the row's retained listener at+2C. The existing copy/destruction routines
retain/decrement the listener's actual Interlocked LONG at+4.

A92B78 calls the existing BF681B allocation contract with24h; A92B7F cleans the
one stack DWORD. Initialization writes root CEB130, refs1, then D5B610. The byte
stores occur at offsets8,A,B,D,E,F,10,9,C,11,12,13, followed by positive-zero
float stores at14,18,1C,20. All24h bytes are initialized. The source keeps this
order and uses the existing malloc/new-handler domain, matching scalar free.

Only after initialization does A92BDA capture action+2C. The import call at
A92BE5 decrements captured listener+4; zero invokes the captured current slot0
at A92BF5 with no flags or other stack arguments. A92BF8 publishes the fresh
pointer after that callback returns. There is no local EH owner: an allocation
exception leaves the old cell untouched, while an old-slot exception leaves the
fresh allocation unpublished and unfreed. The source does not add rollback.
The focused fixture does not inject either failure.

D5B610 contains precisely the two pointers BD30E0 and A92150; the bytes after
those pointers are the literal `holdPress`. The finite source adapter validates
the captured slot0 profile and calls shared BD30E0, which rereads the current
profile and invokes slot04 with flags1. Unknown profiles produce a binding
error. A92150 stamps D5B610, calls shared BD30F0 to stamp CEB130, optionally
frees the captured original address when flags bit0 is set, and returns that
address. It does not decrement refs or clean additional payloads.

Primary repaired A92150's missing ADD ESP4 at A9216B after the returning free
call, refreshed its export and saved Ghidra. The worker verified12 instructions,
zero gaps and final RET4 through A92173; it made no Ghidra writes. Correct CRT
library names remain unchanged. There are no missing starts or excluded-tail
calls in the two owned routines. The report contains every owned CALL, the
producer call and relevant shared dispatch calls with original ABI evidence.

Validation: Win32 Release `/W4 /WX` build passed, both existing CTests passed,
and all8 native seeds matched disk. Both complete owned byte spans matched the
installed image. The CALL audit checked4 direct rows with0 failures; four import
or vtable rows were checked manually against the assembly.

One ignored manifested archive-only fixture exercised2 actual action records,
4 listener allocations and4 frees, copy retention, replacement while shared,
last-reference release before publication, nonfree scalar profile reset,
unknown-profile rejection, and actual BD0400 manager drain. Three frees used
zero-reference finite dispatch and one used direct scalar deletion. The action
publication and manager slots/critical-section fields were null after drain.
This is source lifecycle evidence, not native differential, application or game
validation. No SDK, device, cursor or game call was executed. The runner accepts
`-CoreWorktree` and links only that tree's headers and archives:
`local/run_native_input_action_listener_probe.ps1`.

The post-tick callback is deferred. Missing wrapper6965A0 is exactly11 bytes:
MOV ECX,[E188A8], then tail JMP4D8CD0 at6965A6, last byte6965AA, endexclusive
6965AB. The existing typed deadline routine cannot close the raw callback:
4D8CD0 updates the global raw map E18A7C through4D6900, whereas WorldTickState
owns a separate std::map. Its raw map producer and canonical game clock must be
bound first. `local/native_input_deadlines_next_ag.md` records the bounded
follow-up, including the newly found missing static initializer CC9E30.
