# Game-side process storage for shared native type IDs (BI)

`bsp::game::GameNativeTypeStorage` owns the root, node, light, directional,
file, memory-file and physical-file guard/descriptor cells for one application
process. Its `light_types()` and `stream_types()` return stable reference
aggregates. The raw VFS input must borrow `stream_types()` from this same
owner; later native type consumers must borrow its `light_types()` and
`stream_types()`. Copy and move are disabled so neither guards nor view
references can silently split into another domain.

The host retains this storage through the raw `01090AA0` singleton drain and
subsequent native type consumers. Drain of the shared singleton manager can
clear the separately published `0109DB7C` counter owner, but must not reset
these process descriptor guards or IDs. This class creates neither a counter,
manager, nor VFS-private type domain. Its initializer methods borrow the
caller's existing `TypeIdCounterLifetime` and common `LightTypeBootstrap`
only for the call. They require the latter to bind **all** light cells from
this owner and reject a mixed bootstrap before any type writes. The caller
must ensure the counter and bootstrap belong to the application's existing
shared lifetime; the storage class cannot infer the counter's publication.

The host constructs one common bootstrap from `light_types()` and its existing
counter. At the actual CRT positions, it calls the separate entries in order:

1. `initialize_memory_00cd8fc0(existing_counter, common_bootstrap)`;
2. the distinct physical-provider pool initializer `00CD9010` supplied by
   the physical pool packet, with its own process/CRT retention;
3. `initialize_physical_00cd9030(existing_counter, common_bootstrap)`.

These entries use the already reconstructed `NativeStreamTypeIds` bodies.
Each makes a temporary adapter over `stream_types()` and passes the same
counter/bootstrap; the process owner stores no adapter or borrowed context.
The file entry `00BE4530` runs inside either native child entry when its
guard is zero. Repeated calls preserve nonzero guards, descriptor values and
counter state. No constructor consumes an ID or executes an initializer.

The represented native cells are root guard/descriptor `0109DB80/0109DB84`,
node `0108FF54/0108FF90`, light `0109010D/0109018C`, directional
`0109010E/0109019C`, file `0109DB54/0109DB58`, memory file
`0109DB94/0109DBA0`, and physical file `0109DC2C/0109DC30`. These names
identify the source globals' roles; C++ members are host-owned storage, not
fixed-address mappings. The descriptors' native name words are evidence
addresses, not host pointers.

This packet supplies the retained storage and callable startup boundaries.
The primary integration must place its owner in the application lifetime,
register `src/game_native_type_storage.cpp` with the Win32 build, pass its
stream view to `GameNativeVfsRuntimeInputs`, and schedule the three separate
CRT steps without moving the physical pool between type initializers. Source
and focused child-process checks cannot establish original game execution,
CRT admission, shutdown order, or gameplay behavior.
