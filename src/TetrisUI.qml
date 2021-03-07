import QtQuick 2.15
import QtQuick.Shapes 1.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: root
    width: tetrisConfig.blockSize * tetrisConfig.maxColumn
    height: tetrisConfig.blockSize * (tetrisConfig.maxRow + tetrisConfig.rowOver)
    property var heightInline: tetrisConfig.blockSize * tetrisConfig.maxRow
    property var tetrisGame: null
    property var tetris: null
    property alias background: background
    property alias active: fk
    property alias shadow: shadow
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

    Item {
        anchors.fill: parent
        layer.enabled: true
        visible: true

        Item {
            id: background
            anchors.fill: parent
            property double range: 1
            NumberAnimation {
                id: animationDead
                target: background
                property: "range"
                from: 1
                to: 0
                duration: 1000
            }
            function playDead() {
                animationDead.start()
            }
            function stop() {
                animationDead.stop()
            }
           /* layer.enabled: true
            layer.effect: ShaderEffect {
                property variant src: background
                property double range: background.range
                property double unVisible: (root.tetrisGame.RowOver)
                                           / (root.tetrisGame.maxRow + root.tetrisGame.RowOver)
                vertexShader: "
uniform highp mat4 qt_Matrix;
attribute highp vec4 qt_Vertex;
attribute highp vec2 qt_MultiTexCoord0;
varying highp vec2 coord;
void main() {
coord = qt_MultiTexCoord0;
gl_Position = qt_Matrix * qt_Vertex;
}"
                fragmentShader: "
varying highp vec2 coord;
uniform sampler2D src;
uniform float range;
uniform float unVisible;
uniform lowp float qt_Opacity;
void main() {
lowp vec4 tex = texture2D(src, coord);
float test= step(coord.y,range);
tex.rgb=mix(vec3(0.6,0.6,0.6),tex.rgb,test);
tex.a=tex.a*step(unVisible,coord.y);
gl_FragColor =vec4(tex.rgb,tex.a);
}"
            }*/
        }

        Mino {
            id: fk
            tight: false
            property int rs: 0
            property int xyDuration: 50
            property int rDuration: 60
            property bool banAni: true
            rotation: 0
            transformOrigin: Item.Center
      //      layer.enabled: false
            Behavior on x {
                id: xb
                enabled: fk.banAni && tetrisConfig.operTransition
                SmoothedAnimation {
                    duration: Math.ceil(
                                  fk.xyDuration / Math.ceil(
                                      Math.abs(
                                          xb.targetValue - fk.x) / tetrisConfig.blockSize + .1))
                    velocity: -1
                }
            }
            Behavior on y {
                id: yb
                enabled: fk.banAni && tetrisConfig.operTransition
                SmoothedAnimation {
                    duration: Math.ceil(
                                  fk.xyDuration / Math.ceil(
                                      Math.abs(
                                          yb.targetValue - fk.y) / tetrisConfig.blockSize + .1))
                    velocity: -1
                }
            }
            Behavior on rotation {
                id: rsb
                enabled: fk.banAni && tetrisConfig.operTransition
                RotationAnimation {
                    duration: fk.rDuration
                    direction: RotationAnimator.Shortest
                }
            }
            function toggleBehavior(is) {
                fk.banAni = is
            }
        }

        Mino {
            id: shadow
            visible: true
            property int rs: 0
            property int xyDuration: 50
            property bool banAni: true
            property double yMoving
            tight: false
            transformOrigin: Item.Center
            rotation: 0
            shadow: true
            y: Math.floor(shadow.yMoving / tetrisConfig.blockSize) * tetrisConfig.blockSize
            Behavior on x {
                id: xb1
                enabled: shadow.banAni && tetrisConfig.operTransition
                SmoothedAnimation {
                    duration: Math.ceil(
                                  shadow.xyDuration / Math.ceil(
                                      Math.abs(
                                          xb1.targetValue - shadow.x) / tetrisConfig.blockSize + .1))
                    velocity: -1
                }
            }
            Behavior on yMoving {
                id: yb1
                enabled: shadow.banAni && tetrisConfig.operTransition
                SmoothedAnimation {
                    duration: Math.ceil(
                                  shadow.xyDuration / Math.ceil(
                                      Math.abs(
                                          yb1.targetValue - shadow.yMoving) / tetrisConfig.blockSize + .1))
                    velocity: -1
                }
            }
            function toggleBehavior(is) {
                shadow.banAni = is
            }
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
            delegate: Mino {
                ListView.onPooled: create(-1)
                ListView.onReused: create(minoType)
                Component.onCompleted: {
                    create(minoType)
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

    Mino {
        anchors.topMargin: 10
        anchors.top: holdText.bottom
        anchors.right: holdText.right
        id: hold
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
