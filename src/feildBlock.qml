import QtQuick 2.15

Rectangle {
    id: root
    property bool dying: false
    property bool spawned: false
    opacity: 0

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

    states: [
        State {
            name: "AliveState"
            when: spawned == true && dying == false
             PropertyChanges {
                target: root
                opacity: 1
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
                script: root.destroy(100)
            }
        }
    ]

    NumberAnimation {
        target: root
        properties: "opacity"
        from: 0
        to: 1
        duration: 100
        easing.type: Easing.InOutBack
        loops: Animation.Infinite
        // running:true
    }

    function banAimate(is) {
        that.enabled = is
    }
    function banFlash(is) {
        opacityAni.enabled = is
    }
}
