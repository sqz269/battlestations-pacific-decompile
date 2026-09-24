# Platform messages in the application

Native references: BED3B0 message arms, BEC1A0 loop cells +42/+43/+181,
BECEE0 device/cache/power sequence and 73DC7C online construction.

The application binds the control-message fragment and the text-message
adapter recovered in PLATFORM_CONTROL_MESSAGES_CC10.md and
PLATFORM_TEXT_MESSAGES_CC10.md. WM_GETMINMAXINFO can precede WM_CREATE:
the control handler retains a null window-extra receiver without dereferencing
it on that arm. The minimum tracking size is 100 by 100. WM_PAINT discards
DefWindowProcA's return; SC_SCREENSAVE suppression compares the full WPARAM
to F140, without the usual FFF0 mask. The sizing-loop arms modify the captured
window-extra platform; settings changes modify the explicit active platform.

Win32PlatformFields now derives from the existing semantic PlatformLoopState.
GameStartupHost::loop_ references that same object. This removes the old
startup-only frame-enable copy, so WM_ENTERSIZEMOVE and WM_EXITSIZEMOVE affect
the cell actually read by BEC1A0. Exit and loop-finished also share storage.
This is a typed C++ composition, not an original platform ABI or field layout.
The separate native +30/+41 focus storage remains the existing binding.

GameStartupHost owns one persistent PlatformTextInput for its platform lifetime.
The explicit active-platform binding publishes that same text dependency and
clears both pointers before window retirement. Native text arms use the explicit
receiver, not the window-extra receiver. No WM_CHAR or WM_KEYDOWN path enables
text capture. Actual frontend text-owner activation and dispatch are still
unbound; this change does not claim functioning text editing or clipboard paste.
Older offset-labelled fields in Win32PlatformFields remain constructor
projection data, not independent text-queue producers or consumers.

The follow-up TEXT_EDITOR_ACTIVATION_CC10.md now reconstructs A966E0 over
borrowed actual owner fields and that same queue. Its input getter and update
are separate call-site contracts, passing the exact returned owner with zero.
It is build/call checked; connecting the actual frontend owner remains open.

The online placeholder now follows the window's device/cache sequence. Native
BECEE0 performs those operations, then power initialization at BED223, before
returning to 73DC27; the online constructor is later at 73DC7C. The parent-PID
pipe peer required by online startup remains unresolved.

The exact power producer, BEBF70 stop routine and BECE41 screensaver call are
available as recovered source. Production power mutation remains explicitly
unbound: the native normal-shutdown schedule for restoring saved policy is not
established. BECE30 enables the screensaver but does not itself call BEBF70.
No invented restore call or global power-setting mutation is introduced here.

Activation source and recovered GUI/media providers are recorded separately in
PLATFORM_ACTIVATION_CC10.md. The application's canonical GUI/media lifetime and
renderer focus dependencies are not composed, so WM_ACTIVATE remains unbound.
Empty substitute owners would not demonstrate native focus propagation.

Validation and publication revisions are recorded in
reports/platform_application_cc10.json. Build, focused differential fixtures,
real Windows message behavior and game runtime evidence are distinct scopes;
none proves original ABI compatibility or full gameplay parity.

At merged revision fe57f93c5, MSVC Win32 and all three CTests passed. The
actual game window accepted the 100px minimum and returned 1 for exact F140.
Entering the sizing loop held a pending close for one second; exiting it
allowed the next frame to consume the close and finish. The unrestricted run
presented 49 frames, skipped one Present, and exited with code 0. An earlier
positive-frame-target run passed message checks but correctly failed the
harness's unmet frame-count requirement after the deliberately early close.

Seven bounded stored Ghidra bodies had returning-free discovery repaired and
two platform destructors were defined from verified bytes. Complete body tails,
zero remaining listed call gaps and refreshed exports are recorded separately
in reports/platform_flow_repair_cc10.json. Existing names/comments were retained;
the two new destructor names are descriptive hypotheses. These are analysis
repairs, not additional executable lifetime bindings.
