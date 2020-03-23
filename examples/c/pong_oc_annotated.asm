.RST
    $4920    78        SEI
    $4921    20 79 49  JSR $4979
    $4924    a9 10     LDA #$10
    $4926    8d 06 49  STA $4906
    $4929    58        CLI

.L_MAIN_LOOP
    $492a    ea        NOP
    $492b    4c 2a 49  JMP $492a

.L_IRQ
    $492e    ad 03 49  LDA $4903
    $4931    c9 01     CMP #$01
    $4933    f0 0b     BEQ $4940
    $4935    c9 06     CMP #$06
    $4937    f0 0d     BEQ $4946
    $4939    c9 08     CMP #$08
    $493b    f0 0f     BEQ $494c
    $493d    4c 52 49  JMP $4952

.L_IRQ_KEYDOWM
    $4940    20 83 49  JSR $4983
    $4943    4c 52 49  JMP $4952

.L_IRQ_CLOCK
    $4946    20 d4 49  JSR $49d4
    $4949    4c 52 49  JMP $4952

.L_IRQ_TIMER
    $494c    20 73 49  JSR $4973
    $494f    4c 52 49  JMP $4952

.L_IRQ_EXIT
    $4952    40        RTI

.L_START_SHORT_BEEP
    $4953    a9 d9     LDA #$d9
    $4955    8d 08 49  STA $4908
    $4958    a9 00     LDA #$00
    $495a    8d 11 49  STA $4911
    $495d    a9 0a     LDA #$0a
    $495f    8d 10 49  STA $4910
    $4962    60        RTS

.L_START_LONG_BEEP
    $4963    a9 d9     LDA #$d9
    $4965    8d 08 49  STA $4908
    $4968    a9 00     LDA #$00
    $496a    8d 11 49  STA $4911
    $496d    a9 1e     LDA #$1e
    $496f    8d 10 49  STA $4910
    $4972    60        RTS

.L_STOP_BEEP
    $4973    a9 19     LDA #$19
    $4975    8d 08 49  STA $4908
    $4978    60        RTS

.L_RESET_GAME
    $4979    a9 00     LDA #$00
    $497b    85 02     STA $02
    $497d    85 03     STA $03
    $497f    20 f0 49  JSR $49f0
    $4982    60        RTS

.L_HANDLE_KEYPRESS
    $4983    ad 04 49  LDA $4904
    $4986    85 08     STA $08
    $4988    c9 16     CMP #$16
    $498a    f0 0f     BEQ $499b
    $498c    c9 12     CMP #$12
    $498e    f0 19     BEQ $49a9
    $4990    c9 08     CMP #$08
    $4992    f0 23     BEQ $49b7
    $4994    c9 0a     CMP #$0a
    $4996    f0 2d     BEQ $49c5
    $4998    4c d3 49  JMP $49d3

.L_HANDLE_KEYPRESS_W
    $499b    a5 00     LDA $00
    $499d    c9 00     CMP #$00
    $499f    f0 32     BEQ $49d3
    $49a1    18        CLC
    $49a2    e9 01     SBC #$01
    $49a4    85 00     STA $00
    $49a6    4c d3 49  JMP $49d3

.L_HANDLE_KEYPRESS_S
    $49a9    a5 00     LDA $00
    $49ab    c9 1b     CMP #$1b
    $49ad    f0 24     BEQ $49d3
    $49af    18        CLC
    $49b0    69 01     ADC #$01
    $49b2    85 00     STA $00
    $49b4    4c d3 49  JMP $49d3

.L_HANDLE_KEYPRESS_I
    $49b7    a5 01     LDA $01
    $49b9    c9 00     CMP #$00
    $49bb    f0 16     BEQ $49d3
    $49bd    18        CLC
    $49be    e9 01     SBC #$01
    $49c0    85 01     STA $01
    $49c2    4c d3 49  JMP $49d3

.L_HANDLE_KEYPRESS_K
    $49c5    a5 01     LDA $01
    $49c7    c9 1b     CMP #$1b
    $49c9    f0 08     BEQ $49d3
    $49cb    18        CLC
    $49cc    69 01     ADC #$01
    $49ce    85 01     STA $01
    $49d0    4c d3 49  JMP $49d3

.L_HANDLE_KEYPRESS_EXIT
    $49d3    60        RTS

.L_HANDLE_CLOCK
    $49d4    20 a4 4a  JSR $4aa4
    $49d7    20 ff 49  JSR $49ff
    $49da    20 31 4a  JSR $4a31
    $49dd    20 b4 4a  JSR $4ab4
    $49e0    20 d4 4a  JSR $4ad4
    $49e3    20 c4 4a  JSR $4ac4
    $49e6    20 ec 4a  JSR $4aec
    $49e9    20 1d 4b  JSR $4b1d
    $49ec    20 2d 4b  JSR $4b2d
    $49ef    60        RTS

.L_RESET_POSITIONS
    $49f0    a9 20     LDA #$20
    $49f2    85 04     STA $04
    $49f4    85 05     STA $05
    $49f6    a9 01     LDA #$01
    $49f8    85 06     STA $06
    $49fa    a9 01     LDA #$01
    $49fc    85 07     STA $07
    $49fe    60        RTS

.L_UPDATE_BALL
    $49ff    a5 06     LDA $06
    $4a01    d0 0a     BNE $4a0d
    $4a03    a5 04     LDA $04
    $4a05    18        CLC
    $4a06    e9 01     SBC #$01
    $4a08    85 04     STA $04
    $4a0a    4c 17 4a  JMP $4a17

.L_UPDATE_BALL_X_INC
    $4a0d    a5 04     LDA $04
    $4a0f    18        CLC
    $4a10    69 01     ADC #$01
    $4a12    85 04     STA $04
    $4a14    4c 17 4a  JMP $4a17

.L_UPDATE_BALL_X_END
    $4a17    a5 07     LDA $07
    $4a19    d0 0a     BNE $4a25
    $4a1b    a5 05     LDA $05
    $4a1d    18        CLC
    $4a1e    e9 01     SBC #$01
    $4a20    85 05     STA $05
    $4a22    4c 30 4a  JMP $4a30

.L_UPDATE_BALL_Y_INC
    $4a25    a5 05     LDA $05
    $4a27    18        CLC
    $4a28    69 01     ADC #$01
    $4a2a    85 05     STA $05
    $4a2c    ea        NOP
    $4a2d    4c 30 4a  JMP $4a30

.L_UPDATE_BALL_Y_END
    $4a30    60        RTS

.L_CHECK_BALL_COLLISIONS
    $4a31    a5 05     LDA $05
    $4a33    c9 00     CMP #$00
    $4a35    d0 07     BNE $4a3e
    $4a37    a9 01     LDA #$01
    $4a39    85 07     STA $07
    $4a3b    20 53 49  JSR $4953

.L_CHECK_BALL_COLLISIONS_NO_TOP_HIT
    $4a3e    a5 05     LDA $05
    $4a40    c9 23     CMP #$23
    $4a42    d0 07     BNE $4a4b
    $4a44    a9 00     LDA #$00
    $4a46    85 07     STA $07
    $4a48    20 53 49  JSR $4953

.L_CHECK_BALL_COLLISIONS_NO_BOTTOM_HIT
    $4a4b    a5 05     LDA $05
    $4a4d    c5 00     CMP $00
    $4a4f    b0 16     BCS $4a67
    $4a51    a5 05     LDA $05
    $4a53    18        CLC
    $4a54    e5 00     SBC $00
    $4a56    c9 08     CMP #$08
    $4a58    90 0d     BCC $4a67
    $4a5a    a5 04     LDA $04
    $4a5c    c9 02     CMP #$02
    $4a5e    d0 07     BNE $4a67
    $4a60    a9 01     LDA #$01
    $4a62    85 06     STA $06
    $4a64    20 53 49  JSR $4953

.L_CHECK_BALL_COLLISIONS_NO_LEFT_HIT
    $4a67    a5 05     LDA $05
    $4a69    c5 01     CMP $01
    $4a6b    b0 16     BCS $4a83
    $4a6d    a5 05     LDA $05
    $4a6f    18        CLC
    $4a70    e5 01     SBC $01
    $4a72    c9 08     CMP #$08
    $4a74    90 0d     BCC $4a83
    $4a76    a5 04     LDA $04
    $4a78    c9 02     CMP #$02
    $4a7a    d0 07     BNE $4a83
    $4a7c    a9 00     LDA #$00
    $4a7e    85 06     STA $06
    $4a80    20 53 49  JSR $4953

.L_CHECK_BALL_COLLISIONS_NO_RIGHT_HIT
    $4a83    a5 04     LDA $04
    $4a85    c9 00     CMP #$00
    $4a87    d0 0d     BNE $4a96
    $4a89    a5 03     LDA $03
    $4a8b    18        CLC
    $4a8c    69 01     ADC #$01
    $4a8e    85 03     STA $03
    $4a90    20 5f 4b  JSR $4b5f
    $4a93    20 63 49  JSR $4963

.L_CHECK_BALL_COLLISIONS_INC_1_SCORE
    $4a96    a5 02     LDA $02
    $4a98    18        CLC
    $4a99    69 01     ADC #$01
    $4a9b    85 02     STA $02
    $4a9d    20 5f 4b  JSR $4b5f
    $4aa0    20 63 49  JSR $4963

.L_CHECK_BALL_COLLISIONS_EXIT
    $4aa3    60        RTS

.L_CHECK_SCORE
    $4aa4    a5 02     LDA $02
    $4aa6    c9 0a     CMP #$0a
    $4aa8    f0 06     BEQ $4ab0
    $4aaa    a5 03     LDA $03
    $4aac    c9 0a     CMP #$0a
    $4aae    f0 00     BEQ $4ab0

.L_CHECK_SCORE_RESET
    $4ab0    20 79 49  JSR $4979

.L_CHECK_SCORE_END
    $4ab3    60        RTS

.L_SET_COLOR_BLACK
    $4ab4    a9 00     LDA #$00
    $4ab6    8d 0c 49  STA $490c
    $4ab9    a9 80     LDA #$80
    $4abb    8d 0b 49  STA $490b
    $4abe    a9 81     LDA #$81
    $4ac0    8d 0b 49  STA $490b
    $4ac3    60        RTS

.L_SET_COLOR_WHITE
    $4ac4    a9 ff     LDA #$ff
    $4ac6    8d 0c 49  STA $490c
    $4ac9    a9 80     LDA #$80
    $4acb    8d 0b 49  STA $490b
    $4ace    a9 81     LDA #$81
    $4ad0    8d 0b 49  STA $490b
    $4ad3    60        RTS

.L_CLEAR_SCREEN
    $4ad4    a9 00     LDA #$00
    $4ad6    8d 0c 49  STA $490c
    $4ad9    8d 0d 49  STA $490d
    $4adc    a9 40     LDA #$40
    $4ade    8d 0e 49  STA $490e
    $4ae1    a9 24     LDA #$24
    $4ae3    8d 0f 49  STA $490f
    $4ae6    a9 00     LDA #$00
    $4ae8    8d 0b 49  STA $490b
    $4aeb    60        RTS

.L_DRAW_RACKETS
    $4aec    a9 01     LDA #$01
    $4aee    8d 0c 49  STA $490c
    $4af1    8d 0e 49  STA $490e
    $4af4    a5 00     LDA $00
    $4af6    8d 0d 49  STA $490d
    $4af9    18        CLC
    $4afa    69 08     ADC #$08
    $4afc    8d 0f 49  STA $490f
    $4aff    a9 03     LDA #$03
    $4b01    8d 0b 49  STA $490b
    $4b04    a9 3e     LDA #$3e
    $4b06    8d 0c 49  STA $490c
    $4b09    8d 0e 49  STA $490e
    $4b0c    a5 01     LDA $01
    $4b0e    8d 0d 49  STA $490d
    $4b11    18        CLC
    $4b12    69 08     ADC #$08
    $4b14    8d 0f 49  STA $490f
    $4b17    a9 03     LDA #$03
    $4b19    8d 0b 49  STA $490b
    $4b1c    60        RTS

.L_DRAW_BALL
    $4b1d    a5 04     LDA $04
    $4b1f    8d 0c 49  STA $490c
    $4b22    a5 05     LDA $05
    $4b24    8d 0d 49  STA $490d
    $4b27    a9 02     LDA #$02
    $4b29    8d 0b 49  STA $490b
    $4b2c    60        RTS

.L_DRAW_SCORE
    $4b2d    a9 00     LDA #$00
    $4b2f    8d 0c 49  STA $490c
    $4b32    a9 23     LDA #$23
    $4b34    8d 0d 49  STA $490d
    $4b37    a5 02     LDA $02
    $4b39    8d 0e 49  STA $490e
    $4b3c    a9 23     LDA #$23
    $4b3e    8d 0f 49  STA $490f
    $4b41    a9 03     LDA #$03
    $4b43    8d 0b 49  STA $490b
    $4b46    a9 00     LDA #$00
    $4b48    8d 0c 49  STA $490c
    $4b4b    a9 22     LDA #$22
    $4b4d    8d 0d 49  STA $490d
    $4b50    a5 03     LDA $03
    $4b52    8d 0e 49  STA $490e
    $4b55    a9 22     LDA #$22
    $4b57    8d 0f 49  STA $490f
    $4b5a    a9 03     LDA #$03
    $4b5c    8d 0b 49  STA $490b
    $4b5f    60        RTS

.L_RESET_BALL_POSITION
    $4b60    a9 20     LDA #$20
    $4b62    85 04     STA $04
    $4b64    85 05     STA $05
    $4b66    a9 01     LDA #$01
    $4b68    85 06     STA $06
    $4b6a    a9 01     LDA #$01
    $4b6c    85 07     STA $07
    $4b6e    60        RTS
