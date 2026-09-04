# My Hatred of C++'s Wastefulness

How a 140,288-byte "hook one function, discard the call" DLL got taken apart, measured, argued
with, and eventually rebuilt from nothing into a 2,048-byte one — and what that gap turned out
to actually be made of.

**Legend:** 🧑 = gir489 (human) &nbsp;·&nbsp; 🤖 = Claude Code (AI)

Same rule as [JOURNEY.md](JOURNEY.md): every step is tagged with who did it, and the wrong turns
stay in — including the one 🧑 personally called off mid-investigation.

---

## Prologue: The Ask

🧑 wanted the release build tightened up properly: figure out RDR2's actual minimum-spec CPU,
target codegen at whatever instruction set both of Rockstar's listed minimums (Intel i5-2500K,
AMD FX-6300) genuinely support, max out every other release optimization, and strip debug
symbols — *"as much optimizations as possible on release, strip debug symbols, and all that good
jazz."*

- 🤖 confirmed the official minimum spec, worked out that AVX1 is the actual ceiling both listed
  CPUs share (Sandy Bridge and Piledriver both predate AVX2/FMA3), and set `/arch:AVX`, LTCG,
  speed-favored codegen, and fully stripped Release debug info — verified by an actual clean
  rebuild each time, not just editing XML and hoping.
- 🧑 asked a sharp follow-up: *"I noticed it's /MT and not /MD does that matter? Technically an
  ASI is a DLL, right?"* — 🤖 explained why static-linking the CRT is specifically correct for a
  DLL injected into someone else's process (no redistributable dependency, no shared-CRT-heap
  hazard with other `.asi` mods in the same address space).
- 🧑 then asked for `Logger` to compile out of Release entirely — 🤖 made `Logger.h` collapse to
  inline no-ops outside `_DEBUG` and excluded `Logger.cpp` from the Release build, so every
  existing call site kept working unchanged.

None of this touched *size* directly yet. It set up the next question.

## Chapter 1 — Building the Instrument

🧑 asked for a new build configuration, **`Analyize`** (🧑's spelling, kept deliberately) — same
codegen as Release, but with every `.obj`/`.asm`/`.map` artifact preserved instead of discarded,
specifically so the binary could actually be inspected rather than guessed about.

- 🤖 added the config to both the `.vcxproj` and `.sln` (mapping the vendored MinHook submodule,
  which has no `Analyize` config of its own, onto its existing `Release|x64`), enabled
  `/FAs` + a linker `.map` file, and verified a clean build produced real `.asm` listings
  alongside the `.obj`s.
- This is the config every measurement from here on comes from. Without it, everything that
  follows would have been vibes.

## Chapter 2 — What's Actually In There

🧑, unimpressed by a guess, asked directly: *"Tell me what is taking up 137KBs of space?"*

- 🤖 parsed the `.map` file's full symbol table by hand (address deltas between consecutive
  publics, aggregated per source object) instead of asserting a cause, and got a real answer:
  **only ~4KB of the 76KB `.text` section was this project's own code.** Everything else was
  statically-linked CRT machinery that came along as a package deal:
  - **13.2KB** — the C++ exception unwinder (`libvcruntime:frame.obj`) alone, pulled in because
    `std::vector`/`std::mutex` *could* throw, even though nothing in the codebase ever caught
    anything.
  - **13KB** — locale name↔LCID lookup tables, the single biggest chunk in the whole binary,
    for a mod that never calls a locale-aware function on purpose.
  - **6.2KB** — floating-point `log`/`log10` conversion tables. Nothing here does floating-point
    math.
  - **3.5KB** — C++ name-demangling tables. **3.5KB** — `<system_error>` string tables, pulled in
    by `std::mutex`.
  - **~5.5KB** — MinHook's actual hooking engine (`MH_CreateHook`, the x64 disassembler). This
    part, at least, was real.
- 🧑 followed up: *"Is all of it C++ cruft from the runtime?"* — 🤖 gave the honest breakdown
  instead of a flat yes: roughly 40KB pure CRT waste, ~5.6KB of stubborn EH residue, and ~20KB
  that was either the actual mod or mandatory PE/Windows bookkeeping (import table, `.pdata`,
  section headers) that no amount of C++ cleanup would ever remove.

## Chapter 3 — Cutting the Real Fat

🧑 opened a dedicated branch and set a number: *"We're striving for smallest binary here. Ideal
would be 3KBs."* 🤖 committed the already-finished `Analyize` work to `master` first, then
branched `size-optimization` off it to keep the experimental work isolated.

- 🤖 replaced `std::vector<PatternByte>` in `PatternScan.cpp` with a fixed 64-byte stack array
  (every real AOB pattern in the repo is well under that), and `std::mutex` in `PatchWorker.cpp`
  with a raw `SRWLOCK` — removing the only two throwing, heap-touching constructs in the whole
  active build.
- 🤖 disabled RTTI and C++ exception handling project-wide (`/GR-`, no `/EHsc`) now that nothing
  could actually throw, and pointed the linker's raw PE entry point straight at `DllMain` via
  `/ENTRY:DllMain` — skipping `_DllMainCRTStartup`'s locale/argv/mbcs initialization chain
  entirely. `NativeSigHook`'s constructor got marked `constexpr` so its one static instance
  stayed constant-initialized rather than depending on a CRT dynamic-init pass that no longer ran.
- **Result: 140,288 → 96,256 bytes (−31%).** `frame.obj` alone dropped from 13.2KB to 5.6KB, and
  the `<system_error>` tables vanished completely.
- Along the way: overriding the entry point turned out to silently disable MSVC's implicit
  default-library selection — the very next build failed with `strtoul`/`malloc`/
  `_CxxThrowException` all unresolved, fixed only by explicitly listing
  `libcmt.lib;libucrt.lib;libvcruntime.lib;kernel32.lib`. And `strtoul` itself got replaced with
  an eight-line hand-rolled hex-nibble parser on the theory that UCRT's locale-aware, `strtod`-
  sharing implementation was the reason locale/log tables kept surviving — that theory was mostly
  **wrong**: it only saved 1.5KB, proving the real anchor was somewhere else.

## Chapter 4 — The Rabbit Hole, Called Off

🤖 went looking for that real anchor: dumped `main.asm` and found MinHook's `MH_Uninitialize`
(`HeapFree`/`HeapDestroy`/`VirtualFree`) fully inlined straight into `DllMain`, then started
tracing whether MinHook's own `HeapAlloc` calls in `hook.c` were what kept the whole UCRT
locale/log-table cluster alive — checking `buffer.c`/`hook.c`/`trampoline.c` for allocator calls
mid-investigation.

🧑 cut it off directly:

> *"I think we're doing down a rabbit hole. Disregard this, switch back to the main branch."*

- 🤖 committed the in-progress state as an explicit WIP (still landing 96,256 bytes, still not
  runtime-verified) so nothing was lost, then returned to `master` clean. No further forensic
  digging happened on that thread — the next idea made it moot anyway.

## Chapter 5 — The Nuclear Option: Pure MASM

🧑's actual insight: MinHook exists to disassemble a target function and build a trampoline back
to the original — infrastructure this mod never uses, since `StowWeaponsHook` deliberately never
calls the original. All it really needs is *find the address, write one `RETN` byte.*

> *"Why don't we just use MASM? All we need to do is resolve the address, and byte patch with
> RETN, MinHook is really only for debugging... Start a tangent branch. Create a wholy new MASM
> project. Simple DllMain, that all it does is wait 2 seconds ..., then writes to the absolute
> offset of RDR2.EXE+89EE14 with RETN."*

- 🤖 branched `masm-tangent` off `master` and wrote a genuinely from-scratch project: a hand-written
  x64 `DllMain` that spawns a thread, sleeps 2 seconds, resolves the module base via
  `GetModuleHandleA`, flips the target page writable, writes `0xC3`, and restores the original
  protection — four raw `kernel32` calls, zero CRT, zero MinHook. Correct x64 calling-convention
  bookkeeping (shadow space, 16-byte stack alignment at every `call`, a non-volatile register to
  survive calls) done by hand.
- **Result: 2,560 bytes**, first build, no further tuning. `dumpbin` confirmed exactly the four
  expected imports and the correct entry point — nothing hidden, nothing extra.
- The comparison this produced: 140,288 → 96,256 (still MinHook) → **2,560 (no MinHook, no
  CRT)**. MinHook plus the CRT it drags in was **~98% of the original binary.**
- Flagged honestly, not glossed over: this version has none of `PatchWorker`'s pattern-scan
  safety net (blind fixed delay, hardcoded RVA, no verification before patching), no SEH unwind
  info on the hand-written prologues, and was never actually injected into a running game.

## Chapter 6 — Section Merging, and a Bug Caught Just in Time

🧑, still not done: *"Crazy idea. Can we merge sections to reduce size?"*

- 🤖 checked the section layout first: three sections, each burning a full 512-byte
  `FileAlignment` block regardless of content — the 4-byte `g_oldProtect` variable alone was
  costing an entire 512-byte block by itself. Applied `/MERGE:.rdata=.text /MERGE:.data=.text`.
- The first build **looked like a clean win** — 2,048 bytes — but a `dumpbin` check before
  declaring victory caught a real bug the merge had silently introduced: `.text` came out
  `Execute Read` with **no Write**, and `g_oldProtect` — written to at runtime by `VirtualProtect`
  itself as its `lpflOldProtect` out-parameter — was now sitting on a read-only page. Left as-is,
  the very first patch attempt in-game would have access-violated the host process.
- 🤖 fixed it with `/SECTION:.text,RWE`, rebuilt, and re-verified the section was genuinely
  writable and all four imports were still intact before reporting the number.
- **Result: 2,048 bytes**, confirmed correct this time.

## Chapter 7 — Poking the Floor

🧑 was suspicious of the round number: *"2048 sounds supcicously like a file minimum limit or
something."*

- 🤖 showed the actual arithmetic — `SizeOfHeaders` (1024) + `.text` (512) + `.idata` (512), all
  multiples of the 512-byte `FileAlignment` in use, not a PE-format floor — then tested the claim
  directly rather than just asserting it: `/ALIGN:32`, rebuilt, **1,248 bytes**. It worked
  mechanically.
- It also came with the linker's own warning: `LNK4108: /ALIGN specified without /DRIVER; image
  may not run`. `/ALIGN` below the CPU page size is a kernel-driver mechanism, not a documented,
  supported path for a usermode DLL loaded via `LoadLibrary`. 🤖 reverted it immediately rather
  than ship a number the toolchain itself was flagging as unreliable — 800 bytes wasn't worth
  gambling on undocumented loader behavior in someone else's game process.
- Final, verified state: **2,048 bytes**, exactly matching what was already committed.

---

## What actually exists now

- **`master`** — the real mod, unchanged: MinHook, AOB pattern-scan-and-retry, `Logger` gated to
  Debug builds. Ships on the safer, verified approach for good reason.
- **`size-optimization`** — parked mid-investigation at 96,256 bytes (−31%). CRT-trimmed but
  still MinHook-based; the custom-entry-point change in particular was never runtime-verified.
- **`masm-tangent`** — 2,048 bytes (−98.5%). No CRT, no MinHook, no safety net. A genuine proof
  that MinHook plus the C++ runtime was nearly the entire original binary — not a replacement for
  the real mod.

| Build | Size | vs. original |
|---|---|---|
| C++ Release (MinHook + full CRT) | 140,288 B | — |
| `size-optimization` (CRT-trimmed, still MinHook) | 96,256 B | −31% |
| `masm-tangent` (no MinHook, no CRT, sections merged) | 2,048 B | −98.5% |

## Retrospective

- **The instrument came before the answer.** Nothing in Chapter 2 onward would have been
  anything but a guess without the `Analyize` config's `.map`/`.asm` output built first —
  📏 measure, don't assert, held all the way through.
- **A theory got tested and found mostly wrong, on purpose.** The `strtoul`-as-root-cause guess
  in Chapter 3 only bought 1.5KB against a predicted double-digit-KB saving — reported as a
  miss, not quietly dropped.
- **🧑 called off a live investigation before it produced a result.** Chapter 4's `/verbose:lib`
  dig into MinHook's allocator usage was heading toward diminishing returns and got cut before it
  went anywhere — the WIP was preserved, not discarded, but no false conclusion got drawn from an
  unfinished thread.
- **The single biggest idea — dropping MinHook entirely — was 🧑's, not the AI's.** The whole
  98%-of-the-binary result in Chapter 5 followed directly from recognizing that a hooking engine
  with a disassembler and trampoline builder was pure overhead for a hook that never calls its
  original.
- **A measured "win" got caught being wrong before it shipped.** Chapter 6's section merge looked
  like a clean 2,048-byte success on first build; `dumpbin` before-the-fact, not player crash
  reports after-the-fact, is what caught the read-only-page bug.
- **Knowing where to stop was as deliberate as the optimization itself.** Chapter 7 had a working
  path to 1,248 bytes and walked away from it because the toolchain's own warning said it
  shouldn't be trusted — the same discipline that kept `size-optimization` and `masm-tangent` off
  `master` in the first place.
