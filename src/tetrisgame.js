let blockSize = 19
let blockSize_ = 18.5
let maxColumn = 10
let maxRow = 20
let RowOver= 2
let maxIndex = maxColumn * (maxRow+RowOver)
let board = new Array(maxIndex)
let boardK = new Array(9)
let component
let activeComponent
let minoComponent
let nexts = new Array()

let Piece = { None: -1, O: 0, I: 1, T: 2, L: 3, J: 4, S: 5, Z: 6 }

let minos = [
    [[0, 0, 0, 0], [0, 1, 1, 0], [0, 1, 1, 0], [0, 0, 0, 0]],
    [[0, 0, 0, 0], [1, 1, 1, 1], [0, 0, 0, 0], [0, 0, 0, 0]],
    [[0, 1, 0], [1, 1, 1], [0, 0, 0]],
    [[0, 0, 1], [1, 1, 1], [0, 0, 0]],
    [[1, 0, 0], [1, 1, 1], [0, 0, 0]],
    [[0, 1, 1], [1, 1, 0], [0, 0, 0]],
    [[1, 1, 0], [0, 1, 1], [0, 0, 0]],
]

function index(column, row) {
    return column + (row * maxColumn);
}

function startNewGame() {
    for (let i = 0; i < maxIndex; i++) {
        if (board[i] != null) {
            board[i].destroy();
            board[i] = null;
        }
    }
    maxIndex = (maxRow+RowOver )* maxColumn;
}

function getTypeColor(piece) {
    let str;
    switch (piece) {
         case -2:str = Qt.rgba(115 / 255, 115 / 255, 115 / 255); break;
        case Piece.O: str = Qt.rgba(245 / 255, 220 / 255, 0 / 255); break;
        case Piece.I: str = Qt.rgba(94 / 255, 158 / 255, 160 / 255); break;
        case Piece.T: str = Qt.rgba(138 / 255, 43 / 255, 227 / 255); break;
        case Piece.L: str = Qt.rgba(0 / 255, 0 / 255, 255 / 255); break;
        case Piece.J: str = Qt.rgba(255 / 255, 166 / 255, 0 / 255); break;
        case Piece.S: str = Qt.rgba(51 / 255, 204 / 255, 51 / 255); break;
        case Piece.Z: str = Qt.rgba(255 / 255, 0 / 255, 0 / 255); break;
    }
    return str;
}

function mapFresh(_data){
    let map=_data.map;
 //   console.log(map)
    for(let i =0;i<map.length;i++){
        if(board[i] != null){
             board[i].destroy(0)
            board[i]=null
        }
        if(map[i]!=-1){
    if (board[i] == null){
        let x=parseInt(i/maxColumn)
        let y= i%maxColumn
       // console.log(x,y)
        createBlock(y,x, board, background,true)
       board[i].color = getTypeColor(map[i])
    }
    }
    }
}

function hardDropFresh(_data) {
    let changes = _data.changes;
    let clear = _data.clear;
    for (let i  of changes) {
        if (board[index(i.x, i.y)] == null)
            createBlock(i.x, i.y, board, background);
        board[index(i.x, i.y)].color = getTypeColor(i.type)
    }

    if (clear.clearArr.length != 0) {
        for (let i  of clear.clearArr) {
            for (let column = 0; column < maxColumn; column++) {
                if (board[index(column, i)] != null)
                    board[index(column, i)].dying = true;
                board[index(column, i)] = null;
            }
        }

        function getDropI(index) {
            let num = 0;
            for (let i of clear.clearArr) {
                if (index < i)
                    num++
            }
            return num;
        }

        for (let column = 0; column < maxColumn; column++) {
            for (let row = maxRow +RowOver - 1; row >= 0; row--) {
                let obj = board[index(column, row)];
                let droprow = getDropI(row);
                if (obj != null && droprow > 0) {
                    obj.y = (droprow + row) * blockSize;
                    board[index(column, row + droprow)] = obj;
                    board[index(column, row)] = null;
                }
            }
        }
    }
}

function fresh(_data) {
    let banAnimation = false
    if (_data.isNewSpawn) {
         banAnimation = true;
        fk.toggleBehavior(false);
        shadow.toggleBehavior(false);
        createActive(_data.active.type, fk);
        createActive(_data.active.type, shadow, true);
    }
   /* if (banAnimation) {
        fk.toggleBehavior(false);
        shadow.toggleBehavior(false);
    }*/
    fk.width = minos[_data.active.type].length * blockSize;
    fk.height = minos[_data.active.type].length * blockSize;
    fk.rotation = _data.active.rs * 90
    fk.rs = _data.active.rs
    fk.x = blockSize * _data.active.x
    fk.y = blockSize * _data.active.y
    fk.toggleBehavior(true);

    if (_data.active.drop == 0)
        shadow.visible = false;
    else {
        shadow.visible = true
        shadow.width = minos[_data.active.type].length * blockSize;
        shadow.height = minos[_data.active.type].length * blockSize;
        shadow.rotation = _data.active.rs * 90
        shadow.x = blockSize * _data.active.x
        shadow.y = blockSize * (_data.active.y + _data.active.drop)
        shadow.toggleBehavior(true);
    }
}


function freshNext(_data) {
    if (_data.force) {
        for (let i = 0; i < nextsC.children.length; i++){
         nextsC.children[i].visible=false;
         nextsC.children[i].destroy();
        }
        nexts.length=0;
    }

    if (nexts.length == 0) {
        for (let i of _data.next) {
            nextBlock(i, nexts,nextsC)
        }
    }
    else {
        nexts[0].destroy()
        nexts[0].visible=false
        nexts.splice(0,1)
        nextBlock(_data.nextNew,nexts,nextsC)
    }
}


function freshHold(_data) {
    for (let i = 0; i < hold.children.length; i++){
        hold.children[i].visible=false;
        hold.children[i].destroy();
    }

    if (_data.type != -1)
        hold.create(_data.type)
}

function createActive(type, parent, isShadow = false) {
    parent.minoType = type
    parent.visible=false
    for (let i of parent.board) {
        if (i != null){
            i.visible=false
            i.destroy(0);
            }
    }
    parent.board.length = 0
    let len = minos[type].length;
    for (let row = 0; row < len; row++) {
        for (let column = 0; column < len; column++) {
            let x = createActiveBlock(column, row, parent.board, parent);
            if (minos[type][row][column]) {
                if (!isShadow)
                    parent.board[index(column, row)].color = getTypeColor(type);
                else {
                    parent.board[index(column, row)].color = "transparent";
                    parent.board[index(column, row)].border.width = 1
                    parent.board[index(column, row)].border.color = getTypeColor(type);
                }
            }
        }
    }
    parent.visible=true
}


function createBlock(column, row, board = null, parent,instant=false) {
    if (component == null) {
        component = Qt.createComponent("feildBlock.qml");
    }
    if (component.status == Component.Ready) {
        let dynamicObject = component.createObject(parent);
        if (dynamicObject == null) {
            return false;
        }
        if(instant)
        dynamicObject.banFlash(false);
        dynamicObject.banAimate(false);
        dynamicObject.x = column * (blockSize) + 0.25;
        dynamicObject.y = row * (blockSize) + 0.25;
        dynamicObject.width = blockSize_
        dynamicObject.height = blockSize_;
        dynamicObject.spawned = true;
        dynamicObject.banAimate(true);
        if(instant)
        dynamicObject.banFlash(true);
        if (board != null)
            board[index(column, row)] = dynamicObject;
    } else {
        return false;
    }
    return true;
}

function createActiveBlock(column, row, board = null, parent) {
    if (activeComponent == null) {
        activeComponent = Qt.createComponent("block.qml");
    }

    if (activeComponent.status == Component.Ready) {
        let dynamicObject = activeComponent.createObject(parent);
        if (dynamicObject == null) {
            return false;
        }
        dynamicObject.x = column * (blockSize) + 0.25;
        dynamicObject.y = row * (blockSize) + 0.25;
        dynamicObject.width = blockSize_
        dynamicObject.height = blockSize_;
        if (board != null)
            board[index(column, row)] = dynamicObject;
    } else {
        return false;
    }
    return true;
}

function nextBlock(type, _nextsList=null,parent) {
    if (minoComponent == null)
        minoComponent = Qt.createComponent("Mino.qml");
    if (minoComponent.status == Component.Ready) {
        let dynamicObject = minoComponent.createObject(parent);
        if (dynamicObject == null) {
            return false;
        }
            _nextsList.push(dynamicObject)
        dynamicObject.create(type)
    } else {
        return false;
    }


    return true;
}
