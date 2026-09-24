#pragma once

#include <cstdint>

namespace bsp {
struct NativePlatformFocusOwnersContext;
struct NativeStringRawPoolContext;

// One operation for each native page virtual call. The provider must dispatch
// the CURRENT profile of the supplied actual page. D5BE38 selects AA8320 at
// +20 (scene-node release) and AA38F0 at +04 (scalar deletion). A combined host
// tree-retirement method cannot substitute for both calls: the page pointer
// and vector bounds are reloaded between them. There are no default providers.
struct NativeGuiPageLifetimeHost {
    virtual ~NativeGuiPageLifetimeHost() = default;
    virtual void call_current_page_virtual_20(void* actual_page) = 0;
    virtual void call_current_page_virtual_04(void* actual_page,
        std::uint32_t flags) = 0;
};

struct NativeGuiMediaFocusLifetimeContext {
    NativePlatformFocusOwnersContext& owners;
    NativeStringRawPoolContext& strings;
    NativeGuiPageLifetimeHost& pages;
};

// Complete A4C510[72]: ECX list header (unused0/head4/count8), RET. Detach
// links/count before freeing the captured chain; compare against current head
// after each free; free current sentinel then clear head. Payload is untouched.
void destroy_native_media_list_00a4c510(void* actual_header) noexcept;
// Complete A4C600[29]: ECX18h owner, RET. Destroy owner+4 list, clear CURRENT
// F8AEF8, then reset ORIGINAL owner profile to CE3818. No unregister operation.
void destroy_native_media_manager_00a4c600(void* actual_owner,
    NativePlatformFocusOwnersContext&) noexcept;
// Complete A4C620[49]: ECX owner, DWORD flags stack slot, RET4/EAX original
// receiver even when freed. Only bit0 selects free after nondeleting cleanup.
void* scalar_delete_native_media_manager_00a4c620(void* actual_owner,
    std::uint32_t flags, NativePlatformFocusOwnersContext&) noexcept;

// Complete AA51D0[83]: ECX header(unused0/head4/count8), RET. Each node has
// next0/previous4/string length8/dataC. Capture next before returning its string
// via actual00419CC0/BD1510 and freeing the node. The sentinel remains live.
void clear_native_gui_string_list_00aa51d0(void* actual_header,
    NativeStringRawPoolContext&);
// Complete AA5B20[29]: clear above, free current sentinel, then clear head.
void destroy_native_gui_string_list_00aa5b20(void* actual_header,
    NativeStringRawPoolContext&);

// Partial domain: the entire AA6340[354] manager schedule is represented, but
// populated page virtual20/04 bodies remain required external providers.
// Preserve returning CRT validation, captured cursor/end, reloads after page
// calls, +28/+2C canonical releases and list/vector/tree/base ordering. Source
// C++ unwinding follows the five inspected FH3 states; this is not native FH3,
// hardware-exception or binary ABI equivalence.
void destroy_native_gui_manager_00aa6340_fragment(void* actual_owner,
    NativeGuiMediaFocusLifetimeContext&);
// Partial domain inherited from AA6340; complete AA6540[30] wrapper schedule.
// ECX owner, DWORD flags, RET4/EAX original receiver; bit0 chooses final free.
void* scalar_delete_native_gui_manager_00aa6540_fragment(void* actual_owner,
    std::uint32_t flags, NativeGuiMediaFocusLifetimeContext&);
} // namespace bsp
