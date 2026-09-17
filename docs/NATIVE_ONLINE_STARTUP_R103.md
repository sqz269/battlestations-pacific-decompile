# Recovered native online constructor, SDK and IPC (R103)

Addresses: 00a3f530, 00a3f5d0, 00a3f670, 00a3f840, 00a3f9d0, 00a3fdc0, 00a40df0, 00a4bc80, 00a4bd40, 00a4bd80, 00a4bde0, 00a4be50, 00a4c000, 00a4c030, 00a4c250, 00a4c280

An older orchestration branch contained the complete raw constructor, SDK
forwarding and IPC worker, but those changes were absent from main. This packet
recovers its four modules, records their original commits and exact hashes, and
validates them against the current build and original executable. It replaces
R102's standalone lifetime module with this complete shared implementation.
There is one raw online lifetime path and one raw2Ch IPC representation.

## Constructor and ownership

A40DF0 first registers the exact3F0h allocation through A3F530, then initializes
only the observed fields. It preserves the +360 vector prefix and +398/+39C,
abandons old vector storage and +3A0 without freeing them, and stores callback
DWORDs at +20/+24. The device and mutable present-parameters pointer are obtained
from separate reads of the current renderer; the SDK receives renderer+1A28.

The constructor ignores SDK startup results and tests the actual WSADATA version
after partial or failed writes. The 400-byte caller preimage remains explicit.
Mismatching version calls cleanup. The complete DWORD NTOHS return reaches the
port setter; listener bits remain at +1C, including zero/minus-one. The actual
+3AC slot is initialized by the raw IPC provider. After concrete sign-in reset
and the full native pump return, the constructor clears +14C and +12C without
freeing a buffer a nested callback may just have installed.

Constructor unwind is vector then base. It does not clean IPC, +14C, +3A0 or SDK
state. The base retains its captured first section and second manager/current
publication behavior. Normal derived destruction closes IPC, releases/clears
+14C, releases/clears the vector, and unregisters the current publication. The
full lifetime retains the original two +14C tests. Native scalar flags bit0
consumes a default-initialized `new NativeOnlineManagerStorage` allocation;
matching CRT memory callbacks own storage and achievement-vector buffers.

The shared drain keeps `native_online` at offset148 and now borrows the complete
lifetime context. Legacy projected and raw bindings remain mutually exclusive.
R102's archives preserve its previous implementation; this packet's fresh
evidence describes the replacement.

## Raw IPC and SDK boundaries

A4C030 allocates exactly2Ch, preserving allocation preimages at +14/+20 until
written. Services are outside that storage. Slot initialization, close guards,
encode/decode, seven loop phases and the stdcall worker use the raw endpoint.
Capacity/output preimages remain explicit. Creation failures retain native
cleanup and exit behavior. Worker runtime binding is immutable and must point
to services retained until process exit. The native destructor ignores the
1000ms wait result before freeing storage: this behavior is not a safe-join
claim. Worker failures other than E_ABORT retain the native process-termination
path.

The startup adapter borrows an already loaded XLive module. Seven ordinals and
the language import were checked against original IAT/stub bytes. It forwards
the full 1Ch initialization and 400-byte WSADATA images and raw SDK result bits;
it does not turn SDK failures into success or an invented output value.

## Current validation

- Strict MSVC Win32 build and all three existing CTests passed.
- 2,463 body/data bytes and 42 SDK-stub bytes match live Ghidra and the installed
  PE. All 16 normal function listings are gap-free. The report includes all
  74 CALL sites; the mechanical verifier checks the 54 direct calls. Import and
  tail targets retain separate evidence.
- Four recovered local fixtures were rebuilt against the current library.
  The parent comparison executes copied original567-byte constructor and159-byte
  destructor bodies with actual reconstructed children and fixture SDK/OS/pipe
  boundaries. Six cases match all3F0h/2Ch owner images, SDK images, and89 ordered
  observations; all30 binary capture pairs match. It also tests current-owner
  replacement during shared scalar drain and source-only SDK-exception cleanup.
- Nine original/source IPC cases match. A real Win32 thread runs the new worker
  with fixture pipe services and exits with E_ABORT. No network endpoint opens.
  Base-publication and SDK forwarding fixtures pass separately.
- Five original-code arrays in those fixtures match the installed PE. Original
  parent FH3 handlers are not executed, and its WSADATA preimage is seeded at the
  fake SDK boundary. These are explicit limits of the comparison.
- The ordinary application smoke exits after two ticks/one Present, joins the
  renderer worker and ends with device/API COM counts0/0. It still does not call
  the new online constructor.

Original recovered source commits: `910992029` (base owner), `d6dac38fd` (SDK),
`f6c6d2cbb` (IPC), `2b6fd3ef8` (full lifetime). No new permanent tests were added.
The report preserves source hashes, adapted fixture inputs, outputs, current
dependency manifests, saved Ghidra evidence and immutable artifact receipts.

## Follow-up dependencies

Compose these services into the application with its canonical raw online,
renderer, clock, string, profile and IPC domains. Migrate projected online
consumers before publishing the raw owner. Retain real stack/allocation preimage
requirements, IPC service lifetime and original timeout behavior. Then bind the
procedural sampler loading-message pump and continue material/preload startup.
The initial sampler stack word, live online startup, native ABI/FH3/SEH and
gameplay remain unproved.

Evidence: `reports/native_online_startup_r103.json`.
