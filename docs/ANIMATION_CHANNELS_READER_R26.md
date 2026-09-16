# AnimationChannels reader and group construction (R26)

This packet reconstructs the actual `D6328C` slot `+20` reader at `00B8AD80`
and its native group-construction chain. The source uses the existing
`14h` (20-byte) item allocation, actual structured-node handles,
raw pooled strings, and pointer-vector storage. Interfaces carry explicit
source services; they are not callable replacements for the original binary ABI.

| Entry | Complete physical body | Behavior |
|---|---|---|
| `00B8AD80` | `00B8AD80..00B8AF04`, 389 bytes | Read item children and publish groups |
| `00B78C40` | `00B78C40..00B78CFF`, 192 bytes | Construct an actual `20h` group |
| `00B76710` | `00B76710..00B7676E`, 95 bytes | Reserve channel-pointer capacity |
| `00B774E0` | `00B774E0..00B7752F`, 80 bytes | Resize and zero added pointer cells |
| `00B77A10` | `00B77A10..00B77A26`, 23 bytes | Resize to zero, then free pointer storage |

These five ordinary bodies total 779 bytes. The primary integrator repaired
the saved Ghidra listing after the returning `_free` calls at `00B7675C` and
`00B77A1D`, including the latter function's truncated body. The recovered
post-free stores publish the replacement pointer/capacity; the destructor
tail restores the stack and returns. The worker made no Ghidra mutations.

## Reader ordering and ownership

The reader starts its register-captured current group at null. Each iteration
creates a real child handle and compares its current tag case-insensitively.
An `AnimationGroupName` child supplies a temporary pooled string. Allocation
of `20h` precedes the concrete group constructor; a null allocation publishes
a null group. The result is captured before pointer-vector reserve. Reserve
uses the previously established `B76790`/`B1C500` source reuse. The append
stores the captured group (unless the wrapping destination address is zero),
increments the item count, then destroys the temporary string.

The group constructor stamps `D62ED4`, copies the name to its actual `+8`
string, creates eleven null channel-pointer entries at `+10/+14/+18`, repeats
the native zeroing loop, and stores positive float zero at `+1C`. Reserve
copies borrowed pointers, frees the old allocation, and only then publishes
the new pointer/capacity. Neither vector resize nor destruction releases the
pointed channel objects.

`ChannelAnimation` calls the required `NativeAnimationChannelBodyCalls`
service with the actual item, child handle address, and captured current group.
That group can be null before a name appears. Unknown children use the actual
skip-and-detach implementation. The normal child release occurs before the
next `HasRemaining` check. `NativeAnimationChannelsReaderCalls` intercepts
only `00B8AD80`; every other captured reader target forwards unchanged.

## Exception schedule

The reader's `DFBF3C` unwind map and `CC292B` handler establish three states:

- State 0 releases the child handle.
- State 1 destroys the temporary name, then releases the child.
- State 2 frees the raw group allocation, then performs state-1 cleanup.

The raw allocation guard ends immediately after construction, before reserve
or publication. A completed group therefore remains allocated if reserve,
temporary-name cleanup, a later channel call, or child release fails. Published
entries/counts remain observable. Normal name cleanup is disarmed before it
runs, and normal child release is likewise disarmed before dispatch.

The constructor's `DFAD3C` unwind map and `CC1D2E` handler destroy its channel
vector, its name, and its reference base in that order. A caller construction
failure subsequently frees the allocation. The source implements this C++
exception schedule; a second exception during unwind cleanup terminates.
Original FH3/SEH identity, hardware faults, and private stack/register aliases
are outside this interface.

## Remaining channel dependency

Follow-on R27 supplies this packet's required service with
`NativeAnimationChannelBodyReader`; see
[`ANIMATION_CHANNEL_BODY_R27.md`](ANIMATION_CHANNEL_BODY_R27.md). It completes
the channel/key reader and publication chain described below. Group/channel
destruction and application wiring remain separate lifetime contracts.

`00B8AAD0` is a required concrete service, with no stand-in implementation.
Its 641-byte body allocates a `28h` channel, copies its name, reads two DWORDs,
classifies eleven channel names, and processes `AnimationKey` children. It
depends on `00B8A0A0` (121 bytes: DWORD plus nine x87 float32 reads into a
`28h` key, then `00B77870` append) and `00B771C0` (86 bytes: publish channel
by index and update group `+1C` using x87 comparison and SSE stores). The
remaining x87 comparison, invalid index/null group, key-array append, and
channel ownership contracts require separate recovery. This packet does not
claim a complete executable AnimationChannels parser or animation engine.

## Validation

Detailed metadata, names proposed for primary review, ABI contracts, exact
call rows, byte receipts, and validation results are in
[`animation_channels_reader_r26.json`](../reports/animation_channels_reader_r26.json).
The complete bodies, both unwind regions/maps, and current `D632AC` reader
slot total 980 bytes and all match the installed PE. Aggregate SHA-256:
`041ed046231d395857bbe21095199ddebfb0883de9567af062cfe578e7a7b302`.
All 29 exact call/tail rows pass live Ghidra verification.

The source passes independent MSVC Win32 `/W4 /WX /EHsc /fp:strict /MD`
compilation and the full Release build. All eight seeds matched disk before
the build, and all three existing CTests passed. One ignored actual-storage
probe passed six real child cleanups, two group publications, null/current
group forwarding, unknown detach, group retention and child cleanup after a
deliberate channel-service exception, finite dispatch, and borrowed-pointer
preservation through vector growth/shrink. The probe uses an explicit fixed
image base to leave the required numeric node-table page available; its
channel callback verifies only the required call contract, not `B8AAD0`.

Byte/build/fixture evidence does not establish original ABI, whole resource
deletion, executable admission, or gameplay.
