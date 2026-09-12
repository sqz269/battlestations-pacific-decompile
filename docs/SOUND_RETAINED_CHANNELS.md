# Retained sound channels

Addresses: 00A7F2F0, 00A7F0F0, 0054D510; integration of 00A7C5F0,
004C7FA0, 004E7BB0, 00A7DA40, 00A7DB80, 00A7DCD0, 00A7DAD0,
00A7DC20, 00A7DD60, 00A7DB20, 00A7DC70, 00A7DDB0, 00A7B520,
00A7B5D0, 00A7B690. Existing 0054D4C0 now accepts its narrower reference host.

These descriptive names are hypotheses, not recovered symbols. Source is in
`sound_retained_factory.cpp`, the three worker modules linked below, and the
existing channel and shutdown runtimes. Evidence comes from the saved
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, verified against the
installed executable. `reports/sound_retained_channels.json` records byte spans,
call sites, native signatures, validation and remaining limits.

## Producer and retained storage

The scene routine 004ED910 acquires an actual 7Ch sample at 004EDDCA through
A83FD0, publishes it in a 14h source record at 004EDDE4, and calls A7F2F0 at
004EDE43 for indices 0..2. The source has sample+0, NativeString+4/+8 and two
floats+C/+10. [Source record evidence](SOUND_RETAINED_RECORD.md) covers its
copy, ordinary destruction and reference assignment, including callback rereads.

A7F2F0 owns two different arrays: manager+74 holds channel references, and +80
holds actual sample references. The channel additionally owns its base sample4C
and copied source5C. This is three sample references per retained slot; the
active-entry array and +74 independently retain the channel. These are not two
interchangeable pointer types. Canonical +74 values point to SoundLevelEntry
subobjects; +80 continues to use actual sample storage and atomic native+4.

## Factory and helpers

A7F2F0 takes ECX=manager, stack source-record pointer and signed index, RET8,
with no meaningful return value. It exits when sound-enabled70 is false, or
when current80[index] equals the current source sample. The latter gate also
skips source-record refresh. For an existing74 pointer it calls current virtual8
with zero, leaving ownership in place until later assignment.

| Index | Native allocation | Constructor | Vtable | Update slot30 | Scalar slot4 |
|---|---|---|---|---|---|
| 0 | 70h | A7DA40 | D5AD58 | A7B520 | A7DB20 |
| 1 | 74h | A7DB80 | D5ADA0 | A7B5D0 | A7DC70 |
| 2 | 70h | A7DCD0 | D5ADE8 | A7B690 | A7DDB0 |

On successful allocation each branch constructs the literal `3DEffect`
(CE7860), looks it up through A7ACF0 on the then-current global sound owner,
then calls A7F0F0 on the original receiver's class table. A7F0F0 takes ECX=table,
stack signed index, RET4/EAX=slot; if index >= count it calls A7C2C0(index+1),
then rereads storage and returns that slot. Missing descriptors are not created.

The constructor returns one owned reference. 54D510 adopts it into the temporary
without retaining. This helper takes ECX=slot, stack replacement pointer,
RET4/EAX=slot address. It captures and releases an old reference before clearing
and publishing the replacement; equal pointers still perform that release.
54D4C0 differs: it accepts a source slot, skips equal pointers, and publishes and
retains the replacement before releasing the captured old reference. Its existing
C++ interface takes the source value and now needs only VoiceReferenceHost.

The common factory tail appends to the active array at A7F501/A7D5C0, assigns
channel74 at A7F50F/54D4C0, assigns sample80 at A7F519/4E7BB0, and releases the
temporary last. A null allocation still reaches this tail with a null channel.
FuncInfo DEB738 has ten states: temporary cleanup at state0; allocation cleanup
for each branch; and conditional class-name cleanup. C++ guards preserve the
order: name cleanup before raw allocation cleanup on construction failure, and
temporary-reference cleanup after successful construction. They do not emulate
the original SEH ABI or native allocation addresses and sizes.

## Runtime integration

[Constructor/destructor evidence](SOUND_RETAINED_CHANNEL_OWNER.md) establishes
the Air/Air/Underwater listener lookups, explicit source ownership, base cleanup
on record-copy failure, and correctly typed scalar deletion. The three vtables
share existing stop, refresh, completed, nonvirtual, handle, audibility, progress,
configuration and pause implementations. SoundChannelRuntime now selects their
actual update and scalar destructor; both existing bank virtual adapters accept
their identical slot10/14 methods.

[Update evidence](SOUND_RETAINED_UPDATE.md) establishes that the coordinate is
current-owner+C4 matrix element13 (native+F8). The first two methods use the
positive-coordinate ramp, preserving x87 stores and unordered clamp behavior.
The scaled method rounds scale70 times gain before the sample-options multiply.
The third method uses zero for a positive coordinate and one otherwise,
including unordered input. No world-height semantic is asserted.

SoundRetainedShutdownRuntime binds stop/release for canonical74 channels and
actual80 samples to these implementations. Alternate engine deletion remains a
required service with no default body. This closes the retained-array boundary
documented in [the previous shutdown packet](SOUND_SHUTDOWN.md); its historical
report remains a record of that earlier state.

## Validation and limits

Validation results and exact tested source hashes are recorded in the report.
The update worker compares 5,040 executions with original native bytes across
NaNs, signed zero, x87 precision and rounding modes. Record and owner probes
cover actual atomic ownership, string allocation failure and unwind ordering.
The integration fixture uses installed FMOD/VFS assets with the no-sound output
device, creates all three profiles, checks replacement and unchanged-sample
gates, and carries retained references through full shutdown. Event-backed
samples remain stopped during the bank-channel update portion of that fixture.

Supported factory indices are 0..2; native out-of-range accesses are outside
the reconstructed domain. Class indices must be nonnegative, below INT32_MAX,
and identify a valid configured descriptor when constructing. Contexts share
the same canonical owner/services, and supplied source/array storage survives
callbacks. Canonical containers and allocation failure behavior retain their
previous documented limits. Native ABI compatibility, audible playback and
gameplay have not been established. Application phase5/A88770 composition is a
separate packet and the application-host files are leased by another owner.
