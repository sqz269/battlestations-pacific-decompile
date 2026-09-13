# Native keyboard storage provider (AX)

`NativeInputKeyboardStorage` implements all fourteen operations required by
`NativeInputKeyboardLibrary` using reconstructed source. The consumer interface
remains; its borrowed `NativeStringStorage` must outlive the provider, containers
and consuming contexts. The provider calls no original code.

| Operations | Source module | Native entries |
| --- | --- | --- |
| Device map index | native_input_device_index | 0055C110 |
| Word, descriptor and packed-bit vector map index | native_input_vector_map_index | 006A44B0, 006A45C0, 006A4CA0 |
| Populated scale copy/destruction, scalar-tree and float index | native_input_scale_maps | 0055B400, 0055B490, 006A5AA0, 00444BE0 |
| Integer-set find, three tree increments, bit advance | native_input_keyboard_iterators | 00546840, 00552770, 00552D40, 005540C0, 0048D3B0 |
| Scale-map range erase | native_input_settings_tree_cleanup | 0055B230 |

Device insertion reproduces seven independent empty heads at each native
default/pair/node lifetime, interleaved with vectors in producer order. Vector
indexing leaves opaque destination words untouched. These roots only reach
fresh empty mapped-value copies; general populated device/vector copy is outside
the claim. Scale copying covers populated outer and inner trees, native
left-before-right allocation order, colors, keys, scalar bits and independent
heads. The corrected 0055B490 destructor clears its header after freeing the head.

The strict Win32 component build and both existing CTests pass. Copied-native
fixtures exercise the actual settings-table constructor and lifetime paths with
the concrete source provider: 122,545 lifetime, 309,711 range-erasure, 168,378
insertion, 1,448 vector-storage and 335 checked-string values match. Keyboard apply
matches 372 values across finite, NaN and retained sensitivity-category scenarios,
with 30 native resizes and 30 native rebinds. All 9,409 valid bit-position pairs
match, and the zero-distance bypass passes. Fixtures leave no pooled strings and
perform no device polling or game calls. The publication report pins the final
combined revision and artifact hashes; component results alone do not validate
the published source.

Integration independently checks 40 live/disk-equal envelopes and 241 owned
direct CALL sites, including external/shared callees. Five root bodies and the
partial scale eraser required saved-body repairs; 00552E20 required an iterator
ABI correction. Annotation receipts preserve old names, prototypes and comments.
Descriptive STL names remain hypotheses; established library names are retained.

The executable host still needs raw Lua/VFS services, settings-singleton
publication/deletion, actual game/configuration ownership and callback wiring;
see `NATIVE_INPUT_HOST_INTEGRATION_GAP_AX.md`. Fixture results establish bounded
source behavior, not executable reachability, original exception/CRT ABI
compatibility or gameplay. The original installation and single-instance state
were preserved.

The independent populated scale-tree fixture also matched 900 values across six
partial/full/empty erasures and a populated copy. Its native destructor was not
in the retained AQ capture; keyboard apply separately exercises the native
scale destructor. The final combined build and all three fixture runs pass.

`NATIVE_INPUT_PREIMAGE_PROVENANCE_AY.md` distinguishes independent native flag
stack cells from the shared bounded source seed policy. Low-byte initialization
does not establish zero high bytes or identity between native stack frames.
