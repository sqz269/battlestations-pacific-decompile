# Native unit lifecycle services

Addresses: 00803BA0, 004BCA80. Read-only provider context: 00B0D7B0, 00B4EC90.

`mark_native_recon_slot_dirty_00803ba0` reconstructs the complete 56-byte normal body. It resolves the actual recon table entry and writes its existing +25 byte only when that entry is nonnull. It then captures the current world from E188A8, reads its signed local-player index, and accepts only 0 through 7. The same captured world's player record supplies +28. A matching recon index clears that world's +193C byte. A null recon slot still performs this world check; an in-range null player pointer has no native recovery branch.

The source borrows field lvalues through required side-effect-free mappings. Existing recon-slot and world/player offset evidence is reused; no native object layout, private slot table, source world or shadow flag is introduced. Native ECX is the recon index and is not bounds-checked against the three normal contexts. Callers must provide readable actual index storage.

`publish_native_controlled_listener_004bca80` reconstructs the complete 19-byte normal body. It stores the incoming ECX handle in the actual E188DC publication, then loads F8D39C and invokes the required complete B0D7B0 provider on that captured renderer. The handle is one stack argument and B0D7B0 returns with RET4. No work follows that update, and no null check or rollback is invented.

The complete 25-byte B0D7B0 listing writes renderer+1C0, reads its current +30, and conditionally calls B4EC90. The latter marks +250 and repeatedly propagates root registration from its current node list. Those renderer/node operations remain an explicit production boundary in this packet. New C++ view/provider interfaces are not native ABI replacements.

One focused fixture compares six source/original-byte scenarios. Recon cases cover both signed out-of-range player bounds, context mismatch, and null-slot context match. Publication cases cover a missing renderer child and a null-handle update whose callback replaces both global publications. Both sides execute the original B0D7B0 body with an explicit B4EC90 capture boundary, preserving the update's later mutations. Native and source recon layouts differ, and surrounding storage is checked for unwanted writes.

All 75 owned bytes and 25 provider bytes match live Ghidra and the installed PE. Five owned relocation operands and the provider's conditional call relocation are retained. Each side consumed 29,048 identical serialized input bytes and produced 88 identical output bytes. The strict Win32 fixture passes; combined CMake/CTest validation is recorded separately by the W integration report. Initial compiler response-file syntax failure is retained. Complete renderer behavior, actual application bindings, native exceptions/concurrency and gameplay remain unproved.
