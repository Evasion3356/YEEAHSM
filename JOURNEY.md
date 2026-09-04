# The YEEAHSM Journey

How a "keep your weapons on you" RDR2 mod actually got built — from first principles, through
two confirmed dead ends, one AI mistake caught by evidence, and a live-debugger breakthrough.

**Legend:** 🧑 = gir489 (human) &nbsp;·&nbsp; 🤖 = Claude Code (AI)

Every non-trivial step below is tagged with who actually did it. Where it's not obvious, the
narrative says so explicitly. Nothing here is smoothed over — the wrong turns are left in,
including the AI's.

---

## Prologue: The Ask

🧑 wanted a mod to stop RDR2 from auto-stowing Arthur's weapons onto his horse — something every
existing Nexus mod either didn't do at all, or did in a way that caused other problems. 🧑 had
prior native-hacking experience (having built HorseMenu, an RDR2 cheat
menu), and the original working theory was to hook whatever native removes the weapon on mount
state change.

🤖's first response was a plan, not code: several architectural options (block-at-source vs.
snapshot-and-restore the loadout), before either of us had any evidence for which native — if
any — was actually responsible.

## Chapter 1 — Building the Harness

🧑 redirected: before chasing the real bug, prove the tooling works on something known-good.

- 🤖 built the initial scaffold: a `Logger`, an AOB `PatternScan`er, and a project wired against
  the ScriptHookRDR2 SDK, with MinHook vendored as a proper git submodule (not a zip-copy) at
  🧑's request.
- 🤖 validated the pipeline on **known-good** targets first — hooking `nativeInit`/`nativePush64`/
  `nativeCall`, exported directly by `ScriptHookRDR2.dll`, resolvable via plain `GetProcAddress`
  with zero guessing involved.
- 🧑 supplied the first real IDA signature for an *internal engine* function: `GET_PLAYER_PED`,
  including the exact prologue bytes and the decompiled body.
- 🤖 built `PlayerPedHook` from that signature — the first hook resolved via AOB pattern scan
  rather than an export table.
- 🧑 supplied a second signature, `DOES_ENTITY_EXIST`, specifically to cross-check the pipeline
  against a native with a different argument shape.
- 🧑 tested in-game: entity `0` correctly never existed; real handles correctly returned `1`.
  Harness proven.

## Chapter 2 — Five Suspects, One Negative Result

🧑 supplied IDA signatures for all five natives that looked like plausible candidates for
"strip the player's weapon": `REMOVE_WEAPON_FROM_PED`, `REMOVE_ALL_PED_WEAPONS`,
`HOLSTER_PED_WEAPONS`, `HIDE_PED_WEAPONS`, `GIVE_WEAPON_TO_PED`.

- 🤖 noticed the boilerplate was about to be copy-pasted five times and generalized it into
  `NativeSigHook<Ret, Args...>` — a small template wrapping MinHook create/enable/remove behind
  a signature-typed interface.
- 🤖 wired up all five hooks.
- 🧑 tested by mounting/dismounting a horse with weapons drawn: **none of the five fired.** A
  clean negative result — useful precisely because it ruled out five plausible-looking natives
  in one pass.

## Chapter 3 — The Instant-Store Flag (and an AI Mistake)

🤖 went looking through the (user-refreshed) community NativeDB comments and found
`_GET_LONGARMS_INSTANTLY_STORE_ON_DISMOUNT` / `_SET_INSTANTLY_STORE_LONGARMS_ON_DISMOUNT`,
documented as reading/writing an internal ped flag tied to "store longarm on horse near a safe
zone."

- 🤖 proposed forcing this flag on, to make its effect deterministic for testing — but picked the
  **wrong hash** (`0x641351E9AD103890`) based only on it being called in the same script tick as
  a plausible eligibility check, never having actually seen its decompiled behavior.
- 🧑 caught it — not from a hunch, but by actually checking in IDA: 0x641351E9AD103890's real
  decompiled function was doing something completely unrelated to what had been asked about.
  Separately, 🧑 confirmed the two natives already suspected as correct
  (`_GET_LONGARMS_INSTANTLY_STORE_ON_DISMOUNT` / `_SET_INSTANTLY_STORE_LONGARMS_ON_DISMOUNT`)
  were genuinely doing bit-bang math on a pointer dereference — matching the documented
  "selects one of two internal bits" behavior.
- 🤖 acknowledged the error directly: the call-site correlation was a hypothesis, not a
  verification, and the real setter takes 3 args, not the 2 the wrong hash's call site showed.
- 🧑 supplied the full decompile of both the real `GET` and `SET` functions, confirming they
  manipulate two packed bits at `WeaponComponent+0x1B5`.
- 🤖 built the hook — including a small trick where the `GET` pattern was deliberately cut to end
  exactly at a `CALL` opcode, letting the code decode that call's `rel32` displacement at runtime
  to recover the address of the shared "resolve ped to WeaponComponent" helper *without* needing
  a second IDA signature — then printed the live, ASLR-correct bit address on every call for a
  Cheat Engine breakpoint.

## Chapter 4 — Finding the Real Gate

- 🧑 used that printed address to set a live memory breakpoint and found the actual gating
  instruction directly, live, in the debugger: `RDR2.exe+9E29A6`.
- 🧑 supplied the full decompiled body of the containing function — a genuinely large one,
  `sub_1409E2828` — including the exact line:

  ```c
  v26 = *(BYTE*)(v13 + 397); // 397 == 0x1B5
  v98 = (v26 & 6) != 0 || (!a9 && (v26 & 8) != 0);
  ```

  `v98` gated the entire weapon-relocation block later in the function.

- 🤖 matched the ASM 🧑 found (`mov al,[rsi+0x18D]` / `test al,6` / `jne`) to that exact C line,
  and proposed patching the 6-byte `mov` to `xor al,al` + 4 NOPs — same effect as zeroing the
  byte, but resolved via pattern scan (ASLR-safe) instead of a hardcoded address.
- 🤖 built `BytePatch` (a generic AOB-resolved code patcher, distinct from `NativeSigHook` — this
  edits an instruction in place rather than redirecting control flow) and
  `DismountWeaponStripPatch`.
- 🧑 tested: **dismount weapon-stripping fixed.** ✅

## Chapter 5 — A Second, Unrelated Bug

🧑 reported new behavior: entering camp *also* stows weapons — clearly a second bug, since the
dismount patch was already active.

- 🤖 did script archaeology across ~580 decompiled RDR2 scripts (both a retail build and a
  developer build with surviving debug strings) and found `player_camp.c`'s `func_14()` /
  `func_69()`, which forces both weapon hand-slots to `WEAPON_UNARMED` every tick while camp is
  active.
- 🧑 pushed back, correctly skeptical: *"wasn't that the make-camp-anywhere script, not the SP
  main camp?"*
- 🤖 re-investigated and found hard evidence the file governs the *real* gang hideout too —
  hardcoded strings for the gang's white-flag morale system, the camp stew pot, and dozens of
  `CAMP_RESUPPLY_*` entries tied to real story locations (Clemens Point, Van Horn, Strawberry).
  The script-side trigger was confirmed — but *what actually moves the weapon* was still
  unexplained.

## Chapter 6 — Two Dead Ends, Ruled Out Properly

Rather than guess, both remaining hypotheses were tested to a real conclusion instead of being
abandoned on a hunch:

- 🤖 built `CampScriptCheck`, gating a discard decision on `SCRIPT::HAS_SCRIPT_LOADED` /
  `SCRIPT::DOES_SCRIPT_EXIST("player_camp")`, called inline from inside the hook.
- 🧑 tested and read the resulting log together with 🤖: `HAS_SCRIPT_LOADED` was `false` on
  every single call, `DOES_SCRIPT_EXIST` was `true` on every single call — including during
  clearly unrelated ambient NPC activity. **Confirmed dead end**: neither native tracks
  "camp is currently active."
- 🧑 proposed a cleaner test: force the `WeaponComponent+0x1B5` `GET`/`SET` pair to always
  read/write `0`, directly testing whether camp shares the dismount mechanism.
- 🤖 implemented it on an isolated git branch (`experiment/force-longarms-flag`).
- 🧑 tested: **camp still stowed weapons.** Ruled out — camp does not depend on the dismount flag
  at all, despite living in the same function family.
- In parallel, 🧑 asked for a deep dive correlating `shop_camp_butchertable`/`shop_camp_lockbox`
  scripts — 🤖 confirmed these are real, working donation scripts sharing a *separate* "no
  weapons near a shopkeeper" system, unrelated to camp arrival specifically, but useful
  confirmation of the shared-include architecture 🧑 had theorized about (RDR2 scripts inline
  parent/grandparent includes at compile time, GTA5-style).

## Chapter 7 — Back to Basics

After two confirmed dead ends, 🧑 refocused hard:

> *"We need to figure out how arthur is being told to stow the weapons on the horse, wasn't
> that the original ask? Figure that out."*

- 🤖 went back to `func_14()` and noticed a third call that had been glossed over the first time
  around: `TASK::TASK_SWAP_WEAPON` — completely undocumented in the native DB, every parameter
  typed `Any`.
- 🤖 surveyed **every** `TASK_SWAP_WEAPON` call site across the entire script corpus and found
  its 4th and 5th parameters are `false` at literally every call site, with zero exceptions — a
  real signal that whatever they gate is never touched by script code.
- 🧑 supplied the decompile of the real handler, `sub_141090A84`.
- 🤖 traced it and correctly identified it as task-*dispatch* plumbing (queue/start the task,
  either via a fast direct path or the generic named-task system) — not the actual
  body/horse-vs-decision logic, which had to live deeper, inside the task object itself.
- 🤖 proposed a different angle: use IDA's **Xrefs** on `_ATTACH_WEAPON_TO_HORSE_HOLSTER`
  directly. Its documented behavior ("visually attaches the specified weapon to a horse
  holster/rack") matches exactly what we were chasing, and a full-repo script search had already
  proven zero scripts call it — meaning any caller IDA found had to be internal engine code, with
  no script-usage noise to filter through.

## Chapter 8 — The Breakthrough

This one was 🧑's, entirely.

Re-reading the *already-found* `sub_1409E2828` more carefully — the same function from Chapter
4 — 🧑 spotted a call inside the `v98`-gated block that hadn't been chased down yet:

```c
sub_14089EE14(v20, v41, (*(DWORD*)(v40 + 2352) >> 11) & 1, v12, 1); // RETN this function
```

🧑 patched it live in the debugger to `retn` immediately and reported back:

> *"That function I RETN out, if I do that, arthur never stows his weapons on the horse."*

- 🤖's role at this point was recognizing **why** this resolved the earlier contradiction: if
  `sub_1409E2828`'s `v98` gate had already been neutered (Chapter 4) and camp stowing was *still*
  happening (Chapter 5), then whatever moved the weapon couldn't be exclusively reached through
  that gate. `sub_14089EE14` being independently effective proved it: it's a **shared utility**,
  reached by dismount through the (already-patched) gate *and* by camp through a completely
  different, still-unmapped path — explaining cleanly why neither prior flag-based fix touched
  camp behavior.
- 🧑 supplied the IDA prototype and AOB pattern for `sub_14089EE14`.
- 🤖 built `StowWeaponsHook` — first observation-only (log call count, every argument, and the
  caller's real return address, since this function is reached via a direct internal call rather
  than the native-dispatch table, making `retaddr` genuinely useful for once), per 🧑's explicit
  "let's see how many times it's called" direction.
- 🧑 then directed the hook to stop calling through entirely, matching the manual RETN test.
- 🧑 asked for return addresses printed as `RDR2.exe+0x<offset>` instead of raw absolute
  addresses (useless under ASLR) — 🤖 added `PatternScan::ToModuleOffset`.
- 🧑 directed final cleanup: retire `DismountWeaponStripPatch` entirely. Patching the shared
  function covers every caller — dismount, camp, anything else — in one place, making the
  gate-side patch redundant.

## Chapter 9 — Naming, and Getting the Engineering Right

**Naming.** 🧑 wanted a recursive backronym in the LAME/YAML tradition. 🤖 proposed several
options (`ARTHUR`, `NOPE`, `MICAH`); 🧑 steered toward a "Yet Another X" / western "yeehaw"
flavor instead. 🤖 proposed `YEEHAW` = *"Yer Equipment's Extra Holsterin' Ain't Wanted."* 🧑
landed on **YEEAHS** — *Yet Another Horse Stow mod* — then extended it one more letter to
**YEEAHSM** to fold "Mod" into the acronym properly, noting that said fast it comes out
"yeehasm," which lands close enough to "yes'em" to be the actual final name.

**The size investigation.** 🧑 flagged the shipped `.asi` as absurdly large for "one hook"
(~319KB) and demanded real verbose-build evidence, not a guess:

> *"turn on the most verbose output ASM/OBJ Visual Studio has, and figure out what the fuck is
> taking up so much space."*

- 🤖 located the actual MSVC toolchain on the machine, ran a real `Release|x64` build with
  `/VERBOSE` linker output and a `.map` file, and parsed the results directly rather than
  asserting a cause.
- The data: `Logger.obj` alone carried 839 symbols — more than double every other object file
  combined — and the next dozen heaviest contributors were all locale/facet/exception-formatting
  CRT internals pulled in *transitively* because of it.
- 🤖 explained the root cause precisely: every `std::basic_ios`-derived stream (`ofstream`,
  `ostringstream`) carries a `std::locale`; facet lookup is dynamic and RTTI-based (so a program
  *could* `imbue()` a different locale at runtime), which means the linker can't prove any facet
  is dead code and has to keep the entire locale/facet/codecvt/`ios_base::failure` object graph
  linked in — a well-documented, architectural cost of `<iostream>` specifically, not a mistake.
- 🤖 proposed and implemented a `<cstdio>`-based rewrite, measuring a real **32.6% size
  reduction** (319KB → 215KB) as proof, not just a claim.
- 🧑 pushed back — twice. First on swapping `localtime_s` for the Win32-only `GetLocalTime()`
  (leaves the C++ standard for a small extra saving); then on the whole `<cstdio>` swap itself:

  > *"You're breaking C++ paradigm to reduce size... I want to maintain strict adherence to the
  > C++ paradigm, using C runtime function calls is stupid to save space. I just want to know
  > what's taking up so much space."*

- 🤖 reverted both changes completely, back to `std::ofstream`/`std::ostringstream`/`std::chrono`
  throughout, and delivered the explanation on its own, without the code changes attached — 🧑's
  engineering-values call to make, not the AI's to make for them.

---

## What actually shipped

- **`StowWeaponsHook`** — hooks `sub_14089EE14` and discards every call, fixing both the
  dismount and camp-arrival weapon-stow behavior from a single patch point.
- **`PatchWorker`** — a background retry thread handling the fact that RDR2's executable is
  still unpacking/deobfuscating itself when an ASI loader typically injects; retries every
  500ms for up to 2 minutes rather than assuming the first attempt works.
- **`PatternScan`** / **`NativeSigHook`** — the general-purpose AOB scanner and MinHook wrapper
  built up over the course of the investigation, still around for whoever picks this codebase up
  next.
- A `std::ofstream`-based logger, kept exactly as idiomatic C++ as `std::iostream` allows —
  deliberately, at a measured and accepted cost in binary size.

## Retrospective

- **Two dead ends were confirmed, not guessed past.** `CampScriptCheck` and the forced-flag
  experiment both had real hypotheses, real tests, and real negative results — that discipline
  is why Chapter 7's "go back to first principles" note actually had somewhere useful to land.
- **The one real AI mistake** (the wrong native hash in Chapter 3) was caught because 🧑 insisted
  on evidence from the actual decompile over a plausible-looking script correlation — a pattern
  that repeated at every major turn in this project.
- **The actual breakthrough was 100% human.** Live RETN-patch testing in a debugger, on a
  function found by re-reading earlier work more carefully — no amount of static analysis or
  script archaeology got there on its own. AI's contribution at that moment was recognizing
  *why* it resolved the standing contradiction and turning a one-session manual patch into a
  permanent, safe, removable hook.
- **The size tradeoff was a deliberate, informed human decision**, made after being handed the
  real technical cost rather than a vague tradeoff — and it stood, even against a measured 32.6%
  size reduction on the table.
