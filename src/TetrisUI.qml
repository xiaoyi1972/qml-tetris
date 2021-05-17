import QtQuick 2.15
import QtQuick.Shapes 1.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Tetris 1.0
import "tetrisgame.js" as TetrisGame

Item {
    id: root
    width: tetrisConfig.blockSize * tetrisConfig.maxColumn
    height: tetrisConfig.blockSize * (tetrisConfig.maxRow + tetrisConfig.rowOver)
    property var heightInline: tetrisConfig.blockSize * tetrisConfig.maxRow
    property alias tetris: tetris
    property var view: new TetrisGame.View(this, this.tetris)
    property alias nexts: nextsC
    property alias hold: hold
    property alias effectClear: effectClear
    property alias clearText: clearText

    Shape {
        id: displayBorder
        width: tetrisConfig.blockSize * tetrisConfig.maxColumn
        height: tetrisConfig.blockSize * tetrisConfig.maxRow
        anchors.top: parent.top
        anchors.topMargin: tetrisConfig.blockSize * tetrisConfig.rowOver
        ShapePath {
            strokeWidth: 1
            strokeColor: "black"
            startX: 0
            startY: 0
            PathLine {
                x: 0
                y: displayBorder.height
            }
            PathLine {
                relativeX: displayBorder.width
                relativeY: 0
            }
            PathLine {
                relativeX: 0
                y: 0
            }
            PathMove {
                relativeX: 0
                relativeY: displayBorder.height
            }
            PathLine {
                relativeX: 10
                relativeY: 0
            }
            PathLine {
                relativeX: 0
                y: 0
            }
        }
    }

    Tetris {
        id: tetris
        width: 200
        height: 400
        Component.onCompleted: {
            restart()
        }
    }

    Item {
        id: trashStatus
        height: displayBorder.height
        width: 10
        anchors.top: displayBorder.top
        anchors.left: displayBorder.right
        Rectangle {
            id: factTrash
            height: (Math.min(
                         tetris.trash,
                         tetrisConfig.maxRow) / tetrisConfig.maxRow) * parent.height
            width: parent.width
            color: "orange"
            anchors.bottom: parent.bottom
        }
        Rectangle {
            Behavior on height {
                SmoothedAnimation {
                    velocity: 150
                }
            }
            height: factTrash.height
            width: parent.width
            color: "red"
            anchors.bottom: parent.bottom
        }
    }

    Column {
        anchors.top: trashStatus.top
        anchors.left: trashStatus.right
        anchors.margins: 5
        anchors.topMargin: 0
        spacing: 10
        Text {
            id: nexts
            font.pointSize: 16
            text: "预览"
        }

        ListView {
            id: nextsC
            interactive: false
            width: parent.width
            height: contentHeight
            currentIndex: 0
            spacing: 20
            reuseItems: true
            model: ListModel {}
            delegate: TetrisTetro {
                tight: true
                ListView.onPooled: visible = false
                ListView.onReused: {
                    type = minoType
                    visible = true
                }
                Component.onCompleted: {
                    type = minoType
                }
            }
            displaced: Transition {
                NumberAnimation {
                    properties: "x,y"
                    duration: 100
                }
                NumberAnimation {
                    properties: "opacity"
                    duration: 0
                    to: 1
                }
            }
            add: Transition {
                NumberAnimation {
                    properties: "opacity"
                    from: 0
                    to: 1
                    duration: 150
                }
            }
        }
    }

    Text {
        id: holdText
        anchors.margins: 5
        anchors.topMargin: 0
        anchors.top: displayBorder.top
        anchors.right: root.left
        font.pointSize: 16
        text: "暂存"
        horizontalAlignment: Text.AlignRight
    }

    TetrisTetro {
        id: hold
        tight: true
        anchors.topMargin: 10
        anchors.top: holdText.bottom
        anchors.right: holdText.right
    }

    ColumnLayout {
        anchors.right: root.left
        anchors.bottom: root.bottom
        anchors.margins: 5
        anchors.topMargin: 0
        layoutDirection: Qt.RightToLeft
        spacing: 10

        Text {
            id: effectClear
            font.pointSize: 16
            text: ""
            horizontalAlignment: Text.AlignRight
        }

        Text {
            id: clearText
            font.pointSize: 16
            text: ""
            horizontalAlignment: Text.AlignRight
        }
    }

    Rectangle {
        id: toolBar
        width: parent.width
        height: 30
        anchors.top: root.bottom
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
                tetris.restart()
                tetris.focus = true
            }
        }
    }
}
