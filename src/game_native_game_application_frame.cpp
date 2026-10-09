#include "bsp/game_native_game_application_frame.hpp"
#include "bsp/native_game_construction.hpp"
#include "bsp/native_string.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <exception>
#include <new>
#include <stdexcept>
#include <type_traits>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native game application frame requires MSVC Win32.
#endif

namespace bsp::game {
GameNativeGameApplicationFrame::GameNativeGameApplicationFrame(
    GameNativeGameRuntime& runtime,void* volatile& application,
    char* volatile& duplicate,NativeStringRawPoolContext& strings)
    :runtime_(runtime),application_game_14_(application),
     duplicate_name_00e1ae78_(duplicate),strings_(strings) {
    static_assert(sizeof(NativeGameStorage)==0x71a0);
    static_assert(std::is_trivially_default_constructible_v<NativeGameStorage>);
    static_assert(std::is_trivially_destructible_v<NativeGameStorage>);
    static_assert(sizeof(void*)==4&&sizeof(name_header_)==8);
    if(runtime_.phase()!=GameNativeGameRuntime::Phase::fresh)
        throw std::invalid_argument("Application frame requires an exclusive fresh native game runtime");
}
GameNativeGameApplicationFrame::~GameNativeGameApplicationFrame() {
    // Source retained-owner rejection policy; NOT recovered native EH cleanup.
    if((phase_!=Phase::fresh&&phase_!=Phase::destroyed&&phase_!=Phase::retired
            &&phase_!=Phase::diagnostic_retired)
        ||(allocation_!=Allocation::absent&&allocation_!=Allocation::released)
        ||!name_resolved()||!runtime_retired())std::terminate();
}
bool GameNativeGameApplicationFrame::active() const noexcept {
    return phase_==Phase::constructing||phase_==Phase::deleting;
}
bool GameNativeGameApplicationFrame::name_resolved() const noexcept {
    return name_==Name::unprepared||name_==Name::returned||name_==Name::externally_resolved;
}
bool GameNativeGameApplicationFrame::runtime_retired() const noexcept {
    const auto p=runtime_.phase();
    return (p==GameNativeGameRuntime::Phase::fresh&&!constructor_entered_)
        ||(p==GameNativeGameRuntime::Phase::destroyed&&scalar_completed_)
        ||(p==GameNativeGameRuntime::Phase::diagnostic_retired&&constructor_entered_);
}
void GameNativeGameApplicationFrame::record_failure() noexcept {
    failed_stage_=active_stage_;failed_native_site_=native_site_;
    active_stage_=Stage::none;phase_=Phase::failed;
}
NativeGameStorage* GameNativeGameApplicationFrame::construct() {
    if(phase_!=Phase::fresh||runtime_.phase()!=GameNativeGameRuntime::Phase::fresh)
        throw std::logic_error("Application frame construction is one-shot");
    phase_=Phase::constructing;active_stage_=Stage::allocation;native_site_=0x73e150;
    try {
        void* block=singleton_lifetime_allocate({SingletonAllocationKind::object,0x71a0,0x71a0});
        allocation_address_=reinterpret_cast<std::uintptr_t>(block);
        retained_=static_cast<NativeGameStorage*>(block);allocation_=Allocation::retained;
        ::new (block) NativeGameStorage;
        std::memset(block,0,0x71a0);
        parent_cleanup_state_=0x27;parent_state_recorded_=true;
        captured_name_=duplicate_name_00e1ae78_;
        active_stage_=Stage::name_preparation;native_site_=0x73e188;name_=Name::preparing;
        construct_native_string_header_0041e870(name_header_,strings_,captured_name_);
        name_=Name::complete;observed_name_mask_|=0x2;
        parent_cleanup_state_=0x28;
        active_stage_=Stage::game_construction;native_site_=0x73e1a1;constructor_entered_=true;
        auto* result=runtime_.construct(*retained_,name_header_);
        constructor_completed_=true;constructor_result_address_=reinterpret_cast<std::uintptr_t>(result);
        native_site_=0x73e1af;application_game_14_=result;application_published_=true;
        parent_cleanup_state_=-1;
        active_stage_=Stage::name_return;native_site_=0x73e1ce;name_=Name::return_attempted;
        destroy_native_string_header_0041dd20(name_header_,strings_);
        name_=Name::returned;active_stage_=Stage::none;phase_=Phase::live;
        return result;
    } catch(...) {record_failure();throw;}
}
NativeGameStorage* GameNativeGameApplicationFrame::scalar_delete(std::uint32_t flags) {
    if(active()||!constructor_completed_||scalar_attempted_||!name_resolved()
        ||allocation_!=Allocation::retained||runtime_.phase()!=GameNativeGameRuntime::Phase::live)
        throw std::logic_error("Application frame scalar requires its first live owner and resolved name");
    scalar_attempted_=true;scalar_flags_=flags;phase_=Phase::deleting;
    active_stage_=Stage::scalar_delete;native_site_=0x4de270;
    // A qualified override can release then throw. Never retain a supposedly
    // live pointer across this call; success with bit 0 clear restores ownership.
    allocation_=Allocation::indeterminate;retained_=nullptr;
    try {
        auto* result=runtime_.scalar_delete(flags);
        scalar_result_address_=reinterpret_cast<std::uintptr_t>(result);scalar_completed_=true;
        if(flags&1)allocation_=Allocation::released;
        else {allocation_=Allocation::retained;retained_=reinterpret_cast<NativeGameStorage*>(allocation_address_);}
        active_stage_=Stage::none;phase_=Phase::destroyed;
        return result;
    } catch(...) {record_failure();throw;}
}
void GameNativeGameApplicationFrame::free_retained_storage() {
    if(active()||allocation_!=Allocation::retained||!name_resolved()||!runtime_retired()
        ||(runtime_.phase()==GameNativeGameRuntime::Phase::destroyed&&(scalar_flags_&1)))
        throw std::logic_error("Application frame free requires resolved graph/name and proven retained allocation");
    singleton_lifetime_free(retained_);
    retained_=nullptr;allocation_=Allocation::released;phase_=Phase::retired;
}
void GameNativeGameApplicationFrame::acknowledge_name_resolution() {
    if(active()||name_resolved())
        throw std::logic_error("Only a pending application name can be externally resolved");
    name_=Name::externally_resolved;
}
void GameNativeGameApplicationFrame::acknowledge_diagnostic_retirement(DiagnosticDisposition disposition) {
    if(active()||phase_==Phase::retired||phase_==Phase::diagnostic_retired
        ||!name_resolved()||!runtime_retired())
        throw std::logic_error("Application diagnostic retirement requires resolved name and nonlive runtime");
    if(disposition==DiagnosticDisposition::no_allocation) {
        if(allocation_address_||allocation_!=Allocation::absent)
            throw std::logic_error("Application allocation was not absent");
    } else if(disposition==DiagnosticDisposition::already_released) {
        if(!allocation_address_||allocation_==Allocation::absent)
            throw std::logic_error("Application retirement requires the original allocated address");
        retained_=nullptr;allocation_=Allocation::released;
    } else throw std::invalid_argument("Unknown application allocation disposition");
    phase_=Phase::diagnostic_retired;
}
GameNativeGameApplicationFrame::Status GameNativeGameApplicationFrame::status() const noexcept {
    return {phase_,active_stage_,failed_stage_,allocation_,name_,runtime_.phase(),
        allocation_address_,constructor_result_address_,scalar_result_address_,captured_name_,
        native_site_,failed_native_site_,observed_name_mask_,scalar_flags_,parent_cleanup_state_,
        parent_state_recorded_,constructor_entered_,constructor_completed_,application_published_,
        scalar_attempted_,scalar_completed_};
}
void* GameNativeGameApplicationFrame::name_header_for_diagnostics() noexcept {
    return name_==Name::unprepared?nullptr:name_header_;
}
} // namespace bsp::game
