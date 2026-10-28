#pragma once

#include "bsp/gui_lua_reader.hpp"
#include "bsp/native_lua_field_values.hpp"
#include <cstddef>
#include <cstdint>

namespace bsp {

// Views of actual table globals, not a replacement object registry. The native
// entries have stride10h and hold a 32-bit object pointer at+C. Read these aliases
// at each call; the two table owners still supply allocation/population/lifetime.
struct ObjectHandleTables {
    const std::int32_t& first_begin_00f89a0c;
    const std::int32_t& first_end_00f89a10;
    const void* const& first_entries_00f89a54;
    const std::int32_t& second_begin_00f89a60;
    const void* const& second_entries_00f89aa8;
};

// Native ECX=full input word, RET, EAX=object. Full-word zero is checked BEFORE
// truncating to16 bits. Signed split comparison, unchecked wrapping address
// arithmetic and raw entry+C read. The selected native address must be valid.
void* object_from_handle_006ad080(std::uint32_t, const ObjectHandleTables&) noexcept;

// Native ECX=object, RET; zero for null, otherwise zero-extended word at+174h.
std::uint16_t handle_from_object_006ad0c0(const void*) noexcept;

// Native ECX=LuaObject, tail-jumps to lua_touserdata using state+4/index+8.
void* lua_object_to_userdata_00b662d0(GuiLuaHost&, GuiLuaRef);
// Native ECX=table LuaObject, RET, EAX=user pointer. Lookup literal "Ptr", convert
// with lua_touserdata, destroy the temporary LuaObject, then return captured bits.
void* object_from_lua_table_00888aa0(GuiLuaHost&, GuiLuaRef);

using HandleToObjectCallback = void* (*)(std::uint32_t, const ObjectHandleTables&);
using ContextValueCallback = void* (*)(GuiLuaHost&, GuiLuaRef);
using ObjectToHandleCallback = std::uint16_t (*)(const void*);

// Typed C++ counterparts of0109CED4/CED8/CEDC. Function signatures add explicit
// native-state views; these are callable reconstruction interfaces, not nativeABI.
struct ObjectHandleResolverSlots {
    HandleToObjectCallback handle_to_object{};
    ContextValueCallback context_value{};
    ObjectToHandleCallback object_to_handle{};
};

// Native ECX/EDX/stack callbacks -> three unconditional global stores; RET4.
void set_object_handle_resolvers_00bd4fc0(ObjectHandleResolverSlots&,
    HandleToObjectCallback, ContextValueCallback, ObjectToHandleCallback) noexcept;
// Native cdecl RET; installs006AD080/00888AA0/006AD0C0 through00BD4FC0.
void install_object_handle_resolvers_006ad0d0(ObjectHandleResolverSlots&) noexcept;

// Connect the live reader to the current installed callbacks and actual table
// aliases. Evaluated GuiTable cannot represent native userdata/metatable lookup;
// that entry explicitly rejects this adapter instead of fabricating a handle.
class ObjectHandleReaderResolver final : public GuiLuaHandleResolver {
public:
    ObjectHandleReaderResolver(const ObjectHandleResolverSlots& slots,
        const ObjectHandleTables& tables) : slots_(slots), tables_(tables) {}
    std::int32_t resolve_by_number(std::int32_t) override;
    std::int32_t resolve_by_table(const GuiTable*) override;
    std::int32_t resolve_by_live_table(GuiLuaHost&, GuiLuaRef) override;
private:
    const ObjectHandleResolverSlots& slots_;
    const ObjectHandleTables& tables_;
};

class NativeLuaFieldResolverAdapter;
// Persistent per-field invocation backing. It is host metadata plus the SAME
// 14h temporary that the admitted raw 00888AA0 path constructs and destroys.
// Retain the frame, adapter, raw input/reader/Lua and table providers on failure;
// never destroy/replay an entered or failed frame. Complete frames may be reused.
struct NativeLuaFieldResolverFrame final {
    enum class Phase { fresh, entered, complete, failed };
    NativeLuaObjectStorage temporary{};
    Phase phase{Phase::fresh};
    std::uint32_t active_site{}, result_bits{};
    HandleToObjectCallback loaded_number{};
    ContextValueCallback loaded_table{};
    NativeLuaFieldResolverFrame() = default;
    ~NativeLuaFieldResolverFrame() noexcept;
    NativeLuaFieldResolverFrame(const NativeLuaFieldResolverFrame&) = delete;
    NativeLuaFieldResolverFrame& operator=(const NativeLuaFieldResolverFrame&) = delete;
private:
    friend class NativeLuaFieldResolverAdapter;
    NativeLuaFieldResolverAdapter* binding_{};
    NativeLuaFieldResolverFrame* previous_{};
    bool active_{};
};

// Qualified source ABI bridge for the actual-storage BD63B0 field reader.
// The source slots remain owned/installed by the startup host (006AD0D0).
// Every invocation reads their CURRENT typed target; no callback cast or
// cached source target is used. Tables are aliases of the canonical live banks.
// The table arm admits only the installed object_from_lua_table_00888aa0 target
// and composes its genuine raw B67800 -> B662D0 -> B67700 schedule, without a
// GuiLuaRef/registry projection. Other table targets are outside this domain.
//
// The distinct fixed fastcall cells below are application adapters, not writes
// to0109CED4/CED8, a BD4FC0 replacement, or drop-in binary/game ABI proof.
// Serialize entries, activation and lifetime; providers outlive every scope.
class NativeLuaFieldResolverAdapter final {
public:
    NativeLuaFieldResolverAdapter(const ObjectHandleResolverSlots&, ObjectHandleTables,
        const char* actual_ptr_literal_00cfad08);
    ~NativeLuaFieldResolverAdapter() noexcept;
    NativeLuaFieldResolverAdapter(const NativeLuaFieldResolverAdapter&) = delete;
    NativeLuaFieldResolverAdapter& operator=(const NativeLuaFieldResolverAdapter&) = delete;

    NativeLuaFieldNumberResolver const volatile& number_cell() const noexcept { return number_cell_; }
    NativeLuaFieldTableResolver const volatile& table_cell() const noexcept { return table_cell_; }
    bool requires_process_retention() const noexcept;

    // Nesting (including another adapter) restores the previous binding on
    // normal return/C++ unwind. TLS points at the caller-retained FRAME, not
    // this automatic scope object. A Lua nonlocal transfer can bypass this
    // destructor: the active frame/count remain retained, require _Exit and
    // must never be resumed or silently repaired. Entered failures are sticky.
    class Activation final {
    public:
        Activation(NativeLuaFieldResolverAdapter&, NativeLuaFieldResolverFrame&);
        ~Activation() noexcept;
        Activation(const Activation&) = delete;
        Activation& operator=(const Activation&) = delete;
    private:
        NativeLuaFieldResolverFrame& frame_;
    };
private:
    static NativeLuaFieldResolverFrame& active();
    static std::uint32_t __fastcall resolve_number(std::int32_t);
    static std::uint32_t __fastcall resolve_table(NativeLuaObjectStorage*);
    static void fail(NativeLuaFieldResolverFrame&) noexcept;
    const volatile ObjectHandleResolverSlots& slots_;
    ObjectHandleTables tables_;
    const char* ptr_literal_;
    NativeLuaFieldNumberResolver const volatile number_cell_{&resolve_number};
    NativeLuaFieldTableResolver const volatile table_cell_{&resolve_table};
    std::size_t active_scopes_{};
    bool failed_{};
};

} // namespace bsp
