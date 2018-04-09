; this program beeps audio channel 1

; some commonly used addresses
.def ADDR_CLOCK1 0x4906
.def ADDR_AUDIO1 0x4908

.RST

  ; setup the audio channel
  lda #$D1
  sta ADDR_AUDIO1

  ; setup toggle byte
  lda #$18
  sta TOGGLE

  ; setup the clock
  lda #$16
  sta ADDR_CLOCK1

  ; loop
.LOOP
  nop
  jmp .LOOP

.IRQ
  lda ADDR_AUDIO1
  ldx TOGGLE
  sta TOGGLE
  txa
  sta ADDR_AUDIO1
  rti

  ; used to store the last value inside audio channel 1
.org 0x00
  .BYTE TOGGLE
