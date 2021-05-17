let component = null;

function index(column, row) {
    return column + (row * tetrisConfig.maxColumn);
}

class View {
     constructor(_component,_tetrisComponent) {
        this.nextsC = _component.nexts
        this.hold = _component.hold
        this.effectClear=_component.effectClear
        this.clearText=_component.clearText;
        this.maxIndex = tetrisConfig.maxColumn * (tetrisConfig.maxRow + tetrisConfig.rowOver)
        this.tetrisComponent=_tetrisComponent
        this.hardDropFresh = this.hardDropFresh.bind(this);
        this.freshNext = this.freshNext.bind(this);
        this.freshHold = this.freshHold.bind(this);
        this.startNewGame = this.startNewGame.bind(this);
        this.test = this.test.bind(this);
        tetrisConfig.my.push(this.tetrisComponent)
     /*   onSendAttack: function (trash) {
            w.getTrash(trash)
        }*/
     //   console.log(tetrisConfig.my)
        this.init()
    }

    init(){
     //   this.tetrisComponent.whl.connect(this.fresh)
        this.tetrisComponent.next.connect(this.freshNext)
        this.tetrisComponent.holdFresh.connect(this.freshHold)
        this.tetrisComponent.restartGame.connect(this.startNewGame)
         this.tetrisComponent.harddropFresh.connect(this.hardDropFresh)
         this.tetrisComponent.sendAttack.connect(this.test)
   //     this.tetrisComponent.mapFresh.connect(this.mapFresh)
    }

    test(attack){
        let other=tetrisConfig.my.filter((num) => {
                                                            return num !=this.tetrisComponent;
                                                          });
       other[0].getTrash(attack)
    }

    startNewGame() {
        this.effectClear.text=""
        this.clearText.text=""
    }

    hardDropFresh(_data) {
       let specialClear = _data.specialClear
       this.effectClear.text = specialClear
       this.clearText.text = _data.clearStatus
        if (_data.dead) {
            buxing.restartGames()
            return
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
        if(_data.type==-1)
        this.hold.visible=false
        else
        this.hold.visible=true
        this.hold.type=_data.type
    }
}
