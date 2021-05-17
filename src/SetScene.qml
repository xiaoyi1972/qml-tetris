import QtQuick 2.7
import QtQuick.Controls 2.15
import QtQml 2.2
import QtQuick.Layouts 1.3
//import QtQuick.Controls.Basic

Item {
    id: root
    property var tetris: null
    visible: true
    focus: false
    signal replayCall

    ColumnLayout {
        anchors.fill: parent
        spacing: 0
        RowLayout {
            ListView {
                id: leftBar
                currentIndex: 0
                spacing: 1
                Layout.minimumHeight: parent.height
                Layout.minimumWidth: parent.width / 100 * 15
                model: ListModel {
                    ListElement {
                        name: "操作"
                    }
                    ListElement {
                        name: "画面"
                    }
                    ListElement {
                        name: "录像"
                    }
                }
                delegate: BasicTabButton {
                    font.pixelSize: 16
                    font.bold: true
                    width: parent.width
                    text: name
                    onClicked: {
                        leftBar.currentIndex = index
                    }
                    Component.onCompleted: {
                        if (index == leftBar.currentIndex)
                            checked = true
                    }
                }
            }

            StackLayout {
                id: hao1
                currentIndex: leftBar.currentIndex
                property int preIndex
                Component.onCompleted: {
                    preIndex = currentIndex
                }

                ScrollView {
                    id: mn
                    clip: true
                    width: parent.width
                    height: parent.height
                    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
                    property int visibleValue: visible ? 0 : 1
                    FadeBehavior on visibleValue {}
                    Column {
                        width: hao1.width
                        spacing: 10
                        padding: 20
                        RowLayout {
                            width: parent.width
                            Text {
                                text: "自移延迟"
                                color: "black"
                                font {
                                    family: "Loma"
                                    pixelSize: 17
                                }
                            }
                            SpinBox {
                                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                value: TetrisConfig.dasDelay
                                editable: true
                                to: validator.top
                                validator: IntValidator {
                                    bottom: 0 // @disable-check M16
                                }
                                textFromValue: function (value) {
                                    return value
                                }
                                Layout.preferredHeight: 25
                                Layout.rightMargin: 30
                                onValueChanged: {
                                    TetrisConfig.dasDelay = value
                                }
                            }
                        }

                        RowLayout {
                            width: parent.width
                            Text {
                                text: "重复延迟"
                                color: "black"
                                font {
                                    family: "Loma"
                                    pixelSize: 17
                                }
                            }
                            SpinBox {
                                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                value: TetrisConfig.arrDelay
                                editable: true
                                to: validator.top
                                validator: IntValidator {
                                    bottom: 0 // @disable-check M16
                                }
                                textFromValue: function (value) {
                                    return value
                                }
                                Layout.preferredHeight: 25
                                Layout.rightMargin: 30
                                onValueChanged: {
                                    TetrisConfig.arrDelay = value
                                }
                            }
                        }

                        RowLayout {
                            width: parent.width
                            Text {
                                text: "软降延迟"
                                color: "black"
                                font {
                                    family: "Loma"
                                    pixelSize: 17
                                }
                            }
                            SpinBox {
                                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                value: TetrisConfig.softdropDelay
                                editable: true
                                to: validator.top
                                validator: IntValidator {
                                    bottom: 0 // @disable-check M16
                                }
                                textFromValue: function (value) {
                                    return value
                                }
                                Layout.preferredHeight: 25
                                Layout.rightMargin: 30
                                onValueChanged: {
                                    TetrisConfig.softdropDelay = value
                                }
                            }
                        }

                        RowLayout {
                            width: parent.width
                            Text {
                                text: "左移"
                                color: "black"
                                font {
                                    family: "Loma"
                                    pixelSize: 17
                                }
                            }
                            TextField {
                                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                Layout.preferredHeight: 25
                                Layout.preferredWidth: 50
                                Layout.rightMargin: 30
                                text: tetrisConfig.retKey(TetrisConfig.leftKey)
                                readOnly: true
                                Keys.onPressed: {
                                    if (!tetrisConfig.isConflict(event.key)) {
                                        text = tetrisConfig.retKey(event.key)
                                        TetrisConfig.leftKey = event.key
                                    }
                                    event.accepted = true
                                }
                            }
                        }

                        RowLayout {
                            width: parent.width
                            Text {
                                text: "右移"
                                color: "black"
                                font {
                                    family: "Loma"
                                    pixelSize: 17
                                }
                            }
                            TextField {
                                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                Layout.preferredHeight: 25
                                Layout.preferredWidth: 50
                                Layout.rightMargin: 30
                                text: tetrisConfig.retKey(TetrisConfig.rightKey)
                                readOnly: true
                                Keys.onPressed: {
                                    //      tetrisConfig.config.right_move = ""
                                    if (!tetrisConfig.isConflict(event.key)) {
                                        text = tetrisConfig.retKey(event.key)
                                        TetrisConfig.rightKey = event.key
                                    }
                                    event.accepted = true
                                }
                            }
                        }

                        RowLayout {
                            width: parent.width
                            Text {
                                text: "软降"
                                color: "black"
                                font {
                                    family: "Loma"
                                    pixelSize: 17
                                }
                            }
                            TextField {
                                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                Layout.preferredHeight: 25
                                Layout.preferredWidth: 50
                                Layout.rightMargin: 30
                                text: tetrisConfig.retKey(
                                          TetrisConfig.softDropKey)
                                readOnly: true
                                Keys.onPressed: {
                                    // tetrisConfig.config.soft_drop = ""
                                    if (!tetrisConfig.isConflict(event.key)) {
                                        text = tetrisConfig.retKey(event.key)
                                        TetrisConfig.softDropKey = event.key
                                    }
                                    event.accepted = true
                                }
                            }
                        }

                        RowLayout {
                            width: parent.width
                            Text {
                                text: "硬降"
                                color: "black"
                                font {
                                    family: "Loma"
                                    pixelSize: 17
                                }
                            }
                            TextField {
                                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                Layout.preferredHeight: 25
                                Layout.preferredWidth: 50
                                Layout.rightMargin: 30
                                text: tetrisConfig.retKey(
                                          TetrisConfig.harddropKey)
                                readOnly: true
                                Keys.onPressed: {
                                    tetrisConfig.config.hard_drop = ""
                                    if (!tetrisConfig.isConflict(event.key)) {
                                        text = tetrisConfig.retKey(event.key)
                                        TetrisConfig.harddropKey = event.key
                                    }
                                    event.accepted = true
                                }
                            }
                        }

                        RowLayout {
                            width: parent.width
                            Text {
                                text: "顺时针旋转"
                                color: "black"
                                font {
                                    family: "Loma"
                                    pixelSize: 17
                                }
                            }
                            TextField {
                                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                Layout.preferredHeight: 25
                                Layout.preferredWidth: 50
                                Layout.rightMargin: 30
                                text: tetrisConfig.retKey(TetrisConfig.ccwKey)
                                readOnly: true
                                Keys.onPressed: {
                                    tetrisConfig.config.rotate_normal = ""
                                    if (!tetrisConfig.isConflict(event.key)) {
                                        text = tetrisConfig.retKey(event.key)
                                        TetrisConfig.ccwKey = event.key
                                    }
                                    event.accepted = true
                                }
                            }
                        }

                        RowLayout {
                            width: parent.width
                            Text {
                                text: "逆时针旋转"
                                color: "black"
                                font {
                                    family: "Loma"
                                    pixelSize: 17
                                }
                            }
                            TextField {
                                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                Layout.preferredHeight: 25
                                Layout.preferredWidth: 50
                                Layout.rightMargin: 30
                                text: tetrisConfig.retKey(TetrisConfig.cwKey)
                                readOnly: true
                                Keys.onPressed: {
                                    tetrisConfig.config.rotate_reverse = ""
                                    if (!tetrisConfig.isConflict(event.key)) {
                                        text = tetrisConfig.retKey(event.key)
                                        TetrisConfig.cwKey = event.key
                                    }
                                    event.accepted = true
                                }
                            }
                        }

                        RowLayout {
                            width: parent.width
                            Text {
                                text: "暂存"
                                color: "black"
                                font {
                                    family: "Loma"
                                    pixelSize: 17
                                }
                            }
                            TextField {
                                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                Layout.preferredHeight: 25
                                Layout.preferredWidth: 50
                                Layout.rightMargin: 30
                                text: tetrisConfig.retKey(TetrisConfig.holdKey)
                                readOnly: true
                                Keys.onPressed: {
                                    tetrisConfig.config.hold = ""
                                    if (!tetrisConfig.isConflict(event.key)) {
                                        text = tetrisConfig.retKey(event.key)
                                        TetrisConfig.holdKey = event.key
                                    }
                                    event.accepted = true
                                }
                            }
                        }

                        RowLayout {
                            width: parent.width
                            Text {
                                text: "重开"
                                color: "black"
                                font {
                                    family: "Loma"
                                    pixelSize: 17
                                }
                            }
                            TextField {
                                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                Layout.preferredHeight: 25
                                Layout.preferredWidth: 50
                                Layout.rightMargin: 30
                                text: tetrisConfig.retKey(
                                          TetrisConfig.restartKey)
                                readOnly: true
                                Keys.onPressed: {
                                    tetrisConfig.config.restart = ""
                                    if (!tetrisConfig.isConflict(event.key)) {
                                        text = tetrisConfig.retKey(event.key)
                                        TetrisConfig.restartKey = event.key
                                    }
                                    event.accepted = true
                                }
                            }
                        }

                        RowLayout {
                            width: parent.width
                            Text {
                                text: "回放"
                                color: "black"
                                font {
                                    family: "Loma"
                                    pixelSize: 17
                                }
                            }
                            TextField {
                                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                Layout.preferredHeight: 25
                                Layout.preferredWidth: 50
                                Layout.rightMargin: 30
                                text: tetrisConfig.retKey(TetrisConfig.replay)
                                readOnly: true
                                Keys.onPressed: {
                                    tetrisConfig.config.replay = ""
                                    if (!tetrisConfig.isConflict(event.key)) {
                                        text = tetrisConfig.retKey(event.key)
                                        TetrisConfig.replay = event.key
                                    }
                                    event.accepted = true
                                }
                            }
                        }

                        RowLayout {
                            width: parent.width
                            Text {
                                text: "机器人托管"
                                color: "black"
                                font {
                                    family: "Loma"
                                    pixelSize: 17
                                }
                            }
                            TextField {
                                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                                Layout.preferredHeight: 25
                                Layout.preferredWidth: 50
                                Layout.rightMargin: 30
                                text: tetrisConfig.retKey(TetrisConfig.bot)
                                readOnly: true
                                Keys.onPressed: {
                                    //tetrisConfig.config.ai = ""
                                    if (!tetrisConfig.isConflict(event.key)) {
                                        text = tetrisConfig.retKey(event.key)
                                        TetrisConfig.bot = event.key
                                    }
                                    event.accepted = true
                                }
                            }
                        }
                    }
                }

                Column {
                    width: parent.width
                    spacing: 10
                    padding: 20
                    property int visibleValue: visible ? 0 : 1
                    FadeBehavior on visibleValue {}
                    RowLayout {
                        width: parent.width
                        Text {
                            text: "阴影"
                            color: "black"
                            font {
                                family: "Loma"
                                pixelSize: 17
                            }
                        }
                        Switch {
                            Layout.preferredHeight: 25
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            Layout.rightMargin: 30
                            checked: tetrisConfig.shadow
                            onCheckedChanged: tetrisConfig.shadow = checked
                        }
                    }
                    RowLayout {
                        width: parent.width
                        Text {
                            text: "机器人耗费时间"
                            color: "black"
                            font {
                                family: "Loma"
                                pixelSize: 17
                            }
                        }
                        SpinBox {
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            value: TetrisConfig.botTimeCost
                            editable: true
                            to: validator.top
                            validator: IntValidator {
                                bottom: 1 // @disable-check M16
                            }
                            textFromValue: function (value) {
                                return value
                            }
                            Layout.preferredHeight: 25
                            Layout.rightMargin: 30
                            onValueChanged: {
                                TetrisConfig.botTimeCost = value
                            }
                        }
                    }
                }

                Column {
                    padding: 20
                    spacing: 10
                    width: parent.width
                    height: parent.height
                    property int visibleValue: visible ? 0 : 1
                    FadeBehavior on visibleValue {}
                    Label {
                        font.family: "Loma"
                        font.pixelSize: 17
                        text: "录像回放◀"
                        color: "black"
                    }
                    Column {
                        spacing: 10
                        width: parent.width - parent.padding
                        ScrollView {
                            id: view
                            width: parent.width - 10
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
                                    color: "transparent"
                                    border.width: 1
                                }
                                text: ""
                            }
                        }

                        Button {
                            width: 60
                            height: 30
                            text: "播放"
                            onClicked: {
                                root.replayCall()
                                if (tetris != null) {
                                    tetris.focus = true
                                    tetris.replay(replayData.text)
                                }
                                replayData.text = ""
                            }
                        }
                    }
                }
            }
        }
    }
}
