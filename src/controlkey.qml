import QtQuick 2.0

QtObject {
    property var minos: [[0, 6, 6, 0], [0, 15, 0, 0], [2, 7, 0], [4, 7, 0], [1, 7, 0], [6, 3, 0], [3, 6, 0]]
    property var tightMinos: [[3, 3], [15], [2, 7], [4, 7], [1, 7], [6, 3], [3, 6]]
    property int blockSize:20
    property int maxColumn:10
    property int maxRow:20
    property int rowOver:2
    property var piece: {"None": -1,"O": 0, "I": 1,"T": 2,"L": 3,"J": 4,"S": 5,"Z": 6}
    property bool operTransition: true
    property bool shadow: true

    function getTypeColor(_piece) {
        let str
        switch (_piece) {
        case -2:
            str = Qt.rgba(115 / 255, 115 / 255, 115 / 255)
            break
        case piece.O:
            str = Qt.rgba(245 / 255, 220 / 255, 0 / 255)
            break
        case piece.I:
            str = Qt.rgba(57 / 255, 195 / 255, 199 / 255)
            break
        case piece.T:
            str = Qt.rgba(138 / 255, 43 / 255, 227 / 255)
            break
        case piece.L:
            str = Qt.rgba(255 / 255, 166 / 255, 0 / 255)
            break
        case piece.J:
            str = Qt.rgba(0 / 255, 0 / 255, 255 / 255)
            break
        case piece.S:
            str = Qt.rgba(51 / 255, 204 / 255, 51 / 255)
            break
        case piece.Z:
            str = Qt.rgba(255 / 255, 0 / 255, 0 / 255)
            break
        }
        return str
    }

    function isConflict(thiskey) {
        let obj = {}
        let real = 0
        obj[TetrisConfig.leftKey] = 1
        obj[TetrisConfig.rightKey] = 1
        obj[TetrisConfig.softDropKey] = 1
        obj[TetrisConfig.harddropKey] = 1
        obj[TetrisConfig.ccwKey] = 1
        obj[TetrisConfig.cwKey] = 1
        obj[TetrisConfig.holdKey] = 1
        obj[TetrisConfig.restartKey] = 1
        obj[TetrisConfig.replay] = 1
        obj[TetrisConfig.bot] = 1
        if (thiskey in obj)
            real = 1
        return real
    }

    function retKey(event_code) {
        let Key = new Map([[0x1000000, "Escape"], // misc keys
                           [0x01000001, "Tab"], [0x01000002, "Backtab"], [0x01000003, "Backspace"], [0x01000004, "Return"], [0x01000005, "Enter"], [0x01000006, "Insert"], [0x01000007, "Delete"], [0x01000008, "Pause"], [0x01000009, "Print"], [0x0100000a, "SysReq"], [0x0100000b, "Clear"], [0x01000010, "Home"], // cursor movement
                           [0x01000011, "End"], [0x01000012, "Left"], [0x01000013, "Up"], [0x01000014, "Right"], [0x01000015, "Down"], [0x01000016, "PageUp"], [0x01000017, "PageDown"], [0x01000020, "Shift"], // modifiers
                           [0x01000021, "Control"], [0x01000022, "Meta"], [0x01000023, "Alt"], [0x01000024, "CapsLock"], [0x01000025, "NumLock"], [0x01000026, "ScrollLock"], [0x01000030, "F1"], // function keys
                           [0x01000031, "F2"], [0x01000032, "F3"], [0x01000033, "F4"], [0x01000034, "F5"], [0x01000035, "F6"], [0x01000036, "F7"], [0x01000037, "F8"], [0x01000038, "F9"], [0x01000039, "F10"], [0x0100003a, "F11"], [0x0100003b, "F12"], [0x20, "Space"], // 7 bit printable ASCII
                           [0x30, "0"], [0x31, "1"], [0x32, "2"], [0x33, "3"], [0x34, "4"], [0x35, "5"], [0x36, "6"], [0x37, "7"], [0x38, "8"], [0x39, "9"], [0x40, "At"], [0x41, "A"], [0x42, "B"], [0x43, "C"], [0x44, "D"], [0x45, "E"], [0x46, "F"], [0x47, "G"], [0x48, "H"], [0x49, "I"], [0x4a, "J"], [0x4b, "K"], [0x4c, "L"], [0x4d, "M"], [0x4e, "N"], [0x4f, "O"], [0x50, "P"], [0x51, "Q"], [0x52, "R"], [0x53, "S"], [0x54, "T"], [0x55, "U"], [0x56, "V"], [0x57, "W"], [0x58, "X"], [0x59, "Y"], [0x5a, "Z"]])
        return Key.get(event_code)
    }
}
