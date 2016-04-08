.include "Base.inc"

; -----------------------------------------------------------------------------
.section "SetupMain" free
; -----------------------------------------------------------------------------
  SetupMain:
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
  Main:
    call AwaitFrameInterrupt

    call GetInputPorts

    ld hl,FrameCounter
    inc (hl)
  jp Main

.ends
