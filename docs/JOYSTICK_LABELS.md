# Joystick binding labels

Addresses: 00A99710, 00425E10.

A99710 is joystick vtable +3C, a binding display-name getter. Its normal native
body is reconstructed in `joystick_labels.cpp`; +38 remains force output. The
original ABI takes this in ECX, output pointer and code on the stack, returns
the output in EAX and ends with RET8 at A998EF (three bytes).

The getter first constructs output from the empty string at CE3A0C, then reads
binding +29C+code*12. Kind0 returns that empty string. Otherwise it copies the
object name at +290+object_index*28, with the native self-copy guard, and reloads
binding kind after the storage calls. Kind3/5 appends `/Left`, 4/6 `/Right`,
7 `/Up`, and 8 `/Down`; other kinds retain the name. The suffix bytes are at
D5B830/D5B838/D5B848/D5B840. No localization or Xbox-specific label fallback is
performed by this function. Overwriting an already live output preserves the
original constructor's nonfreeing overwrite, including output/name aliasing.

A425E10 is the native string append helper (ECX destination, stack source,
RET4 at 425E45). It snapshots source length and old destination length, skips
an empty source, resizes preserving data, reloads both pointers and calls the
actual CRT memcpy. Self-append consequently reads the relocated source. The
Left branch of A99710 inlines the same operation but captures the temporary
suffix pointer before resizing the output. Neither operation adds separators.

These are new typed interfaces. Native invalid pointers, wrapped lengths and
invalid indices remain outside the supported domain; index guards run after
the output constructor. Native cleanup is established by FuncInfo DECEEC and
the five-entry unwind map DECF10. States1..4 destroy the suffix through CB6AA1,
CB6A99, CB6A89 or CB6A91, then transition to state0. CB6A70 tests/clears the
constructed-result bit and invokes 41DD20 on the caller-provided output.
The C++ cleanup preserves this order and actual-header destructor behavior:
storage is freed while the output length/pointer remain unchanged. The failed
result is not a live owning string and must not be freed again. This recovers
the cleanup actions, without reproducing compiler SEH metadata or frame ABI.
