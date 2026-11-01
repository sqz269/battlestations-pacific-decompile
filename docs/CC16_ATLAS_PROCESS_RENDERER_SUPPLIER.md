# Atlas process renderer supplier

`GameNativeRendererApplication::atlas_process_texture_cache(data)` is a ready-only Source supplier for the concrete atlas process. It checks this application's ready phase, its private allocation identity written only after the real B32410 constructor returns normally, and the same read-only data owner. It returns the existing texture cache by reference.

The supplier reads only Source metadata. It does not read the mutable renderer publication, raw receiver/profile/slot, invent captured Native operands, create an owner, retain a resource, or execute a Native body. The existing AEEAF0 captured-dispatch accessor remains unchanged and owns its distinct argument epochs.

The process calls this supplier before explicit normal construction, loading and retirement preparation. It does not call it during cold graph binding or the actual popped scalar after shared drain begins. Failure propagates to the process's retained failure graph. The caller retains the real renderer/cache/RO/provider graph; this supplier gives no Native Origin, NoThrow, ABI, cleanup, Host-retention or activation authority. Build and actual process use remain separate verification.
