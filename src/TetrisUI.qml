import QtQuick 2.15
import QtQuick.Shapes 1.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: root
    width: root.tetrisGame.blockSize * root.tetrisGame.maxColumn
    height: root.tetrisGame.blockSize * (root.tetrisGame.maxRow + root.tetrisGame.RowOver)
    property var heightInline: root.tetrisGame.blockSize * root.tetrisGame.maxRow
    property var controlkey: null
    property var tetrisGame: null
    property var tetris: null
    property alias feildLayer: feildLayer
    property alias background: background
    property alias active: fk
    property alias shadow: shadow
    property alias nexts: nextsC
    property alias hold: hold
    property alias effectClear: effectClear
    property alias clearText: clearText

    Shape {
        id: displayBorder
        width: root.tetrisGame.blockSize * root.tetrisGame.maxColumn
        height: root.tetrisGame.blockSize * root.tetrisGame.maxRow
        anchors.top: parent.top
        anchors.topMargin: root.tetrisGame.blockSize * root.tetrisGame.RowOver
        ShapePath {
            strokeWidth: 1
            strokeColor: "black"
            startX: 0
            startY: 0
            PathLine {
                x: 0
                y: root.tetrisGame.blockSize * (root.tetrisGame.maxRow)
            }
            PathLine {
                x: root.tetrisGame.blockSize * root.tetrisGame.maxColumn
                y: root.tetrisGame.blockSize * (root.tetrisGame.maxRow)
            }
            PathLine {
                x: root.tetrisGame.blockSize * root.tetrisGame.maxColumn
                y: 0
            }
        }
    }

    ShaderEffect {
        id: feildLayer
        anchors.fill: parent
        visible: true
        property variant src: background
        property double range: 1
        property double unVisible: (root.tetrisGame.RowOver)
                                   / (root.tetrisGame.maxRow + root.tetrisGame.RowOver)
        NumberAnimation {
            id: animationDead
            target: feildLayer
            property: "range"
            from: 1
            to: 0
            //  loops: Animation.Infinite
            duration: 1000
        }
        function playDead() {
            animationDead.start()
        }
        function stop(){
            animationDead.stop()
        }
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
    }

    Item {
        id: background
        anchors.fill: parent
        layer.enabled: true
        visible: false
    }

    Item {
        id: fk
        property int minoType: -1
        property var board: []
        property int rs: 0
        property int xyDuration: 50
        property int rDuration: 60
        x: 0
        y: 0
        transformOrigin: Item.Center
        rotation: 0
        Behavior on x {
            id: xb
            SmoothedAnimation {
                duration: fk.xyDuration / Math.ceil(
                              Math.abs(xb.targetValue - fk.x) / 19 + .1)
            }
        }
        Behavior on y {
            id: yb
            SmoothedAnimation {
                duration: fk.xyDuration / Math.ceil(
                              Math.abs(yb.targetValue - fk.y) / 19 + .1)
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
        property int xyDuration:50
        x: 0
        y: 0
        transformOrigin: Item.Center
        rotation: 0
        Behavior on x {
            id: xb1
            SmoothedAnimation {
                duration: shadow.xyDuration / Math.ceil(
                              Math.abs(xb1.targetValue - shadow.x) / 19 + .1)
            }
        }
        Behavior on y {
            id: yb1
            SmoothedAnimation {
                duration: shadow.xyDuration / Math.ceil(
                              Math.abs(yb1.targetValue - shadow.y) / 19 + .1)
            }
        }

        function toggleBehavior(is) {
            xb1.enabled = is
            yb1.enabled = is
        }
    }

    Column {
        anchors.top: displayBorder.top
        anchors.left: root.right
        anchors.margins: 5
        anchors.topMargin: 0
        spacing: 10
        Text {
            id: nexts
            font.pointSize: 16
            text: "预览"
        }
        Column {
            id: nextsC
            spacing: 20
            move: Transition {
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
                    duration: 150
                    from: 0
                    to: 1
                }
            }
        }
    }

        Text {
            id:holdText
            anchors.margins: 5
            anchors.topMargin: 0
            anchors.top: displayBorder.top
            anchors.right: root.left
            font.pointSize: 16
            text: "暂存"
             horizontalAlignment: Text.AlignRightA
        }
        Mino {
            anchors.topMargin: 10
            anchors.top:holdText.bottom
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
            //    font.bold: true
            text: ""
            horizontalAlignment: Text.AlignRight
        }

        Text {
            id: clearText
            font.pointSize: 16
            // visible: false
            //font.bold: true
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
                if (setscene.visible === false) {
                    tetris.setKeyboard(controlkey.config)
                    tetris.focus = true
                }
            }
        }
    }
}
