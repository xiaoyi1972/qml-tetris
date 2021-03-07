import QtQuick 2.15
import QtQuick.Shapes 1.15

  Rectangle {
    id: root
    property bool dying: false
    property bool spawned: false
    opacity: 1

    Behavior on y {
        id: that
        SmoothedAnimation {
            velocity: -1
            duration: 100 //100
        }
    }

    OpacityAnimator {
        id: opA
        target: root
        from: 1
        to: 0
        duration: 100
        onFinished: {
            root.visible = false
            root.destroy()
        }
    }

    ColorAnimation {
        id: colorAni
        target: root
        property: "color"
        from: Qt.tint(
                  root.color, Qt.hsla(
                      root.color.hslHue, Math.max(
                          Math.ceil(root.color.hslSaturation * 100) - 30,
                          0) / 100,
                      Math.max(Math.ceil(
                                   root.color.hslLightness * 100 + 50), 100) / 100, root.color.a))
        to: root.color
        duration: 130
    }

    states: [
        State {
            name: "AliveState"
            when: spawned == true && dying == false
            PropertyChanges {
                target: root
            }
        },
        State {
            name: "DeathState"
            when: dying == true
            StateChangeScript {
                script: {
                    colorAni.stop()
                    opA.start()
                }
            }
        }
    ]

    function banAimate(is) {
        that.enabled = is
    }

    function playColor() {
        colorAni.start()
    }
}
