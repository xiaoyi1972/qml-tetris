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
    title: qsTr("tetrix")

    Tetris {
        id: w
        focus: true
        property var viewTetris: new TetrisGame.View(m)
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
    }

    TetrisUI {
        id: m
        tetris: w
        tetrisGame: TetrisGame
        controlkey: ControlKey
        anchors.centerIn: parent
    }

    SetScene {
        id: setscene
        tetris: w
        anchors.fill: m
        controlkey: ControlKey
    }
}
