// FadeBehavior.qml
import QtQuick 2.15

Behavior {
    id: root
    property Item fadeTarget: targetProperty.object
    enabled: !root.targetValue
    NumberAnimation {
        target: root.fadeTarget
        property: "opacity"
        from:root.targetValue
        to: !root.targetValue
        easing.type: Easing.InQuart
        duration: 90
    }
}
