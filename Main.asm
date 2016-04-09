.include "Base.inc"
.include "Spritelib.inc"

; -----------------------------------------------------------------------------
.section "SetupMain" free
; -----------------------------------------------------------------------------
  SpritePalette:
    .include "Data\Sprite-Palette.inc"
  SpritePaletteEnd:
  BackgroundPalette:
    .db $35

SetupMain:
  ld a,0
  ld b,1
  ld hl,BackgroundPalette
  call LoadCRam
  ld a,16
  ld b,SpritePaletteEnd-SpritePalette
  ld hl,SpritePalette
  call LoadCRam

  ld a,ENABLE_DISPLAY_ENABLE_FRAME_INTERRUPTS_NORMAL_SPRITES
  ld b,1
  call SetRegister
  ei
  call AwaitFrameInterrupt
  jp Main
.ends


; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
.struct Object
  Status db
  Y db
  X db
  MetaSpritePointer dw
.endst

.ramsection "Main variables" slot 3
  Swabby instanceof Object
  FrameCounter db
.ends
; -----------------------------------------------------------------------------
.section "Main" free
; -----------------------------------------------------------------------------
  SwabbyInitString:
    .db 1 20 20
    .dw SwabbyFlying1

  SwabbyFlying1:
    .db 6
    .db 0 0 0 8 8 8
    .db 0 0 8 1 16 2 0 3 8 4 16 5
  SwabbyFlying1Tiles:

  Main:
    call AwaitFrameInterrupt

    call LoadSAT

    call GetInputPorts
    ld hl,FrameCounter
    inc (hl)
  jp Main

.ends
