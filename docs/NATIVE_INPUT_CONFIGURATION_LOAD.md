# Actual input descriptor and script-loading prefix

Addresses: 006974F0; 00698A10 (prefix through00698B5E). Compiler handlers:
00C7EB90; 00C7EE45. Producers: 00698680; 00698730.

The descriptor routine now uses the existing native Lua object operations and
the actual embedded configuration. The loader's initial script stage uses the
same configuration's Lua owner, concrete action cleanup and required application
string/bootstrap/file services. The remaining modifier/action parser is not
implemented by this module. The prefix must not replace a complete698A10 call.

## Descriptor6974F0

Native ECX is the configuration; its three stack arguments are an actual14h
output, a Lua object pointer and a low-byte swap flag. EAX returns the output
and RET0Ch consumes the arguments. All three callers are inside698A10, at
69941A/6994D6/6995F7; their complete containing body and setups were checked.

The routine gets Lua index1, converts it to an integer and destroys the returned
temporary. It repeats that sequence for index2. It gets index3 and checks nil,
then destroys that temporary. A nonnil result causes a second index3 lookup,
integer conversion and destruction. That repeated lookup is observable through
a Lua metatable. Integer conversion uses the existing live CRT conversion mode.

Only after these temporary destructions does it read F88A30, then
configuration byte4C9 or byte4CC, then the supplied swap flag. A false gate skips
the pair vector entirely. The true branch searches actual10h checked DWORD-vector
rows in the outer header at configuration+4D0. The first matching pair exchanges
its first and second codes. Native validation may return; captured row pointers
and endpoint/header reloads are preserved. The owner comparison at69762D compares
a register with itself, so the diagnostic call697631 is unreachable.

Output stores occur at+C(code),+4(third/default0),+0(first),+8(zero), then
**byte**+10(zero). Padding11..13 remains unchanged. Neither a descriptor-owned Lua
state nor a copied Lua table or registration is introduced.

## Script prefix698A10..698B5E

The actual configuration is524h, beginning with the existing4C8h Lua owner at
offset **zero**. The prefix always runs698730 cleanup first. Byte4C8 only guards
opening that Lua owner with mask1; it does not guard script execution.

Every invocation reads live F88A30, constructs and executes `X360COMP=true` or
`X360COMP=false`, ignores that protected chunk's status and releases the string.
This flag publication differs from bootstrap's0108FF20. A fresh8h path header is
resized to29 with preserve enabled, then receives the30-byte terminated literal
`Scripts\datatables\Inputs.lua` when its current data pointer is nonnull.

The realB69D40 file-and-overrides routine receives that path and flag0. The path
is released before byte4C8 becomes1. The fragment stops before globals acquisition
at698B5F. `InputModifiers`, `Inputs`, action/binding production and the later tick
are outside this source entry. The required VFS/bootstrap/string services remain
borrowed application bindings; a missing service is not replaced with success.

## Cleanup and validation

Descriptor FH3 states0/1/2 release the temporary at EBP-34; state3 releases EBP-20.
All four unwind to state-1 throughB67700. Prefix states0/1/2 release the native
string at EBP-14C through41DD20. Ownership begins only after construction returns,
and normal destruction resets the state first. The source exception scopes
preserve that distinction and do not retry a throwing normal destructor.

`reports/native_input_configuration_load.json` records disk/live byte agreement,
45 checked CALL rows, ABI/cleanup evidence, final archive/source hashes and one
ignored native differential fixture. The fixture executes the original534-byte
descriptor and335-byte prefix. Its explicit return tail discards the pending
globals argument, restores saved registers and uses the original epilogue; it
does not pretend the prefix naturally returns at698B5F.

The native and source drivers share real Lua5.1.1, raw object/cleanup and file
loading leaves. Installed `fundamentals.lua` and `Inputs.lua` execute through
external fixture streams. Two prefix calls prove one bootstrap, repeated Inputs
table creation, current X360COMP values and action unregister. Five descriptor
phases cover nil/default, repeated third lookup, code reversal, live flag changes,
padding and two returning CRT repairs. All Lua temporary registrations, fixture
streams and string allocations are cleaned up. Win32 build and both existing
CTests pass; eight native seed byte comparisons pass.

This is component evidence, not native physical/archive VFS routing, complete
application startup or gameplay proof. Original FH3 exception execution, hardware
faults, arbitrary native stack aliases and malformed ranges are not covered by
these new C++ interfaces. No hardware polling, game launch or single-instance
bypass occurred. The full698A10 parser remains the next dependency.
