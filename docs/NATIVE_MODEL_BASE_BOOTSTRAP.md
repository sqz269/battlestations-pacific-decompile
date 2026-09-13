# Model-base static bootstrap

Addresses: 00CD7EB0, 00CD7F20, 00CE0E60

The new Win32 C++ interfaces reconstruct three complete bodies, with original
no-argument signatures. The descriptive names remain hypotheses. The type
adapter borrows the actual guard, descriptor words, shared node/root descriptor
and the same `TypeIdCounterLifetime` as `LightTypeBootstrap`. The caller must
wire that shared counter explicitly; this class does not create a singleton.

| Source method | Native address | Coverage and ABI |
| --- | --- | --- |
| `ModelBaseTypeBootstrap::initialize_static_type_descriptor_00cd7eb0` | CD7EB0..CD7EFE | complete; no-arg RET, unspecified return |
| `initialize_static_model_base_node_pool_00cd7f20` | CD7F20..CD7F35 | complete; no-arg RET, EAX atexit status |
| `destroy_static_model_base_node_pool_00ce0e60` | CE0E60..CE0E69 | complete; no-arg tail jump |

The CD7EB0 guard is consumed before writing name D62DE0 and calling the real
B6F110 node initializer on the actual 0108FF90 descriptor. It captures both
node and root tokens before publishing either. It then calls the shared
006FAC20 counter getter, increments its current +04 value, and publishes the
old value. There is no rollback on failure or second getter.

Bind the separate actual38h owner at 0109008C to the application's same
`AllocatorListDomain` before pool startup. The binding installs concrete
D62C78/B6EA60 trim dispatch before list publication. CD7F20 initializes
through B6E980, then registers CE0E60 with `std::atexit`; failure returns the
actual registration status and leaves the pool initialized. CE0E60 destroys
that same owner through B6E3D0 and removes its host trim binding. The raw owner
and allocator domain must remain alive until this callback runs. Binding
rejects null and any later change of pool or list identity. Native startup and
registered exit are a single lifecycle; manually destroying a registered pool
would leave an invalid callback. The 0108FF58 plain-node binding is separate.

Numeric edges: CD7ECF→B6F110, CD7EEA→006FAC20, CD7F25→B6E980,
CD7F2F→BF6FF5, CE0E65→B6E3D0. CRT cells CE356C and CE3574 establish table
membership and relative storage order only. CRT walker/order, CD7F00 neighbor,
B74B80/CE0E50, and application startup wiring remain named-but-incomplete.
This is source reconstruction, not original binary ABI compatibility or game
validation. Existing generic pool and singleton provider contracts are reused.

BG neighbor recheck: CD7F00/B74B80/CE0E50 are already complete NativeModelPool providers in src/native_model_pool.cpp and include/bsp/native_model_pool.hpp, introduced by 7372836d92c03f93cf01c0e53a860dcafe9a465e and integrated by 4ff70e294b22fdd8a9318e974a8bae137f48eaad. Current Ghidra owns CD7F00..CD7F15, B74B80..B74C52 and CE0E50..CE0E59. The earlier incomplete-neighbor statements are superseded by this recheck. Reuse the distinct canonical 01090054/D62DD0 NativeModelPool and the shared AllocatorListDomain; do not duplicate these bodies or substitute the 0109008C/D62C78 model-base pool. CRT walker/invocation order and application startup/lifetime wiring remain unverified.
