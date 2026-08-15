import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import DndTracker

Item {
    id: page
    required property string filePath
    property var details: ({})
    property var saves: ({})
    property var skills: []
    property var deathSuccess: [false, false, false]
    property var deathFail: [false, false, false]
    property var weapons: []
    property var spellSlots: [0,0,0,0,0,0,0,0,0,0]
    property var expendedSlots: [[],[],[],[],[],[],[],[],[],[]]
    property var spellTexts: ["","","","","","","","","",""]
    property string spellAbility: "int"
    property int currentTab: 0
    signal backRequested()

    readonly property var tabNames: ["Основное", "Навыки", "Описание", "Оружие", "Магия"]
    readonly property var statCodes: ["str", "dex", "con", "int", "wis", "cha"]
    readonly property var statNames: ["СИЛ", "ЛОВ", "ТЕЛ", "ИНТ", "МУД", "ХАР"]
    readonly property var abilityCodes: ["none", "str", "dex", "con", "int", "wis", "cha"]
    readonly property var abilityNames: ["Нет", "Сила", "Ловкость", "Тело", "Интеллект", "Мудрость", "Харизма"]
    readonly property var casterAbilityCodes: ["int", "wis", "cha"]
    readonly property var casterAbilityNames: ["Интеллект", "Мудрость", "Харизма"]

    function clone(value) {
        return JSON.parse(JSON.stringify(value === undefined || value === null ? [] : value))
    }

    function reload() {
        details = App.characterDetails(filePath)
        saves = clone(details.saves || {})
        skills = clone(details.skills || [])
        deathSuccess = clone(details.deathSuccess || [false, false, false])
        deathFail = clone(details.deathFail || [false, false, false])
        weapons = clone(details.weapons || [])
        spellSlots = clone(details.spellSlots || [0,0,0,0,0,0,0,0,0,0])
        expendedSlots = clone(details.expendedSlots || [[],[],[],[],[],[],[],[],[],[]])
        spellTexts = clone(details.spellTexts || ["","","","","","","","","",""])
        spellAbility = details.spellAbility || "int"
    }

    function statScore(code) {
        if (code === "str") return strSpin.value
        if (code === "dex") return dexSpin.value
        if (code === "con") return conSpin.value
        if (code === "int") return intSpin.value
        if (code === "wis") return wisSpin.value
        if (code === "cha") return chaSpin.value
        return 10
    }

    function statModifier(code) {
        return Math.floor((statScore(code) - 10) / 2)
    }

    function signed(value) {
        return (value >= 0 ? "+" : "") + value
    }

    function skillBonus(skill) {
        if (!skill) return 0
        return statModifier(skill.baseStat) + (skill.profLevel || 0) * proficiencySpin.value
    }

    function passivePerception() {
        for (let i = 0; i < skills.length; ++i) {
            if (skills[i].key === "perception")
                return 10 + skillBonus(skills[i])
        }
        return 10 + statModifier("wis")
    }

    function weaponHit(weapon) {
        if (!weapon) return 0
        const mod = weapon.ability === "none" ? 0 : statModifier(weapon.ability)
        return mod + (weapon.isProf ? proficiencySpin.value : 0) + (weapon.magicBonus || 0)
    }

    function spellModifier() {
        return statModifier(spellAbility)
    }

    function setDeath(which, index, checked) {
        let copy = clone(which === "success" ? deathSuccess : deathFail)
        while (copy.length < 3) copy.push(false)
        copy[index] = checked
        if (which === "success") deathSuccess = copy
        else deathFail = copy
    }

    function setSkillLevel(index, level) {
        let copy = clone(skills)
        if (index >= 0 && index < copy.length) {
            copy[index].profLevel = level
            skills = copy
        }
    }

    function setSpellSlots(level, count) {
        let slots = clone(spellSlots)
        while (slots.length < 10) slots.push(0)
        slots[level] = count
        spellSlots = slots

        let spent = clone(expendedSlots)
        while (spent.length < 10) spent.push([])
        let pips = spent[level] || []
        while (pips.length < count) pips.push(false)
        if (pips.length > count) pips.splice(count)
        spent[level] = pips
        expendedSlots = spent
    }

    function setSpellSpent(level, slot, checked) {
        let spent = clone(expendedSlots)
        while (spent.length < 10) spent.push([])
        let pips = spent[level] || []
        while (pips.length <= slot) pips.push(false)
        pips[slot] = checked
        spent[level] = pips
        expendedSlots = spent
    }

    function setSpellText(level, text) {
        let copy = clone(spellTexts)
        while (copy.length < 10) copy.push("")
        copy[level] = text
        spellTexts = copy
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
            inspiration: inspirationCheck.checked,
            hp: hpSpin.value,
            hpMax: hpMaxSpin.value,
            hpTemp: hpTempSpin.value,
            armorClass: acSpin.value,
            initiative: page.statModifier("dex"),
            speed: speedField.text,
            hitDie: hitDieField.text,
            str: strSpin.value,
            dex: dexSpin.value,
            con: conSpin.value,
            int: intSpin.value,
            wis: wisSpin.value,
            cha: chaSpin.value,
            saves: page.saves,
            skills: page.skills,
            deathSuccess: page.deathSuccess,
            deathFail: page.deathFail,
            personality: personalityEdit.text,
            ideals: idealsEdit.text,
            bonds: bondsEdit.text,
            flaws: flawsEdit.text,
            features: featuresEdit.text,
            equipment: equipmentEdit.text,
            weapons: page.weapons,
            casterClass: casterField.text,
            spellAbility: page.spellAbility,
            spellSlots: page.spellSlots,
            expendedSlots: page.expendedSlots,
            spellTexts: page.spellTexts
        }
        if (App.saveCharacterBasics(filePath, values)) {
            saveState.text = "Сохранено ✓"
            saveFlash.restart()
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
                spacing: 8

                AppButton {
                    text: "← Назад"
                    onClicked: {
                        page.save()
                        page.backRequested()
                    }
                }
                Label {
                    Layout.fillWidth: true
                    text: nameField.text.length ? nameField.text : (page.details.name || "Персонаж")
                    color: Theme.text
                    font.pixelSize: 18
                    font.weight: Font.Bold
                    elide: Text.ElideRight
                }
                Label {
                    id: saveState
                    visible: opacity > 0
                    color: Theme.success
                    opacity: 0
                }
                AppButton {
                    text: "Сохранить"
                    primary: true
                    onClicked: page.save()
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 52
            color: Theme.surface

            Flickable {
                anchors.fill: parent
                contentWidth: tabRow.implicitWidth
                contentHeight: height
                flickableDirection: Flickable.HorizontalFlick
                boundsBehavior: Flickable.StopAtBounds
                clip: true

                Row {
                    id: tabRow
                    height: parent.height
                    Repeater {
                        model: page.tabNames
                        Button {
                            required property string modelData
                            required property int index
                            width: Math.max(104, implicitContentWidth + 28)
                            height: 52
                            text: modelData
                            onClicked: page.currentTab = index

                            contentItem: Label {
                                text: parent.text
                                color: page.currentTab === index ? Theme.accentStrong : Theme.textMuted
                                font.weight: page.currentTab === index ? Font.DemiBold : Font.Normal
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            background: Item {
                                Rectangle {
                                    anchors.left: parent.left
                                    anchors.right: parent.right
                                    anchors.bottom: parent.bottom
                                    height: 3
                                    color: Theme.accent
                                    visible: page.currentTab === index
                                }
                            }
                        }
                    }
                }
            }
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: page.currentTab

            Flickable {
                id: basicsFlick
                contentWidth: width
                contentHeight: basicsContent.implicitHeight + 32
                clip: true
                boundsBehavior: Flickable.StopAtBounds
                ScrollBar.vertical: ScrollBar {}

                ColumnLayout {
                    id: basicsContent
                    width: basicsFlick.width
                    spacing: 14

                    Surface {
                        Layout.fillWidth: true
                        Layout.leftMargin: 16
                        Layout.rightMargin: 16
                        Layout.topMargin: 16
                        implicitHeight: identityLayout.implicitHeight + 28

                        ColumnLayout {
                            id: identityLayout
                            anchors.fill: parent
                            anchors.margins: 14
                            spacing: 10

                            Label {
                                text: "ПЕРСОНАЖ"
                                color: Theme.text
                                font.pixelSize: 16
                                font.weight: Font.Bold
                            }

                            GridLayout {
                                Layout.fillWidth: true
                                columns: basicsFlick.width < 620 ? 1 : 2
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

                            GridLayout {
                                Layout.fillWidth: true
                                columns: 2
                                columnSpacing: 10

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Label { text: "УРОВЕНЬ"; color: Theme.textMuted; font.pixelSize: 11 }
                                    NumberSpinBox { id: levelSpin; Layout.fillWidth: true; from: 1; to: 40; value: page.details.level || 1 }
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Label { text: "МАСТЕРСТВО"; color: Theme.textMuted; font.pixelSize: 11 }
                                    NumberSpinBox { id: proficiencySpin; Layout.fillWidth: true; from: 0; to: 12; value: page.details.proficiency || 2 }
                                }
                            }

                            CheckBox {
                                id: inspirationCheck
                                text: "Вдохновение"
                                checked: page.details.inspiration || false
                            }
                        }
                    }

                    Surface {
                        Layout.fillWidth: true
                        Layout.leftMargin: 16
                        Layout.rightMargin: 16
                        implicitHeight: combatLayout.implicitHeight + 28

                        ColumnLayout {
                            id: combatLayout
                            anchors.fill: parent
                            anchors.margins: 14
                            spacing: 12

                            Label {
                                text: "БОЙ"
                                color: Theme.text
                                font.pixelSize: 16
                                font.weight: Font.Bold
                            }

                            GridLayout {
                                Layout.fillWidth: true
                                columns: basicsFlick.width < 560 ? 2 : 3
                                columnSpacing: 10
                                rowSpacing: 10

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Label { text: "HP"; color: Theme.textMuted }
                                    NumberSpinBox { id: hpSpin; Layout.fillWidth: true; from: -999; to: 9999; value: page.details.hp || 0 }
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Label { text: "MAX"; color: Theme.textMuted }
                                    NumberSpinBox { id: hpMaxSpin; Layout.fillWidth: true; from: 0; to: 9999; value: page.details.hpMax || 0 }
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Label { text: "TEMP"; color: Theme.textMuted }
                                    NumberSpinBox { id: hpTempSpin; Layout.fillWidth: true; from: 0; to: 9999; value: page.details.hpTemp || 0 }
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Label { text: "КД"; color: Theme.textMuted }
                                    NumberSpinBox { id: acSpin; Layout.fillWidth: true; from: 0; to: 99; value: page.details.armorClass || 0 }
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Label { text: "ИНИЦИАТИВА"; color: Theme.textMuted }
                                    Rectangle {
                                        Layout.fillWidth: true
                                        implicitHeight: Theme.touchTarget
                                        radius: Theme.radiusSmall
                                        color: Theme.surfaceRaised
                                        border.width: 1
                                        border.color: Theme.border
                                        Label {
                                            anchors.centerIn: parent
                                            text: page.signed(page.statModifier("dex"))
                                            color: Theme.accentStrong
                                            font.pixelSize: 17
                                            font.weight: Font.DemiBold
                                        }
                                    }
                                }
                                LabeledField {
                                    id: speedField
                                    Layout.fillWidth: true
                                    label: "СКОРОСТЬ"
                                    text: page.details.speed || ""
                                }
                            }

                            LabeledField {
                                id: hitDieField
                                Layout.fillWidth: true
                                label: "КОСТЬ ХИТОВ"
                                text: page.details.hitDie || ""
                            }

                            Label {
                                text: "СПАСБРОСКИ ОТ СМЕРТИ"
                                color: Theme.textMuted
                                font.pixelSize: 11
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                Label {
                                    text: "Успех"
                                    color: Theme.success
                                    Layout.preferredWidth: 70
                                }
                                Repeater {
                                    model: 3
                                    SquareCheckBox {
                                        required property int index
                                        fillColor: Theme.success
                                        checked: !!page.deathSuccess[index]
                                        onToggled: page.setDeath("success", index, checked)
                                    }
                                }
                                Item { Layout.fillWidth: true }
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                Label {
                                    text: "Провал"
                                    color: Theme.danger
                                    Layout.preferredWidth: 70
                                }
                                Repeater {
                                    model: 3
                                    SquareCheckBox {
                                        required property int index
                                        fillColor: Theme.danger
                                        checked: !!page.deathFail[index]
                                        onToggled: page.setDeath("fail", index, checked)
                                    }
                                }
                                Item { Layout.fillWidth: true }
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

                            Label {
                                text: "ХАРАКТЕРИСТИКИ"
                                color: Theme.text
                                font.pixelSize: 16
                                font.weight: Font.Bold
                            }

                            GridLayout {
                                Layout.fillWidth: true
                                columns: basicsFlick.width < 540 ? 2 : (basicsFlick.width < 900 ? 3 : 6)
                                columnSpacing: 10
                                rowSpacing: 12

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Label { text: "СИЛ"; color: Theme.textMuted }
                                    NumberSpinBox { id: strSpin; Layout.fillWidth: true; from: 1; to: 30; value: page.details.str || 10 }
                                    Label { text: page.signed(page.statModifier("str")); color: Theme.accentStrong }
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Label { text: "ЛОВ"; color: Theme.textMuted }
                                    NumberSpinBox { id: dexSpin; Layout.fillWidth: true; from: 1; to: 30; value: page.details.dex || 10 }
                                    Label { text: page.signed(page.statModifier("dex")); color: Theme.accentStrong }
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Label { text: "ТЕЛ"; color: Theme.textMuted }
                                    NumberSpinBox { id: conSpin; Layout.fillWidth: true; from: 1; to: 30; value: page.details.con || 10 }
                                    Label { text: page.signed(page.statModifier("con")); color: Theme.accentStrong }
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Label { text: "ИНТ"; color: Theme.textMuted }
                                    NumberSpinBox { id: intSpin; Layout.fillWidth: true; from: 1; to: 30; value: page.details.int || 10 }
                                    Label { text: page.signed(page.statModifier("int")); color: Theme.accentStrong }
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Label { text: "МУД"; color: Theme.textMuted }
                                    NumberSpinBox { id: wisSpin; Layout.fillWidth: true; from: 1; to: 30; value: page.details.wis || 10 }
                                    Label { text: page.signed(page.statModifier("wis")); color: Theme.accentStrong }
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Label { text: "ХАР"; color: Theme.textMuted }
                                    NumberSpinBox { id: chaSpin; Layout.fillWidth: true; from: 1; to: 30; value: page.details.cha || 10 }
                                    Label { text: page.signed(page.statModifier("cha")); color: Theme.accentStrong }
                                }
                            }
                        }
                    }

                    Item { Layout.preferredHeight: 16 }
                }
            }

            Flickable {
                id: skillsFlick
                contentWidth: width
                contentHeight: skillsContent.implicitHeight + 32
                clip: true
                boundsBehavior: Flickable.StopAtBounds
                ScrollBar.vertical: ScrollBar {}

                ColumnLayout {
                    id: skillsContent
                    width: skillsFlick.width
                    spacing: 14

                    Surface {
                        Layout.fillWidth: true
                        Layout.leftMargin: 16
                        Layout.rightMargin: 16
                        Layout.topMargin: 16
                        implicitHeight: savesLayout.implicitHeight + 28

                        ColumnLayout {
                            id: savesLayout
                            anchors.fill: parent
                            anchors.margins: 14
                            spacing: 10

                            Label { text: "СПАСБРОСКИ"; color: Theme.text; font.pixelSize: 16; font.weight: Font.Bold }
                            GridLayout {
                                Layout.fillWidth: true
                                columns: skillsFlick.width < 620 ? 2 : 3
                                columnSpacing: 8
                                rowSpacing: 6
                                Repeater {
                                    model: 6
                                    CheckBox {
                                        required property int index
                                        property string code: page.statCodes[index]
                                        text: page.statNames[index] + "  " + page.signed(page.statModifier(code) + (checked ? proficiencySpin.value : 0))
                                        checked: !!page.saves[code]
                                        onToggled: {
                                            let copy = page.clone(page.saves)
                                            copy[code] = checked
                                            page.saves = copy
                                        }
                                    }
                                }
                            }
                        }
                    }

                    Surface {
                        Layout.fillWidth: true
                        Layout.leftMargin: 16
                        Layout.rightMargin: 16
                        implicitHeight: skillList.implicitHeight + 74

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 14
                            spacing: 8

                            RowLayout {
                                Layout.fillWidth: true
                                Label { text: "НАВЫКИ"; color: Theme.text; font.pixelSize: 16; font.weight: Font.Bold; Layout.fillWidth: true }
                                Label {
                                    visible: skillsFlick.width >= 500
                                    text: "Пассивное восприятие: " + page.passivePerception()
                                    color: Theme.accentStrong
                                }
                            }
                            Label {
                                visible: skillsFlick.width < 500
                                text: "Пассивное восприятие: " + page.passivePerception()
                                color: Theme.accentStrong
                            }

                            ColumnLayout {
                                id: skillList
                                Layout.fillWidth: true
                                Repeater {
                                    model: page.skills
                                    RowLayout {
                                        required property var modelData
                                        required property int index
                                        Layout.fillWidth: true
                                        spacing: 6

                                        AppButton {
                                            text: modelData.profLevel === 2 ? "◎" : (modelData.profLevel === 1 ? "●" : "○")
                                            implicitWidth: 46
                                            onClicked: page.setSkillLevel(index, ((modelData.profLevel || 0) + 1) % 3)
                                        }
                                        Label { text: page.signed(page.skillBonus(modelData)); color: Theme.accentStrong; Layout.preferredWidth: 42 }
                                        Label { text: modelData.label || modelData.key; color: Theme.text; Layout.fillWidth: true; elide: Text.ElideRight }
                                        Label { text: (modelData.baseStat || "").toUpperCase(); color: Theme.textMuted }
                                    }
                                }
                            }
                            Label { text: "○ нет · ● владение · ◎ экспертиза"; color: Theme.textMuted; font.pixelSize: 11 }
                        }
                    }

                    Item { Layout.preferredHeight: 16 }
                }
            }

            Flickable {
                id: textFlick
                contentWidth: width
                contentHeight: textContent.implicitHeight + 32
                clip: true
                boundsBehavior: Flickable.StopAtBounds
                ScrollBar.vertical: ScrollBar {}

                ColumnLayout {
                    id: textContent
                    width: textFlick.width
                    spacing: 14

                    GridLayout {
                        Layout.fillWidth: true
                        Layout.leftMargin: 16
                        Layout.rightMargin: 16
                        Layout.topMargin: 16
                        columns: textFlick.width < 760 ? 1 : 2
                        columnSpacing: 14
                        rowSpacing: 14

                        Surface {
                            Layout.fillWidth: true
                            implicitHeight: 190
                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 12
                                Label { text: "ЧЕРТЫ ХАРАКТЕРА"; color: Theme.textMuted }
                                AppTextArea { id: personalityEdit; Layout.fillWidth: true; Layout.fillHeight: true; text: page.details.personality || "" }
                            }
                        }
                        Surface {
                            Layout.fillWidth: true
                            implicitHeight: 190
                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 12
                                Label { text: "ИДЕАЛЫ"; color: Theme.textMuted }
                                AppTextArea { id: idealsEdit; Layout.fillWidth: true; Layout.fillHeight: true; text: page.details.ideals || "" }
                            }
                        }
                        Surface {
                            Layout.fillWidth: true
                            implicitHeight: 190
                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 12
                                Label { text: "ПРИВЯЗАННОСТИ"; color: Theme.textMuted }
                                AppTextArea { id: bondsEdit; Layout.fillWidth: true; Layout.fillHeight: true; text: page.details.bonds || "" }
                            }
                        }
                        Surface {
                            Layout.fillWidth: true
                            implicitHeight: 190
                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 12
                                Label { text: "СЛАБОСТИ"; color: Theme.textMuted }
                                AppTextArea { id: flawsEdit; Layout.fillWidth: true; Layout.fillHeight: true; text: page.details.flaws || "" }
                            }
                        }
                    }

                    Surface {
                        Layout.fillWidth: true
                        Layout.leftMargin: 16
                        Layout.rightMargin: 16
                        implicitHeight: 230
                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12
                            Label { text: "УМЕНИЯ И ОСОБЕННОСТИ"; color: Theme.textMuted }
                            AppTextArea { id: featuresEdit; Layout.fillWidth: true; Layout.fillHeight: true; text: page.details.features || "" }
                        }
                    }
                    Surface {
                        Layout.fillWidth: true
                        Layout.leftMargin: 16
                        Layout.rightMargin: 16
                        implicitHeight: 230
                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 12
                            Label { text: "СНАРЯЖЕНИЕ"; color: Theme.textMuted }
                            AppTextArea { id: equipmentEdit; Layout.fillWidth: true; Layout.fillHeight: true; text: page.details.equipment || "" }
                        }
                    }

                    Item { Layout.preferredHeight: 16 }
                }
            }

            Item {
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 12

                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: "АТАКИ И ОРУЖИЕ"; color: Theme.text; font.pixelSize: 20; font.weight: Font.Bold; Layout.fillWidth: true }
                        AppButton { text: "+ Оружие"; primary: true; onClicked: weaponDialog.openNew() }
                    }

                    ListView {
                        id: weaponList
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        model: page.weapons
                        spacing: 8
                        clip: true
                        boundsBehavior: Flickable.StopAtBounds
                        ScrollBar.vertical: ScrollBar {}

                        delegate: Surface {
                            required property var modelData
                            required property int index
                            width: ListView.view.width
                            implicitHeight: 112

                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 6

                                RowLayout {
                                    Layout.fillWidth: true
                                    Label {
                                        text: modelData.name || "Оружие"
                                        color: Theme.text
                                        font.pixelSize: 17
                                        font.weight: Font.Bold
                                        Layout.fillWidth: true
                                        elide: Text.ElideRight
                                    }
                                    Label {
                                        text: "Попадание " + page.signed(page.weaponHit(modelData))
                                        color: Theme.accentStrong
                                        font.weight: Font.DemiBold
                                    }
                                    AppButton {
                                        text: "×"
                                        danger: true
                                        implicitWidth: 42
                                        onClicked: {
                                            let copy = page.clone(page.weapons)
                                            copy.splice(index, 1)
                                            page.weapons = copy
                                        }
                                    }
                                }

                                RowLayout {
                                    Layout.fillWidth: true
                                    Label {
                                        text: modelData.damage || "Урон не указан"
                                        color: Theme.textMuted
                                        Layout.fillWidth: true
                                        elide: Text.ElideRight
                                    }
                                    AppButton {
                                        text: "Изменить"
                                        onClicked: weaponDialog.openExisting(index, modelData)
                                    }
                                }
                            }
                        }

                        Label {
                            anchors.centerIn: parent
                            visible: weaponList.count === 0
                            text: "Оружия пока нет"
                            color: Theme.textMuted
                        }
                    }
                }
            }

            Flickable {
                id: magicFlick
                contentWidth: width
                contentHeight: magicContent.implicitHeight + 32
                clip: true
                boundsBehavior: Flickable.StopAtBounds
                ScrollBar.vertical: ScrollBar {}

                ColumnLayout {
                    id: magicContent
                    width: magicFlick.width
                    spacing: 14

                    Surface {
                        Layout.fillWidth: true
                        Layout.leftMargin: 16
                        Layout.rightMargin: 16
                        Layout.topMargin: 16
                        implicitHeight: magicHeader.implicitHeight + 28

                        ColumnLayout {
                            id: magicHeader
                            anchors.fill: parent
                            anchors.margins: 14
                            spacing: 10

                            Label { text: "МАГИЯ"; color: Theme.text; font.pixelSize: 18; font.weight: Font.Bold }

                            GridLayout {
                                Layout.fillWidth: true
                                columns: magicFlick.width < 700 ? 1 : 2
                                columnSpacing: 12
                                rowSpacing: 10

                                LabeledField { id: casterField; Layout.fillWidth: true; label: "КЛАСС ЗАКЛИНАТЕЛЯ"; text: page.details.casterClass || "" }

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Label { text: "ХАРАКТЕРИСТИКА"; color: Theme.textMuted; font.pixelSize: 11 }
                                    ComboBox {
                                        id: spellAbilityCombo
                                        Layout.fillWidth: true
                                        model: page.casterAbilityNames
                                        currentIndex: Math.max(0, page.casterAbilityCodes.indexOf(page.spellAbility))
                                        onActivated: page.spellAbility = page.casterAbilityCodes[currentIndex]
                                    }
                                }

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Label { text: "СЛ СПАСБРОСКА"; color: Theme.textMuted; font.pixelSize: 11 }
                                    Label { text: 8 + proficiencySpin.value + page.spellModifier(); color: Theme.accentStrong; font.pixelSize: 24; font.weight: Font.Bold }
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Label { text: "БОНУС АТАКИ"; color: Theme.textMuted; font.pixelSize: 11 }
                                    Label { text: page.signed(proficiencySpin.value + page.spellModifier()); color: Theme.accentStrong; font.pixelSize: 24; font.weight: Font.Bold }
                                }
                            }
                        }
                    }

                    GridLayout {
                        Layout.fillWidth: true
                        Layout.leftMargin: 16
                        Layout.rightMargin: 16
                        columns: magicFlick.width < 840 ? 1 : 2
                        columnSpacing: 12
                        rowSpacing: 12

                        Repeater {
                            model: 10
                            Surface {
                                required property int index
                                property int level: index
                                Layout.fillWidth: true
                                implicitHeight: 250

                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 12
                                    spacing: 8

                                    RowLayout {
                                        Layout.fillWidth: true
                                        Label {
                                            text: level === 0 ? "ЗАГОВОРЫ" : "УРОВЕНЬ " + level
                                            color: Theme.text
                                            font.weight: Font.Bold
                                            Layout.fillWidth: true
                                        }
                                        Label { visible: level > 0; text: "Ячейки"; color: Theme.textMuted }
                                        NumberSpinBox {
                                            visible: level > 0
                                            Layout.preferredWidth: 116
                                            from: 0
                                            to: 20
                                            value: page.spellSlots[level] || 0
                                            onValueChanged: if (level > 0 && value !== (page.spellSlots[level] || 0)) page.setSpellSlots(level, value)
                                        }
                                    }

                                    Flow {
                                        visible: level > 0
                                        Layout.fillWidth: true
                                        spacing: 5
                                        Repeater {
                                            model: level > 0 ? (page.spellSlots[level] || 0) : 0
                                            SquareCheckBox {
                                                required property int index
                                                fillColor: Theme.accent
                                                checked: !!((page.expendedSlots[level] || [])[index])
                                                onToggled: page.setSpellSpent(level, index, checked)
                                                ToolTip.visible: hovered
                                                ToolTip.text: checked ? "Ячейка потрачена" : "Ячейка доступна"
                                            }
                                        }
                                    }

                                    AppTextArea {
                                        Layout.fillWidth: true
                                        Layout.fillHeight: true
                                        text: page.spellTexts[level] || ""
                                        placeholderText: level === 0 ? "Заговоры…" : "Заклинания уровня " + level + "…"
                                        onTextChanged: if (text !== (page.spellTexts[level] || "")) page.setSpellText(level, text)
                                    }
                                }
                            }
                        }
                    }

                    Item { Layout.preferredHeight: 16 }
                }
            }
        }
    }

    Dialog {
        id: weaponDialog
        property int editIndex: -1
        property bool creating: false
        title: creating ? "Новое оружие" : "Редактирование оружия"
        modal: true
        anchors.centerIn: Overlay.overlay
        standardButtons: Dialog.Save | Dialog.Cancel

        function openNew() {
            creating = true
            editIndex = -1
            weaponName.text = "Новое оружие"
            weaponDamage.text = "1к8"
            weaponAbility.currentIndex = 1
            weaponProf.checked = true
            weaponMagic.value = 0
            weaponNotes.text = ""
            open()
        }

        function openExisting(index, weapon) {
            creating = false
            editIndex = index
            weaponName.text = weapon.name || ""
            weaponDamage.text = weapon.damage || ""
            weaponAbility.currentIndex = Math.max(0, page.abilityCodes.indexOf(weapon.ability || "str"))
            weaponProf.checked = !!weapon.isProf
            weaponMagic.value = weapon.magicBonus || 0
            weaponNotes.text = weapon.notes || ""
            open()
        }

        contentItem: ColumnLayout {
            implicitWidth: Math.min(440, Math.max(300, page.width - 40))
            spacing: 10

            LabeledField { id: weaponName; Layout.fillWidth: true; label: "НАЗВАНИЕ" }
            LabeledField { id: weaponDamage; Layout.fillWidth: true; label: "УРОН / ВИД" }

            GridLayout {
                Layout.fillWidth: true
                columns: page.width < 520 ? 1 : 3
                columnSpacing: 10
                rowSpacing: 8

                ColumnLayout {
                    Layout.fillWidth: true
                    Label { text: "ХАРАКТЕРИСТИКА"; color: Theme.textMuted; font.pixelSize: 11 }
                    ComboBox { id: weaponAbility; Layout.fillWidth: true; model: page.abilityNames }
                }
                CheckBox { id: weaponProf; text: "Владение" }
                ColumnLayout {
                    Layout.fillWidth: true
                    Label { text: "БОНУС"; color: Theme.textMuted; font.pixelSize: 11 }
                    NumberSpinBox { id: weaponMagic; Layout.fillWidth: true; from: -10; to: 10 }
                }
            }

            Label {
                text: "Итог: " + page.signed((weaponAbility.currentIndex === 0 ? 0 : page.statModifier(page.abilityCodes[weaponAbility.currentIndex])) + (weaponProf.checked ? proficiencySpin.value : 0) + weaponMagic.value)
                color: Theme.accentStrong
            }

            AppTextArea {
                id: weaponNotes
                Layout.fillWidth: true
                implicitHeight: 90
                placeholderText: "Заметки"
            }
        }

        onAccepted: {
            const weapon = {
                name: weaponName.text,
                damage: weaponDamage.text,
                ability: page.abilityCodes[weaponAbility.currentIndex],
                isProf: weaponProf.checked,
                magicBonus: weaponMagic.value,
                notes: weaponNotes.text
            }
            let copy = page.clone(page.weapons)
            if (creating) copy.push(weapon)
            else if (editIndex >= 0 && editIndex < copy.length) copy[editIndex] = weapon
            page.weapons = copy
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
