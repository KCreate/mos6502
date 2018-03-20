; This is a test program for the 6502 Emulator.
; It will copy the value 0x1C to every address
; from VRAM - VRAM+0xFF

.def VRAM 0x4000
.def ILLEGAL_OPCODE 0xFF

.RST
  LDX #$00      ; initialize counter variable
  LDA #$1C      ; initialize template variable

.HEAD
  STA VRAM, X   ; copy the template byte to VRAM + counter
  INX           ; increment the counter

  CPX #$FF      ; if the counter variable has reached 0xFF
  BEQ .END      ; we jump to the end
  JMP .HEAD     ; jump back to the loop condition

.END
  ILLEGAL_OPCODE
