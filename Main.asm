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
  NextEventIndex db
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

  _EventTable:
    .dw _Event0 _Event1 _Event2
  _EventTableEnd:
    _Event0:
      ld hl,SwabbyInitString
      call CreateObject
      ld (PlayerObjectHandle),a
      jp _EndEvents
    _Event1:
      ld a,(PlayerObjectHandle)
      call DestroyObject
      jp _EndEvents
    _Event2:
      nop ; Do nothing... (event handler loops on this last element).
      jp _EndEvents

  Main:
    call AwaitFrameInterrupt
    call LoadSAT

    ld a,(FrameCounter)
    inc a
    ld (FrameCounter),a
    call z,_MakeEvent

    call GetInputPorts

    call ObjectFrame

  jp Main

  _MakeEvent:
    ld a,(NextEventIndex)
    add a,a
    ld d,0
    ld e,a
    ld hl,_EventTable
    add hl,de
    ld a,(hl)
    inc hl
    ld h,(hl)
    ld l,a
    jp (hl)
    ; Refer to event table in the data section above.
    ; .... when the event is over, we jump back to _EndEvents below.
  _EndEvents:
    ld a,(NextEventIndex)
    cp ((_EventTableEnd-_EventTable)/2)-1
    ret z
    inc a
    ld (NextEventIndex),a
  ret
.ends
