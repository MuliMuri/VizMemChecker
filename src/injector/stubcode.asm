; ; stubcode.s
; .386
; .model flat

; .code

; public HANDLER_PreCall
; public SaveRegsAndJmpBack

; extern g_context:DWORD

; HANDLER_PreCall PROC FAR:
; 	;
; 	; stack map
; 	;
; 	; push &node
; 	; push address of HANDLER_xxx
; 	;

;     mov DWORD PTR [g_context], eax

; 	; pop eax					; address of HANDLER_xxx
; 	; pop g_node

; 	; mov g_context.ecx, ecx

; 	; mov ecx, ss:[esp]
; 	; mov g_context.ret, ecx		; address of caller

; 	; mov g_context.edx, edx
; 	; mov g_context.ebx, ebx
; 	; mov g_context.esp, esp		; now in here, the address is same to raw
; 	; mov g_context.ebp, ebp
; 	; mov g_context.esi, esi
; 	; mov g_context.edi, edi

; 	; jmp eax
; HANDLER_PreCall ENDP

; SaveRegsAndJmpBack PROC:
;     mov eax, g_context.eax
;     mov ecx, g_context.ecx
;     mov edx, g_context.edx
;     mov ebx, g_context.ebx
;     mov esp, g_context.esp
;     mov ebp, g_context.ebp
;     mov esi, g_context.esi
;     mov edi, g_context.edi
;     jmp g_execBuffer
	
; SaveRegsAndJmpBack ENDP