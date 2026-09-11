# Fonts and fingerprint payload in application startup

Addresses: 0073bae0, 0053bc00, 00be9620, 00be9630, 00be9760, 0053c740.

The executable now owns font metadata, decoded glyph data and retained D3D9
atlas/alpha textures through `GameFontHost`. It executes `Fonts/Fonts.lua` with
the existing VFS/Lua services and current runtime globals, using the exact
`Fonts\` root and selected language font path. Each font invokes the recovered
stream loader in GFX, alpha, DAT order. The same application D3D9 device creates
the actual textures; imports outlive them and fonts close before device/VFS.
Native global singleton allocation and the renderer's texture-cache identity
are not reproduced by this application RAII binding.

The supplied mip reduction is zero: native constructor00b32769 stores EBX=0
at renderer+1D84. The later ApplyAll texture-detail setter remains unbound in
this executable path; no texture-quality setting is inferred from its label.

After fonts, startup opens `fonts/arial19.dat` in VFS mode2 and constructs its
payload, matching0073bc0d..0073bc17. GUI resources00aa5e20 remain an explicit
unimplemented application phase until real scene/model/widget contracts can
be bound. The new GUI resource and Lua page modules do not by themselves prove
that the interface renders. The executable still presents the startup frame;
this batch does not establish gameplay or native font shader/cache parity.

## Correction: arial19.dat contains the fingerprint payload

The earlier `FallbackGlyphTable` names were hypotheses inferred from the file
name and 512-byte extent. Consumer0053c740 supplies the adjusted payload to
the child named `Fingerprint_Text`: name literal00cedb40, getter0053d1b9,
data adjustment0053d1c2, child lookup0053d1d2 and text setter0053d1d9. It is
not established as a glyph-width table. New descriptive Ghidra names use
`FingerprintPayload`; these remain hypotheses, not recovered symbols.

Constructor00be9760 takes ECX=this, returns EAX=this, plain RET00be988b.
The native object is204h bytes, data begins+4, vtable00d68bb0. It opens the
file with mode2, checks both halves of the64-bit length against200h, reads
512 bytes once with null actual-count pointer, releases the stream, then
calls00be9630 with data and key00e15120. On length mismatch it releases the
stream and defines only data[0]=0. Null streams and short reads are not safe
native paths; the C++ binding reports them instead of dereferencing/decoding
undefined memory. The getter0053bc00 is a no-argument cdecl lazy singleton;
00be9620 takes ECX=this and returns this+4 without consuming stack arguments.

Decoder00be9630 has two stack pointers and RET8 at00be96f1; ECX is unused.
Assembly00be9665..00be96d8 XORs source/key16-bit words16..255, retains mask
AAAAh and compacts bits1,3,...15 to one byte. EBX is output-base minus16,
EDI starts16, so the loop defines output[0..239]. The final REP MOVSD copies
512 bytes from that stack buffer, including272 bytes the function never wrote.
`FingerprintPayload::byte()` returns no value outside its defined prefix;
zero-filled backing is never passed off as the original undefined bytes.

The input and512-byte key matched the saved Ghidra image and installed PE.
An ignored Win32 probe executes the original196-byte routine from a private
executable allocation, with the actual key and installed payload plus33
focused additional inputs. All34 defined240-byte outputs agree with the
reconstruction. The probe also checks the constructor's mismatch and short
read boundaries. Native uninitialized output bytes are neither compared nor
exposed. This is a focused decoder differential check, not a full-game test.

Hashes, source ownership, further validation and annotation records are in
`reports/font_runtime_startup.json`; raw captures and fixtures remain in `local/`.

## Follow-up packets

- Bind GUI page roots/children to real allocated scene nodes and recovered
  per-class property/visibility/load hooks before running00aa5e20 in the game.
- Bind settings ApplyAll to retained font reload00ac3610/00ad51d0, preserving
  the destructive reload and same-path check; do not substitute initial load.
- Compose global singleton lifetime and native renderer texture-cache ownership
  across startup owners before claiming the original allocator/refcount ABI.

The font registry destructor's three erroneous free call-site overrides were
cleared under the Ghidra write lock. Internal continuations are restored and the
tail is disassembled through RET00ac37c0, but the stored function body still ends
at00ac379a. Its supported script-based extension is disabled by the current
bridge configuration. No function was deleted/recreated; the byte-verified tail
and this remaining analysis limit are recorded in the flow-repair report.


## Integrated validation

Win32 Release and both existing math CTests pass. The rebuilt executable ran
60 frames at640x480 and exited0 with six fonts,12 actual retained font textures,
19 VFS resource opens including the fingerprint payload, and240 defined payload
bytes. A separate forced-unwind probe verifies that every font texture uses the
application renderer API and that all12 image owners and the window are released.
The original installed executable and personal options retain their hashes,
sizes and modification times. GUI render and gameplay remain unvalidated.

Sixteen reviewed names/comments were saved and verified,11 prior comment fields
preserved, and16 exports refreshed. Twenty-four worker artifacts were preserved
with hashes. The report identifies the tested source commit and29 selected source
hashes, separately from19 artifact hashes and the runtime executable hash.
