import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import Tetris 1.0
import "tetrisgame.js" as TetrisGame
import "controlKey.js" as ControlKey

Window {
    id: window
    width: 640
    height: 500
    visible: true
    title: qsTr("堆垛机")

    /*Component.onCompleted: {
        let str;
        for(let index=0;index<7;index++){
        switch(index){
        case 0: str = Qt.rgba(245 / 255, 220 / 255, 0 / 255); break;
        case 1: str = Qt.rgba(94 / 255, 158 / 255, 160 / 255); break;
        case 2: str = Qt.rgba(138 / 255, 43 / 255, 227 / 255); break;
        case 3: str = Qt.rgba(255 / 255, 166 / 255, 0 / 255); break;
        case 4: str = Qt.rgba(0 / 255, 0 / 255, 255 / 255); break;
        case 5: str = Qt.rgba(51 / 255, 204 / 255, 51 / 255); break;
        case 6: str = Qt.rgba(255 / 255, 0 / 255, 0 / 255); break;
        }
        let ms=`Qt.hsla(${Math.ceil(str.hslHue*360)},${Math.ceil(str.hslSaturation*100)},${Math.ceil(str.hslLightness*100)},${str.a})`
        console.log(ms)
        }
    let test=Qt.hsla(54/360,100/100,49/100,1);
        console.log(`Qt.rgba(${Math.ceil(test.r*255)},${Math.ceil(test.g*255)},${Math.ceil(test.b*255)},${test.a})`)
    }*/

    Tetris {
        id: w
        focus: true
        property var viewTetris: new TetrisGame.View(m,this)
        Component.onCompleted: {
            viewTetris.startNewGame()
            restart()
        }
    }

  /*  Tetris {
        id: w1
        focus: true
        property var viewTetris: new TetrisGame.View(m1)
        onWhl: viewTetris.fresh(a)
        onHarddropFresh: viewTetris.hardDropFresh(a)
        onNext: viewTetris.freshNext(a)
        onHoldFresh: viewTetris.freshHold(a)
        onRestartGame: viewTetris.startNewGame()
        onMapFresh: viewTetris.mapFresh(a)
        Component.onCompleted: {
            viewTetris.startNewGame()
            restart()
        }
    }*/

    Row {
        spacing: 30
        anchors.centerIn: parent
        TetrisUI {
            id: m
            tetris: w
            tetrisGame: TetrisGame
            controlkey: ControlKey
           // anchors.centerIn: parent
            scale: 1
            SetScene {
                id: setscene
                tetris: w
                anchors.fill: m
                controlkey: ControlKey
            }
        }

  /*      TetrisUI {
            id: m1
            tetris: w1
            tetrisGame: TetrisGame
            controlkey: ControlKey
            //anchors.centerIn: parent
            scale: 0.7
        }*/
    }


}
