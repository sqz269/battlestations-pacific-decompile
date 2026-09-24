# Raw GUI interaction properties through transform

This is the next partial projection of `AAA710`, exactly
`[AAABD6,AAACD4)` (254 bytes), ending with the five-byte call at AAACCF.
The composition helper runs the existing ten-property prefix followed by this
stage, covering `[AAA710,AAACD4)` (1,476 bytes). Key enumeration, child creation
and the function epilogue remain outside this implementation. Names are
descriptive hypotheses, not recovered symbols.

The original whole-function ABI is ECX actual widget, stack actual visitor,
RET4. This stage has no native return. Its source interface shares the same
initialized `NativeGuiWidgetPropertyScratch` and actual widget/visitor with
the prefix, plus genuine visibility and transform contexts. At the native
entry EDI is widget, ESI visitor, EBX zero; the old position EBP is overwritten
before its next use. At the end EBP identifies widget+84.

The current widget +5C call runs first. Only return value1 skips Visible.
For other values, the visitor's current +0C receives key Visible/D5C1E4,
tag3 destination widget+E4 and inline default byte1. The resulting byte is
captured before reloading the widget's current profile and +34 target.
Recovered targets A9E110 and AA8530 use their existing raw providers;
every other target requires its real supplied dispatch implementation.

MouseBlock/D5C1D8 is always read into widget+84 with tag3/default byte0.
A current nonzero value writes byte1 to widget+78 and skips the MouseHit
reader call. Otherwise MouseHit/D5C1CC reads tag3/default byte0 into +78.
The complete raw AA7220 transform follows either branch. The visitor profile
and +0C target are reloaded at every read; final metadata follows that target
load. MouseBlock writes field metadata before its tag, while the other two
reads write the tag first. All raw active field, key, fallback and metadata
stores retain their order.

As in the prefix, the three pairs occupy scratch+0/+8/+10 and metadata is
scratch+28. The same 7F8h local region survives composition. Boolean payloads
have meaningful low-byte semantics. This source does not reconstruct the
upper three fallback payload bytes produced by native call return addresses
or transient stack reuse; derived visitors must respect the tag3 payload
contract. The wrapper is not an outer ABI or private-stack alias substitute.

All 254 bytes match live Ghidra and the installed PE. Six exact call sites
were decoded: five current indirect dispatches and one direct AA7220 call.
Strict MSVC Win32 Release and the three existing CTests passed. No new test
was added: the prefix's eight raw Lua comparisons and the existing visibility
and transform comparisons are separate callee evidence. The new stage and
composition are static/build checked, not runtime compared or game validated.
Native FH3/SEH, escaping callbacks and invalid storage remain outside the
admitted domain.

The missing native handler CB7571 was separately defined under the Ghidra
write lock using its exact ten live/PE bytes, ending CB757A. It loads EAX
with DEDDEC and jumps to BF6B43. Naming/definition is analysis evidence;
its native exception behavior remains unported.
