# Native input settings lifetime

Addresses: 005547D0, 006AB6B0, 006AA460, 006AB800, 00BD0400

`native_input_settings_lifetime.cpp` composes the actual settings constructor
and table parser with lazy publication, populated destruction and scalar
deletion. It continues `NATIVE_INPUT_SETTINGS_STARTUP.md` and
`NATIVE_INPUT_SETTINGS_TABLES.md`; their separate prefixes remain available.
The source borrows the application's actual E198E8 settings publication and
01090AA0 raw manager publication. It creates no substitute lifetime domain.

| Entry | Original interface and extent |
| --- | --- |
| 005547D0 | No arguments; EAX settings; RET; 192 bytes |
| 006AB6B0 | ECX fresh 540h storage; EAX same address; RET; 332 bytes |
| 006AA460 | ECX settings; no result; RET; complete 480-byte normal sequence |
| 006AB800 | ECX settings, one stack flags DWORD; EAX original address bits; RET4; 30 bytes |

The descriptive names are hypotheses, not recovered symbols. These functions
use explicit C++ service interfaces, not original callable virtual tables.
CF81CC's original slot zero contains 006AB800; the base profile CE3818 contains
00412440. The raw manager's deletion bindings now accept a borrowed
`NativeInputSettingsLifetimeContext` and route CF81CC to the scalar deleter.

## Construction and publication

The full constructor calls the existing raw prefix, which builds five native
tree heads, three checked vectors and the persistent Lua owner at +78. It then
calls the complete raw table parser. All partially populated members are owned
by the constructor until parsing returns. Failure after prefix construction
closes persistent Lua, destroys members in reverse construction order, clears
the current E198E8 publication and writes the CE3818 base profile. Failure
inside the prefix remains covered by that prefix's empty-member cleanup.

The getter first captures E198E8 and returns that value immediately if nonnull.
Otherwise it obtains the raw manager, captures its section at +10, enters it
and increments the actual recursion word at section+18. It rechecks E198E8,
allocates 540h, constructs, publishes, obtains the manager again, and only then
reloads the settings publication for registration through BD0C30. It releases
the originally captured section before reloading the slow return value.

Constructor failure frees the captured allocation and releases the section.
Registration failure retains the published allocation. Native getter state0
uses C6D5B0 -> 411EE0 for section cleanup; state1 additionally frees the saved
allocation through C6D5B8. The source reuses the existing raw lifetime access
and registration routines, including their allocation and validation behavior.

## Destruction and exceptional cleanup

The destructor first stamps CF81CC. It calls B65E80 on owner+78, changes from
state9 to state8, then calls B669A0 on that same owner. Both original bodies
perform the same close-and-clear operation. The second call remains distinct;
the code does not collapse the calls across the state transition.

It then destroys the following actual members:

| Offset | Storage | Required native library call |
| --- | --- | --- |
| +6C | Duplicate device tree | 006A7AA0 |
| +60 | Controller-name tree | 006A6A20 |
| +54 | Signed-key count tree | 0069FE70 |
| +40 | Conflict-pair row vector | 0069EEA0 |
| +30 | Conflict-group row vector | 006A6EE0 |
| +24 | Input-name tree | 006A1AA0 |
| +14 | Device-name order vector | 00432050 |
| +08 | Device tree | 006A7AA0 |

Tree cleanup captures the current head and minimum node for checked erase,
then reloads and frees the current head and clears header+4/+8. Vector cleanup
captures begin/end, destroys a nonnull range, reloads and frees the current
begin, and clears header+4/+8/+C. Opaque header words remain untouched. Normal
range calls receive the settings address as their second stack word; the
consumed full-member unwind contracts receive the member header address.

The normal state writes are 9, 8, 7, 6, 5, 3, 2, 1, 0. State5 stays armed
during the +40 pair-vector cleanup: a throwing range call would run that full
member's unwind cleanup before continuing lower members. The source preserves
this distinction. Constructor and destructor FH3 tables 00DACF70 and 00DACE7C
each contain ten states with the same reverse
member order. The corresponding funclets and consumed member bodies were
checked; original FH3 execution itself is not reproduced or tested.

After member cleanup, destruction clears E198E8 unconditionally, even when it
currently points elsewhere, and stamps CE3818. It does not unregister. Scalar
deletion tests flags bit0 after destruction and frees the same allocation only
when that bit is set. It returns the original pointer bits even after freeing.
Callers performing early deletion must provide the separate unregister order.

## Verification and remaining work

The strict MSVC Win32 build and both existing CTests pass. Eight seed spans
match disk and Ghidra. One ignored manifested differential fixture compares
122,545 words between captured native code and the source across direct
construction/flags2 deletion and lazy construction/flags1 deletion. It uses
the installed scripts, actual populated native containers, the real raw
manager implementation and isolated Win32 critical sections. It checks
registration identity, released recursion depth, fast getters, parser guard
replay, cleared headers/publication, base profile and pooled-string release.
The source getter also passes a controller-script open failure after earlier
tables have populated: cleanup releases all pooled strings, publishes nothing,
registers nothing and releases the captured section. An additional source
integration check drains a raw manager holding fully populated settings through
the CF81CC binding. At the persistent Lua finalizer the settings pointer has
already been popped, while the manager's section still exists. A differing
settings publication does not suppress deletion; it is cleared by the actual
destructor. The manager then releases its section and vector storage.

The fixture contains 418 native spans. Four owned routines were freshly checked
against live bytes; reused library spans retain the prior AP live evidence and
their disk hashes were checked again. Eighteen CALL rows pass the live function
ownership audit. Fourteen further direct CALLs in the complete destructor tail
are recorded separately as pending saved ownership, alongside the getter's two
Win32 import calls. They are not counted as passed ownership checks.

Flow repair decoded the destructor tail after its returning free call, but the
saved Ghidra body still ends at 006AA4CA. Extending the body was rejected by the
bridge because script execution is disabled. The attempted operation and prior
metadata are preserved in the body-extension report; no full saved-body repair
is claimed. This limitation is distinct from the complete live/disk byte check
and native execution of 006AA460..006AA63F.

Production tree/vector providers remain required. The borrowed settings context
must outlive raw manager drain and provide those same container services.
The scalar-deletion comparison also verifies that explicit early deletion
leaves registration alone, then removes that pointer before manager cleanup.
Application construction/wiring, arbitrary private-stack aliases,
malformed storage, original FH3/hardware-fault behavior and gameplay remain
unvalidated. No running game was accessed and no agents were dispatched.
