; This is a test program for the 6502 Emulator.
; It will copy the value 0x1C to every address
; from VRAM - VRAM+0xFF

.def VRAM 0x4000
.def ILLEGAL_OPCODE 0xFF

.RST
  ldx #$00      ; initialize counter variable
  lda #$1C      ; initialize template variable

.HEAD
  sta VRAM, x   ; copy the template byte to VRAM + counter
  cpx #$FF      ; if the counter variable has reached 0xFF
  beq .END      ; we jump to the end
  inx           ; increment the counter
  jmp .HEAD     ; jump back to the loop condition
.END
  nop
  jmp .END      ; endless loop for testing purposes
