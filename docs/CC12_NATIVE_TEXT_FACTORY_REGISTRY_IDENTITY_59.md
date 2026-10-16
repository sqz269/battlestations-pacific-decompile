# Native Text factory registry identity 59

Source commit `7e8390fecd4bfe6c46e0a9145517263d5a27d738` requires a
font producer's bound Text factory to use its same stable registry, as well as
its same actual font resources. Packet 56's borrowed-existing profile supplies
the real already-owned FontHost registry without a replacement singleton
manager, new publication cell or ownership transfer.

`GuiTextRuntimeFactory::native_font_registry()` reads the factory's exact
font-name lookup profile. The inspector returns the borrowed registry or reads
the current legacy publication. It never calls the lazy getter or allocates,
publishes or registers a registry. A null legacy publication stays null.

`GameNativeFontApplication::bind_text_factory()` requires this identity to
equal its own registry before assigning the factory pointer. Existing ready,
export, resource identity, repeated-factory and empty-allocation checks remain.
The normal FontHost's existing packet-53 catch makes an admitted bind rejection
sticky, including a null or foreign registry. There is no retry/reset path.

Identity inspection and lifetime are externally serialized. The borrowed
registry, or legacy publication storage and real manager, must stay alive and
the lookup association must remain valid through retirement. A metadata check
does not make a mutable legacy publication immutable.

The canonical MSVC Win32 Release build passed, with all three existing CTests
passing in 8.62 seconds. The four packet-56 Source blobs remain unchanged.
Independent Source review by `native_font_teardown_review_36` passed at the
exact commit, including side-effect-free inspection, pre-assignment rejection
and unchanged host sticky-failure handling; the reviewer ran no extra tests.
No new tests, CMake entries, Original body recovery or Ghidra edits were needed.
The receipt records exact Source/object/build hashes and a checked archive.

This does not execute a successful real Text factory binding or its failure
path. Native Text remains inactive. The earlier normal startup/screenshot/
shutdown receipt is `reports/cc12_native_gui_combined_55.json`; it is not a new
runtime test of this registry admission change. Original ABI and gameplay
validation remain outstanding.
