import QtQuick
import QtQuick.Controls
import DndTracker

Button {
    id: control
    implicitHeight: Theme.touchTarget
    padding: 12
    property bool primary: false
    property bool danger: false

    contentItem: Text {
        text: control.text
        color: control.primary ? Theme.onAccent : (control.danger ? Theme.danger : Theme.text)
        font.pixelSize: 14
        font.weight: Font.DemiBold
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    background: Rectangle {
        radius: Theme.radiusSmall
        color: control.down || control.hovered
               ? (control.primary ? Theme.accentStrong : Theme.surfaceHover)
               : (control.primary ? Theme.accent : Theme.surfaceRaised)
        border.width: control.primary ? 0 : 1
        border.color: control.danger ? Theme.danger : Theme.border
    }
}
