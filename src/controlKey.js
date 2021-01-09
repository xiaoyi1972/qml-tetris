 let config={
      lock_delay:30,
      das_delay:65,
      das_repeat:12,
      softDropSpeed:8,
      shadow:true,
      left_move:Qt.Key_Left,
      right_move:Qt.Key_Right,
      soft_drop:Qt.Key_Down,
      hard_drop:Qt.Key_Space,
      rotate_normal:Qt.Key_X,
      rotate_reverse:Qt.Key_Z,
      hold:Qt.Key_C,
      restart:Qt.Key_R,
      replay:Qt.Key_Q,
      ai:Qt.Key_W,
      background:"#000000",
      blockstyle:3,
}

    function isConflict(thiskey){
       let obj={}
       let real=0
        obj[config.left_move]=1
        obj[config.right_move]=1
        obj[config.soft_drop]=1
        obj[config.hard_drop]=1
        obj[config.rotate_normal]=1
        obj[config.rotate_reverse]=1
        obj[config.hold]=1
        obj[config.restart]=1
        obj[config.replay]=1
        obj[config.ai]=1
        if(thiskey in obj)
         real=1
        return real
    }

function retKey(event_code){
        let Key={
                 0x1000000:"Escape",                // misc keys
                 0x01000001:"Tab",
                 0x01000002:"Backtab",
                 0x01000003:"Backspace",
                 0x01000004:"Return",
                 0x01000005:"Enter",
                 0x01000006:"Insert",
                 0x01000007:"Delete",
                 0x01000008:"Pause",
                 0x01000009:"Print",
                 0x0100000a:"SysReq",
                 0x0100000b:"Clear",
                 0x01000010:"Home",                // cursor movement
                 0x01000011:"End",
                 0x01000012:"Left",
                 0x01000013:"Up",
                 0x01000014:"Right",
                 0x01000015:"Down",
                 0x01000016:"PageUp",
                 0x01000017:"PageDown",
                 0x01000020:"Shift",                // modifiers
                 0x01000021:"Control",
                 0x01000022:"Meta",
                 0x01000023:"Alt",
                 0x01000024:"CapsLock",
                 0x01000025:"NumLock",
                 0x01000026:"ScrollLock",
                 0x01000030:"F1",                // function keys
                 0x01000031:"F2",
                 0x01000032:"F3",
                 0x01000033:"F4",
                 0x01000034:"F5",
                 0x01000035:"F6",
                 0x01000036:"F7",
                 0x01000037:"F8",
                 0x01000038:"F9",
                 0x01000039:"F10",
                 0x0100003a:"F11",
                 0x0100003b:"F12",
                 0x20:"Space",                // 7 bit printable ASCII
                 0x30:"0",
                 0x31:"1",
                 0x32:"2",
                 0x33:"3",
                 0x34:"4",
                 0x35:"5",
                 0x36:"6",
                 0x37:"7",
                 0x38:"8",
                 0x39:"9",
                 0x40:"At",
                 0x41:"A",
                 0x42:"B",
                 0x43:"C",
                 0x44:"D",
                 0x45:"E",
                 0x46:"F",
                 0x47:"G",
                 0x48:"H",
                 0x49:"I",
                 0x4a:"J",
                 0x4b:"K",
                 0x4c:"L",
                 0x4d:"M",
                 0x4e:"N",
                 0x4f:"O",
                 0x50:"P",
                 0x51:"Q",
                 0x52:"R",
                 0x53:"S",
                 0x54:"T",
                 0x55:"U",
                 0x56:"V",
                 0x57:"W",
                 0x58:"X",
                 0x59:"Y",
                 0x5a:"Z",
    }
        return Key[event_code]
}
