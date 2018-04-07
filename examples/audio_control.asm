; this example program plays a sound for 1 second at a time
; after each audio pulse it pauses for a second
; after the pause the audio will change it's wave form

; some commonly used addresses
.def ADDR_CLOCK1 0x4906
.def ADDR_AUDIO1 0x4908

.RST

  ; setup the audio channel
  lda #$C8
  sta ADDR_AUDIO1

  ; setup the 1 second clock
  lda #$64
  sta ADDR_CLOCK1

  ; loop
.LOOP
  nop
  jmp .LOOP

.IRQ

  ; change the wave function of the audio channel
  ;
  ; A      <- audio1
  ; X      <- A
  ; A      <- A & 0xCF
  ; TMP    <- A
  ; A      <- X
  ; A      <- A + 0x10
  ; A      <- A & 0x30
  ; A      <- A + TMP
  ; audio1 <- A
  lda ADDR_AUDIO1
  tax
  and #$CF
  sta TMP
  txa
  adc #$10
  and #$30
  adc TMP
  sta ADDR_AUDIO1
  rti

  ; used to store temporary results when changing wave function
.org 0x00
  .BYTE TMP
