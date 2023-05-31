; ; stubcode.s
; .386
; .model flat

; .code

; public HANDLER_PreCall
; public SaveRegsAndJmpBack

; extern g_regs:DWORD

; HANDLER_PreCall PROC FAR:
; 	;
; 	; stack map
; 	;
; 	; push &node
; 	; push address of HANDLER_xxx
; 	;

;     mov DWORD PTR [g_regs], eax

; 	; pop eax					; address of HANDLER_xxx
; 	; pop g_node

; 	; mov g_regs.ecx, ecx

; 	; mov ecx, ss:[esp]
; 	; mov g_regs.ret, ecx		; address of caller

; 	; mov g_regs.edx, edx
; 	; mov g_regs.ebx, ebx
; 	; mov g_regs.esp, esp		; now in here, the address is same to raw
; 	; mov g_regs.ebp, ebp
; 	; mov g_regs.esi, esi
; 	; mov g_regs.edi, edi

; 	; jmp eax
; HANDLER_PreCall ENDP

; SaveRegsAndJmpBack PROC:
;     mov eax, g_regs.eax
;     mov ecx, g_regs.ecx
;     mov edx, g_regs.edx
;     mov ebx, g_regs.ebx
;     mov esp, g_regs.esp
;     mov ebp, g_regs.ebp
;     mov esi, g_regs.esi
;     mov edi, g_regs.edi
;     jmp g_execBuffer
	
; SaveRegsAndJmpBack ENDP