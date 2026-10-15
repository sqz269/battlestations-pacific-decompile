# Raw DWORD vector erase and clear

Whole006977F0..00697852 is99B/40instructions;whole00697890..006978C8 is57B/26.
The new Source uses actual rawDWORD storage and accepted currentSDK diagnostics.
Existing narrower storage fragments and memmove_s fixtures remain preserved.
Names are descriptive hypotheses, not recovered symbols.

Native erase takes ECX receiver,unusedEDX and five stackwords: output,
firstowner,firstposition,lastowner,lastposition;RET14h/EAX originaloutput.
It checks nonnull firstowner==lastowner only, without owner==receiver. Positions
are captured after a returning report. Equal positions skip end operations.
Otherwise capture CURRENTend, precompute first+SAR2(end-last)*4, then progressively
copy CURRENT DWORDs forward with captured displacement until equality. No null
skip or memmove is added. After a nonempty copy, reload firstowner; an empty
tail retains the initially captured owner. Publish end, then outputnode, then
outputowner. The return retains the output pointer captured before its stores.

Native clear takes ECX receiver,unusedEDX,no stackinputs,RET/no semanticEAX.
Capture ORIGINALend before the first begin>end report. Then capture CURRENTbegin
and compare to freshend for a second report. Call actualerase with receiver as
both owners, capturedbegin and ORIGINALend, using private8B output. Returning
handlers may mutate header fields; do not substitute end=begin or reload the
captured endpoints after the reports. No additional guard, rollback or EH exists.

Fresh complete typed bodies,156live/installed bytes and66currentstarts agree;
one bounded assignment-caller instruction also agrees. Five native transfers:

- `00697803 CALL 00bf6713` in006977f0.
- `006978a0 CALL 00bf6713` in00697890.
- `006978ad CALL 00bf6713` in00697890.
- `006978bd CALL 006977f0` in00697890.
- `00697bf5 CALL 00697890` in00697bd0.

The combined Source at dc431000048ce895c863dac22d422045bdab7509 passed
MSVC Win32 and all three existing suites in7.61s. Independent review passed
15checks with no findings, including all156PE/live bytes and66currentstarts.
All five report transfers and five Markdown claims passed. Compiled Source
retains the progressive loop, conditional owner reload and endpoint captures;
erase emits RET14h and clear emits plainRET. No new tests were added.
Reached storage and finite equality termination are required. Source private
arguments/outputframes, flags/register residue, nativeCRT/Watson/SEH/fault/ABI
and gameplay identity remain unproved; no new native execution is claimed.
