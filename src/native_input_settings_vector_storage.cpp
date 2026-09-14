#include "bsp/native_input_settings_vector_storage.hpp"
#include "bsp/global_config.hpp"
#include "bsp/native_singleton_vector_allocation.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstdlib>
#include <cstring>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
enum class Kind { word, descriptor, string, words, strings, pairs };
Word width(Kind kind) noexcept {
    switch (kind) {
    case Kind::word: return 4;
    case Kind::descriptor: return 20;
    case Kind::string: return 8;
    default: return 16;
    }
}
Kind child(Kind kind) noexcept {
    switch (kind) {
    case Kind::words: return Kind::word;
    case Kind::strings: return Kind::string;
    default: return Kind::strings;
    }
}
Word address(const void* p) noexcept { return reinterpret_cast<Word>(p); }
void* pointer(Word p) noexcept { return reinterpret_cast<void*>(p); }
void* at(void* p, Word n) noexcept { return static_cast<unsigned char*>(p) + n; }
Word read(const void* p, Word n) noexcept {
    Word value; std::memcpy(&value, static_cast<const unsigned char*>(p) + n, 4); return value;
}
void write(void* p, Word n, Word value) noexcept { std::memcpy(at(p,n), &value, 4); }
Word distance(Word first, Word last, Kind kind) noexcept {
    const auto bytes = static_cast<std::int32_t>(last - first);
    return static_cast<Word>(bytes / static_cast<std::int32_t>(width(kind)));
}
Word size(const void* header, Kind kind) noexcept {
    const Word first = read(header,4); return first ? distance(first,read(header,8),kind) : 0;
}
void construct(Kind, void*, const void*, NativeStringStorage*);
void destroy(Kind, void*, NativeStringStorage*) noexcept;
void destroy_range(Kind kind, void* first, void* last, NativeStringStorage* strings) noexcept {
    for (Word p=address(first), end=address(last); p!=end; p+=width(kind)) destroy(kind,pointer(p),strings);
}
void copy_vector(Kind kind, void* destination, const void* source, NativeStringStorage* strings) {
    const Word count=size(source,kind);
    write(destination,4,0);write(destination,8,0);write(destination,12,0);
    if (!count) return;
    if (count > 0xffffffffu / width(kind)) native_singleton_length_error_00bd0590();
    const Word bytes=count*width(kind);
    void* const fresh=singleton_lifetime_allocate({SingletonAllocationKind::object,bytes,bytes});
    write(destination,4,address(fresh));write(destination,8,address(fresh));write(destination,12,address(fresh)+bytes);
    if (read(source,8)<read(source,4)) _invalid_parameter_noinfo();
    const Word first=read(source,4), end=read(source,8);
    if (end<first) _invalid_parameter_noinfo();
    Word completed=address(fresh);
    try {
        for (Word p=first;p!=end;p+=width(kind)) {
            construct(kind,pointer(completed),pointer(p),strings);completed+=width(kind);
        }
    } catch (...) {
        destroy_range(kind,fresh,pointer(completed),strings);singleton_lifetime_free(fresh);throw;
    }
    write(destination,8,completed);
}
void construct(Kind kind, void* destination, const void* source, NativeStringStorage* strings) {
    if (kind==Kind::word || kind==Kind::descriptor) {
        if (destination) std::memcpy(destination,source,width(kind));
    } else if (kind==Kind::string) {
        if (!destination) return;
        const bool same=destination==source;
        write(destination,0,0);write(destination,4,0);
        if (!same) copy_native_string_header_00be0a30_fragment(destination,*strings,source);
    } else copy_vector(child(kind),destination,source,strings);
}
void relocate(Kind kind, void* destination, void* source, NativeStringStorage* strings) {
    if (kind==Kind::words || kind==Kind::strings || kind==Kind::pairs) {
        // 006A2310/006A3200/006A6D20 construct an empty header, then swap
        // begin, end and capacity individually. Existing nested buffers retain
        // both their identity and spare capacity; the old element becomes empty.
        write(destination,4,0);write(destination,8,0);write(destination,12,0);
        for (Word offset=4;offset<=12;offset+=4) {
            const Word previous=read(destination,offset);
            write(destination,offset,read(source,offset));write(source,offset,previous);
        }
    } else construct(kind,destination,source,strings);
}
void destroy(Kind kind, void* value, NativeStringStorage* strings) noexcept {
    if (kind==Kind::word || kind==Kind::descriptor) return;
    if (kind==Kind::string) { destroy_native_string_header_0041dd20(value,*strings);return; }
    const Word first=read(value,4);
    if (first) {
        destroy_range(child(kind),pointer(first),pointer(read(value,8)),strings);
        singleton_lifetime_free(pointer(read(value,4)));
    }
    write(value,4,0);write(value,8,0);write(value,12,0);
}
struct OwnedValue {
    Kind kind;
    void* value;
    NativeStringStorage* strings;
    ~OwnedValue() noexcept { destroy(kind,value,strings); }
};
void resize(Kind kind, void* header, Word requested, const void* value, NativeStringStorage* strings,
    NativeCheckedDwordPublication publication = NativeCheckedDwordPublication::begin_capacity_end) {
    const Word old_size=size(header,kind);
    if (requested==old_size) return;
    const Word captured_end=read(header,8);
    if (captured_end<read(header,4)) _invalid_parameter_noinfo();
    if (requested<old_size) {
        const Word first=read(header,4);
        if (read(header,8)<first) _invalid_parameter_noinfo();
        const Word new_end=first+requested*width(kind);
        if (read(header,8)<new_end || new_end<read(header,4)) _invalid_parameter_noinfo();
        if (!header) _invalid_parameter_noinfo();
        destroy_range(kind,pointer(new_end),pointer(captured_end),strings);
        write(header,8,new_end);return;
    }
    // Original count-insertion copies the value even when spare capacity
    // suffices. The incoming wrapper value remains independently owned.
    alignas(4) unsigned char saved[20];
    construct(kind,saved,value,strings);
    OwnedValue temporary{kind,saved,strings};
    const Word maximum=0xffffffffu/width(kind);
    const Word added=requested-old_size;
    if (maximum-old_size<added) native_singleton_length_error_00bd0590();
    const Word first=read(header,4);
    const Word capacity=first ? distance(first,read(header,12),kind) : 0;
    if (capacity>=requested) {
        Word completed=read(header,8);
        const Word new_first=completed;
        try {
            for (Word n=0;n<added;++n) { construct(kind,pointer(completed),saved,strings);completed+=width(kind); }
        } catch (...) { destroy_range(kind,pointer(new_first),pointer(completed),strings);throw; }
        write(header,8,read(header,8)+added*width(kind));return;
    }
    Word grown=capacity>maximum-(capacity>>1) ? 0 : capacity+(capacity>>1);
    if (grown<requested) grown=requested;
    const Word bytes=grown*width(kind);
    void* const fresh=singleton_lifetime_allocate({SingletonAllocationKind::object,bytes,bytes});
    Word completed=address(fresh);
    try {
        for (Word p=read(header,4);p!=captured_end;p+=width(kind)) {
            relocate(kind,pointer(completed),pointer(p),strings);completed+=width(kind);
        }
        for (Word n=0;n<added;++n) { construct(kind,pointer(completed),saved,strings);completed+=width(kind); }
    } catch (...) { destroy_range(kind,fresh,pointer(completed),strings);singleton_lifetime_free(fresh);throw; }
    const Word final_size=size(header,kind)+added;
    const Word old_first=read(header,4);
    if (old_first) {
        destroy_range(kind,pointer(old_first),pointer(read(header,8)),strings);
        singleton_lifetime_free(pointer(read(header,4)));
    }
    // The scalar DWORD instantiations publish begin first; descriptors and
    // owning nested/string instantiations publish it after capacity and end.
    const bool begin_first = kind==Kind::word &&
        publication==NativeCheckedDwordPublication::begin_capacity_end;
    if (begin_first) write(header,4,address(fresh));
    write(header,12,address(fresh)+bytes);write(header,8,address(fresh)+final_size*width(kind));
    if (!begin_first) write(header,4,address(fresh));
}
} // namespace

void resize_native_input_settings_descriptors_006a0db0(void* header, Word count, std::array<Word,5> value) {
    resize(Kind::descriptor,header,count,value.data(),nullptr);
}
void resize_native_input_settings_words_00492210(void* header, Word count, Word value) {
    resize(Kind::word,header,count,&value,nullptr);
}
void resize_native_checked_dword_storage(void* header, Word count, Word value,
    NativeCheckedDwordPublication publication) {
    resize(Kind::word,header,count,&value,nullptr,publication);
}
void resize_native_input_settings_bits_0049df50(void* header, Word count, Word value) {
    const Word old=read(header,0);if (count==old) return;
    const Word words=(count+31u)>>5, zero=0;
    resize(Kind::word,at(header,4),words,&zero,nullptr);
    write(header,0,count);
    if (count>old) {
        const Word first=read(header,8);
        for (Word bit=old;bit<count;++bit) {
            void* const word=pointer(first+(bit>>5)*4);const Word mask=1u<<(bit&31u);
            write(word,0,(value&255u) ? read(word,0)|mask : read(word,0)&~mask);
        }
    } else if (count&31u) {
        void* const last=pointer(read(header,8)+(words-1u)*4u);
        write(last,0,read(last,0)&((1u<<(count&31u))-1u));
    }
}
void resize_native_input_settings_strings_0049e050(void* header, Word count, std::array<Word,2> value, NativeStringStorage& strings) {
    OwnedValue incoming{Kind::string,value.data(),&strings};resize(Kind::string,header,count,value.data(),&strings);
}
void resize_native_input_settings_name_pairs_006a6350(void* header, Word count, std::array<Word,4> value, NativeStringStorage& strings) {
    OwnedValue incoming{Kind::strings,value.data(),&strings};resize(Kind::strings,header,count,value.data(),&strings);
}
void resize_native_input_settings_groups_006a79a0(void* header, Word count, std::array<Word,4> value, NativeStringStorage& strings) {
    OwnedValue incoming{Kind::pairs,value.data(),&strings};resize(Kind::pairs,header,count,value.data(),&strings);
}
void resize_native_input_settings_conflict_pairs_006a4710(void* header, Word count, std::array<Word,4> value) {
    OwnedValue incoming{Kind::words,value.data(),nullptr};resize(Kind::words,header,count,value.data(),nullptr);
}
void destroy_native_input_settings_conflict_pair_range_0069eea0(void* first, void* last) {
    destroy_range(Kind::words,first,last,nullptr);
}
void destroy_native_input_settings_group_range_006a6ee0(void* first, void* last, NativeStringStorage& strings) {
    destroy_range(Kind::pairs,first,last,&strings);
}
} // namespace bsp
