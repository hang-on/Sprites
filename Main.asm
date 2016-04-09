.include "Base.inc"
.include "Spritelib.inc"

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
.ends
; -----------------------------------------------------------------------------
.section "Main" free
; -----------------------------------------------------------------------------
  SwabbyMetaSprite:
    .db 6
    .db 0 0 0 8 8 8
    .db 0 0 8 1 16 2 0 3 8 4 16 5

  Main:
    call AwaitFrameInterrupt

    call LoadSAT

    call GetInputPorts

    call BeginMetaSprites
    ld a,20
    ld b,20
    ld hl,SwabbyMetaSprite
    call AddMetaSprite
    ld a,40
    ld b,40
    ld hl,SwabbyMetaSprite
    call AddMetaSprite
    ld a,70
    ld b,70
    ld hl,SwabbyMetaSprite
    call AddMetaSprite
    call FinalizeMetaSprites

    ld hl,FrameCounter
    inc (hl)
  jp Main

.ends
