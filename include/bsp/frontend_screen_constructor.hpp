#pragma once

namespace bsp {

// The sole dependency of the credits and mission-tree constructors. The raw
// entries delegate to their original hosts; the menu binds its registered flags.
struct FrontendScreenConstructorHost {
    virtual ~FrontendScreenConstructorHost() = default;
    virtual void construct_frontend_base() = 0;
};

} // namespace bsp
