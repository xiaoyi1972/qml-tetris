import QtQuick 2.7
import QtQuick.Controls 2.15
import QtQml 2.2
import QtQuick.Layouts 1.3

Item {
    id: root
    property var controlkey: null
    property var tetris: null
    visible: false
    focus: false
    z: 98

    Rectangle {
        anchors.fill: root
        color: Qt.rgba(0, 0, 0, 0.5)
    }

    ScrollView {
        id: scrollview1
        anchors.fill: root
        //ScrollBar.vertical.policy: ScrollBar.AlwaysOn
        clip: true

        /*
         style:ScrollViewStyle{
             id:scrollviewstyle1
             handle: Rectangle {
                         implicitWidth: 10
                         implicitHeight: 30
                         radius:3
                         color: "grey"
                     }
                     scrollBarBackground: Rectangle {
                         implicitWidth: 10
                         implicitHeight: 30
                         color: "transparent"
                     }
                     decrementControl: Rectangle {
                         implicitWidth: 10
                         implicitHeight: 0
                         color: "transparent"
                     }
                     incrementControl: Rectangle {
                         implicitWidth: 10
                         implicitHeight: 0
                         color: "transparent"
                     }
                     scrollToClickedPosition:true
                     transientScrollBars:true
         }*/
        Column {
            spacing: 35
            leftPadding: 5
            rightPadding: 5
            topPadding: 10
            bottomPadding: 10

            //anchors.fill:canvas
            Column {
                spacing: 10
                Row {
                    Label {
                        font.family: "Loma"
                        font.pixelSize: 24
                        color: "white"
                        text: "操作设置"
                    }
                }

                Row {
                    spacing: 5
                    Text {
                        id: text1
                        font.family: "Loma"
                        font.pixelSize: 20
                        color: "white"
                        text: "加速延迟:"
                    }
                    TextField {
                        height: 25
                        text: root.controlkey.config.das_delay
                        validator: IntValidator {
                            bottom: 0
                        }
                        width: 50
                        anchors.verticalCenter: parent.verticalCenter
                        onEditingFinished: {
                            root.controlkey.config.das_delay = text
                        }
                    }
                }

                Row {
                    spacing: 5
                    Text {
                        font.family: "Loma"
                        font.pixelSize: 20
                        color: "white"
                        text: "重复延迟:"
                    }
                    TextField {
                        height: 25
                        text: root.controlkey.config.das_repeat
                        validator: IntValidator {
                            bottom: 0
                        }
                        width: 50
                        anchors.verticalCenter: parent.verticalCenter
                        onEditingFinished: {
                            root.controlkey.config.das_repeat = text
                        }
                    }
                }

                Row {
                    spacing: 5
                    Text {
                        font.family: "Loma"
                        font.pixelSize: 20
                        color: "white"
                        text: "软降延迟:"
                    }
                    TextField {
                        height: 25
                        text: root.controlkey.config.softDropSpeed
                        validator: IntValidator {
                            bottom: 0
                        }
                        width: 50
                        anchors.verticalCenter: parent.verticalCenter
                        onEditingFinished: {
                            root.controlkey.config.softDropSpeed = text
                        }
                    }
                }


                /*Row{
     spacing:5
     Text{
         font.family:"Loma"
         font.pixelSize: 20
          color:"white"
         text:"锁定延迟:"
     }
     TextField{height:25
         text:root.controlkey.config.lock_delay
         inputMethodHints:Qt.ImhDigitsOnly
         width:50
         anchors.verticalCenter: parent.verticalCenter
         onEditingFinished:{
         root.controlkey.config.lock_delay=text
         }
     }
     }*/


                /*  Row{
     spacing:5
     Text{
         font.family:"Loma"
         font.pixelSize: 20
          color:"white"
         text:"软降速度:"
     }
     TextField{height:25
         text:root.controlkey.config.softDropSpeed
         inputMethodHints:Qt.ImhDigitsOnly
         width:50
         anchors.verticalCenter: parent.verticalCenter
         onEditingFinished:{
         root.controlkey.config.softDropSpeed=text
         }
     }
     }

     Row{
     spacing:5
     CheckBox{
         checked:root.controlkey.config.shadow
         Text{text:"阴影"
              font.family:"Loma"
              font.pixelSize: 20
              color:"white"
              anchors.left:parent.right
              anchors.verticalCenter: parent.verticalCenter
     }
         onClicked:{
             root.controlkey.config.shadow=checked
         }
     }
     }*/
            }

            Column {
                spacing: 10
                Row {
                    Label {
                        font.family: "Loma"
                        font.pixelSize: 24
                        color: "white"
                        text: "按键设置"
                    }
                }

                Row {
                    spacing: 5
                    Text {
                        font.family: "Loma"
                        font.pixelSize: 20
                        color: "white"
                        text: "左移:"
                    }
                    TextField {
                        height: 25
                        text: root.controlkey.retKey(
                                  root.controlkey.config.left_move)
                        width: 50
                        anchors.verticalCenter: parent.verticalCenter
                        readOnly: true
                        Keys.onPressed: {
                            root.controlkey.config.left_move = ""
                            if (!root.controlkey.isConflict(event.key)) {
                                text = root.controlkey.retKey(event.key)
                                root.controlkey.config.left_move = event.key
                            }
                            event.accepted = true
                        }
                    }
                }

                Row {
                    spacing: 5
                    Text {
                        font.family: "Loma"
                        font.pixelSize: 20
                        text: "右移:"
                        color: "white"
                    }
                    TextField {
                        height: 25
                        focus: true
                        text: root.controlkey.retKey(
                                  root.controlkey.config.right_move)
                        width: 50
                        anchors.verticalCenter: parent.verticalCenter
                        readOnly: true
                        Keys.onPressed: {
                            root.controlkey.config.right_move = ""
                            if (!root.controlkey.isConflict(event.key)) {
                                text = root.controlkey.retKey(event.key)
                                root.controlkey.config.right_move = event.key
                            }
                            event.accepted = true
                        }
                    }
                }

                Row {
                    spacing: 5
                    Text {
                        font.family: "Loma"
                        font.pixelSize: 20
                        text: "软降:"
                        color: "white"
                    }
                    TextField {
                        height: 25
                        text: root.controlkey.retKey(
                                  root.controlkey.config.soft_drop)
                        width: 50
                        anchors.verticalCenter: parent.verticalCenter
                        readOnly: true
                        Keys.onPressed: {
                            root.controlkey.config.soft_drop = ""
                            if (!root.controlkey.isConflict(event.key)) {
                                text = root.controlkey.retKey(event.key)
                                root.controlkey.config.soft_drop = event.key
                            }
                            event.accepted = true
                        }
                    }
                }

                Row {
                    spacing: 5
                    Text {
                        font.family: "Loma"
                        font.pixelSize: 20
                        text: "硬降:"
                        color: "white"
                    }
                    TextField {
                        height: 25
                        text: root.controlkey.retKey(
                                  root.controlkey.config.hard_drop)
                        width: 50
                        anchors.verticalCenter: parent.verticalCenter
                        readOnly: true
                        Keys.onPressed: {
                            root.controlkey.config.hard_drop = ""
                            if (!root.controlkey.isConflict(event.key)) {
                                text = root.controlkey.retKey(event.key)
                                root.controlkey.config.hard_drop = event.key
                            }
                            event.accepted = true
                        }
                    }
                }

                Row {
                    spacing: 5
                    Text {
                        font.family: "Loma"
                        font.pixelSize: 20
                        text: "顺时针旋转:"
                        color: "white"
                    }
                    TextField {
                        height: 25
                        text: root.controlkey.retKey(
                                  root.controlkey.config.rotate_normal)
                        width: 50
                        anchors.verticalCenter: parent.verticalCenter
                        readOnly: true
                        Keys.onPressed: {
                            root.controlkey.config.rotate_normal = ""
                            if (!root.controlkey.isConflict(event.key)) {
                                text = root.controlkey.retKey(event.key)
                                root.controlkey.config.rotate_normal = event.key
                            }
                            event.accepted = true
                        }
                    }
                }

                Row {
                    spacing: 5
                    Text {
                        font.family: "Loma"
                        font.pixelSize: 20
                        text: "逆时针旋转:"
                        color: "white"
                    }
                    TextField {
                        height: 25
                        text: root.controlkey.retKey(
                                  root.controlkey.config.rotate_reverse)
                        width: 50
                        anchors.verticalCenter: parent.verticalCenter
                        readOnly: true
                        Keys.onPressed: {
                            root.controlkey.config.rotate_reverse = ""
                            if (!root.controlkey.isConflict(event.key)) {
                                text = root.controlkey.retKey(event.key)
                                root.controlkey.config.rotate_reverse = event.key
                            }
                            event.accepted = true
                        }
                    }
                }

                Row {
                    spacing: 5
                    Text {
                        font.family: "Loma"
                        font.pixelSize: 20
                        text: "暂存:"
                        color: "white"
                    }
                    TextField {
                        height: 25
                        text: root.controlkey.retKey(
                                  root.controlkey.config.hold)
                        width: 50
                        anchors.verticalCenter: parent.verticalCenter
                        readOnly: true
                        Keys.onPressed: {
                            root.controlkey.config.hold = ""
                            if (!root.controlkey.isConflict(event.key)) {
                                text = root.controlkey.retKey(event.key)
                                root.controlkey.config.hold = event.key
                            }
                            event.accepted = true
                        }
                    }
                }

                Row {
                    spacing: 5
                    Text {
                        font.family: "Loma"
                        font.pixelSize: 20
                        text: "重新开始:"
                        color: "white"
                    }
                    TextField {
                        height: 25
                        text: root.controlkey.retKey(
                                  root.controlkey.config.restart)
                        width: 50
                        anchors.verticalCenter: parent.verticalCenter
                        readOnly: true
                        Keys.onPressed: {
                            root.controlkey.config.restart = ""
                            if (!root.controlkey.isConflict(event.key)) {
                                text = root.controlkey.retKey(event.key)
                                root.controlkey.config.restart = event.key
                            }
                            event.accepted = true
                        }
                    }
                }

                Row {
                    spacing: 5
                    Text {
                        font.family: "Loma"
                        font.pixelSize: 20
                        text: "回放:"
                        color: "white"
                    }
                    TextField {
                        height: 25
                        text: root.controlkey.retKey(
                                  root.controlkey.config.replay)
                        width: 50
                        anchors.verticalCenter: parent.verticalCenter
                        readOnly: true
                        Keys.onPressed: {
                            root.controlkey.config.replay = ""
                            if (!root.controlkey.isConflict(event.key)) {
                                text = root.controlkey.retKey(event.key)
                                root.controlkey.config.replay = event.key
                            }
                            event.accepted = true
                        }
                    }
                }


                /*  Row{
         spacing:5
         Text{
             font.family:"Loma"
             font.pixelSize: 20
             text:"AI:"
             color:"white"
         }
         TextField{height:25
           text:root.controlkey.retKey(root.controlkey.config.ai)
           width:50
           anchors.verticalCenter: parent.verticalCenter
           readOnly:true
           Keys.onPressed: {
               root.controlkey.config.ai=""
               if(!root.controlkey.isConflict(event.key)){
               text=root.controlkey.retKey(event.key)
                   root.controlkey.config.ai=event.key
                   }
               event.accepted=true
           }
         }
         }*/
            }

            Column {
                spacing: 10

                Label {
                    font.family: "Loma"
                    font.pixelSize: 24
                    text: "录像回放◀"
                    color: "white"
                }

                Column {
                    spacing: 10
                    ScrollView {
                        id: view
                        width: root.width - 10
                        height: 100
                        clip: true
                        TextArea {
                            id: replayData
                            wrapMode: TextEdit.Wrap
                            selectByMouse: true
                            font.pixelSize: 14
                            background: Rectangle {
                                width: view.width
                                height: view.height
                                color: "white"
                            }
                            text: ""
                        }
                    }

                    Button {
                        width: 60
                        height: 30
                        text: "播放"
                        onClicked: {
                            root.visible = !root.visible
                            if (root.visible == false) {
                                if (tetris != null) {
                                    tetris.focus = true
                                    tetris.replay(replayData.text)
                                }
                                replayData.text = ""
                            }
                        }
                    }
                }


                /*   Row{
         spacing:5
         Text{
             font.family:"Loma"
             font.pixelSize: 20
             text:"方块风格:"
             color:"white"
         }
         TextField{height:25
           text:root.controlkey.config.blockstyle
           width:50
           anchors.verticalCenter: parent.verticalCenter
           onEditingFinished:{
           root.controlkey.config.blockstyle=text
           }
         }
         }


         Row{
         spacing:5
         Text{
             font.family:"Loma"
             font.pixelSize: 20
             text:"背景色:"
             color:"white"
         }
         TextField{height:25
           text:root.controlkey.config.background
           width:50
           anchors.verticalCenter: parent.verticalCenter
         }
         }*/
            }
        }
    }
}
