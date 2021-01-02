import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import QtQuick.Shapes 1.15
import Tetris 1.0
import "tetrisgame.js" as TetrisGame
import "controlKey.js" as ControlKey

Window {
    width: 640
    height: 500
    visible: true
    title: qsTr("tetrix")

    Tetris {
        id: w
        focus: true
        onWhl: TetrisGame.fresh(a)
        onHarddropFresh: TetrisGame.hardDropFresh(a)
        onNext: TetrisGame.freshNext(a)
        onHoldFresh: TetrisGame.freshHold(a)
        onRestartGame: TetrisGame.startNewGame()
        onMapFresh: TetrisGame.mapFresh(a)
        Component.onCompleted: {
            TetrisGame.startNewGame()
            w.restart()
        }
    }

    SetScene {
        id: setscene
        controlkey: ControlKey
    }

    Item {
        id: screen
        width: TetrisGame.blockSize * TetrisGame.maxColumn
        height: TetrisGame.blockSize * (TetrisGame.maxRow + TetrisGame.RowOver)
        anchors.centerIn: parent
        SystemPalette {
            id: activePalette
        }

        Shape {
            id: displayBorder
            width: TetrisGame.blockSize * TetrisGame.maxColumn
            height: TetrisGame.blockSize * TetrisGame.maxRow
            anchors.top: parent.top
            anchors.topMargin: TetrisGame.blockSize * TetrisGame.RowOver
            ShapePath {
                strokeWidth: 1
                strokeColor: "black"
                startX: 0
                startY: 0
                PathLine {
                    x: 0
                    y: TetrisGame.blockSize * (TetrisGame.maxRow)
                }
                PathLine {
                    x: TetrisGame.blockSize * TetrisGame.maxColumn
                    y: TetrisGame.blockSize * (TetrisGame.maxRow)
                }
                PathLine {
                    x: TetrisGame.blockSize * TetrisGame.maxColumn
                    y: 0
                }
            }
        }

        Item {
            id: background
            anchors.fill: parent
            Item {
                id: fk
                property int minoType: -1
                property var board: []
                property int rs: 0
                property int xyDuration: 30
                property int rDuration: 60
                x: 0
                y: 0
                z: 3
                transformOrigin: Item.Center
                rotation: 0
                Behavior on x {
                    id: xb
                    SmoothedAnimation {
                      duration: fk.xyDuration
                    }
                }
                Behavior on y {
                    id: yb
                    SmoothedAnimation {
                       duration: fk.xyDuration
                    }
                }
                Behavior on rotation {
                    id: rsb
                    RotationAnimation {
                        duration: fk.rDuration
                        direction: RotationAnimator.Shortest
                    }
                }
                function toggleBehavior(is) {
                    xb.enabled = is
                    yb.enabled = is
                    rsb.enabled = is
                }
            }

            Item {
                id: shadow
                property int minoType: -1
                property var board: []
                property int xyDuration: 30
                x: 0
                y: 0
                z: 1
                transformOrigin: Item.Center
                rotation: 0
                Behavior on x {
                    id: xb1
                    SmoothedAnimation {
                        duration: shadow.xyDuration
                    }
                }
                Behavior on y {
                    id: yb1
                    SmoothedAnimation {
                        duration: shadow.xyDuration
                    }
                }


                /* Behavior on rotation {
                     id:rsb1
                     NumberAnimation { properties:"rotation"; duration: 30;easing.type:Easing.OutInQuad }
                     }*/
                function toggleBehavior(is) {
                    xb1.enabled = is
                    yb1.enabled = is
                    /*  rsb1.enabled=is;*/
                }
            }
        }

        Column {
            anchors.top: displayBorder.top
            anchors.left: screen.right
            anchors.margins: 5
            anchors.topMargin: 0
            spacing: 10
            Text {
                id: nexts
                font.pointSize: 16
                text: "预览"
             //   anchors.top: screen.top
             //   anchors.left: screen.right
            }
            Column {
                id: nextsC
                spacing: 20
                move: Transition {
                    NumberAnimation {
                        properties: "x,y"
                        duration: 50
                    }
                }
            }
        }

        Column {
            anchors.top: displayBorder.top
            anchors.right: screen.left
            anchors.margins: 5
            anchors.topMargin: 0
            spacing: 10
            Text {
                font.pointSize: 16
                text: "暂存"
            }
            Mino {
                id: hold
             //   anchors.right: screen.left
            }
        }

        Rectangle {
            id: toolBar
            width: parent.width
            height: 30
            //  color: activePalette.window
            anchors.top: screen.bottom
            anchors.margins: 5
            Button {
                width: 50
                height: 30
                anchors {
                    left: parent.left
                    verticalCenter: parent.verticalCenter
                }
                text: "重开"
                onClicked: {
                    w.restart()
                    //  TetrisGame.startNewGame()
                    w.focus = true
                }
            }

            Button {
                width: 50
                height: 30
                anchors {
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                }
                text: "设置"
                onClicked: {
                    setscene.visible = !setscene.visible
                    if (setscene.visible == false) {
                        w.setKeyboard(ControlKey.config)
                        w.focus = true
                    }
                }
            }
        }
    }
}
