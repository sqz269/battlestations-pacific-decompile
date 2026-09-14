# Native FileStore provider enumeration (BF)

`00D689E8` is the FileStore provider table. Its `+14` slot at `00D689FC`
contains `00BE6480` in both the installed PE and the saved Ghidra memory.
The complete `00BE6480..00BE654C` function is 205 bytes, SHA-256
`ebe69187dc5e615e57365b8e5037d35ce1c9340370fb2de2cedc4979782a9c36`.
All 77 decoded instructions belong to the saved body; seven direct CALL sites
are in the report and passed the call verifier. Proposed source name:
`enumerate_native_filestore_names_00be6480`.

The original ABI is ECX actual FileStore provider, then stack directory,
extension, flags and output vector, RET10h. The source accepts the same raw
objects plus borrowed string storage and invalid-parameter callbacks. The
flags word is never loaded by the native body. It reads only the provider's
primary resident tree at `+14`: `+18` is the head; its left link is the first
node. Each 1Ch node holds a counted name at `+C/+10`. The existing `BE4C30`
iterator advances in tree order; the existing `4CDC20` vector helper appends
an independent pooled copy of the complete name. No stream/payload is read.

For each name, the native method first requires nonnull name and directory
data pointers. The first case-sensitive `strstr(name,directory)` must point to
the start of the name. The extension search uses the first `strstr` match,
or `FFFFFFFF` if either pointer is null or no match exists. That index is
compared to the wrapping DWORD difference `name.length-extension.length`.
This deliberately preserves the absent-extension collision when extension
length is exactly name length plus one. An empty but nonnull directory matches;
a null directory does not. Neither path normalization, separator boundary,
case folding nor recursive flag filtering occurs here.

The provider node and all input headers are borrowed. The output vector owns
each appended string through the existing actual pool service. This is a
source interface, not original binary/EH ABI proof. It assumes an initialized
actual FileStore resident tree and retained pool/callback lifetimes. No game
startup or provider-visitor runtime fixture has been run. Primary owns CMake
registration and combined archive validation.
