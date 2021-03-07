import QtQuick 2.0

Rectangle {
    id: rect
    property alias color: rect.color
    property alias border: rect.border
    property bool shadow: false
    scale: root.shadow ? 0.95 : 1
    color: "transparent"
}
