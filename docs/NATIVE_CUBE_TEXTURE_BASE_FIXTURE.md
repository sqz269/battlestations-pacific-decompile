# Native cube texture base: independent fixture

The complete `B34020` constructor and complete five-byte `B34090` forwarding
entry pass against the primary agent's already compiled implementation. No
production correction was needed. This packet adds only this evidence document
and `reports/native_cube_texture_base_fixture.json`; its runner remains ignored
under `local/cube_base_fixture`.

## Exact code and provider boundaries

Fresh guarded Ghidra queries verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, before each read. Three spans, totaling 81 bytes,
matched the installed executable, SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
All eight original instruction seeds matched that same executable.

| Span | Original ABI and fixture treatment |
| --- | --- |
| `[B34020,B34067)`, 71 bytes | ECX actual owner, stack borrowed COM and flags, EAX same owner, RET8. Execute the complete original body. Bind only the two absolute serial operands at `B34051` and `B3405A` to the same actual backed DWORD. |
| `[B34090,B34095)`, 5 bytes | Execute the entire unchanged `E9 BB FE FF FF` JMP to `B33F50`; native ECX owner and RET are adapted at the dependency boundary. |
| `[B33F50,B33F55)`, 5 bytes | Guard the original entry bytes, then install one explicit JMP to a fixture ABI adapter. That adapter immediately calls the genuine primary `destroy_native_logical_texture_named_base_00b33f50` with the actual owner and current string storage. |

The reference runs in a private sparse image read from the installed PE. The
original installation and saved analysis were unchanged. Relative control flow
between the copied entries is preserved. All 18 postimages across both modes
and three serial bindings were checked: the two serial operands and the named
base boundary JMP are the complete mutation allowlist. Thus all 63 other
constructor bytes and all five owned forwarding bytes remain exact.

The fixture links the frozen primary
`build/cube-base-primary-check/bsp_core.linked.lib`, SHA-256
`e868288d29c1a2d75b09e520ea75648a792c9976d74b7b5307a381563e11ee6c`.
It compiles only fixture code, with no production recompilation, source alias,
replacement destructor, or replacement string provider. Eight primary header
and source snapshots are pinned and were rechecked against the current primary
files. Eleven behavior providers resolve to that exact library in the link
map, including both owned entries, named-base destruction, actual string-header
destruction, both `PooledStringStorage` methods, and the real `SizedStoragePool`
construction/configuration/allocation/release path. The instantiated pooled
string vtable points to those actual library allocation and release methods.

## Ordered stores and shared serial aliases

An actual writable owner page contains the complete observed `40h` bytes and
the standalone serial backing. Read-only protection and a vectored exception
handler let each actual store execute once, then single-step and snapshot all
16 owner DWORDs, current serial and real pool counters. This records equal-value
stores as well as changed bytes. Observation does not substitute writes or
alter production fields. Each constructor issues exactly 11 stores:

`profile CEB130`, `profile D5F1F4`, `+04=1`, `+08=0`, `+0C=0`,
`+14=0`, `+10=borrowed COM`, `+1C=flags`, `+20=current serial`,
`current serial += 1`, `profile D5F280`.

The native store PCs are `B34022`, `B3402A`, `B34035`, `B34038`, `B3403B`,
`B3403E`, `B34045`, `B3404C`, `B34055`, `B34058`, and `B3405E`.
Every host store PC was verified as an actual store instruction inside the
linked primary constructor. One focused sequence uses three meaningful serial
bindings, with flags `76543210` and borrowed COM word `12345678`:

| Actual serial backing | Observed result |
| --- | --- |
| Separate DWORD, initially `FFFFFFFF` | Owner `+20=FFFFFFFF`; the same serial wraps to `00000000`. |
| Exact alias at owner `+00` | Owner `+20=D5F1F4`, taken from the intermediate profile; increment happens before final `D5F280`. |
| Partial DWORD alias at owner `+1E` | The `+20` store changes overlapping serial bytes. Reloading them produces serial `76547655`, owner `+20=00087654`, and flags `76553210`. A cached old-serial increment would diverge. |

All three return the same actual owner. Every snapshot preserves `+18` and
`+24..+3F`, including the raw-pool slot token at `+30`. The third serial reference
is backed and deliberately unaligned; the original x86 instructions and the
primary implementation both perform the actual DWORD access. This checks the
explicit Win32 source contract, not portable unaligned C++ behavior.

## Genuine current-name destruction

The ordinary wrap sequence starts with a real seven-byte pooled name already
in the owner. Construction abandons its header without returning that buffer.
Before destruction the fixture puts a different, real 14-byte name allocation
and length 13 into the current owner header. Both paths use an actual
`PooledStringStorage` over `SizedStoragePool` with the recovered pool A geometry.

Native `B34090` dispatches once through its unchanged JMP and the explicit
`B33F50` ABI adapter. The host uses the actual linked forwarding function.
Both reach the same genuine existing named-base/string providers. Two observed
stores install `D5F1F4`, then `CEB130`; both PCs are in the primary named-base
destructor. The pool's free-block count is zero at the first profile store and
one at the final profile store. The current length and pointer remain intact,
as do all other owner words, including count, borrowed COM, serial and tail.

An actual subsequent allocation of size 14 returns that exact current-name
pointer with its contents intact and reduces the free count to zero. A new
size-seven allocation does not return the abandoned old name, whose original
contents remain intact. All fixture-owned buffers are returned only after the
recorded operation and these reuse checks. No recording subclass replaces
`NativeStringStorage`, and no fictitious pool or string release is supplied.

## Result and limits

Strict MSVC Win32 `/W4 /WX /O2 /Oy- /MD /EHsc /fp:strict` fixture compilation
passed. Both process runs exited successfully. Their complete normalized traces
match **1,152 DWORDs / 4,608 bytes**, including **35 actual stores**: three sets
of 11 constructor stores and two destructor profile stores. Only the two actual
name-buffer pointer identities are normalized. Raw store PCs are retained and
checked separately; serial and owner data are compared exactly.

The original `B33F50`, native pool singleton, and native sized-string allocator
are dependency boundaries here. Their native instruction bodies are not
independently revalidated by a comparison that intentionally shares the genuine
existing C++ providers. This proves the complete owned forwarding entry's
dispatch and its composed real-provider behavior. It does not establish original
EH dispatch, arbitrary throwing string providers, original global-address
integration, binary replacement ABI compatibility, or in-game/render behavior.

The report pins the runner, preparation/verifier scripts, source snapshots,
library, executable, map, postimages, raw and decoded traces and live-byte
captures. No new permanent test target, Ghidra mutation, ledger edit, or shared
build/configuration change belongs to this packet.
