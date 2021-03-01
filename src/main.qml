import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import Tetris 1.0
import "tetrisgame.js" as TetrisGame

Window {
    id: window
    width: 680
    height: 500
    visible: true
    title: qsTr("堆垛机")

    ControlKey{
       id:keyboard
    }

    ListView {
        id: guideBar
        interactive: false
        anchors.top: parent.top
        anchors.left: parent.left
        width: parent.width
        height: 22
        currentIndex: 0
        orientation: Qt.Horizontal
        spacing: 30
        model: ListModel {
            ListElement {
                name: "游戏"
            }
            ListElement {
                name: "设置"
            }
        }
        delegate: BasicTabButton {
            width: guideBar.width / guideBar.count
            height: parent.height
            text: name
            font.pixelSize: 18
            font.bold: true
            checked: guideBar.currentIndex == index
            onClicked: {
                guideBar.currentIndex = index
                view.forceActiveFocus()
            }
        }
    }

    SwipeView {
        id: view
        currentIndex: guideBar.currentIndex
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        width: parent.width
        height: parent.height - guideBar.height
        focus: true
        contentItem.focus: true
        onCurrentIndexChanged: {
            if (guideBar.currentIndex != currentIndex) {
                guideBar.currentIndex = currentIndex
            }
        }

        FocusScope {
            id: battlePage
            clip: true
            focus: true
            Tetris {
                id: w
                focus: true
                property var viewTetris: new TetrisGame.View(m, this)
                onSendAttack: function (trash) {
                    w1.getTrash(trash)
                }
                Component.onCompleted: {
                    viewTetris.startNewGame()
                    restart()
                }
            }

            Tetris {
                id: w1
                //  focus: true
                property var viewTetris: new TetrisGame.View(m1, this)
                onSendAttack: function (trash) {
                    w.getTrash(trash)
                }
                Component.onCompleted: {
                    viewTetris.startNewGame()
                    restart()
                }
            }

            Row {
                spacing: 130
                anchors.centerIn: parent
                id: buxing
                function restartGames() {
                    w.restart()
                    w1.restart()
                }
                TetrisUI {
                    id: m
                    tetris: w
                    tetrisGame: TetrisGame
                    scale: 0.8
                }

                TetrisUI {
                    id: m1
                    tetris: w1
                    tetrisGame: TetrisGame
                    scale: 0.8
                }
            }
        }

        Item {
            id: settingPage
            SetScene {
                id: setscene
                tetris: w
                anchors.fill: parent
                onReplayCall: function () {
                    view.currentIndex = 0
                }
            }
        }
    }
}
