#pragma once
#include <atomic>
namespace bsp {
class NativeMeshCanonicalRecord;
class NativeMeshCanonicalAccess;
class NativePostEffectModelFrame;
class NativeRenderObservedZeroAccess;
class NativeRenderObservedZeroFrame;

// One external C++ node is reserved in each real incarnation before Native
// effects. It is never a Native count, zero action, death or return receipt.
class NativeRenderObservedZeroHold final {
private:
    friend class NativeMeshCanonicalRecord;
    friend class NativeMeshCanonicalAccess;
    friend class NativePostEffectModelFrame;
    friend class NativeRenderObservedZeroAccess;
    friend class NativeRenderObservedZeroFrame;
    NativeRenderObservedZeroHold() noexcept = default;
    std::atomic<bool> claimed_{false};
    NativeRenderObservedZeroHold* next_{};
    NativeRenderObservedZeroFrame* frame_{};
    NativeMeshCanonicalRecord* mesh_{};
    NativePostEffectModelFrame* model_{};
};

// Caller-owned metadata, reserved before effects and kept through its actual
// returned observations, health fences and catches. Attachment is private and
// only follows the real decrement-zero and SAME resolve/alias observation.
// Move transfers the whole C++ hold without a Native retain. Destruction is a
// last-use fence for this metadata, never a disposal/Native completion action.
// Move assignment also ends its old destination metadata: the real caller
// must establish that destination's complete last-use fence, not merely an
// inactive invocation counter. This cannot reset a still-used Native HOME.
class NativeRenderObservedZeroFrame final {
public:
    NativeRenderObservedZeroFrame() noexcept = default;
    ~NativeRenderObservedZeroFrame() noexcept;
    NativeRenderObservedZeroFrame(NativeRenderObservedZeroFrame&&) noexcept;
    NativeRenderObservedZeroFrame& operator=(NativeRenderObservedZeroFrame&&) noexcept;
    NativeRenderObservedZeroFrame(const NativeRenderObservedZeroFrame&) = delete;
    NativeRenderObservedZeroFrame& operator=(const NativeRenderObservedZeroFrame&) = delete;
private:
    friend class NativeRenderObservedZeroAccess;
    friend class NativeMeshCanonicalAccess;
    friend class NativeMeshCanonicalRecord;
    friend class NativePostEffectModelFrame;
    NativeRenderObservedZeroHold* head_{};
    int uncaught_on_attach_{};
    std::atomic<unsigned> active_invocations_{0};
};
} // namespace bsp
