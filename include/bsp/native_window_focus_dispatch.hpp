#pragma once
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native window focus dispatch requires MSVC Win32.
#endif

namespace bsp {
namespace game { class GameNativeReadOnlyData; }
struct BinkHandle;
using NativeWindowBinkPause = std::int32_t(__stdcall*)(BinkHandle*, std::int32_t);

// Borrow the actual supported PE table mapper and the shipped _BinkPause@8
// import. Tables remain numeric evidence; dispatch resolves reviewed source
// bodies, never treats original code addresses as callable host pointers.
struct NativeWindowFocusContext {
    const game::GameNativeReadOnlyData& actual_tables;
    NativeWindowBinkPause actual_bink_pause;
};

// AA33A0: ECX actual88h GUI manager; one full DWORD pause slot; RET4.
// Walk raw vector+18/+1C, capturing its end before each returning validation
// call. Each page invokes AA8E40; re-read bounds before advancing by four.
void pause_native_gui_movie_pages_00aa33a0(void* actual_gui,
    std::uint32_t pause, NativeWindowFocusContext& context);

// AA8E40: ECX actual widget; one DWORD pause slot; RET4. Walk circular list
// head+68/entry(next0,child8), read current virtual5C, pause type10's current
// decoder, then recurse for every child. Re-read node payload and sentinel
// after calls. No snapshot, null skipping or cycle suppression is added.
void pause_native_gui_movie_children_00aa8e40(void* actual_widget,
    std::uint32_t pause, NativeWindowFocusContext& context);

// AAC8A0: ECX widget, DWORD pause slot; null+F0 returns RET4, otherwise reload
// +F0 and tail-dispatch current decoder virtual14. Full source composition for
// native A4CB60 targets; unsupported virtual targets fail explicitly.
void pause_native_gui_movie_decoder_00aac8a0(void* actual_widget,
    std::uint32_t pause, NativeWindowFocusContext& context);

// A9E110: ECX widget, EAX raw+60, RET. All current5C calls in this module
// verify that their actual numeric target is this recovered leaf.
std::uint32_t read_native_gui_widget_type_00a9e110(void* actual_widget) noexcept;

// A4CB60: ECX decoder shared base (allocation+10), DWORD slot, RET4.
// Capture handle+4 before reading the low pause byte. With no handle, return.
// Byte zero captures CE3800 and writes+14 before _BinkPause@8(captured,byte).
// Preserve noncanonical bytes and ignore the import's return value.
void pause_native_movie_decoder_00a4cb60(void* actual_shared_base,
    std::uint32_t pause, NativeWindowFocusContext& context);

// A7A3F0: ECX actual sound manager, DWORD mask slot, RET4. Capture +8C array
// and +90 count/end once. For each raw entry, descriptor+44/class+8 selects
// (1u << (class &31)); a matching mask stores only byte+14=1.
void dirty_native_sound_classes_00a7a3f0(void* actual_sound_manager,
    std::uint32_t mask) noexcept;

// A7A480/A7A4A0: ECX actual sound manager; RET. Transition byte+50 first,
// then dirty mask0000FFFF; repeated pause/unpause does nothing. These consume
// an already-constructed raw sound manager; they do not create one.
void pause_native_sound_manager_00a7a480(void* actual_sound_manager) noexcept;
void resume_native_sound_manager_00a7a4a0(void* actual_sound_manager) noexcept;

// Actual storage and synchronous CRT returning-validation behavior are retained.
// New context parameters are not original register/FH3/SEH ABI. The raw GUI/
// sound producers and full message handler must still be integrated separately.
}
