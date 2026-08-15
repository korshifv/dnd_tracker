import QtQuick
import QtQuick.Controls
import DndTracker

TextArea {
    id: control
    activeFocusOnPress: false
    color: Theme.text
    placeholderTextColor: Theme.textMuted
    selectionColor: Theme.accent
    selectedTextColor: Theme.onAccent
    wrapMode: TextArea.Wrap
    selectByMouse: true
    padding: 10

    // Keep drags/swipes from summoning the Android keyboard, but treat a real
    // tap as an explicit request to edit. TapHandler cancels once the pointer
    // turns into a drag, so scrolling and editing no longer fight each other.
    TapHandler {
        gesturePolicy: TapHandler.DragThreshold
        onTapped: function(eventPoint, button) {
            control.forceActiveFocus()
            control.cursorPosition = control.positionAt(eventPoint.position.x, eventPoint.position.y)
            Qt.inputMethod.show()
        }
    }

    background: Rectangle {
        color: Theme.surfaceRaised
        radius: Theme.radiusSmall
        border.width: control.activeFocus ? 2 : 1
        border.color: control.activeFocus ? Theme.accent : Theme.border
    }
}
