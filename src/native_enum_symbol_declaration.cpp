#include "bsp/native_enum_symbol_declaration.hpp"
#include "bsp/native_enum_dictionary_lookup.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native enum symbol declaration requires MSVC Win32.
#endif
namespace bsp {
namespace {
static_assert(sizeof(void*)==4&&sizeof(NativeString)==8);
std::int32_t signed_word(const void* owner,std::uint32_t byte_offset) noexcept {
    std::int32_t result;
    __asm {
        mov eax,owner
        mov edx,byte_offset
        mov eax,dword ptr [eax+edx]
        mov result,eax
    }
    return result;
}
void put_word(void* owner,std::uint32_t byte_offset,std::int32_t value) noexcept {
    __asm {
        mov eax,owner
        mov edx,byte_offset
        mov ecx,value
        mov dword ptr [eax+edx],ecx
    }
}
void* map_receiver(void* owner) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uint32_t>(owner)+4u);
}
}

void declare_native_enum_symbol_008f2e40(void* owner,const char* symbol,
    std::int32_t value,NativeEnumSymbolDeclarationContext& context) {
    NativeString query;
    construct_native_string_header_0041e870(&query,context.actual_strings,symbol);
    void* const map=map_receiver(owner);
    std::uint32_t bucket;
    const bool duplicate=find_native_enum_symbol_node_0048d480(map,query,bucket,
        context.actual_enum_empty_00e186ed,context.actual_query_empty_00e17654)!=nullptr;
    // Native captures the lookup result before returning the CURRENT header.
    destroy_native_string_header_0041dd20(&query,context.actual_strings);
    if(duplicate) {
        // Complete Original004254B0 is the single byte C3. It dereferences no
        // diagnostic argument and has no ordinary effect; no stub is needed.
        return;
    }

    std::int32_t candidate=signed_word(owner,0x10c);
    if(value<candidate)candidate=value;
    // Original's second signed CMP precedes its first physical metadata store.
    const bool take_incoming=value>candidate;
    put_word(owner,0x10c,candidate);
    if(take_incoming)candidate=value;
    put_word(owner,0x110,candidate);

    // The first buffer is already returned. Reconstruct the same actual8h
    // header, then let the whole insertion provider own/copy the key normally.
    construct_native_string_header_0041e870(&query,context.actual_strings,symbol);
    insert_native_enum_symbol_word_008f2850(map,query,static_cast<std::uint32_t>(value),
        context.actual_initialized_symbol_pool_00e17578,context.actual_strings,
        context.actual_enum_empty_00e186ed,context.actual_query_empty_00e17654);
    destroy_native_string_header_0041dd20(&query,context.actual_strings);
}
} // namespace bsp
