#include "bsp/native_system_constant_registry.hpp"
#include <cstring>
#include <exception>
#include <limits>
#include <new>
#include <stdexcept>

namespace bsp {
namespace {
using Owner=NativeSystemConstantRegistryStorage;
using Binding=NativeSystemConstantRegistryLifetimeBinding;
using Operation=NativeSystemConstantRegistryOperation;
using Array=NativeCompiledShaderConstants;
using ArrayOperation=NativeCompiledShaderArrayOperation;
using Record=NativeCompiledShaderConstantStorage;
struct Literal { const char* name; std::uint32_t second,first,array_count,id; };
constexpr Literal literals[]={
#include "shader_system_registry.inc"
};
static_assert(std::size(literals)==52);
Record* row(Record* data,std::int32_t index) noexcept {
    return reinterpret_cast<Record*>(reinterpret_cast<std::uintptr_t>(data)
        +static_cast<std::uint32_t>(index)*0x20u);
}
void require_array(const Array& a) {
    if(a.count_04<0 || a.capacity_08<a.count_04
        || a.capacity_08>(std::numeric_limits<std::int32_t>::max)()/0x20
        || (a.capacity_08 && !a.data_00))
        throw std::logic_error("system constant array is outside the readable native domain");
}
ArrayOperation& start_array(Operation& op,std::int32_t request) {
    require_array(op.owner->constants_04);
    op.array=std::make_unique<ArrayOperation>();
    auto& a=*op.array;
    a.array=&op.owner->constants_04;a.requested=request;
    a.phase=ArrayOperation::Phase::running;
    return a;
}
void fail(Operation& op) noexcept {
    if(op.array && op.array->phase==ArrayOperation::Phase::running)
        op.array->phase=ArrayOperation::Phase::failed;
    op.phase=Operation::Phase::failed;
}
void reserve(Array& a,std::int32_t request,NativeStringStorage& strings,ArrayOperation& op,Operation& parent) {
    if(request<1)request=1;
    if(request<=a.capacity_08)return;
    if(request>(std::numeric_limits<std::int32_t>::max)()/0x20)
        throw std::length_error("system constant allocation exceeds readable native extent");
    op.step=ArrayOperation::Step::allocation;op.allocated_capacity=request;
    const auto bytes=static_cast<std::uint32_t>(request)*0x20u;
    auto* data=static_cast<Record*>(singleton_lifetime_allocate({SingletonAllocationKind::object,bytes,bytes}));
    op.unpublished_data=data;op.step=ArrayOperation::Step::copying;
    for(std::int32_t i=0;i<a.count_04;++i) {
        if(i>=request)throw std::logic_error("system constant callback exceeded acquired extent");
        op.current_record=row(data,i);
        parent.current_copy_source=row(a.data_00,i);
        if(op.current_record)copy_native_compiled_shader_constant_00b38310(op.current_record,*parent.current_copy_source,strings);
        op.copied_rows=i+1;op.current_record=nullptr;parent.current_copy_source=nullptr;
    }
    op.step=ArrayOperation::Step::releasing_old;
    for(std::int32_t i=0;i<a.count_04;++i) {
        destroy_native_string_header_0041dd20(&row(a.data_00,i)->name_length_14,strings);
        op.released_rows=i+1;
    }
    singleton_lifetime_free(a.data_00);
    op.step=ArrayOperation::Step::publication;
    a.data_00=data;a.capacity_08=request;op.unpublished_data=nullptr;
}
void resize(Array& a,std::int32_t request,NativeStringStorage& strings,ArrayOperation& op,Operation& parent) {
    if(a.capacity_08<request)reserve(a,request,strings,op,parent);
    op.step=ArrayOperation::Step::default_rows;
    for(std::int32_t i=a.count_04;i<request;++i) {
        op.current_record=row(a.data_00,i);
        if(op.current_record)initialize_native_compiled_shader_constant_00b5bb40(op.current_record,strings);
        ++op.initialized_rows;op.current_record=nullptr;
    }
    op.step=ArrayOperation::Step::shrinking;
    while(request<a.count_04) {
        --a.count_04;
        destroy_native_string_header_0041dd20(&row(a.data_00,a.count_04)->name_length_14,strings);
        ++op.released_rows;
    }
    a.count_04=request;
}
void append(Array& a,const Record& source,NativeStringStorage& strings,ArrayOperation& op,Operation& parent) {
    op.append_source=&source;
    if(a.count_04==a.capacity_08) {
        auto request=static_cast<std::int32_t>(static_cast<std::uint32_t>(a.capacity_08)*2u);
        if(request<2)request=1;
        reserve(a,request,strings,op,parent);
    }
    op.step=ArrayOperation::Step::append_row;op.current_record=row(a.data_00,a.count_04);
    parent.current_copy_source=&source;
    if(op.current_record)copy_native_compiled_shader_constant_00b38310(op.current_record,source,strings);
    ++a.count_04;op.initialized_rows=1;op.current_record=nullptr;parent.current_copy_source=nullptr;
}
void base_construct(Owner& owner,Binding& binding,Operation& op) {
    op.step=Operation::Step::base;owner.vtable_00=0x00d626f4;
    {
        CapturedSoundLifetimeSection captured(binding.lifetime_access());
        binding.publication()=&owner;op.base_published=true;
        auto manager=binding.lifetime_access().get_manager_00415350();
        manager->register_object(binding.publication());
    }
}
void base_destroy(Owner& owner,Binding& binding,Operation& op) {
    op.step=Operation::Step::destroy_base;owner.vtable_00=0x00d626f4;
    {
        CapturedSoundLifetimeSection captured(binding.lifetime_access());
        auto manager=binding.lifetime_access().get_manager_00415350();
        manager->unregister_object(binding.publication());binding.publication()=nullptr;
    }
    owner.vtable_00=0x00ce3818;
}
void registry_destroy(Owner& owner,Binding& binding,Operation& op) {
    owner.vtable_00=0x00d62a3c;op.step=Operation::Step::destroy_array;
    auto& child=start_array(op,0);
    resize(owner.constants_04,0,binding.strings(),child,op);
    singleton_lifetime_free(owner.constants_04.data_00);
    child.phase=ArrayOperation::Phase::complete;
    base_destroy(owner,binding,op);
}
void literal(Owner& owner,const Literal& value,Binding& binding,Operation& op) {
    auto& strings=binding.strings();
    op.step=Operation::Step::literal_name;
    ::new(&op.temporary_name) NativeString;
    op.name_live=true;op.captured_name_data=nullptr;
    op.temporary_name.resize_0041dd40(strings,static_cast<std::uint32_t>(std::strlen(value.name)),true);
    op.captured_name_data=op.temporary_name.data();
    if(op.captured_name_data)std::memcpy(op.captured_name_data,value.name,op.temporary_name.length()+1u);
    op.step=Operation::Step::literal_record;op.record_live=true;
    construct_native_system_constant_00b5bbc0(&op.temporary_record,op.temporary_name,
        value.id,value.second,value.first,value.array_count,strings);
    op.step=Operation::Step::append;op.source=&op.temporary_record;
    auto& child=start_array(op,owner.constants_04.count_04+1);
    append(owner.constants_04,op.temporary_record,strings,child,op);
    child.phase=ArrayOperation::Phase::complete;
    op.step=Operation::Step::release_record;
    destroy_native_string_header_0041dd20(&op.temporary_record.name_length_14,strings);
    op.record_live=false;
    op.step=Operation::Step::release_name;
    // B5BF70 retains the pre-append name pointer, but reloads CURRENT length.
    if(op.captured_name_data)strings.release(op.captured_name_data,op.temporary_name.length()+1u);
    op.name_live=false;op.captured_name_data=nullptr;op.source=nullptr;
}
} // namespace
NativeSystemConstantRegistryOperation::~NativeSystemConstantRegistryOperation() {
    if(phase==Phase::running || phase==Phase::failed)std::terminate();
}
Record* construct_native_system_constant_00b5bbc0(void* fresh,const NativeString& name,
    std::uint32_t id,std::uint32_t second,std::uint32_t first,std::uint32_t array_count,
    NativeStringStorage& strings) {
    auto* record=::new(fresh) Record;
    record->name_length_14=0;record->name_data_18=nullptr;
    record->words_00[0]=0xffff;record->words_00[1]=second*first;
    copy_native_string_header_00be0a30_fragment(&record->name_length_14,strings,&name);
    record->words_00[3]=first;record->words_00[2]=second;
    record->words_00[4]=array_count;record->word_1c=id;
    return record;
}
void reserve_native_system_constants_00b5bd10(Owner& owner,std::int32_t request,Binding& binding,Operation& op) {
    binding.begin(owner,op);
    try {auto& child=start_array(op,request);reserve(owner.constants_04,request,binding.strings(),child,op);
        child.phase=ArrayOperation::Phase::complete;binding.complete(op);}
    catch(...){fail(op);throw;}
}
void resize_native_system_constants_00b5be10(Owner& owner,std::int32_t request,Binding& binding,Operation& op) {
    if(request<0)throw std::invalid_argument("negative system constant count");
    binding.begin(owner,op);
    try {auto& child=start_array(op,request);resize(owner.constants_04,request,binding.strings(),child,op);
        child.phase=ArrayOperation::Phase::complete;binding.complete(op);}
    catch(...){fail(op);throw;}
}
void append_native_system_constant_00b5bed0(Owner& owner,const Record& source,Binding& binding,Operation& op) {
    binding.begin(owner,op);op.source=&source;
    try {auto& child=start_array(op,owner.constants_04.count_04+1);
        append(owner.constants_04,source,binding.strings(),child,op);
        child.phase=ArrayOperation::Phase::complete;binding.complete(op);}
    catch(...){fail(op);throw;}
}
Owner* construct_native_system_constant_base_00b5b9e0(Owner& owner,Binding& binding,Operation& op) {
    binding.begin(owner,op);
    try {base_construct(owner,binding,op);binding.complete(op);return &owner;}
    catch(...){fail(op);throw;}
}
void destroy_native_system_constant_base_00b5ba80(Owner& owner,Binding& binding,Operation& op) {
    binding.begin(owner,op);
    try {base_destroy(owner,binding,op);binding.complete(op);}
    catch(...){fail(op);throw;}
}
Owner* delete_native_system_constant_base_00b5bb20(Owner* owner,Binding& binding,std::uint32_t flags,Operation& op) {
    binding.begin(*owner,op);
    try {base_destroy(*owner,binding,op);op.step=Operation::Step::free_owner;
        if(flags&1)singleton_lifetime_free(owner);binding.complete(op);return owner;}
    catch(...){fail(op);throw;}
}
Owner* construct_native_system_constant_registry_00b5bf70(void* fresh,Binding& binding,Operation& op) {
    if(!fresh || reinterpret_cast<std::uintptr_t>(fresh)%alignof(Owner))
        throw std::invalid_argument("system constant registry requires aligned fresh10h storage");
    auto* owner=static_cast<Owner*>(fresh);binding.begin(*owner,op);
    try {
        ::new(fresh) Owner;base_construct(*owner,binding,op);
        owner->vtable_00=0x00d62a3c;op.step=Operation::Step::arrays;
        owner->constants_04={nullptr,0,0};op.arrays_initialized=true;
        for(const auto& value:literals){literal(*owner,value,binding,op);++op.literal_index;}
        binding.complete(op);return owner;
    } catch(...){fail(op);throw;}
}
void destroy_native_system_constant_registry_00b5df00(Owner& owner,Binding& binding,Operation& op) {
    binding.begin(owner,op);
    try {registry_destroy(owner,binding,op);binding.complete(op);}
    catch(...){fail(op);throw;}
}
Owner* delete_native_system_constant_registry_00b5df70(Owner* owner,Binding& binding,std::uint32_t flags,Operation& op) {
    binding.begin(*owner,op);
    try {registry_destroy(*owner,binding,op);op.step=Operation::Step::free_owner;
        if(flags&1)singleton_lifetime_free(owner);binding.complete(op);return owner;}
    catch(...){fail(op);throw;}
}
Binding::NativeSystemConstantRegistryLifetimeBinding(void* volatile& published,SingletonLifetimeCallbacks next)
    :published_(published),next_(next) {
    if(!next.destroy_registered || !next.invalid_parameter)
        throw std::invalid_argument("system constant lifetime needs other-owner callbacks");
}
Binding::NativeSystemConstantRegistryLifetimeBinding(void* volatile& published,
    void* volatile& actual_manager,NativeStringStorage& strings)
    :published_(published),next_{},actual_manager_(&actual_manager),strings_(&strings) {}
Binding::~NativeSystemConstantRegistryLifetimeBinding(){if(guarded_)std::terminate();}
SingletonLifetimeCallbacks Binding::callbacks() noexcept {return {this,&destroy_registered,&invalid_parameter};}
void Binding::bind(SingletonLifetimeDomain& lifetime,NativeStringStorage& strings) {
    if(lifetime_ || actual_manager_ || strings_)throw std::logic_error("system constant lifetime already bound");
    lifetime_=&lifetime;strings_=&strings;
}
SingletonLifetimeDomain& Binding::lifetime() {
    if(!lifetime_)throw std::logic_error("system constant lifetime not bound");return *lifetime_;
}
SoundLifetimeAccess Binding::lifetime_access() {
    if(actual_manager_)return SoundLifetimeAccess(*actual_manager_);
    return SoundLifetimeAccess(lifetime());
}
NativeStringStorage& Binding::strings() {
    if(!strings_)throw std::logic_error("system constant strings not bound");return *strings_;
}
void Binding::begin(Owner& owner,Operation& op) {
    if(op.phase!=Operation::Phase::fresh || guarded_)
        throw std::logic_error("system constant operation is one-shot or another operation remains live");
    (void)lifetime_access();(void)strings();
    op.owner=&owner;op.binding=this;op.phase=Operation::Phase::running;guarded_=&op;
}
void Binding::complete(Operation& op) {
    if(guarded_!=&op || op.phase!=Operation::Phase::running)std::terminate();
    op.phase=Operation::Phase::complete;guarded_=nullptr;
}
bool Binding::terminal_allowed(const Owner& owner) const noexcept {return !guarded_ || guarded_->owner!=&owner;}
void Binding::destroy_registered(void* context,void* raw,std::uint32_t flags) noexcept {
    auto& self=*static_cast<Binding*>(context);auto* owner=static_cast<Owner*>(raw);
    if(!self.terminal_allowed(*owner))std::terminate();
    if(owner->vtable_00==0x00d62a3c || owner->vtable_00==0x00d626f4) {
        Operation op;
        if(owner->vtable_00==0x00d62a3c)delete_native_system_constant_registry_00b5df70(owner,self,flags,op);
        else delete_native_system_constant_base_00b5bb20(owner,self,flags,op);
    } else {
        if(!self.next_.destroy_registered)std::terminate();
        self.next_.destroy_registered(self.next_.context,raw,flags);
    }
}
void Binding::invalid_parameter(void* context) {
    auto& self=*static_cast<Binding*>(context);
    if(!self.next_.invalid_parameter)std::terminate();
    self.next_.invalid_parameter(self.next_.context);
}
} // namespace bsp
