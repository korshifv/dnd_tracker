import QtQuick
import QtQuick.Controls
import DndTracker

CheckBox {
    id: control
    property color fillColor: Theme.accent
    padding: 0
    spacing: 0
    implicitWidth: 28
    implicitHeight: 28

    indicator: Rectangle {
        implicitWidth: 24
        implicitHeight: 24
        x: (control.width - width) / 2
        y: (control.height - height) / 2
        radius: 5
        color: control.checked ? control.fillColor : Theme.surfaceRaised
        border.width: control.activeFocus ? 2 : 1
        border.color: control.checked ? control.fillColor : (control.activeFocus ? Theme.accent : Theme.border)

        Behavior on color { ColorAnimation { duration: 100 } }
    }

    contentItem: Item {}
}
