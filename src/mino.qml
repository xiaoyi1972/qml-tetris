import QtQuick 2.0

Item {
    id: root
    property bool tight: true
    property int type: -1
    property var board: []
    property bool shadow: false

    function create(_type) {
        createActive(_type)
        type = _type
    }

    function createActive(type) {
        let empty=(board.length==0)
        let blockSize = tetrisConfig.blockSize
        if(type === -1){
            for(let block of board){
                block.visible = false
            }
            return
        }
        let showMinos = (tight ? tetrisConfig.tightMinos[type] : tetrisConfig.minos[type])
        let len = showMinos.length
        let maxColumn = 0
        let component = null
        for (let row = 0, count = 0; row < len; row++) {
            for (let data = showMinos[row], column = 0; data > 0; data = (data >> 1), column++) {
                if (data & 1) {
                    let obj = null
                    if(empty){
                    obj = createBlock(component,column, row, root, blockSize)
                    board.push(obj)
                    }
                    else{
                        obj = board[count]
                        board[count].x = column * (blockSize)
                        board[count].y = row * (blockSize)
                    }
                    if(obj.shadow){
                    obj.border.color= tetrisConfig.getTypeColor(type)
                    obj.border.width = 1
                    obj.color = "transparent"
                    }
                    else
                   obj.color = tetrisConfig.getTypeColor(type)
                   obj.visible = true
                   count++
                }
                maxColumn=Math.max(maxColumn,column+1)
            }
        }
        root.width = maxColumn * blockSize
        root.height = len * blockSize
    }

    function createBlock(component,column, row, parent, blockSize) {
        if (component === null)
            component = Qt.createComponent("block.qml")
        if (component.status === Component.Ready) {
            var dynamicObject = component.createObject(parent)
            if (dynamicObject === null) {
                console.log("error creating block")
                console.log(component.errorString())
                return false
            }
            dynamicObject.x = column * blockSize
            dynamicObject.y = row * blockSize
            dynamicObject.width = blockSize
            dynamicObject.height = blockSize
            dynamicObject.shadow = root.shadow
        } else {
            console.log("error loading block component")
            console.log(component.errorString())
            return false
        }
        return dynamicObject
    }
}
