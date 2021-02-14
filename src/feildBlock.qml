import QtQuick 2.15

Rectangle {
    id: root
    property bool dying: false
    property bool spawned: false
    opacity: 1

    Behavior on y {
        id: that
        SmoothedAnimation {
            velocity: 200
            duration: 100
        }
    }

    Behavior on opacity {
        id: opacityAni
        OpacityAnimator {
            duration: 100
        }
    }

    ColorAnimation {
        id: colorAni
        target: root
        property: "color"
        from: Qt.tint(root.color, Qt.hsla(
                            root.color.hslHue,
                            Math.max(Math.ceil(root.color.hslSaturation * 100) - 30,0) / 100,
                            Math.max(Math.ceil(root.color.hslLightness * 100 + 50), 100) / 100,
                            root.color.a))
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
            PropertyChanges {
                target: root
                opacity: 0
            }
            StateChangeScript {
                script: {
                    colorAni.stop()
                    root.destroy(100)
                }
            }
        }
    ]

    function banAimate(is) {
        that.enabled = is
    }
    function banFlash(is) {
        opacityAni.enabled = is
    }
    function playColor() {
        colorAni.start()
    }
}
