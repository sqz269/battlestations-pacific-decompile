#pragma once
#include "bsp/native_string.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstdint>

namespace bsp {
// Borrow actual string-pool and CRT validation services. Production must use
// ActualNativeStringPoolStorage over the shared native manager/publications.
// No replacement tree, name table or copied registry is owned by this context.
struct NativeAnimationRegistryContext {
    NativeStringStorage& strings;
    const SingletonLifetimeCallbacks& invalid_parameters;
};

// Raw 20h registry: profile0, references4, tree opaque8/headC/count10,
// sample array data14/count18/capacity1C. Head nodes are1Ch, names C/10,
// assigned integer index14, color18 and sentinel19. Opaque/padding words stay
// untouched. Original ECX self; EAX self/RET for construction, RET for destroy.
void* construct_native_animation_registry_00b79a80(void* actual_registry);
void destroy_native_animation_registry_00b79b00(void* actual_registry,
    NativeAnimationRegistryContext&);
// B79BA0: ECXself,stackflags,EAXoriginalself,RET4. Bit0 frees after payload.
void* delete_native_animation_registry_00b79ba0(void* actual_registry,
    std::uint32_t flags, NativeAnimationRegistryContext&);

// B79740: ECXregistry,stack8h name header,EAXindex,RET4. Existing names return
// their stored index. Missing names capture current tree count before two
// temporary string copies, insert through the existing unique-tree mechanics,
// destroy both temporary strings in reverse order, return the captured index.
std::uint32_t register_native_animation_name_00b79740(void* actual_registry,
    const void* actual_name, NativeAnimationRegistryContext&);

// Complete producers of register calls. They reread the current item backing
// and signed count each iteration; the shared registry remains borrowed.
// B925D0: item+C contains18h records, each name starts at record+4; count+10.
// B8A330: item+8 contains pointers, count+C; B75D20 gives pointed track+8.
void register_native_compact_track_names_00b925d0(void* actual_item,
    void* actual_registry, NativeAnimationRegistryContext&);
void register_native_track_names_00b8a330(void* actual_item,
    void* actual_registry, NativeAnimationRegistryContext&);
const void* __fastcall native_animation_track_name_00b75d20(const void*) noexcept;

// Complete actual0Ch float-array reserve/resize helpers used by registry
// destruction. Original ECXheader/stack signed count/RET4; new source API.
// Reserve copies each current float through x87 (not a raw word copy), then
// frees old storage before data/capacity publication. Resize does not free.
void reserve_native_float_array_008153e0(void* header, std::int32_t capacity);
void resize_native_float_array_00818030(void* header, std::int32_t count);
} // namespace bsp
