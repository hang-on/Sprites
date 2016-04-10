.include "Base.inc"
.include "Spritelib.inc"
.include "Objectlib.inc"

; -----------------------------------------------------------------------------
.section "SetupMain" free
; -----------------------------------------------------------------------------
  SpritePalette:
    .include "Data\Sprite-Palette.inc"
  SpritePaletteEnd:

  SwabbyTiles:
    .include "Data/Swabby-Tiles.inc"
  SwabbyTilesEnd:

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

    ld hl,SwabbyTiles
    ld de,$2000
    ld bc,SwabbyTilesEnd-SwabbyTiles
    call LoadVRam

    ld a,ENABLE_DISPLAY_ENABLE_FRAME_INTERRUPTS_NORMAL_SPRITES
    ld b,1
    call SetRegister
    ei
    call AwaitFrameInterrupt
  jp Main
.ends

; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
.ramsection "Main variables" slot 3
  FrameCounter db
  PlayerObjectHandle db
.ends
; -----------------------------------------------------------------------------
.section "Main" free
; -----------------------------------------------------------------------------
  SwabbyMetaSprite:
    .db 6
    .db 0 0 0 8 8 8
    .db 0 0 8 1 16 2 0 3 8 4 16 5

  SwabbyInitString:
    .db 20 40
    .dw SwabbyMetaSprite

  Main:
    call AwaitFrameInterrupt
    call LoadSAT

    ld a,(FrameCounter)
    cp 0
    jp nz,+
      ld hl,SwabbyInitString
      call CreateObject
      call c,_ObjectOverflow
      ld (PlayerObjectHandle),a
      jp ++
    +:
    cp 127
    jp nz,++
      ld a,(PlayerObjectHandle)
      call DestroyObject
    ++:
    ld hl,FrameCounter
    inc (hl)

    call GetInputPorts

    call ObjectFrame

  jp Main

  _ObjectOverflow:
    ; Trap error.
    nop
  jp _ObjectOverflow

.ends
