#pragma once
#include <cstdint>

namespace bsp {
namespace game { class GameNativeTextureAtlasApplication; }
// Typed borrowed Source service. Only the retained atlas process owner supplies
// the known CFEAE8/737F10 implementation over its private normal actual1Ch birth.
// No Native vtable, erased callback, default provider or arbitrary vcall adapter.
class NativeTextureAtlasDeletionLifetime {
public:
    NativeTextureAtlasDeletionLifetime(const NativeTextureAtlasDeletionLifetime&) = delete;
    NativeTextureAtlasDeletionLifetime& operator=(const NativeTextureAtlasDeletionLifetime&) = delete;
    virtual void* delete_registered_00737f10(void* captured_receiver,
        std::uint32_t full_flags) = 0;
private:
    friend class game::GameNativeTextureAtlasApplication;
    NativeTextureAtlasDeletionLifetime() = default;
    ~NativeTextureAtlasDeletionLifetime() = default;
};
} // namespace bsp
