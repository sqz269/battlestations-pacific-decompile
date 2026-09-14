#include "bsp/native_resource_instance_publication.hpp"
#include "bsp/detail/native_tree_insert_storage.hpp"
#include "bsp/native_hardware_layout_tree_insert.hpp"
#include "bsp/native_input_settings_vector_storage.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <array>
#include <cstdlib>
#include <cstring>

namespace bsp {
namespace {
using Word=std::uint32_t;
using Access=detail::TreeInsertAccess<0x20,0x21>;
static_assert(sizeof(void*)==4);
void* at(const void* p,Word offset=0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(p)+offset);
}
Word word(const void* p,Word offset=0) noexcept {
    return *static_cast<const volatile Word*>(at(p,offset));
}
void put(void* p,Word offset,Word value) noexcept {
    *static_cast<volatile Word*>(at(p,offset))=value;
}
void invalid() { _invalid_parameter_noinfo(); }
struct Iterator { void* owner;void* node; };
struct Result { void* owner;void* node;std::uint8_t inserted; };
struct MirroredAccess:Access {
    static void* volatile& left(void* node) noexcept { return Access::right(node); }
    static void* volatile& right(void* node) noexcept { return Access::left(node); }
};
[[noreturn]] void too_long() {
    NativeLegacySboStringStorage text;
    text.capacity_18=15;text.length_14=0;text.buffer_04.inline_bytes[0]=0;
    struct Cleanup { NativeLegacySboStringStorage& text;
        ~Cleanup() noexcept { native_legacy_sbo_string_destroy_004072d0(text); }
    } cleanup{text};
    native_legacy_sbo_string_assign_counted_00408720(text,"map/set<T> too long",19);
    throw NativeHardwareLayoutTreeLengthError{text};
}
void* empty_type_node(void* left,void* parent,void* right,const void* key,std::uint8_t color) {
    // B890B0 copies the empty mapped vector created by B89C50. Its opaque
    // proxy word10 and final padding22/23 are not written. No populated-vector
    // copy is substituted for this particular operator[] default path.
    void* const node=singleton_lifetime_allocate({SingletonAllocationKind::object,0x24,0x24});
    if(node) {
        put(node,0,reinterpret_cast<Word>(left));put(node,4,reinterpret_cast<Word>(parent));
        put(node,8,reinterpret_cast<Word>(right));put(node,12,word(key));
        put(node,0x14,0);put(node,0x18,0);put(node,0x1c,0);
        *static_cast<std::uint8_t*>(at(node,0x20))=color;
        *static_cast<std::uint8_t*>(at(node,0x21))=0;
    }
    return node;
}
Iterator* link_node(void* tree,Iterator* output,std::uint8_t left,void* parent,const void* key) {
    detail::link_tree_node<Access>(tree,output,left,parent,key,0x0ccccccbu,
        &empty_type_node,&detail::rotate_left_inlined<Access>,
        &detail::rotate_left_inlined<MirroredAccess>,&too_long);
    return output;
}
void* type_pairs(void* tree,Word key) {
    void* selected=detail::lower_bound_tree_node<Access>(tree,
        [key](void* node){return word(node,12)<key;});
    if(selected==Access::head(tree)||key<word(selected,12)) {
        Result output;
        detail::insert_unique_tree_pair<Access,Iterator>(tree,&output,&key,
            [key](void* node){return key<word(node,12);},
            [key](void* node){return word(node,12)<key;},
            [](Iterator& iterator){detail::decrement_tree_iterator<Access>(&iterator,&invalid);},
            &link_node,
            [](Result* result,void* owner,void* node,std::uint8_t inserted){
                result->owner=owner;result->node=node;result->inserted=inserted;
            });
        tree=output.owner;selected=output.node;
    }
    if(!tree)invalid();
    if(selected==Access::head(tree))invalid();
    return at(selected,0x10);
}
void classify(void* instance,void* item,void* node,Word table,Word token,Word offset,
    NativeResourceInstancePublicationContext& context) {
    const Word target=context.calls.item_slot(table,0xc);
    if(context.calls.matches_type(target,item,token)!=0) {
        const std::array<Word,2> pair{reinterpret_cast<Word>(item),reinterpret_cast<Word>(node)};
        append_native_checked_pair_storage(at(instance,offset),pair.data());
    }
}
} // namespace
void __fastcall publish_native_resource_instance_item_00b89e90(void* instance,
    NativeResourceInstancePublicationContext* context,void* item,void* node) {
    const std::array<Word,2> pair{reinterpret_cast<Word>(node),reinterpret_cast<Word>(item)};
    append_native_checked_pair_storage(at(instance,0x20),pair.data());
    const Word profile=word(item);
    const Word entry=context->calls.item_slot(profile,8);
    const Word key=context->calls.item_type_token(entry,item);
    append_native_checked_pair_storage(type_pairs(at(instance,0x30),key),pair.data());
}
void __fastcall publish_native_game_resource_instance_item_0071b710(void* instance,
    NativeResourceInstancePublicationContext* context,void* item,void* node) {
    publish_native_resource_instance_item_00b89e90(instance,context,item,node);
    if(!item)return;
    Word table=word(item);
    Word token=read_native_resource_type44_00721c90(context->type44_00e19be4);
    classify(instance,item,node,table,token,0x3c,*context);
    table=word(item);
    token=read_native_resource_type54_006f9a80(context->type54_00e19a98);
    classify(instance,item,node,table,token,0x4c,*context);
    table=word(item);token=context->type74_00e19b74;
    classify(instance,item,node,table,token,0x5c,*context);
    table=word(item);token=context->type64_00e19b64;
    classify(instance,item,node,table,token,0x6c,*context);
    table=word(item);token=context->type54_00e19b54;
    classify(instance,item,node,table,token,0x7c,*context);
}
} // namespace bsp
