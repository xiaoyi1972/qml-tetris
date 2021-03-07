let component = null;

function index(column, row) {
    return column + (row * tetrisConfig.maxColumn);
}

class View {
    constructor(_component,_tetrisComponent) {
        this.background = _component.background
        this.fk = _component.active
        this.shadow = _component.shadow
        this.nextsC = _component.nexts
        this.hold = _component.hold
        this.effectClear=_component.effectClear
        this.clearText=_component.clearText;
        this.maxIndex = tetrisConfig.maxColumn * (tetrisConfig.maxRow + tetrisConfig.rowOver)
        this.board = new Array(this.maxIndex).fill(null)
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
        this.background.stop()
        this.background.range = 1
        this.shadow.opacity = 1
        this.effectClear.text=""
        this.clearText.text=""
        for (let i = 0; i < this.maxIndex; i++) {
            if (this.board[i] !== null) {
                this.board[i].destroy(0)
                this.board[i] = null
            }
        }
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
                    let x = parseInt(i / tetrisConfig.maxColumn)
                    let y = i % tetrisConfig.maxColumn
                    createBlock(y, x, this.board, this.background, true)
                    this.board[i].color = tetrisConfig.getTypeColor(map[i])
                }
            }
        }
    }

    hardDropFresh(_data) {
       let changes = _data.changes
       let clear = _data.clear
       let specialClear = _data.specialClear
       this.effectClear.text = specialClear
       this.clearText.text = _data.clearStatus
        for (let i  of changes) {
            if (this.board[index(i.x, i.y)] == null)
               createBlock(i.x, i.y, this.board, this.background)
               this.board[index(i.x, i.y)].color = tetrisConfig.getTypeColor(i.type)
               this.board[index(i.x, i.y)].playColor()
        }
        if (_data.dead) {
            this.background.playDead()
            buxing.restartGames()
            return
        }
        if (clear.clearArr.length !== 0) {
            for (let i of clear.clearArr) {
                for (let column = 0; column < tetrisConfig.maxColumn; column++) {
                   if (this.board[index(column, i)] !== null)
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
            for (let column = 0; column < tetrisConfig.maxColumn; column++) {
                for (let row = tetrisConfig.maxRow + tetrisConfig.rowOver - 1; row >= 0; row--) {
                    let obj = this.board[index(column, row)]
                    let droprow = getDropI(row)
                    if (obj !== null && droprow > 0) {
                        obj.y = (droprow + row) * tetrisConfig.blockSize
                        this.board[index(column, row + droprow)] = obj
                        this.board[index(column, row)] = null
                    }
                }
            }
        }
    }

    fresh(_data) {
        if (_data.isNewSpawn) {
            this.fk.toggleBehavior(false)
            this.shadow.toggleBehavior(false)
            this.fk.create(_data.active.type)
            this.shadow.create(_data.active.type)
        }
        this.fk.rotation = _data.active.rs * 90
        this.fk.rs = _data.active.rs
        this.fk.x = tetrisConfig.blockSize * _data.active.x
        this.fk.y = tetrisConfig.blockSize * _data.active.y
        this.fk.toggleBehavior(true)
        if (_data.active.drop === 0)
            this.shadow.opacity = 0
        else {
            this.shadow.opacity = 1
            this.shadow.rotation = _data.active.rs * 90
            this.shadow.x = tetrisConfig.blockSize * _data.active.x
            this.shadow.yMoving = tetrisConfig.blockSize * (_data.active.y + _data.active.drop)
            this.shadow.toggleBehavior(true)
        }
    }

    freshNext(_data) {
        let model = this.nextsC.model
        if (_data.force)
            for (let i = 0; i < this.nextsC.children.length; i++)
                model.clear();
        if (model.count === 0)
            for (let i of _data.next)
                model.append({"minoType":i})
        else {
            model.remove(0,1)
            model.append({"minoType":_data.nextNew})
        }
    }

    freshHold(_data) {
        this.hold.create(_data.type)
    }
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
        dynamicObject.banAimate(false)
        dynamicObject.x = column * (tetrisConfig.blockSize) //+ 0.25
        dynamicObject.y = row * (tetrisConfig.blockSize) //+ 0.25
        dynamicObject.width = tetrisConfig.blockSize //- 0.5
        dynamicObject.height = tetrisConfig.blockSize //-0.5
        dynamicObject.spawned = true
        dynamicObject.banAimate(true)
        if (board  !== null)
            board[index(column, row)] = dynamicObject
    } else {
        return false;
    }
    return true;
}
