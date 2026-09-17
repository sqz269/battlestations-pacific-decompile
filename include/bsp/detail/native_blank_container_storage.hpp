#pragma once
#include <cstdint>
namespace bsp::detail {
// Shared raw storage contracts, not C++ container construction. Native leaves
// guard each wrapped address independently. A null allocator return still
// faults on the +4 store; it must not become a successful empty container.
template<class Allocate> void* allocate_blank_tree_storage(
    std::uint32_t bytes,std::uint32_t color,Allocate allocate){
    static_assert(sizeof(void*)==4);
    void* const p=allocate(bytes);const auto base=reinterpret_cast<std::uint32_t>(p);
    for(std::uint32_t offset=0;offset<=8;offset+=4)
        if(base+offset!=0)*reinterpret_cast<volatile std::uint32_t*>(base+offset)=0;
    *reinterpret_cast<volatile std::uint8_t*>(base+color)=1;
    *reinterpret_cast<volatile std::uint8_t*>(base+color+1)=0;
    return p;
}
template<class Allocate> void* allocate_self_linked_list_storage(
    std::uint32_t bytes,Allocate allocate){
    static_assert(sizeof(void*)==4);
    void* const p=allocate(bytes);const auto base=reinterpret_cast<std::uint32_t>(p);
    if(base!=0)*reinterpret_cast<volatile std::uint32_t*>(base)=base;
    if(base+4!=0)*reinterpret_cast<volatile std::uint32_t*>(base+4)=base;
    return p;
}
} // namespace bsp::detail
