# Actual raw wide-string first-of

The new ordinary Core API covers A9EF60 over actual raw8 haystack/set headers. It searches for any set character through the already qualified raw C0382C _wcscspn boundary; it is not substring/wcsstr, a semantic string object, a new CRT body or a typed wchar view.

Native read order is preserved with fenced byte copies: set DATA+4 FIRST, then hay DATA+4; NULL hay data returns before hay length. With hay data, capture length; zero returns before the set-data NULL test. Set length is never read. After those guards signed-negative start becomes0; otherwise unsigned start>captured length returns-1 while equality still calls CRT.

Form DWORD-wrapped hayData+2*effectiveStart and call real count_native_crt_wide_prefix_excluding_00c0382c once using captured set data. Preserve the provider's exact native SUB/SAR bits, DWORD-add start, THEN read CURRENT hay length. Only equality returns-1; all other DWORD result bits are copied to int32 without an implementation-defined unsigned cast. No >= test, captured-length comparison, source-NUL result interpretation, content/empty-set check or bounds repair is added.

This leaf has no Frame/context/pool/owner/temporary, no writes, selected cleanup, input transfer or automatic destruction. Existing default-potentially-throwing raw CRT qualification remains; no noexcept/error conversion is introduced. Ordinary Source signature replaces ECX/stack/EAX/RET8 explicitly, not the Original physical ABI/FH3/hardware contract.

Caller supplies every genuinely reached raw header field and underlying WORD/control allocation/stop/alias domain under the existing rawCRT header contract. Unaligned/interior/RO/overlapping raw inputs are admitted by that provider; every reached byte/terminal advance still needs its real lifetime/extent, no fault/MMIO/concurrency/private-frame protocol. Outer DWORD offset wrap must reach valid admitted storage. Invalidating writers are excluded. Logical length/NUL inconsistencies and overflow are not normalized into semantic not-found behavior.

No fixed marker/literal belongs in this API. The resolver's CF828C-built set, raw header construction/cleanup and full marker loop remain caller work; a no-marker/empty success shortcut is not granted. Existing wide/typed cc13_seg87 and raw CRT sources remain byte-identical; the old typed helper is not cast over arbitrary raw8.

Evidence: all32 saved Native instructions, frozen b6e52065 contract, actual public rawCRT API and five preserved wide/CRT/typed files. Root and independent actual Source review precede the single strict isolated Win32 object; symbol evidence is recorded separately. No new test, Native entry/probe executable/game, Ghidra/shared metadata/CMake edit, resolver/loading/startup/Font48/ABI/runtime or Native/vendor-body credit belongs to this packet. Root owns registration, coupled checks and later caller admission.
