let blockSize = 19, blockSize_ = 18.5, maxColumn = 10, maxRow = 20, RowOver = 2
let maxIndex = maxColumn * (maxRow + RowOver)
let component = null, activeComponent = null, minoComponent = null
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

class View {
    constructor(_component,_tetrisComponent) {
        this.feildLayer = _component.feildLayer
        this.background = _component.background
        this.fk = _component.active
        this.shadow = _component.shadow
        this.nextsC = _component.nexts
        this.hold = _component.hold
        this.effectClear=_component.effectClear
        this.clearText=_component.clearText;
        this.board = new Array(maxIndex).fill(null)
        this.nexts = []
        this.tetrisComponent=_tetrisComponent
        this.fresh = this.fresh.bind(this);
        this.hardDropFresh = this.hardDropFresh.bind(this);
        this.freshNext = this.freshNext.bind(this);
        this.freshHold = this.freshHold.bind(this);
        this.startNewGame = this.startNewGame.bind(this);
        this.mapFresh = this.mapFresh.bind(this);
        this.init()
    }

    init(){
        this.tetrisComponent.whl.connect(this.fresh)
        this.tetrisComponent.harddropFresh.connect(this.hardDropFresh)
        this.tetrisComponent.next.connect(this.freshNext)
        this.tetrisComponent.holdFresh.connect(this.freshHold)
        this.tetrisComponent.restartGame.connect(this.startNewGame)
        this.tetrisComponent.mapFresh.connect(this.mapFresh)
    }

    startNewGame() {
        this.feildLayer.stop()
        this.feildLayer.range = 1
        this.shadow.opacity = 1
        this.effectClear.text=""
        this.clearText.text=""
        for (let i = 0; i < maxIndex; i++) {
            if (this.board[i] !== null) {
                this.board[i].destroy(0)
                this.board[i] = null
            }
        }
     //   maxIndex = (maxRow + RowOver) * maxColumn;
    }

    mapFresh(_data) {
        let map = _data.map;
        for (let i = 0; i < map.length; i++) {
            if (this.board[i] !== null) {
                this.board[i].destroy(0)
                this.board[i] = null
            }
            if (map[i] !== -1) {
                if (this.board[i] === null) {
                    let x = parseInt(i / maxColumn)
                    let y = i % maxColumn
                    createBlock(y, x, this.board, this.background, true)
                    this.board[i].color = getTypeColor(map[i])
                }
            }
        }
    }

    hardDropFresh(_data) {
        let changes = _data.changes
        let clear = _data.clear
        let specialClear=_data.specialClear
        this.effectClear.text=specialClear
       this.clearText.text=_data.clearStatus
        for (let i  of changes) {
            if (this.board[index(i.x, i.y)] == null)
                createBlock(i.x, i.y, this.board, this.background)
               this.board[index(i.x, i.y)].color = getTypeColor(i.type)
               this.board[index(i.x, i.y)].playColor()
        }
        if (_data.dead) {
            this.feildLayer.playDead()
            return
        }
        if (clear.clearArr.length !== 0) {
            for (let i of clear.clearArr) {
                for (let column = 0; column < maxColumn; column++) {
                    if (this.board[index(column, i)] != null)
                        this.board[index(column, i)].dying = true
                    this.board[index(column, i)] = null
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
                for (let row = maxRow + RowOver - 1; row >= 0; row--) {
                    let obj = this.board[index(column, row)]
                    let droprow = getDropI(row)
                    if (obj !== null && droprow > 0) {
                        obj.y = (droprow + row) * blockSize
                        this.board[index(column, row + droprow)] = obj
                        this.board[index(column, row)] = null
                    }
                }
            }
        }
    }

    fresh(_data) {
        let banAnimation = false
        if (_data.isNewSpawn) {
            banAnimation = true
            this.fk.toggleBehavior(false)
            this.shadow.toggleBehavior(false)
            createActive(_data.active.type, this.fk, false, _data.dead)
            createActive(_data.active.type, this.shadow, true, _data.dead)
        }
        this.fk.width = minos[_data.active.type].length * blockSize
        this.fk.height = minos[_data.active.type].length * blockSize
        this.fk.rotation = _data.active.rs * 90
        this.fk.rs = _data.active.rs
        this.fk.x = blockSize * _data.active.x
        this.fk.y = blockSize * _data.active.y
        this.fk.toggleBehavior(true)
        if (_data.active.drop === 0)
            this.shadow.opacity = 0
        else {
            this.shadow.opacity = 1
            this.shadow.width = minos[_data.active.type].length * blockSize
            this.shadow.height = minos[_data.active.type].length * blockSize
            this.shadow.rotation = _data.active.rs * 90
            this.shadow.x = blockSize * _data.active.x
            this.shadow.y = blockSize * (_data.active.y + _data.active.drop)
            this.shadow.toggleBehavior(true)
        }
    }

    freshNext(_data) {
        if (_data.force) {
            for (let i = 0; i < this.nextsC.children.length; i++) {
                this.nextsC.children[i].visible = false
                this.nextsC.children[i].destroy(0)
            }
            this.nexts.length = 0
        }
        if (this.nexts.length === 0) {
            for (let i of _data.next) {
                nextBlock(i, this.nexts, this.nextsC)
            }
        }
        else {
            this.nexts[0].destroy()
            this.nexts[0].visible = false
            this.nexts.splice(0, 1)
            nextBlock(_data.nextNew, this.nexts, this.nextsC)
        }
    }

    freshHold(_data) {
        for (let i = 0; i < this.hold.children.length; i++) {
            this.hold.children[i].visible = false
            this.hold.children[i].destroy(0)
        }
        if (_data.type !== -1)
            this.hold.create(_data.type)
    }
}

function getTypeColor(piece) {
    let str;
    switch (piece) {
        case -2: str = Qt.rgba(115 / 255, 115 / 255, 115 / 255); break;
        case Piece.O: str = Qt.rgba(245 / 255, 220 / 255, 0 / 255); break;
        case Piece.I: str = Qt.rgba(57 / 255, 195 / 255, 199 / 255); break;
        case Piece.T: str = Qt.rgba(138 / 255, 43 / 255, 227 / 255); break;
        case Piece.L: str = Qt.rgba(255 / 255, 166 / 255, 0 / 255); break;
        case Piece.J: str = Qt.rgba(0 / 255, 0 / 255, 255 / 255); break;
        case Piece.S: str = Qt.rgba(51 / 255, 204 / 255, 51 / 255); break;
        case Piece.Z: str = Qt.rgba(255 / 255, 0 / 255, 0 / 255); break;
    }
    return str;
}


function createActive(type, parent, isShadow = false, deaded = false) {
    parent.minoType = type
    parent.visible = false
    for (let i of parent.board) {
        if (i !== null) {
            i.visible = false
            i.destroy(0)
            i = null
        }
    }
    if (deaded) {
        parent.board.length = 0
        parent.visible = true
        return;
    }
    parent.board.length = 0
    let len = minos[type].length;
    for (let row = 0; row < len; row++) {
        for (let column = 0; column < len; column++) {
            if (minos[type][row][column]) {
                let block = createActiveBlock(column, row, parent.board, parent);
                if (!isShadow)
                    block.color = getTypeColor(type);
                else {
                    block.color = "transparent";
                    block.border.width = 1
                    block.border.color = getTypeColor(type);
                }
            }
        }
    }
    parent.visible = true
}


function createBlock(column, row, board = null, parent, instant = false) {
    if (component === null) {
        component = Qt.createComponent("feildBlock.qml");
    }
    if (component.status === Component.Ready) {
        let dynamicObject = component.createObject(parent);
        if (dynamicObject === null) {
            return false;
        }
        if (instant)
            dynamicObject.banFlash(false)
        dynamicObject.banAimate(false)
        dynamicObject.x = column * (blockSize) + 0.25
        dynamicObject.y = row * (blockSize) + 0.25
        dynamicObject.width = blockSize_
        dynamicObject.height = blockSize_
        dynamicObject.spawned = true
        dynamicObject.banAimate(true)
        if (instant)
            dynamicObject.banFlash(true)
        if (board !== null)
            board[index(column, row)] = dynamicObject

    } else {
        return false;
    }
    return true;
}

function createActiveBlock(column, row, board = null, parent) {
    if (activeComponent === null) {
        activeComponent = Qt.createComponent("block.qml")
    }
    if (activeComponent.status === Component.Ready) {
        let dynamicObject = activeComponent.createObject(parent);
        if (dynamicObject === null) {
            return false;
        }
        dynamicObject.x = column * (blockSize) + 0.25
        dynamicObject.y = row * (blockSize) + 0.25
        dynamicObject.width = blockSize_
        dynamicObject.height = blockSize_
        if (board !== null)
            board.push(dynamicObject)
        return dynamicObject
        //board[index(column, row)] = dynamicObject;
    }
}

function nextBlock(type, _nextsList = null, parent) {
    if (minoComponent === null)
        minoComponent = Qt.createComponent("Mino.qml")
    if (minoComponent.status === Component.Ready) {
        let dynamicObject = minoComponent.createObject(parent)
        if (dynamicObject === null) {
            return false
        }
        _nextsList.push(dynamicObject)
        dynamicObject.create(type)
    } else {
        return false
    }
    return true
}
