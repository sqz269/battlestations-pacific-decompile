# GUI native stream companion registration contexts

This R37 change adds a host-only overload to `GuiNativeGeometryOwners` for
registering a completed native logical stream creator from its vertex and index
lifetime contexts directly. It is the narrow geometry prerequisite identified
by `reports/native_renderer_generated_model_main_r36.json`. It claims no native
address or reconstructed body and changes no native object, factory, renderer
array, reference count, or mapping operation.

## Existing service route

The existing `NativeStreamCloneServices` overload retains its exact guard:

1. `services.geometry` must be this `GuiNativeGeometryOwners` instance.
2. `services.vertices.actual_owners` must be the geometry registration's
   canonical `NativeRenderActualOwners` domain.

It then passes `services.vertices` and `services.indices` to the same internal
registration body. Existing callers gain no new renderer, synchronization, or
physical-context restriction.

## Direct context route

The new overload accepts `NativeLogicalVertexOwnerContext&` and
`NativeLogicalIndexCreationContext&`. The vertex context supplies the canonical
owner domain; the index context has no separate canonical-owner member. Its
published contract instead borrows the same renderer cell, synchronization
globals, and physical owner context as the existing logical lifetime domain.
The direct route therefore requires:

- the vertex canonical owner domain to equal the geometry registration domain;
- the vertex and index contexts to share the physical owner context;
- both contexts to refer to the same current-renderer cell; and
- both contexts to share the renderer synchronization globals.

These checks apply only to the new direct route. They do not narrow the
accepted inputs of the existing services overload.

## Registration and failure schedule

Both public routes reach the same internal body. That body still requires one
completed raw creator with no companion or canonical registration. It appends
one host entry, stores the raw creator, constructs the appropriate logical
vertex or index companion over the existing raw `+04` reference count, publishes
the companion in the caller's acquired frame, binds it to the canonical map,
marks the host entry registered, and finally marks the acquired frame canonically
registered.

Companion allocation or construction failure removes only the empty host entry;
the completed raw creator remains in the acquired frame. Bind failure retains
the companion in both the host entry and acquired frame, with registration bits
unset. No native release, retry, rollback, extra retain, renderer registration,
or pointer clearing was added.

## Validation boundary

The current MSVC Win32 `/MD` strict build passes with all three configured
CTest targets. The broad header rebuild compiles every existing services-route
caller and the new direct overload against the current vertex/index provider
types. No retained focused stream-registration fixture exists in this worktree,
and no permanent test was added for this metadata-only forwarding change.

This establishes the source API and build compatibility. It does not establish
native ABI replacement, original FH3/SEH identity, renderer construction, game
runtime, or gameplay behavior.
