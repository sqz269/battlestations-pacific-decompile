# Actual native frame-clock methods

R35 reconstructs the raw 80h allocation used by D68D50. The semantic `FrameClock`
remains separate. The new storage type has no default initialization: profile
+00, float bits +04, update count +08, five 16-byte ticks/frequency pairs
+10/+20/+30/+40/+50, frequency +60, paused/fixed bytes +68/+69, six untouched
bytes, signed increment +70 and synthetic counter +78.

## Methods and boundaries

- BEDAE0 pauses and captures the current +20 sampler slot before storing paused.
- BEDB20 enables fixed mode before QPC, ignores its BOOL, and computes the low
  64-bit product of frequency and signed 32-bit milliseconds divided by 1000.
  BEDB60 clears only fixed mode.
- BEDBD0 clears paused and the update count, calls QPF into actual +60 and QPC
  into an uninitialized local, ignores both BOOLs, installs origin, and resolves
  the current +08 update slot separately for each of two calls. It preserves
  fixed mode, increment and synthetic counter.
- BEDC30 retains the original DWORD loads/stores and x87 FILD/FDIVP/float32
  spill/FCOMIP schedule. Paused updates self-subtract the interval. A negative
  interval restores current and clears interval without reversing counters;
  unordered comparison does not trigger rollback.
- BEDDC0 captures the current sampler before clearing paused and adds the
  sampled pause duration to origin. Three four-byte getters return +20/+30/+40.

BEDB70 and 00530890 reuse existing timestamp arithmetic on real, separate
16-byte timestamp objects through private ABI bridges. No duplicate public
arithmetic provider is introduced. The bodies use stack outputs disjoint from
input timestamps; arbitrary partial-overlap arithmetic is outside this adapter
contract. BEE080 reuses the existing complete 93-byte native sampler.

Raw methods retain ECX and native stack cleanup. Initialize/pause/resume take
an additional explicit EDX context. Their two admitted profile slots must read
BEDC30/BEE080 from a caller-borrowed D68D50 table and the raw object's profile
word must be D68D50. Unsupported identity/slots terminate at this new source
boundary; original numeric code addresses are never invoked. Each current slot
is read at its native dispatch point, including capture before a flag change
and before initialization's origin-high store.

QPC/QPF failures preserve the native output destination/preimage behavior;
there is no success bool or fallback. Local QPC scratch remains uninitialized,
so a failure that leaves it untouched has indeterminate native input bytes.
No clock is constructed, published to 01090AB0, registered, deleted, bound to
an application or adopted by a worker here. The later owner/lifetime packet
must establish those contracts. This component alone proves no live game loop.

Detailed original spans, ABI, calls, build and fixture boundaries are recorded
in `reports/native_frame_clock_actual_r35.json` after verification.
