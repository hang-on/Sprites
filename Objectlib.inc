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
  CreateObject:
    ; Search for the next free object in OAT
    ld ix,ObjectAttributeTable+256-16
    ld b,16
    -:
      ld a,(ix+0)
      or a
      jp nz,+
        call _LoadElement
        ; ld a,handle
        ; reset carry = no error
        ret
      +:
      ld de,-16
      add ix,de
    djnz -
    ; fall through to something with set carry = error
  ret

  _LoadElement:
    ; Entry: IX = Pointer to first byte in object table element.
    ;        HL = Pointer to initialization data.
    ; Exit:  A = ObjectHandle (the object's index in OAT).
    ld a,(hl)
    ld (ix+1),a
    inc hl
    ld a,(hl)
    ld (ix+2),a
    inc hl
    ld a,(hl)
    ld (ix+3),a
    inc hl
    ld a,(hl)
    ld (ix+4),a
    ld a,b        ; Return with A holding the index.
  ret
.ends

; We will also need an Objectframe and ObjectInit ...
