#include "bsp/native_lua_reader.hpp"
#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native Lua reader reconstruction requires MSVC Win32.
#endif

namespace bsp {

NativeLuaReaderValue::NativeLuaReaderValue(const NativeLuaObjectStorage& source) noexcept {
    copy_construct_native_lua_object_00b66fa0(&object, source);
}

NativeLuaReaderValue::NativeLuaReaderValue(const NativeLuaReaderValue& source) noexcept {
    copy_construct_native_lua_object_00b66fa0(&object, source.object);
}

NativeLuaReaderValue::~NativeLuaReaderValue() {
    destroy_native_lua_object_00b67700(object);
}

NativeLuaReaderStorage::NativeLuaReaderStorage(NativeLuaObjectStorage& root)
    : profile_00(0x00ce44fc) {
    ::new (static_cast<void*>(&objects)) std::vector<NativeLuaReaderValue>();
    try {
        push_copy_00442220(root);
    } catch (...) {
        objects.~vector();
        destroy_native_lua_object_00b67700(root);
        throw;
    }
    destroy_native_lua_object_00b67700(root);
}

NativeLuaReaderStorage::~NativeLuaReaderStorage() {
    profile_00 = 0x00ce44fc;
    objects.~vector();
    profile_00 = 0x00ce374c;
}

void NativeLuaReaderStorage::enter_key_00bd8e20(NativeLuaObjectStorage& scratch,
    std::uint32_t key_kind, std::uint32_t key_bits) {
    if (objects.empty()) throw std::out_of_range("native Lua reader has no parent");
    lookup_native_lua_reader_child_00bd5790(&scratch,
        objects.back().object, key_kind, key_bits);
    try {
        push_copy_00442220(scratch);
    } catch (...) {
        destroy_native_lua_object_00b67700(scratch);
        throw;
    }
    destroy_native_lua_object_00b67700(scratch);
}

void NativeLuaReaderStorage::pop_back_00bd7130() {
    if (!objects.empty()) objects.pop_back();
}

NativeLuaObjectStorage& NativeLuaReaderStorage::current_raw() {
    if (objects.empty()) throw std::out_of_range("native Lua reader has no current object");
    return objects.back().object;
}

const NativeLuaObjectStorage& NativeLuaReaderStorage::current_raw() const {
    if (objects.empty()) throw std::out_of_range("native Lua reader has no current object");
    return objects.back().object;
}

void NativeLuaReaderStorage::push_copy_00442220(const NativeLuaObjectStorage& source) {
    objects.emplace_back(source);
}

void destroy_native_lua_reader_00441a70(NativeLuaReaderStorage& reader,
    std::uint32_t flags) {
    reader.~NativeLuaReaderStorage();
    if (flags & 1u) ::operator delete(&reader);
}

} // namespace bsp
