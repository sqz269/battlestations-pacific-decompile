#pragma once
#include "bsp/native_string.hpp"
#include <exception>
#include <vector>

namespace bsp {
class GuiTextLifetime;
class GuiTextEffectConsumer;
struct GuiTextBufferServices;
struct GuiTextShaderServices;
struct GuiTextSectionOperation;
struct NativeMaterialStorage;

enum class GuiTextEffectConsumerPhase { fresh, running, complete, failed };
enum class GuiTextEffectNamePhase { empty, constructing, live, consuming, consumed, borrow_ended };

struct GuiTextEffectConsumerMaterialCall;
// Source caller metadata only. The provider retains every published frame,
// including its header STORAGE, until the existing renderer drain. Consumed
// name CONTENTS are not readable/replayable. No destructor performs cleanup.
struct GuiTextEffectConsumerSectionCall {
    GuiTextEffectConsumerPhase phase{GuiTextEffectConsumerPhase::fresh};
    GuiTextEffectConsumer* consumer{};
    GuiTextLifetime* lifetime{};
    GuiTextSectionOperation* caller{};
    GuiTextBufferServices* services{};
    std::vector<GuiTextEffectConsumerMaterialCall*> materials;
    std::exception_ptr failure;
};
struct GuiTextEffectConsumerMaterialCall {
    virtual ~GuiTextEffectConsumerMaterialCall() = default;
    GuiTextEffectConsumerPhase phase{GuiTextEffectConsumerPhase::fresh};
    GuiTextEffectConsumerSectionCall* parent{};
    NativeString name;
    GuiTextEffectNamePhase name_phase{GuiTextEffectNamePhase::empty};
    NativeMaterialStorage* result{}; // Diagnostic after the caller consumes its creator.
};
struct GuiTextBorrowedEffectName {
    std::uint32_t length{};
    const char* data{};
};
struct GuiTextEffectConsumerShaderCall {
    virtual ~GuiTextEffectConsumerShaderCall() = default;
    GuiTextEffectConsumerPhase phase{GuiTextEffectConsumerPhase::fresh};
    GuiTextEffectConsumer* consumer{};
    GuiTextLifetime* lifetime{};
    GuiTextShaderServices* services{};
    // Only the selected default is constructed. Overrides borrow their live
    // semantic string bytes without a native allocation, copy, retain or free.
    NativeString default_name;
    GuiTextBorrowedEffectName override_name;
    const void* selected_name{};
    GuiTextEffectNamePhase name_phase{GuiTextEffectNamePhase::empty};
    bool default_selected{};
    bool name_cleanup_armed{};
    bool shader_published{};
    void* result{}; // Diagnostic after transfer into the canonical +1EC slot.
    std::exception_ptr failure;
};

// Per-services Source adaptation, never an Original vtable/register ABI.
// Every begin requires a NULL output slot and publishes stable storage before
// native effects. Only the exact retained caller may enter its nested factory.
// The same providers/services/lifetimes survive failures; failed or active calls
// cannot replay or retire. Completion/failure hooks add no native cleanup.
class GuiTextEffectConsumer {
public:
    virtual ~GuiTextEffectConsumer() = default;
    virtual bool has_incomplete_caller(const GuiTextLifetime&) const noexcept = 0;
    virtual void begin_sections(GuiTextLifetime&, GuiTextSectionOperation&,
        GuiTextBufferServices&, GuiTextEffectConsumerSectionCall*&) = 0;
    virtual void begin_section_material(GuiTextEffectConsumerSectionCall&,
        GuiTextEffectConsumerMaterialCall*&) = 0;
    virtual NativeMaterialStorage* create_section_material(GuiTextEffectConsumerMaterialCall&) = 0;
    virtual void finish_sections(GuiTextEffectConsumerSectionCall&) = 0;
    virtual void fail_sections(GuiTextEffectConsumerSectionCall&, std::exception_ptr) noexcept = 0;
    virtual void begin_shader(GuiTextLifetime&, GuiTextShaderServices&,
        GuiTextEffectConsumerShaderCall*&) = 0;
    virtual void* acquire_font_effect(GuiTextEffectConsumerShaderCall&) = 0;
    virtual void finish_shader(GuiTextEffectConsumerShaderCall&) = 0;
    virtual void fail_shader(GuiTextEffectConsumerShaderCall&, std::exception_ptr) noexcept = 0;
};
} // namespace bsp
