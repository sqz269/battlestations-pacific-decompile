#pragma once
#include "bsp/native_lua_reader_support.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>

namespace bsp {

// A raw 14h Lua object whose address is registered in NativeLuaStateStorage.
// Copying registers the new address before the old object's destruction.
class NativeLuaReaderValue {
public:
    explicit NativeLuaReaderValue(const NativeLuaObjectStorage& source) noexcept;
    NativeLuaReaderValue(const NativeLuaReaderValue& source) noexcept;
    ~NativeLuaReaderValue();
    NativeLuaReaderValue& operator=(const NativeLuaReaderValue&) = delete;

    NativeLuaObjectStorage object;
};
static_assert(sizeof(NativeLuaReaderValue) == 0x14);
static_assert(sizeof(std::vector<NativeLuaReaderValue>) == 0x0c);

// Physical reader layout for the successful raw-reader domain. The four bytes
// at +4 retain their caller-provided preimage; neither native constructor nor
// this constructor initializes them. This is a C++ interface, not an ABI hook.
class NativeLuaReaderStorage {
public:
    // root is the caller-owned, by-value native constructor argument. It is
    // consumed and destroyed after the first vector copy, including on throw.
    explicit NativeLuaReaderStorage(NativeLuaObjectStorage& root);
    ~NativeLuaReaderStorage();
    NativeLuaReaderStorage(const NativeLuaReaderStorage&) = delete;
    NativeLuaReaderStorage& operator=(const NativeLuaReaderStorage&) = delete;

    // scratch must be caller-owned and initialized as an unbound raw object.
    // Its opaque DWORD and padding are retained through the BD5790 lookup.
    void enter_key_00bd8e20(NativeLuaObjectStorage& scratch,
        std::uint32_t key_kind, std::uint32_t key_bits);
    void push_copy_00442220(const NativeLuaObjectStorage& source);
    void pop_back_00bd7130();
    // Borrow only while nonempty and before a push/pop/destruction invalidates
    // the vector element. No copy or new owner reference is made.
    NativeLuaObjectStorage& current_raw();
    const NativeLuaObjectStorage& current_raw() const;
    const std::vector<NativeLuaReaderValue>& values() const noexcept { return objects; }

    volatile std::uint32_t profile_00;
    std::byte proxy_04[4];
private:
    union { std::vector<NativeLuaReaderValue> objects; };
};
static_assert(sizeof(NativeLuaReaderStorage) == 0x14);
static_assert(offsetof(NativeLuaReaderStorage, profile_00) == 0);
static_assert(offsetof(NativeLuaReaderStorage, proxy_04) == 4);

// Scalar deleting wrapper's bit0 free path is supported only for a reader
// allocated with the matching global operator new.
void destroy_native_lua_reader_00441a70(NativeLuaReaderStorage& reader,
    std::uint32_t flags);

} // namespace bsp
