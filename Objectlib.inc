; *****************************************************************************
;                           OBJECT LIBRARY
; *****************************************************************************
; Note: It builds on top of the Spritelib.

.ramsection "Objectlib Variables" slot 3
  ObjectAttributeTable dsb 256 ; 16 objects, 16 bytes each. Abbrev. OAT.
    ; 0: Status
    ; 1    : Y
    ; 2    : X
    ; 3-4  : MetaSpritePointer
    ; 5-15 : Reserved
.ends

; -----------------------------------------------------------------------------
.section "CreateObject" free
; -----------------------------------------------------------------------------
  ; Find an object with Status = 0 in OAT. Load data into this object from a
  ; string pointed to by HL.
  ; Entry: HL = Pointer to object initialization data.
  ; Exit: A = Table index assigned to this object.
  ; Uses: AF,B,DE,HL,IX
  CreateObject:
    ; Search for the next free object in OAT
    ld ix,ObjectAttributeTable
    ld b,0
    -:
      ld a,(ix+0)
      or a                  ; Zero means object is idle.
      jp nz,+
        call _LoadElement
        or a                ; Clear carry (operation successful).
        ld a,b              ; Return with A holding the index.
        ret
      +:
      ld de,16
      add ix,de
      inc b
      ld a,b
      cp 16
    jp nz,-
    scf                      ; Set carry to indicate error.
  ret

  _LoadElement:
    ; Entry: IX = Pointer to first byte in object table element.
    ;        HL = Pointer to initialization data.
    ; Exit:  A = ObjectHandle (the object's index in OAT).
    ; Uses AF,HL,IX - cannot use B!
    ld a,1        ; Switch from idle to active.
    ld (ix+0),a
    ld a,(hl)     ; Get Y position.
    ld (ix+1),a
    inc hl
    ld a,(hl)     ; Get X position.
    ld (ix+2),a
    inc hl
    ld a,(hl)     ; Get LSB and MSB of MetaSpritePointer.
    ld (ix+3),a
    inc hl
    ld a,(hl)
    ld (ix+4),a
  ret
.ends

; -----------------------------------------------------------------------------
.section "DestroyObject" free
; -----------------------------------------------------------------------------
  ; Detroys an object.
  ; Entry: A = Handle of object to destroy.
  ; Uses: AF,B,HL
  DestroyObject:
    ld hl,ObjectAttributeTable
    ld de,16
    ld b,a
    xor a
    cp b
    jp z,+
      -:
        add hl,de
        inc a
        cp b
      jp nz,-
    +:
    xor a             ; Write zero to signal that this obj. is now 'idle'.
    ld (hl),a
  ret
.ends


; -----------------------------------------------------------------------------
.section "ObjectFrame" free
; -----------------------------------------------------------------------------
  ; Frame-by-frame housekeeping of the objects.
  ObjectFrame:
    ; Start with animation, etc...

    ; Finish by drawing the objects as they look by now.
    call DrawObjects
  ret
.ends

; -----------------------------------------------------------------------------
.section "DrawObjects" free
; -----------------------------------------------------------------------------
  ; Utilize the Spritelib for drawing the active objects' metasprites.
  ; Search for the next free object in OAT (from last to first element).
  DrawObjects:
    call BeginMetaSprites
    ld ix,ObjectAttributeTable
    ld b,0
    -:
      ld a,(ix+0)
      or a                  ; Zero means object is idle.
      call nz,_DrawObject
      ld de,-16
      add ix,de
      inc b
      ld a,b
      cp 16
    jp nz,-
    call FinalizeMetaSprites
  ret

  _DrawObject:
    ; Entry: IX = Pointer to OAT element.
    ; Uses: AF,HL,IX (must preserve B!)
    push bc
      ld l,(ix+3)
      ld h,(ix+4)
      ld a,(ix+1)
      ld b,(ix+2)
      call AddMetaSprite
    pop bc
  ret
.ends
