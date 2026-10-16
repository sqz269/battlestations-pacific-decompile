# CRT comparison parent integration

The three recovered comparison parents are registered exactly once in
`bsp_core` at **`a856042b09dd9a389a0b2bfbed1616125f067915`**:

| Original library routine | Source packet | Whole body | Controlled-service cases |
| --- | --- | --- | --- |
| `__stricmp`, BF7FBF | [46](CC12_NATIVE_CRT_STRICMP_PARENT_46.md), `794d8cbb4` | 80 bytes / 32 instructions | 4 |
| `__wcsicmp_l`, C0392A | [48](CC12_NATIVE_CRT_WCSICMP_LOCALE_48.md), `eda6c52d4` | 271 bytes / 105 instructions | 9 |
| `__wcsicmp`, C03A39 | [49](CC12_NATIVE_CRT_WCSICMP_PARENT_49.md), `10e71bbf9` | 145 bytes / 56 instructions | 6 |

Primary Source review found no blocking issue in their dispatch, unsigned
byte/WORD behavior, errno/handler order, locale-view identity, or conditional
normal-return cleanup. Final committed files match the reviewed worker blobs.
The wide locale view/update additionally assert their standard-layout sizes,
offsets, and offset-zero first-member relation. Compatible C++ object/alias
contracts remain required; a matching physical layout is not permission to
cast unrelated CRT objects.

The canonical MSVC Win32 build compiles all three new translation units and
passes **3/3 existing suites, 10.05 s**. Generated project entries and compiled
objects were checked. Worker baseline builds did not yet register the new
parents; each worker separately compiled and exercised its owned Source in a
manifested Win32 probe. This combined build closes that registration gap.

The 19 focused cases execute the complete relocated Original **parent** bodies
with only the audited external operands/calls redirected. They check the
parent behavior over controlled services, including errno **0x16 = 22** before
five-zero handler delivery, null-locale forwarding, full unsigned-WORD
differences, callback read order, and acquired-locale cleanup. These are not
tests of the complete Original CRT subtree, current TLS/locale ownership,
fault/unwind behavior, Original entry ABI, or production native Text drawing.
The Source APIs have explicit additional services/cell arguments. Correct
library names were preserved in Ghidra; comments/history were saved and
affected exports refreshed.

Production comparison still needs an honest live provider. The Original GUI
caller ABA945 calls two-argument `__wcsicmp`; its changed-locale arm supplies
the third null locale to `__wcsicmp_l`. The existing two-argument Source callback
does not establish the latter's Original ABI. The existing sprite frontend is
unchanged by this packet, and no additional game run was needed for these
unactivated parents. The latest bounded startup/shutdown evidence remains
[combined group integration 43](CC12_NATIVE_GUI_RAW_GROUP_INTEGRATION_43.md),
compiled from its separately recorded commit.

The [machine receipt](../reports/cc12_native_gui_crt_parent_integration_50.json)
records the frozen Source/build/object inputs and their verified archive.
