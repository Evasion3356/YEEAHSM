#pragma once

// RDR2.exe is encrypted at rest and only reaches its final in-memory bytes
// after some runtime unpacking/deobfuscation step completes. Our DLL can be
// injected before that finishes, in which case an immediate pattern scan in
// DllMain would miss -- there's no OS synchronization object the game
// exposes for "unpacking done" to wait on, so the practical equivalent is
// retrying on a timer from a background thread until it resolves.
//
// This is safe against false positives: every pattern involved is 8+
// specific bytes, so matching still-encrypted/packed memory by chance is
// astronomically unlikely. The only real risk is a false negative (not
// there yet), which retrying handles.
//
// Currently retries two independent targets until both resolve (or the
// timeout hits): DismountWeaponStripPatch (the shipped fix) and
// HidePedWeaponsHook (a temporary diagnostic hook investigating the
// separate camp-arrival weapon relocation).
namespace PatchWorker
{
    // Spawns the background thread and returns immediately. Call once from
    // DllMain on DLL_PROCESS_ATTACH -- never do this work inline in DllMain
    // itself (blocking there while holding the loader lock is unsafe).
    void Start();

    // Signals the worker to stop retrying and reverts the patch if it was
    // applied. Non-blocking -- deliberately does NOT wait for the worker
    // thread to exit, since waiting on a thread from DLL_PROCESS_DETACH
    // risks deadlocking on the loader lock. Call from DllMain on
    // DLL_PROCESS_DETACH.
    void StopAndRevert();
}
