import QtQuick 2.15

Rectangle {
    id: root
    property bool dying: false
    property bool spawned: false
    opacity: 0
    color: "transparent"

    Behavior on y {
        id: that
        SmoothedAnimation {
            velocity: 200
            duration: 100
        }
    }

    Behavior on opacity {
        id:opacityAni
        NumberAnimation {
            properties: "opacity"
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

    function banAimate(is) {
        that.enabled = is
    }
    function banFlash(is){
       opacityAni.enabled=is
    }
}
