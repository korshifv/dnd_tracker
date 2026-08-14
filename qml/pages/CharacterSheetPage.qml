import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import DndTracker

Item {
    id: page
    required property string filePath
    property var details: ({})
    signal backRequested()

    function reload() {
        details = App.characterDetails(filePath)
    }

    function save() {
        const values = {
            name: nameField.text,
            playerName: playerField.text,
            charClass: classField.text,
            subclass: subclassField.text,
            race: raceField.text,
            background: backgroundField.text,
            alignment: alignmentField.text,
            experience: experienceField.text,
            level: levelSpin.value,
            proficiency: proficiencySpin.value,
            hp: hpSpin.value,
            hpMax: hpMaxSpin.value,
            hpTemp: hpTempSpin.value,
            armorClass: acSpin.value,
            initiative: initSpin.value,
            speed: speedField.text,
            hitDie: hitDieField.text,
            str: strSpin.value,
            dex: dexSpin.value,
            con: conSpin.value,
            int: intSpin.value,
            wis: wisSpin.value,
            cha: chaSpin.value
        }
        if (App.saveCharacterBasics(filePath, values)) {
            saveState.text = "Сохранено ✓"
            saveFlash.restart()
            reload()
        }
    }

    Component.onCompleted: reload()

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 64
            color: Theme.surface
            border.width: 1
            border.color: Theme.border

            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 10
                AppButton { text: "← Назад"; onClicked: page.backRequested() }
                Label {
                    Layout.fillWidth: true
                    text: page.details.name || "Персонаж"
                    color: Theme.text
                    font.pixelSize: 18
                    font.weight: Font.Bold
                    elide: Text.ElideRight
                }
                Label { id: saveState; color: Theme.success; opacity: 0 }
                AppButton { text: "Сохранить"; primary: true; onClicked: page.save() }
            }
        }

        Flickable {
            id: flick
            Layout.fillWidth: true
            Layout.fillHeight: true
            contentWidth: width
            contentHeight: content.implicitHeight + 32
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: ScrollBar {}

            ColumnLayout {
                id: content
                width: flick.width
                spacing: 14

                GridLayout {
                    Layout.fillWidth: true
                    Layout.leftMargin: 16
                    Layout.rightMargin: 16
                    Layout.topMargin: 16
                    columns: flick.width < 760 ? 1 : 2
                    columnSpacing: 14
                    rowSpacing: 14

                    Surface {
                        Layout.fillWidth: true
                        implicitHeight: identityLayout.implicitHeight + 28
                        ColumnLayout {
                            id: identityLayout
                            anchors.fill: parent
                            anchors.margins: 14
                            spacing: 10
                            Label { text: "ПЕРСОНАЖ"; color: Theme.text; font.pixelSize: 16; font.weight: Font.Bold }
                            GridLayout {
                                Layout.fillWidth: true
                                columns: flick.width < 560 ? 1 : 2
                                columnSpacing: 10
                                rowSpacing: 8
                                LabeledField { id: nameField; Layout.fillWidth: true; label: "ИМЯ"; text: page.details.name || "" }
                                LabeledField { id: playerField; Layout.fillWidth: true; label: "ИГРОК"; text: page.details.playerName || "" }
                                LabeledField { id: classField; Layout.fillWidth: true; label: "КЛАСС"; text: page.details.charClass || "" }
                                LabeledField { id: subclassField; Layout.fillWidth: true; label: "ПОДКЛАСС"; text: page.details.subclass || "" }
                                LabeledField { id: raceField; Layout.fillWidth: true; label: "РАСА"; text: page.details.race || "" }
                                LabeledField { id: backgroundField; Layout.fillWidth: true; label: "ПРЕДЫСТОРИЯ"; text: page.details.background || "" }
                                LabeledField { id: alignmentField; Layout.fillWidth: true; label: "МИРОВОЗЗРЕНИЕ"; text: page.details.alignment || "" }
                                LabeledField { id: experienceField; Layout.fillWidth: true; label: "ОПЫТ"; text: page.details.experience || "" }
                            }
                            RowLayout {
                                Layout.fillWidth: true
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Label { text: "УРОВЕНЬ"; color: Theme.textMuted; font.pixelSize: 11 }
                                    SpinBox { id: levelSpin; Layout.fillWidth: true; from: 1; to: 40; editable: true; value: page.details.level || 1 }
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Label { text: "МАСТЕРСТВО"; color: Theme.textMuted; font.pixelSize: 11 }
                                    SpinBox { id: proficiencySpin; Layout.fillWidth: true; from: 0; to: 12; editable: true; value: page.details.proficiency || 2 }
                                }
                            }
                        }
                    }

                    Surface {
                        Layout.fillWidth: true
                        implicitHeight: combatLayout.implicitHeight + 28
                        ColumnLayout {
                            id: combatLayout
                            anchors.fill: parent
                            anchors.margins: 14
                            spacing: 10
                            Label { text: "БОЙ"; color: Theme.text; font.pixelSize: 16; font.weight: Font.Bold }
                            GridLayout {
                                Layout.fillWidth: true
                                columns: 3
                                columnSpacing: 8
                                rowSpacing: 8
                                Label { text: "HP"; color: Theme.textMuted }
                                Label { text: "MAX"; color: Theme.textMuted }
                                Label { text: "TEMP"; color: Theme.textMuted }
                                SpinBox { id: hpSpin; Layout.fillWidth: true; from: -999; to: 9999; editable: true; value: page.details.hp || 0 }
                                SpinBox { id: hpMaxSpin; Layout.fillWidth: true; from: 0; to: 9999; editable: true; value: page.details.hpMax || 0 }
                                SpinBox { id: hpTempSpin; Layout.fillWidth: true; from: 0; to: 9999; editable: true; value: page.details.hpTemp || 0 }
                                Label { text: "КД"; color: Theme.textMuted }
                                Label { text: "ИНИЦ."; color: Theme.textMuted }
                                Label { text: "СКОРОСТЬ"; color: Theme.textMuted }
                                SpinBox { id: acSpin; Layout.fillWidth: true; from: 0; to: 99; editable: true; value: page.details.armorClass || 0 }
                                SpinBox { id: initSpin; Layout.fillWidth: true; from: -20; to: 99; editable: true; value: page.details.initiative || 0 }
                                LabeledField { id: speedField; Layout.fillWidth: true; label: ""; text: page.details.speed || "" }
                            }
                            LabeledField { id: hitDieField; Layout.fillWidth: true; label: "КОСТЬ ХИТОВ"; text: page.details.hitDie || "" }
                        }
                    }
                }

                Surface {
                    Layout.fillWidth: true
                    Layout.leftMargin: 16
                    Layout.rightMargin: 16
                    implicitHeight: statsLayout.implicitHeight + 28
                    ColumnLayout {
                        id: statsLayout
                        anchors.fill: parent
                        anchors.margins: 14
                        spacing: 10
                        Label { text: "ХАРАКТЕРИСТИКИ"; color: Theme.text; font.pixelSize: 16; font.weight: Font.Bold }
                        GridLayout {
                            Layout.fillWidth: true
                            columns: flick.width < 620 ? 2 : 6
                            columnSpacing: 8
                            rowSpacing: 8
                            ColumnLayout { Label { text: "СИЛ"; color: Theme.textMuted }; SpinBox { id: strSpin; from: 1; to: 30; editable: true; value: page.details.str || 10 } }
                            ColumnLayout { Label { text: "ЛОВ"; color: Theme.textMuted }; SpinBox { id: dexSpin; from: 1; to: 30; editable: true; value: page.details.dex || 10 } }
                            ColumnLayout { Label { text: "ТЕЛ"; color: Theme.textMuted }; SpinBox { id: conSpin; from: 1; to: 30; editable: true; value: page.details.con || 10 } }
                            ColumnLayout { Label { text: "ИНТ"; color: Theme.textMuted }; SpinBox { id: intSpin; from: 1; to: 30; editable: true; value: page.details.int || 10 } }
                            ColumnLayout { Label { text: "МУД"; color: Theme.textMuted }; SpinBox { id: wisSpin; from: 1; to: 30; editable: true; value: page.details.wis || 10 } }
                            ColumnLayout { Label { text: "ХАР"; color: Theme.textMuted }; SpinBox { id: chaSpin; from: 1; to: 30; editable: true; value: page.details.cha || 10 } }
                        }
                    }
                }

                Label {
                    Layout.fillWidth: true
                    Layout.leftMargin: 16
                    Layout.rightMargin: 16
                    text: "QML-лист уже редактирует базовые и боевые данные. Не перенесённые секции LSS остаются в JSON нетронутыми."
                    color: Theme.textMuted
                    wrapMode: Text.WordWrap
                }
                Item { Layout.preferredHeight: 16 }
            }
        }
    }

    SequentialAnimation {
        id: saveFlash
        NumberAnimation { target: saveState; property: "opacity"; from: 0; to: 1; duration: 120 }
        PauseAnimation { duration: 1200 }
        NumberAnimation { target: saveState; property: "opacity"; to: 0; duration: 250 }
        ScriptAction { script: saveState.text = "" }
    }
}
