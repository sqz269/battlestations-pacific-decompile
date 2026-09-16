# Application particle pool storage and CRT lifetime

## Result

`GameNativeParticlePoolProcess` retains the actual `38h` source storage for the
`00F8D2D0` particle-model pool and `00F8D344` particle-parameter pool. Both use
`GameNativePhysicalPoolProcess::allocator_list_domain_00e188b4()`, the same
application list used by the physical-provider pool. It creates no second list
head, slab table or substitute allocator implementation.

Each explicit startup method binds its own actual owner and calls the recovered
initializer: `CD7830` for models, `CD78B0` for parameters. The methods serialize
startup, return the original registration status on repeated calls, and reject
retry after an interrupted attempt. A nonzero `atexit` return leaves the native
pool initialized without substitute cleanup. Accessors reject use before the
corresponding startup returns. These are host lifetime guards, not newly
reconstructed native bodies.

The shared list owner finishes construction first, then the particle process
object. Only afterward do explicit native initializers register their CRT
callbacks. Consequently those callbacks run while both C++ bookkeeping objects
are alive. Their C++ destructors do not repeat native pool destruction. Native
clients must be drained before process exit; the pool destructors free slabs
without constructing or destroying their payload objects.

## Original ordering evidence

Live Ghidra data references put `CD7830` at `CE34EC` and `CD78B0` at `CE34F4`.
All twelve bytes `CE34EC..CE34F7` match the configured installed PE:
`30 78 cd 00 50 78 cd 00 b0 78 cd 00`. This supports model-before-parameter
ordering; `CD7850` lies between them and is outside this packet. The adapter does
not claim to reproduce the entire original CRT initializer sequence.

The pool bodies and callbacks remain the existing reconstructed
`native_particle_model_pool_owner`, `native_particle_parameter_pool_owner`,
`native_weak_owner` and `native_physical_provider_pool_lifecycle` implementations.
No original function was added or renamed by this application-storage packet.

## Focused validation

An ignored Win32 executable links the built `bsp_core.lib` and uses the actual
application process owners. No registrar replacement, custom allocator or pool
substitute is supplied. It verifies:

- Both particle accessors reject pre-start use; construction publishes no pool.
- Explicit model, parameter and physical startup create three distinct actual
  owners on the same doubly linked allocator list.
- Repeated particle startup returns the original statuses and preserves the
  actual table pointers.
- Genuine model, parameter and physical raw allocation/return operations work.
  One shared `trim_all_004b46b0` reaches all three real trim implementations.
- Fresh raw slots remain allocated at normal process exit. Interleaved
  `std::atexit` observers verify physical cleanup, then parameter cleanup, then
  model cleanup. The shared list is empty afterward, while the bookkeeping
  owners are still alive. None of those raw slots contains a constructed game
  payload requiring a payload destructor.

The process exited with status zero and all four receipt lines passed. Source,
build command and receipt are retained under ignored `local/`; their hashes and
the linked library hash are in the report. `/MD`, `/W4 /WX`, `/fp:strict` and an
embedded executable manifest were used. The full strict Win32 build and all
three existing CTests passed. All eight configured differential seeds matched
the installed PE.

## Limits

This validates the new source process ownership and actual source CRT callback
timing. It does not execute the original CRT initializer array or prove original
register ABI, FH3/SEH behavior, allocation-fault handling, concurrent pool use,
native rendering or gameplay. The source registration-failure/no-retry guards
were inspected; failure injection is covered only by the separate bounded
parameter-initializer probe, not by this process-owner probe. Particle resource,
texture/render and raw-Lua dispatch installation remain separate work.
