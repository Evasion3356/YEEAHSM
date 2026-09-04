; ============================================================================
; YEEAHSM-MASM -- minimal byte-patch experiment, no MinHook, no CRT.
;
; Waits WAIT_MS for RDR2.exe to finish unpacking, then overwrites the first
; byte of sub_14089EE14 (StowWeapons, RVA 0x89EE14) with a bare RETN (0xC3).
; This is the exact "manual RETN patch" the main YEEAHSM project's history
; already proved live -- same effect as StowWeaponsHook's MinHook-based
; discard, just applied with three raw kernel32 calls instead of a hooking
; engine that has to disassemble the target and build a trampoline.
;
; Built purely to measure how much of the release binary MinHook (plus the
; C++ runtime it pulls in) actually accounts for. Fixed 2s delay instead of
; the main project's pattern-scan retry loop is a deliberate simplification
; for this experiment, not a recommendation for the real mod -- there's no
; verification the target bytes are what we expect before patching them.
; ============================================================================

PAGE_EXECUTE_READWRITE equ 40h
STOW_RVA                equ 89EE14h
WAIT_MS                 equ 2000

.data
    align 4
    g_oldProtect dd 0

.code

extrn Sleep:proc
extrn GetModuleHandleA:proc
extrn VirtualProtect:proc
extrn CreateThread:proc

; DWORD WINAPI WorkerThread(LPVOID) -- runs on its own thread so DllMain
; never blocks while the loader lock is held.
WorkerThread proc
    push rbx
    sub rsp, 20h                       ; shadow space, keeps rsp 16-aligned

    mov ecx, WAIT_MS
    call Sleep

    xor ecx, ecx                       ; GetModuleHandleA(NULL) -> RDR2.exe base
    call GetModuleHandleA
    test rax, rax
    jz WorkerThread_done

    lea rbx, [rax + STOW_RVA]           ; rbx = target address (nonvolatile: survives the calls below)

    lea r9, g_oldProtect                ; lpflOldProtect
    mov r8d, PAGE_EXECUTE_READWRITE
    mov edx, 1                          ; dwSize
    mov rcx, rbx                        ; lpAddress
    call VirtualProtect

    mov byte ptr [rbx], 0C3h            ; RETN

    mov r8d, dword ptr [g_oldProtect]    ; flNewProtect = whatever it was before
    lea r9, g_oldProtect                 ; lpflOldProtect (value discarded, just needs to be valid)
    mov edx, 1
    mov rcx, rbx
    call VirtualProtect

WorkerThread_done:
    xor eax, eax
    add rsp, 20h
    pop rbx
    ret
WorkerThread endp

; BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
; RCX=hinstDLL RDX=fdwReason R8=lpvReserved
PUBLIC DllMain
DllMain proc
    cmp edx, 1                          ; DLL_PROCESS_ATTACH
    jne DllMain_ret

    sub rsp, 38h                        ; shadow space (20h) + 2 stack args (10h), 16-aligned
    mov qword ptr [rsp+28h], 0          ; lpThreadId = NULL
    mov dword ptr [rsp+20h], 0          ; dwCreationFlags = 0
    xor r9d, r9d                        ; lpParameter = NULL
    lea r8, WorkerThread                ; lpStartAddress
    xor edx, edx                        ; dwStackSize = 0 (default)
    xor ecx, ecx                        ; lpThreadAttributes = NULL
    call CreateThread
    add rsp, 38h

DllMain_ret:
    mov eax, 1                          ; TRUE
    ret
DllMain endp

end
