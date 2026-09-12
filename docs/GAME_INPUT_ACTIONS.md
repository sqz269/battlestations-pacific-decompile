# Raw action frame composition

Addresses: 004BEC00; 00A92C40; 00A922A0; 00A91E80; 00A92370; 00A91D60;
00A92090; 00A91A50; 00A92B70; 00A92150. Source composition, no new native entry.

GameInputActions binds the existing GameInputRuntime to the recovered actual
action tick, binding resolution/polling and listener update. Its update obtains
the same lazy raw action publication with4BEC00 before callingA92C40. The tick's
captured backend/profile passes through a dedicated facade entry without another
read. Device slots1C,20,24 use the finite existing runtime directly, retaining
captured receiver/profile/code semantics. No additional owner, action vector,
listener state or device cache is introduced.

All timing, curve, zero and callback cells are borrowed. Paired-axis adjustment
uses the required existing CRT access. F8BBFC dispatch remains a required
application provider with the captured identity; this class neither writes that
publication nor supplies a placeholder callback. The known6965A0 callback needs
the actual game clock and raw12h deadline-map contract. The application also
needs action setup at its first-time initialization stage before a frame caller
can use this composition. GameStartupHost does not instantiate it yet.

The facade's record destructor now has the finiteD5B610 listener provider when
no explicit application override is installed. Its native allocation/release
order and scalar deletion are in NATIVE_INPUT_ACTION_LISTENER_OWNER.md. The
extended facade fixture creates one actual30h record with one actual24h listener
and reaches built-in deletion through the existing BD0400 manager drain.

## Verification boundary

The final batch report records the strict Win32 build and existing checks.
Worker native-byte differential fixtures cover the raw tick/listener and binding
bodies; the actual-storage listener fixture checks lifetime. These are distinct
from the source composition: GameInputActions has not been exercised through an
application frame, and no complete input or gameplay validation is claimed.
The live game and single-instance check remain untouched. No additional test
suite, game launch or hardware polling was added for this composition.
