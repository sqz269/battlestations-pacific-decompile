#include "bsp/native_mpak_offset_copy_storage.hpp"
#include "bsp/native_hardware_layout_tree_insert.hpp"
#include "bsp/native_legacy_sbo_string.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstdlib>
#include <cstring>
#include <cstdint>

namespace bsp {
namespace {
using Word=std::uint32_t;
static_assert(sizeof(void*)==4);
Word read(const void* p,Word o) noexcept {
    Word v;std::memcpy(&v,static_cast<const unsigned char*>(p)+o,4);return v;
}
void write(void* p,Word o,Word v) noexcept {
    std::memcpy(static_cast<unsigned char*>(p)+o,&v,4);
}
struct CompletedMessage {
    NativeLegacySboStringStorage& value;
    ~CompletedMessage() noexcept { native_legacy_sbo_string_destroy_004072d0(value); }
};
[[noreturn]] void too_long() {
    NativeLegacySboStringStorage text;
    text.capacity_18=15;text.length_14=0;text.buffer_04.inline_bytes[0]=0;
    native_legacy_sbo_string_assign_counted_00408720(text,"vector<T> too long",18);
    const CompletedMessage completed{text};throw NativeHardwareLayoutTreeLengthError{text};
}
} // namespace

void* NativeMpakOffsetCopyStorage::copy_00bb6180(void* dst,const void* src) {
    const Word first=read(src,4);
    const Word count=first ? static_cast<Word>(static_cast<std::int32_t>(read(src,8)-first)>>2) : 0;
    write(dst,4,0);write(dst,8,0);write(dst,12,0);
    if(!count)return dst;
    if(count>0x3fffffffu)too_long();
    const Word bytes=count*4;
    void* const backing=singleton_lifetime_allocate({SingletonAllocationKind::object,bytes,bytes});
    const Word fresh=reinterpret_cast<Word>(backing);
    write(dst,4,fresh);write(dst,8,fresh);write(dst,12,fresh+bytes);
    Word captured_end=read(src,8);
    if(read(src,4)>captured_end)_invalid_parameter_noinfo();
    const Word captured_begin=read(src,4);
    if(captured_begin>read(src,8))_invalid_parameter_noinfo();
    const Word current_destination=read(dst,4);
    const Word copy_bytes=static_cast<Word>(static_cast<std::int32_t>(captured_end-captured_begin)>>2)*4;
    if(copy_bytes) {
        // BB6180 ignores the CRT return code and publishes end regardless.
        (void)::memmove_s(reinterpret_cast<void*>(current_destination),copy_bytes,
            reinterpret_cast<const void*>(captured_begin),copy_bytes);
    }
    write(dst,8,current_destination+copy_bytes);
    return dst;
}
} // namespace bsp
