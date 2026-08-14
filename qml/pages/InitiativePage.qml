import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import DndTracker

Item {
    id: page
    signal openCharacter(string filePath)

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 14

        Surface {
            Layout.fillWidth: true
            implicitHeight: 88
            RowLayout {
                anchors.fill: parent
                anchors.margins: 14
                spacing: 10
                ColumnLayout {
                    Layout.fillWidth: true
                    Label {
                        Layout.fillWidth: true
                        text: Initiative.currentTurnName.length
                              ? "Сейчас ходит: " + Initiative.currentTurnName
                              : "Бой не начат"
                        color: Theme.text
                        font.pixelSize: 18
                        font.weight: Font.Bold
                        elide: Text.ElideRight
                    }
                    Label { text: "Раунд " + Initiative.round; color: Theme.textMuted }
                }
                AppButton { text: "Следующий"; primary: true; enabled: Initiative.hasCombatants; onClicked: Initiative.nextTurn() }
                AppButton { text: "Сброс"; onClicked: Initiative.resetCombat() }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            AppButton { text: "+ Боец"; primary: true; onClicked: Initiative.addBlank("") }
            AppButton { text: "Сортировать"; onClicked: Initiative.sortByInitiative() }
            Item { Layout.fillWidth: true }
            AppButton { text: "Очистить"; danger: true; enabled: Initiative.hasCombatants; onClicked: confirmClear.open() }
        }

        ListView {
            id: list
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 10
            clip: true
            model: Initiative
            boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: ScrollBar {}

            delegate: Surface {
                required property int index
                required property string name
                required property int hp
                required property int armorClass
                required property int initiative
                required property string status
                required property string filePath
                required property string groupName
                required property string avatarColor
                required property bool activeTurn

                width: ListView.view.width
                implicitHeight: width < 700 ? 280 : 148
                border.width: activeTurn ? 2 : 1
                border.color: activeTurn ? Theme.accent : Theme.border

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 9

                    RowLayout {
                        Layout.fillWidth: true
                        Rectangle {
                            width: 34
                            height: 34
                            radius: 17
                            color: avatarColor.length ? avatarColor : Theme.accent
                        }
                        TextField {
                            Layout.fillWidth: true
                            text: name
                            color: Theme.text
                            font.weight: Font.DemiBold
                            onEditingFinished: Initiative.setName(index, text)
                            background: Rectangle { color: Theme.surfaceRaised; radius: Theme.radiusSmall; border.color: Theme.border; border.width: 1 }
                        }
                        TextField {
                            Layout.preferredWidth: 150
                            visible: parent.width > 560
                            text: groupName || "Основная группа"
                            placeholderText: "Группа"
                            color: Theme.textMuted
                            onEditingFinished: Initiative.setGroup(index, text)
                            background: Rectangle { color: Theme.surfaceRaised; radius: Theme.radiusSmall; border.color: Theme.border; border.width: 1 }
                        }
                        AppButton { text: "×"; danger: true; implicitWidth: 46; onClicked: Initiative.removeAt(index) }
                    }

                    TextField {
                        Layout.fillWidth: true
                        visible: parent.width <= 560
                        text: groupName || "Основная группа"
                        placeholderText: "Группа"
                        color: Theme.textMuted
                        onEditingFinished: Initiative.setGroup(index, text)
                        background: Rectangle { color: Theme.surfaceRaised; radius: Theme.radiusSmall; border.color: Theme.border; border.width: 1 }
                    }

                    Flow {
                        Layout.fillWidth: true
                        spacing: 8

                        RowLayout {
                            width: 260
                            height: 44
                            Label {
                                text: "HP " + hp
                                color: Theme.text
                                font.weight: Font.Bold
                                Layout.preferredWidth: 62
                            }
                            SpinBox {
                                id: effectAmount
                                from: 1
                                to: 999
                                value: 1
                                editable: true
                                Layout.preferredWidth: 82
                            }
                            AppButton {
                                text: "−"
                                danger: true
                                implicitWidth: 46
                                ToolTip.visible: hovered
                                ToolTip.text: "Урон"
                                onClicked: Initiative.applyDamage(index, effectAmount.value)
                            }
                            AppButton {
                                text: "+"
                                implicitWidth: 46
                                ToolTip.visible: hovered
                                ToolTip.text: "Лечение"
                                onClicked: Initiative.applyHeal(index, effectAmount.value)
                            }
                        }

                        Label {
                            width: 78
                            height: 44
                            text: armorClass > 0 ? "КД " + armorClass : "КД —"
                            color: Theme.text
                            verticalAlignment: Text.AlignVCenter
                        }

                        ColumnLayout {
                            width: 120
                            height: 44
                            spacing: 1
                            Label { text: "Инициатива"; color: Theme.textMuted; font.pixelSize: 10 }
                            SpinBox {
                                Layout.fillWidth: true
                                from: -20
                                to: 99
                                value: initiative
                                editable: true
                                onValueModified: Initiative.setInitiative(index, value)
                            }
                        }

                        TextField {
                            width: Math.max(180, parent.width - 490)
                            height: 44
                            text: status
                            placeholderText: "Статусы…"
                            color: Theme.text
                            onEditingFinished: Initiative.setStatus(index, text)
                            background: Rectangle { color: Theme.surfaceRaised; radius: Theme.radiusSmall; border.color: Theme.border; border.width: 1 }
                        }

                        AppButton {
                            visible: filePath.length > 0
                            text: "Чарник"
                            onClicked: page.openCharacter(filePath)
                        }
                    }
                }
            }

            Label {
                anchors.centerIn: parent
                visible: list.count === 0
                text: "Инициатива пустая\nДобавь бойца или персонажа"
                color: Theme.textMuted
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }

    Dialog {
        id: confirmClear
        title: "Очистить инициативу?"
        modal: true
        anchors.centerIn: Overlay.overlay
        standardButtons: Dialog.Yes | Dialog.Cancel
        onAccepted: Initiative.clearAll()
    }
}
