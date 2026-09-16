# AnimationChannels concrete channel body (R27)

`NativeAnimationChannelBodyReader` supplies the actual `B8AAD0` dependency
required by the R26 `NativeAnimationChannelsReaderCalls`. It holds the same
borrowed `NativeResourceStreamReadContext`; it reads real node handles and
pooled strings, allocates actual channel/key storage, and publishes into the
captured group. No synthetic channel service or default success remains in
this dependency chain. These are explicit-service C++ interfaces, not binary
replacements for the original native calling conventions.

| Entry | Complete physical bytes | Behavior |
|---|---|---|
| `00B8AAD0` | 641 | Read channel name, two DWORDs and key children |
| `00B8A0A0` | 121 | Read DWORD plus nine float32 values, append key |
| `00B771C0` | 86 | Publish channel pointer and update group `+1C` |
| `00B77870` | 70 | Append one `28h` key, growing capacity if needed |
| `00B76680` | 129 | Reserve key-vector storage |

The five bodies total 1,047 bytes. The primary agent repaired the saved
listing at `B766F1..FA`, following the returning `_free` call at `B766EC`.
The existing `B766FB..B76700` tail remained intact. This restores the
replacement-data/capacity stores to the complete 129-byte body. The worker
made no Ghidra writes; primary evidence is in
`reports/native_animation_key_vector_flow_repair_r27.json`.
The primary also defined the verified ten-byte `CC2900..CC2909` EH handler
as a function so its `CC2905` tail jump can participate in exact call checks.

## Actual storage and ownership

The channel allocation is `28h`. The reader writes reference base `CEB130`,
reference count 1, then profile `D632B0`; initializes its pooled name at
`+8/+C` and key vector at `+1C/+20/+24`; leaves `+10` uninitialized; and
stores two incoming DWORDs at `+14/+18`. Their scalar meanings are unknown.
The current profile's first slot is `BD30E0`; whole channel destruction is
outside this packet.

The case-insensitive channel-name indices are:

| Indices | Names |
|---|---|
| 0, 1, 2 | Position.X, Position.Y, Position.Z |
| 3, 4, 5 | Rotation.H, Rotation.P, Rotation.B |
| 6, 7, 8 | Scale.X, Scale.Y, Scale.Z |
| 9, 10 | ZoomFactor, Dissolve |

Every `AnimationKey` reads one DWORD followed by nine x87 scalar values,
each immediately spilled with `FSTP32`. Appending copies ten DWORDs with
`REP MOVSD` and increments count. A full vector grows to the signed maximum
of 1 and wrapping `capacity * 2`; reserve allocates wrapping `capacity * 28h`,
copies existing keys, frees old data, then publishes replacement data/capacity.
Native null-destination skips and signed comparisons are retained.

Each successful key append publishes the same channel pointer into the group.
There is no retain/release on this write. A later channel with the same name
overwrites the old pointer without cleanup. An unrecognized name retains
index `FFFFFFFF`, so publication addresses the DWORD immediately before
vector data. A missing group is not accessed until a matching key has been
read and appended; the publication's first `FLD [group+1C]` then faults.
These routines do not validate or recover those inputs. Channels with no
matching key have no publication or cleanup path in this reader.

## Exact x87 publication schedule

`B771C0` first loads group `+1C` through x87 and spills it to float32. It
then publishes the pointer and reads the last key's `+8` value through another
`FLD/FSTP32` pair. It reloads candidate then old value, executes
`FCOMIP old,candidate`, pops the remaining x87 value, and selects with `JBE`.

The candidate wins when old is less than or equal to candidate **or the
comparison is unordered**. Equal signed zeros select the candidate's sign.
The initial x87 loads/spills can quiet signaling NaNs and set exception flags.
The result is copied with `MOVSS`; it is not recomputed with a C++ maximum.
Source inline assembly retains this memory, spill and comparison order.
The native return value is the last-key `+8` address, also retained by source.
A time/duration interpretation of these fields remains provisional.

## Exception schedule

The `DFBF08` unwind map has two independent states, each returning to `-1`:
state 0 destroys the temporary name through `CC28F0`; state 1 releases the
current child through `CC28F8`. Handler `CC2900` uses FuncInfo `DFBF18`.

The name state starts after `ReadString` returns and is disarmed before normal
temporary destruction. The child state starts after `CreateChild` returns and
is disarmed before normal release. Failure during copy, scalar reads, append,
publication or skip therefore performs only the corresponding temporary or
child cleanup. A second cleanup exception during unwind terminates.

There is no allocation/channel guard at any point. Initial string-read failure,
partially read keys and errors after publication leave the channel allocation
and prior state intact. The source preserves that schedule for C++ exceptions.
Original FH3/SEH identity, hardware-fault delivery and private stack/register
aliases are outside the source interface.

## Validation and limits

The full bodies, EH code/map/FuncInfo, current channel slot and all twelve tags
total 1,265 bytes. All match live Ghidra and the installed PE; aggregate SHA-256
is `14ad85cf5c42ea84e3e7be68d469a755c33727af804b62c00a75a7f573c4af40`.
All 41 exact call/tail rows pass against the saved live listing.
Exact call rows, proposed names, old metadata and build/probe receipts are in
[`animation_channel_body_r27.json`](../reports/animation_channel_body_r27.json).

The ignored focused probe uses actual raw stream/node/pool/channel storage.
It exercises three keys with capacity growth 1/2/4, case-insensitive tags,
unknown-child detach and scalar-read failure that releases the child while
retaining the first published key. It also compares a copied 86-byte original
publication body against source for 72 pairs spanning signed zero, infinities,
subnormals, quiet/signaling NaNs and six masked x87 precision/rounding modes.
Output bits, returned address, complete x87 status word and publication agree.
The `FFFFFFFF` index is exercised using a valid preceding guard cell.

The probe's explicit image base leaves the required numeric `D68BB4` node
profile page free. It uses `/MANIFEST:EMBED`, `/MD` and no permanent tests.
The complete strict Win32 build and three existing CTests are recorded in the
report. Valid live storage, an available x87 stack and clear direction flag
remain preconditions. Build, byte and focused fixture evidence does not prove
original ABI, native allocator/CRT identity, whole resource deletion, executable
admission or gameplay.
