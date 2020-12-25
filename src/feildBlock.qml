import QtQuick 2.15

Rectangle {
    id: root
    property bool dying: false
    property bool spawned: false
    opacity: 0
    color: "transparent"

    Behavior on y {
        id: that
        NumberAnimation {
            properties: "y"
            duration: 100
            easing.type: Easing.Linear
        }
    }

    Behavior on opacity {
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
}
