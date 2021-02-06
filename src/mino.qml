
import QtQuick 2.0
Item {
    property int type: -1
    property int blockSize: 19
    id: root
    function create(_type) {
        let showMinos = [[[1, 1], [1, 1]], [[1, 1, 1, 1]], [[0, 1, 0], [1, 1, 1]], [[0, 0, 1], [1, 1, 1]], [[1, 0, 0], [1, 1, 1]], [[0, 1, 1], [1, 1, 0]], [[1, 1, 0], [0, 1, 1]]]
        let Piece = {
            "None": -1,
            "O": 0,
            "I": 1,
            "T": 2,
            "L": 3,
            "J": 4,
            "S": 5,
            "Z": 6
        }
        let component
        function createActive(type) {
            let len = showMinos[type].length
            let len1 = showMinos[type][0].length
            for (var row = 0; row < len; row++) {
                for (var column = 0; column < len1; column++) {
                    if (showMinos[type][row][column]) {
                        let obj = createBlock(column, row, root, root.blockSize,
                                              root.blockSize - 0.5)
                        obj.color = getTypeColor(type)
                    }
                }
            }
        }

        function getTypeColor(piece) {
            let str
            switch (piece) {
            case Piece.O:
                str = Qt.rgba(245 / 255, 220 / 255, 0 / 255)
                break
            case Piece.I:
                str = Qt.rgba(94 / 255, 158 / 255, 160 / 255)
                break
            case Piece.T:
                str = Qt.rgba(138 / 255, 43 / 255, 227 / 255)
                break
            case Piece.L:
                str = Qt.rgba(255 / 255, 166 / 255, 0 / 255)
                break
            case Piece.J:
                str = Qt.rgba(0 / 255, 0 / 255, 255 / 255)
                break
            case Piece.S:
                str = Qt.rgba(51 / 255, 204 / 255, 51 / 255)
                break
            case Piece.Z:
                str = Qt.rgba(255 / 255, 0 / 255, 0 / 255)
                break
            }
            return str
        }

        function createBlock(column, row, parent, blockSize, blockSize_) {
            if (component == null)
                component = Qt.createComponent("block.qml")

            if (component.status == Component.Ready) {
                var dynamicObject = component.createObject(parent)
                if (dynamicObject == null) {
                    console.log("error creating block")
                    console.log(component.errorString())
                    return false
                }
                dynamicObject.x = column * (blockSize) + 0.25
                dynamicObject.y = row * (blockSize) + 0.25
                dynamicObject.width = blockSize_
                dynamicObject.height = blockSize_
            } else {
                console.log("error loading block component")
                console.log(component.errorString())
                return false
            }
            return dynamicObject
        }

        createActive(_type)
        root.width = showMinos[_type][0].length * root.blockSize
        root.height = showMinos[_type].length * root.blockSize
    }
}
