#include "bsp/native_input_device_sdk.hpp"

#include <stdexcept>

namespace bsp {
namespace {
class AcquisitionScope final {
public:
    explicit AcquisitionScope(std::size_t& count) noexcept : count_(count) { ++count_; }
    ~AcquisitionScope() { --count_; }
private:
    std::size_t& count_;
};
} // namespace

std::size_t NativeInputDeviceSdk::prepare_reference() {
    const std::size_t index = references_.size();
    references_.push_back(nullptr);
    return index;
}

HRESULT NativeInputDeviceSdk::create_device(IDirectInput8A& input, const GUID& guid,
    IDirectInputDevice8A** output, IUnknown* outer) {
    if (output == nullptr) throw std::invalid_argument("raw device SDK output slot is null");
    const std::size_t index = prepare_reference();
    AcquisitionScope scope(active_acquisitions_);
    const HRESULT result = input.CreateDevice(guid, output, outer);
    references_[index] = *output;
    return result;
}

HRESULT NativeInputDeviceSdk::create_effect(IDirectInputDevice8A& device, const GUID& guid,
    const DIEFFECT& effect, IDirectInputEffect** output, IUnknown* outer) {
    if (output == nullptr) throw std::invalid_argument("raw effect SDK output slot is null");
    const std::size_t index = prepare_reference();
    AcquisitionScope scope(active_acquisitions_);
    const HRESULT result = device.CreateEffect(guid, &effect, output, outer);
    references_[index] = *output;
    return result;
}

std::size_t NativeInputDeviceSdk::pending_references() const noexcept {
    std::size_t count = 0;
    for (IUnknown* reference : references_) if (reference != nullptr) ++count;
    return count;
}

void NativeInputDeviceSdk::release_tracked_references() {
    if (active_acquisitions_ != 0)
        throw std::logic_error("raw input SDK references cannot be released during acquisition");
    // Pop before Release; native wrappers have already finished borrowing.
    while (!references_.empty()) {
        IUnknown* const reference = references_.back();
        references_.pop_back();
        if (reference != nullptr) reference->Release();
    }
}
} // namespace bsp
