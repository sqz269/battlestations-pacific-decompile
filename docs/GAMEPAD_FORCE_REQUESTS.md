# Gamepad force requests

`GamepadForceState` supplies the populated common gamepad request state for both
joystick and XInput projections: an unsigned-ID ordered map owning actual game
requests and the two native amplitudes at +218/+21C. The canonical active device
groups remain the existing `InputBindingDeviceGroups`; no second list is created.
`GamepadForceHost` reloads the actual F8BBF4 groups and binds each actual device to
its one force state and +38 output implementation. The context references the
actual E12F2C enabled flag and E12F30 ID counter (image default 1).

Names are hypotheses, and these C++ interfaces do not reproduce native object,
vtable, allocation or exception ABIs. Evidence is read-only `bsp.py ghidra`
analysis of `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, supplemented
by disk assembly where functions or fallthroughs are missing. The JSON report
lists original ABIs, addresses and remaining metadata repairs.

## Request classes

The three native vtables have the same five slots: scalar deleting destructor,
channel getter, value getter (ST0), expiration getter, and update(float). The
typed classes implement the actual game operations rather than callback stand-ins.

| Native vtable / allocation | Typed request | Behavior |
| --- | --- | --- |
| D0DB78 / 14h | ConstantGamepadForceRequest | Stores channel, amplitude and remaining time. Update subtracts elapsed seconds except for exactly FLT_MAX; expiration is remaining <= 0. |
| D0DB8C / 18h | FadingGamepadForceRequest | Starts elapsed at zero, adds each update and returns amplitude*(duration-elapsed)/duration. Expiration is elapsed >= duration. |
| D0DBA0 / 28h | AlternatingGamepadForceRequest | Chooses between two values according to phase and the selected first period. Updates phase with fmod(phase+seconds, period1+period2); expiration compares this wrapped phase against duration. |

The alternating request's expiration deliberately uses wrapped phase, not a
separate cumulative timer. A duration larger than the period therefore does not
expire under ordinary positive updates. Boundaries are strict for period
selection and inclusive for expiration. Ordered comparisons keep NaN from
expiring a request or contributing to a maximum. Native x87 arithmetic and
float stores are retained; BF857A dispatch metadata at E15500 explicitly names
`fmod`, implemented through the real CRT. No CRT routine is reconstructed.

The constructors at 8722D0, 872350 and 8723F0 are standalone game constructors;
effect/action creation at 873450, 873560 and 873750 also inlines these same
layouts before A95BF0 submission. The surrounding action and spatial attenuation
logic is outside this packet. The scalar deleting wrappers have no owned payload
cleanup beyond normal allocation deletion. A94BC0 writes the entire request
DWORD+C before querying its channel and refreshing. For constant/fading requests
this changes amplitude; for alternating requests its low byte changes the
first-period selector. The typed raw payload preserves that behavior.

## Registry operations and ownership

A95780 inserts the `(unsigned ID, request*)` pair and refreshes the incoming
request's channel, without ticking or testing expiration. Native insertion is
unique-key insertion and ignores its success result. A duplicate retains the old
entry, does not delete or take ownership of the incoming request, and still
queries the incoming request's channel. The typed function returns the insertion
result to make this ownership outcome visible; that return is an interface
extension, not a recovered native return contract.

A95BF0 resolves the unsigned active class-2 device index from the current backend.
Missing devices delete the supplied request and return zero. A present device
takes the old global ID, increments with unsigned wrap, changes the next ID from
zero to one, inserts, refreshes, and returns the old ID. It does not search for an
unused ID. Exhausting the ID space can therefore produce the duplicate ownership
case described above. Null requests on present devices and invalid channel
indices are explicit host errors corresponding to unsafe native dereferences.

A94B50 finds a request. A94D40 searches current active gamepads in order and
returns the first match. A957D0 clears a nonzero handle only when that search
fails. A95410 captures the request channel, destroys the request while its map
entry still holds its pointer, erases the entry, then refreshes that channel.
A957F0 removes the first match across active gamepads. Requests are owned and
deleted directly; these objects do not use the game device's intrusive refcount.

A949A0 does nothing if feedback is disabled at entry. Otherwise it scans every
request in unsigned-ID order, calling value **before** channel. It selects the
largest matching positive value, starting at zero, without ticking or checking
expiration. A changed amplitude is stored before +38 output dispatch.

A954C0 snapshots both old amplitudes, zeroes both live fields, then walks the map
in unsigned-ID order. It advances the iterator before invoking update(seconds),
then checks expiration. Expired requests are destroyed before erasure. Surviving
requests contribute only while the live global enabled flag is true; this path
calls channel **before** value. After the walk, one enabled-flag check gates the
two output comparisons, in channel order. Each comparison reads that channel's
then-current live amplitude. Changed values, including NaN inequality, dispatch
to +38. Disabling feedback does not pause request aging or expiration.

A95960 captures the initial class-2 count and pumps occupied devices. A94C50
stores the global enable flag before loading the current backend. Disabling
always dispatches zero, even for a channel already zero, and clears the saved
amplitude **after** that dispatch. Enabling refreshes each channel instead.
The enable flag is reread for each channel. Global walks preserve their captured
initial count and reload the actual backend after each occupied-device operation;
they do not reuse a snapshot of device pointers across callbacks.

## Clearing and base destruction

A95890 destroys every request in ascending key order, keeps the native entries
present during request destruction, clears the registry, and pumps 0.0 against
the still-live derived device. Standard containers replace native tree node and
sentinel mechanics.

A95A80 has a different dispatch boundary: at A95AA2 it first installs D5B670,
whose +38 entry is the actual CRT `__purecall` at BF698E. It destroys all requests,
clears the tree and pumps zero. With feedback enabled and an old nonzero/NaN
amplitude, this reaches purecall instead of the derived joystick/XInput setter.
The typed destructor operation preserves request cleanup and zeroing, then
reports this invalid state with a logic_error. It does not send invented derived
output. Explicit A95890 clearing can establish zero state before base destruction.
The C++ state destructor destroys any residual requests in key order but cannot
perform device output without the externally bound device/context lifetime.

The integrator repaired two false no-return gaps in A95A80 under its write lock.
Bytes now cover the verified return at A95BC4, but the saved function body still
ends A95B93 because server-side Java scripts are disabled. This worker made no
Ghidra changes. A95890 also has a missing fallthrough at A95923 after _free;
its assembly continues through RET at A9595C.

## Actual joystick output

`set_joystick_force_00a98cc0` implements +38 over the genuine SDK
IDirectInputEffect::SetParameters method. It binds references to the existing
effect pointers, kind, direction mode and activity deadline. Correction to an
early packet interpretation: A99940 initializes **both** kind+B10 and mode+B14
to zero at A99992/A99998. Mode is not uninitialized.

An absent effect returns before reading the clock. Otherwise the setter reads
the actual current clock's +14 timestamp using canonical `timestamp_seconds_x87`.
A strictly later time than the activity deadline forces value zero; equality
remains active. It then constructs the 56-byte DIEFFECT: flags 12h, duration
FFFFFFFF, gain 10000, no trigger, other ordinary fields zero. It multiplies value
by exact double 10000.0 and truncates through the compiler/CRT conversion
boundary. Constant mode uses the result; direction mode 1 negates channel 1.
Ramp mode writes equal start/end values and additionally divides channel 1 by
three for direction mode 0. No clamp is added. The effect pointer is reread
after the clock call, then SetParameters receives flags 20000100h
(type-specific parameters plus start). Its HRESULT is ignored. No COM ownership
operation is added. The inspected native SSE conversion path is represented;
extreme values and the old non-SSE CRT conversion fallback are not validated.

Correction to `JOYSTICK_INPUT.md`'s earlier boundary description: vslot **+3C**
at A99710 is a binding display-name getter, appending /Left, /Right, /Up or /Down.
It is not a force command. That independent string operation remains a bounded
follow-up; force-request output dispatch is exclusively +38.

## Validation

MSVC Win32 Release build and both existing CTests pass, with eight native seed
matches. One ignored `local/gamepad_force_fixture.cpp` checks the actual three
request classes plus recording interfaces for call/lifetime ordering, duplicate
ownership, ID wrap, backend replacement during output, captured iteration count,
disable-before-zero ordering, base purecall boundary and every joystick effect
field. The fixture's fake COM interface returns failure to verify ignored
HRESULT behavior. It performs no DLL loads, haptics, hardware, windows, or focus
changes. Build and fixture proof are not hardware or gameplay validation.
