import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import DndTracker

Item {
    id: page
    required property string filePath
    signal backRequested()

    readonly property int sheetWidth: 960
    readonly property int sheetHeight: 1280
    readonly property color paperColor: "#f8f3e8"
    readonly property color inkColor: "#2d2924"
    readonly property color faintInk: "#756f66"
    readonly property color panelColor: "#efe9dd"

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
    property int passivePerceptionOverride: -1

    property string nameValue: ""
    property string playerValue: ""
    property string classValue: ""
    property string subclassValue: ""
    property string raceValue: ""
    property string backgroundValue: ""
    property string alignmentValue: ""
    property string experienceValue: ""
    property int levelValue: 1
    property int proficiencyValue: 2
    property bool inspirationValue: false
    property int hpValue: 0
    property int hpMaxValue: 0
    property int hpTempValue: 0
    property int acValue: 0
    property string speedValue: ""
    property string hitDieValue: ""
    property int strValue: 10
    property int dexValue: 10
    property int conValue: 10
    property int intValue: 10
    property int wisValue: 10
    property int chaValue: 10
    property string personalityValue: ""
    property string idealsValue: ""
    property string bondsValue: ""
    property string flawsValue: ""
    property string featuresValue: ""
    property string equipmentValue: ""
    property string casterClassValue: ""

    property real fitScale: Math.min(1.0, Math.max(0.28, (sheetFlick.width - 24) / sheetWidth))
    property real zoom: fitScale
    property bool manualZoom: false

    readonly property var statCodes: ["str", "dex", "con", "int", "wis", "cha"]
    readonly property var statNames: ["СИЛА", "ЛОВКОСТЬ", "ТЕЛОСЛОЖЕНИЕ", "ИНТЕЛЛЕКТ", "МУДРОСТЬ", "ХАРИЗМА"]

    component PaperField: Item {
        id: field
        property string label: ""
        property string value: ""
        property int textSize: 14
        signal edited(string value)

        Text {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            text: field.label
            color: "#756f66"
            font.pixelSize: 9
            font.weight: Font.DemiBold
            elide: Text.ElideRight
        }
        TextField {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: parent.height - 13
            text: field.value
            color: "#2d2924"
            font.pixelSize: field.textSize
            selectByMouse: true
            leftPadding: 3
            rightPadding: 3
            topPadding: 2
            bottomPadding: 2
            background: Rectangle {
                color: "transparent"
                border.width: 0
                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    height: 1
                    color: "#8f887d"
                }
            }
            onTextEdited: field.edited(text)
        }
    }

    component PaperNumber: Item {
        id: numberField
        property string label: ""
        property int value: 0
        property int minimum: -999
        property int maximum: 9999
        property int textSize: 22
        signal edited(int value)

        Text {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            text: numberField.label
            color: "#756f66"
            font.pixelSize: 8
            font.weight: Font.DemiBold
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
        }
        TextField {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: parent.height - 12
            text: String(numberField.value)
            color: "#2d2924"
            font.pixelSize: numberField.textSize
            font.weight: Font.DemiBold
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            selectByMouse: true
            validator: IntValidator { bottom: numberField.minimum; top: numberField.maximum }
            background: Item {}
            onEditingFinished: {
                const parsed = Number(text)
                if (!Number.isNaN(parsed))
                    numberField.edited(Math.max(numberField.minimum, Math.min(numberField.maximum, parsed)))
            }
        }
    }

    component PaperArea: Rectangle {
        id: area
        property string label: ""
        property string value: ""
        property int textSize: 12
        signal edited(string value)

        color: "#fbf8f0"
        border.width: 1.4
        border.color: "#38332d"
        radius: 5

        Text {
            id: areaLabel
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 5
            text: area.label
            color: "#2d2924"
            font.pixelSize: 9
            font.weight: Font.DemiBold
            horizontalAlignment: Text.AlignHCenter
        }

        TextArea {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: areaLabel.top
            anchors.margins: 6
            text: area.value
            color: "#2d2924"
            font.pixelSize: area.textSize
            wrapMode: TextEdit.Wrap
            selectByMouse: true
            background: Item {}
            padding: 2
            onTextChanged: if (activeFocus) area.edited(text)
        }
    }

    function clone(value) {
        return JSON.parse(JSON.stringify(value === undefined || value === null ? [] : value))
    }

    function asInt(value, fallback) {
        const n = Number(value)
        return Number.isFinite(n) ? Math.trunc(n) : fallback
    }

    function reload() {
        const d = App.characterDetails(filePath)
        details = d
        nameValue = d.name || ""
        playerValue = d.playerName || ""
        classValue = d.charClass || ""
        subclassValue = d.subclass || ""
        raceValue = d.race || ""
        backgroundValue = d.background || ""
        alignmentValue = d.alignment || ""
        experienceValue = d.experience || ""
        levelValue = asInt(d.level, 1)
        proficiencyValue = asInt(d.proficiency, 2)
        inspirationValue = !!d.inspiration
        hpValue = asInt(d.hp, 0)
        hpMaxValue = asInt(d.hpMax, 0)
        hpTempValue = asInt(d.hpTemp, 0)
        acValue = asInt(d.armorClass, 0)
        speedValue = d.speed || ""
        hitDieValue = d.hitDie || ""
        strValue = asInt(d.str, 10)
        dexValue = asInt(d.dex, 10)
        conValue = asInt(d.con, 10)
        intValue = asInt(d.int, 10)
        wisValue = asInt(d.wis, 10)
        chaValue = asInt(d.cha, 10)
        saves = clone(d.saves || {})
        skills = clone(d.skills || [])
        deathSuccess = clone(d.deathSuccess || [false, false, false])
        deathFail = clone(d.deathFail || [false, false, false])
        personalityValue = d.personality || ""
        idealsValue = d.ideals || ""
        bondsValue = d.bonds || ""
        flawsValue = d.flaws || ""
        featuresValue = d.features || ""
        equipmentValue = d.equipment || ""
        weapons = clone(d.weapons || [])
        casterClassValue = d.casterClass || ""
        spellAbility = d.spellAbility || "int"
        spellSlots = clone(d.spellSlots || [0,0,0,0,0,0,0,0,0,0])
        expendedSlots = clone(d.expendedSlots || [[],[],[],[],[],[],[],[],[],[]])
        spellTexts = clone(d.spellTexts || ["","","","","","","","","",""])
        passivePerceptionOverride = d.passivePerceptionOverride === undefined
                ? -1 : asInt(d.passivePerceptionOverride, -1)
    }

    function save() {
        const values = {
            name: nameValue,
            playerName: playerValue,
            charClass: classValue,
            subclass: subclassValue,
            race: raceValue,
            background: backgroundValue,
            alignment: alignmentValue,
            experience: experienceValue,
            level: levelValue,
            proficiency: proficiencyValue,
            inspiration: inspirationValue,
            hp: hpValue,
            hpMax: hpMaxValue,
            hpTemp: hpTempValue,
            armorClass: acValue,
            initiative: statModifier("dex"),
            passivePerceptionOverride: passivePerceptionOverride,
            speed: speedValue,
            hitDie: hitDieValue,
            str: strValue,
            dex: dexValue,
            con: conValue,
            int: intValue,
            wis: wisValue,
            cha: chaValue,
            saves: saves,
            skills: skills,
            deathSuccess: deathSuccess,
            deathFail: deathFail,
            personality: personalityValue,
            ideals: idealsValue,
            bonds: bondsValue,
            flaws: flawsValue,
            features: featuresValue,
            equipment: equipmentValue,
            weapons: weapons,
            casterClass: casterClassValue,
            spellAbility: spellAbility,
            spellSlots: spellSlots,
            expendedSlots: expendedSlots,
            spellTexts: spellTexts
        }
        if (App.saveCharacterBasics(filePath, values)) {
            saveState.text = "Сохранено ✓"
            saveState.opacity = 1
            saveFlash.restart()
            return true
        }
        return false
    }

    function statValue(code) {
        if (code === "str") return strValue
        if (code === "dex") return dexValue
        if (code === "con") return conValue
        if (code === "int") return intValue
        if (code === "wis") return wisValue
        if (code === "cha") return chaValue
        return 10
    }

    function setStatValue(code, value) {
        if (code === "str") strValue = value
        else if (code === "dex") dexValue = value
        else if (code === "con") conValue = value
        else if (code === "int") intValue = value
        else if (code === "wis") wisValue = value
        else if (code === "cha") chaValue = value
    }

    function statModifier(code) {
        return Math.floor((statValue(code) - 10) / 2)
    }

    function signed(value) {
        return (value >= 0 ? "+" : "") + value
    }

    function setSave(code, checked) {
        let copy = clone(saves || {})
        copy[code] = checked
        saves = copy
    }

    function skillName(skill) {
        if (!skill) return ""
        const labels = {
            "acrobatics": "Акробатика",
            "animalHandling": "Уход за животными",
            "animal-handling": "Уход за животными",
            "arcana": "Магия",
            "athletics": "Атлетика",
            "deception": "Обман",
            "history": "История",
            "insight": "Проницательность",
            "intimidation": "Запугивание",
            "investigation": "Расследование",
            "medicine": "Медицина",
            "nature": "Природа",
            "perception": "Внимательность",
            "performance": "Выступление",
            "persuasion": "Убеждение",
            "religion": "Религия",
            "sleightOfHand": "Ловкость рук",
            "sleight-of-hand": "Ловкость рук",
            "stealth": "Скрытность",
            "survival": "Выживание"
        }
        return labels[skill.key] || skill.label || skill.key || "Навык"
    }

    function shortStat(code) {
        const labels = { str: "СИЛ", dex: "ЛОВ", con: "ТЕЛ", int: "ИНТ", wis: "МУД", cha: "ХАР" }
        return labels[code] || String(code || "").toUpperCase()
    }

    function skillBonus(skill) {
        if (!skill) return 0
        return statModifier(skill.baseStat) + asInt(skill.profLevel, 0) * proficiencyValue
    }

    function setSkillLevel(index, level) {
        let copy = clone(skills)
        if (index >= 0 && index < copy.length) {
            copy[index].profLevel = level
            skills = copy
        }
    }

    function computedPassivePerception() {
        for (let i = 0; i < skills.length; ++i) {
            const skill = skills[i]
            if (skill.key === "perception" || skillName(skill) === "Внимательность")
                return 10 + skillBonus(skill)
        }
        return 10 + statModifier("wis")
    }

    function passivePerception() {
        return passivePerceptionOverride >= 0 ? passivePerceptionOverride : computedPassivePerception()
    }

    function toggleDeath(which, index) {
        let copy = clone(which === "success" ? deathSuccess : deathFail)
        while (copy.length < 3) copy.push(false)
        copy[index] = !copy[index]
        if (which === "success") deathSuccess = copy
        else deathFail = copy
    }

    function weaponAt(index) {
        return index >= 0 && index < weapons.length ? weapons[index] : ({})
    }

    function setWeaponField(index, key, value) {
        let copy = clone(weapons)
        while (copy.length <= index)
            copy.push({ name: "", damage: "", ability: "str", isProf: true, magicBonus: 0, notes: "" })
        copy[index][key] = value
        weapons = copy
    }

    function weaponHit(weapon) {
        if (!weapon) return 0
        const ability = weapon.ability || "str"
        const mod = ability === "none" ? 0 : statModifier(ability)
        return mod + (weapon.isProf === false ? 0 : proficiencyValue) + asInt(weapon.magicBonus, 0)
    }

    function changeZoom(delta) {
        manualZoom = true
        zoom = Math.max(0.28, Math.min(1.8, zoom + delta))
    }

    function fitToWidth() {
        manualZoom = false
        zoom = fitScale
    }

    onFitScaleChanged: if (!manualZoom) zoom = fitScale
    Component.onCompleted: reload()

    Timer {
        id: saveFlash
        interval: 1500
        onTriggered: saveState.opacity = 0
    }

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
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 0
                    Label {
                        Layout.fillWidth: true
                        text: page.nameValue.length ? page.nameValue : "Персонаж"
                        color: Theme.text
                        font.pixelSize: 18
                        font.weight: Font.Bold
                        elide: Text.ElideRight
                    }
                    Label {
                        text: "Классический вид"
                        color: Theme.textMuted
                        font.pixelSize: 11
                    }
                }
                Label {
                    id: saveState
                    color: Theme.success
                    opacity: 0
                    Behavior on opacity { NumberAnimation { duration: 140 } }
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
            implicitHeight: 46
            color: Theme.surface
            border.width: 1
            border.color: Theme.border

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                spacing: 8

                Label {
                    Layout.fillWidth: true
                    text: "Бумажный лист D&D 5e · русская разметка"
                    color: Theme.textMuted
                    elide: Text.ElideRight
                }
                AppButton {
                    text: "−"
                    implicitWidth: 42
                    onClicked: page.changeZoom(-0.1)
                }
                AppButton {
                    text: Math.round(page.zoom * 100) + "%"
                    implicitWidth: 72
                    onClicked: page.fitToWidth()
                    ToolTip.visible: hovered
                    ToolTip.text: "По ширине"
                }
                AppButton {
                    text: "+"
                    implicitWidth: 42
                    onClicked: page.changeZoom(0.1)
                }
            }
        }

        Flickable {
            id: sheetFlick
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            contentWidth: paperHost.width
            contentHeight: paperHost.height
            ScrollBar.vertical: ScrollBar {}
            ScrollBar.horizontal: ScrollBar {}

            Item {
                id: paperHost
                width: Math.max(sheetFlick.width, page.sheetWidth * page.zoom + 24)
                height: page.sheetHeight * page.zoom + 24

                Rectangle {
                    id: paper
                    x: Math.max(12, (paperHost.width - page.sheetWidth * page.zoom) / 2)
                    y: 12
                    width: page.sheetWidth
                    height: page.sheetHeight
                    scale: page.zoom
                    transformOrigin: Item.TopLeft
                    color: page.paperColor
                    border.width: 1
                    border.color: "#c7bfb2"

                    Rectangle {
                        x: 28; y: 24; width: 330; height: 92
                        color: "#fbf8f0"; border.width: 1.8; border.color: page.inkColor; radius: 5
                        Text {
                            x: 14; y: 9
                            text: "D&D 5e"
                            color: page.inkColor
                            font.pixelSize: 18
                            font.weight: Font.Black
                        }
                        PaperField {
                            x: 14; y: 39; width: 302; height: 43
                            label: "ИМЯ ПЕРСОНАЖА"
                            value: page.nameValue
                            textSize: 19
                            onEdited: value => page.nameValue = value
                        }
                    }

                    Rectangle {
                        x: 374; y: 24; width: 558; height: 92
                        color: "#fbf8f0"; border.width: 1.8; border.color: page.inkColor; radius: 5

                        PaperField { x: 12; y: 8; width: 166; height: 35; label: "КЛАСС"; value: page.classValue; onEdited: value => page.classValue = value }
                        PaperNumber { x: 184; y: 8; width: 60; height: 35; label: "УРОВЕНЬ"; value: page.levelValue; minimum: 1; maximum: 40; textSize: 15; onEdited: value => page.levelValue = value }
                        PaperField { x: 250; y: 8; width: 140; height: 35; label: "ПРЕДЫСТОРИЯ"; value: page.backgroundValue; onEdited: value => page.backgroundValue = value }
                        PaperField { x: 396; y: 8; width: 150; height: 35; label: "ИГРОК"; value: page.playerValue; onEdited: value => page.playerValue = value }
                        PaperField { x: 12; y: 49; width: 166; height: 35; label: "РАСА"; value: page.raceValue; onEdited: value => page.raceValue = value }
                        PaperField { x: 184; y: 49; width: 206; height: 35; label: "МИРОВОЗЗРЕНИЕ"; value: page.alignmentValue; onEdited: value => page.alignmentValue = value }
                        PaperField { x: 396; y: 49; width: 150; height: 35; label: "ОПЫТ"; value: page.experienceValue; onEdited: value => page.experienceValue = value }
                    }

                    Rectangle {
                        x: 28; y: 140; width: 96; height: 664
                        color: page.panelColor
                        radius: 8
                        border.width: 1
                        border.color: "#d1c8bb"

                        Repeater {
                            model: 6
                            delegate: Rectangle {
                                required property int index
                                x: 8
                                y: 8 + index * 108
                                width: 80
                                height: 94
                                color: "#fbf8f0"
                                border.width: 1.6
                                border.color: page.inkColor
                                radius: 6

                                Text {
                                    anchors.top: parent.top
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    anchors.topMargin: 7
                                    width: parent.width - 8
                                    text: page.statNames[index]
                                    color: page.inkColor
                                    font.pixelSize: index === 2 || index === 3 ? 8 : 9
                                    font.weight: Font.DemiBold
                                    horizontalAlignment: Text.AlignHCenter
                                    elide: Text.ElideRight
                                }

                                TextField {
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    y: 26
                                    width: 58
                                    height: 38
                                    text: String(page.statValue(page.statCodes[index]))
                                    color: page.inkColor
                                    font.pixelSize: 24
                                    font.weight: Font.Bold
                                    horizontalAlignment: Text.AlignHCenter
                                    validator: IntValidator { bottom: 1; top: 30 }
                                    background: Item {}
                                    onEditingFinished: {
                                        const n = Number(text)
                                        if (!Number.isNaN(n)) page.setStatValue(page.statCodes[index], Math.max(1, Math.min(30, n)))
                                    }
                                }

                                Rectangle {
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    anchors.bottom: parent.bottom
                                    anchors.bottomMargin: -12
                                    width: 44
                                    height: 25
                                    radius: 13
                                    color: page.paperColor
                                    border.width: 1.4
                                    border.color: page.inkColor
                                    Text {
                                        anchors.centerIn: parent
                                        text: page.signed(page.statModifier(page.statCodes[index]))
                                        color: page.inkColor
                                        font.pixelSize: 13
                                        font.weight: Font.DemiBold
                                    }
                                }
                            }
                        }
                    }

                    Rectangle {
                        x: 142; y: 140; width: 212; height: 72
                        color: "#fbf8f0"; border.width: 1.5; border.color: page.inkColor; radius: 5
                        Rectangle {
                            x: 8; y: 10; width: 28; height: 28; radius: 4
                            color: page.inspirationValue ? page.inkColor : "transparent"
                            border.width: 1.5; border.color: page.inkColor
                            MouseArea { anchors.fill: parent; onClicked: page.inspirationValue = !page.inspirationValue }
                        }
                        Text { x: 46; y: 15; text: "ВДОХНОВЕНИЕ"; color: page.inkColor; font.pixelSize: 10; font.weight: Font.DemiBold }
                        PaperNumber {
                            x: 7; y: 40; width: 50; height: 28
                            label: ""
                            value: page.proficiencyValue
                            minimum: 0; maximum: 12; textSize: 17
                            onEdited: value => page.proficiencyValue = value
                        }
                        Text { x: 64; y: 49; text: "БОНУС МАСТЕРСТВА"; color: page.inkColor; font.pixelSize: 9; font.weight: Font.DemiBold }
                    }

                    Rectangle {
                        x: 142; y: 224; width: 212; height: 220
                        color: "#fbf8f0"; border.width: 1.5; border.color: page.inkColor; radius: 5
                        Text { anchors.horizontalCenter: parent.horizontalCenter; y: 193; text: "СПАСБРОСКИ"; color: page.inkColor; font.pixelSize: 10; font.weight: Font.DemiBold }
                        Repeater {
                            model: 6
                            delegate: Item {
                                required property int index
                                x: 12; y: 12 + index * 29; width: 188; height: 26
                                Rectangle {
                                    x: 0; y: 7; width: 12; height: 12; radius: 6
                                    color: page.saves[page.statCodes[index]] ? page.inkColor : "transparent"
                                    border.width: 1.2; border.color: page.inkColor
                                    MouseArea { anchors.fill: parent; onClicked: page.setSave(page.statCodes[index], !page.saves[page.statCodes[index]]) }
                                }
                                Text {
                                    x: 22; y: 3; width: 38
                                    text: page.signed(page.statModifier(page.statCodes[index]) + (page.saves[page.statCodes[index]] ? page.proficiencyValue : 0))
                                    color: page.inkColor; font.pixelSize: 12; font.weight: Font.DemiBold
                                }
                                Text { x: 62; y: 4; text: page.statNames[index]; color: page.inkColor; font.pixelSize: 10 }
                            }
                        }
                    }

                    Rectangle {
                        x: 142; y: 456; width: 212; height: 394
                        color: "#fbf8f0"; border.width: 1.5; border.color: page.inkColor; radius: 5
                        Text { anchors.horizontalCenter: parent.horizontalCenter; anchors.bottom: parent.bottom; anchors.bottomMargin: 6; text: "НАВЫКИ"; color: page.inkColor; font.pixelSize: 10; font.weight: Font.DemiBold }
                        Repeater {
                            model: page.skills.length
                            delegate: Item {
                                required property int index
                                property var skill: page.skills[index]
                                x: 9; y: 7 + index * 20; width: 194; height: 19
                                visible: y < 365
                                Rectangle {
                                    x: 0; y: 4; width: 10; height: 10; radius: 5
                                    color: (skill.profLevel || 0) > 0 ? page.inkColor : "transparent"
                                    border.width: 1; border.color: page.inkColor
                                    Rectangle {
                                        anchors.centerIn: parent
                                        width: 4; height: 4; radius: 2
                                        color: page.paperColor
                                        visible: (skill.profLevel || 0) > 1
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: page.setSkillLevel(index, ((skill.profLevel || 0) + 1) % 3)
                                    }
                                }
                                Text { x: 16; y: 1; width: 30; text: page.signed(page.skillBonus(skill)); color: page.inkColor; font.pixelSize: 10; font.weight: Font.DemiBold }
                                Text { x: 48; y: 1; width: 119; text: page.skillName(skill); color: page.inkColor; font.pixelSize: 9; elide: Text.ElideRight }
                                Text { x: 168; y: 1; width: 25; text: page.shortStat(skill.baseStat); color: page.faintInk; font.pixelSize: 8; horizontalAlignment: Text.AlignRight }
                            }
                        }
                    }

                    Rectangle {
                        x: 28; y: 866; width: 326; height: 48
                        color: "#fbf8f0"; border.width: 1.5; border.color: page.inkColor; radius: 5
                        Text {
                            x: 12; y: 10; width: 48
                            text: String(page.passivePerception())
                            color: page.inkColor; font.pixelSize: 20; font.weight: Font.Bold
                            horizontalAlignment: Text.AlignHCenter
                        }
                        Text { x: 68; y: 15; text: "ПАССИВНАЯ МУДРОСТЬ (ВНИМАТЕЛЬНОСТЬ)"; color: page.inkColor; font.pixelSize: 9; font.weight: Font.DemiBold }
                    }

                    PaperArea {
                        x: 28; y: 930; width: 326; height: 318
                        label: "ДРУГИЕ ВЛАДЕНИЯ И ЯЗЫКИ"
                        value: ""
                        enabled: false
                        opacity: 0.72
                    }
                    Text {
                        x: 46; y: 966; width: 288
                        text: "Это поле пока не хранится в формате персонажа. Остальные данные листа остаются полностью редактируемыми."
                        color: page.faintInk; font.pixelSize: 11; wrapMode: Text.WordWrap
                    }

                    Rectangle {
                        x: 374; y: 140; width: 310; height: 710
                        color: page.panelColor; radius: 8
                        border.width: 1; border.color: "#d1c8bb"

                        PaperNumber { x: 12; y: 12; width: 88; height: 78; label: "КЛАСС ДОСПЕХА"; value: page.acValue; minimum: 0; maximum: 99; onEdited: value => page.acValue = value }
                        Rectangle {
                            x: 111; y: 12; width: 88; height: 78; color: "#fbf8f0"; border.width: 1.5; border.color: page.inkColor; radius: 8
                            Text { anchors.horizontalCenter: parent.horizontalCenter; y: 7; text: "ИНИЦИАТИВА"; color: page.faintInk; font.pixelSize: 8; font.weight: Font.DemiBold }
                            Text { anchors.centerIn: parent; anchors.verticalCenterOffset: 7; text: page.signed(page.statModifier("dex")); color: page.inkColor; font.pixelSize: 25; font.weight: Font.Bold }
                        }
                        PaperField { x: 210; y: 12; width: 88; height: 78; label: "СКОРОСТЬ"; value: page.speedValue; textSize: 19; onEdited: value => page.speedValue = value }

                        Rectangle {
                            x: 12; y: 104; width: 286; height: 104; color: "#fbf8f0"; border.width: 1.5; border.color: page.inkColor; radius: 5
                            PaperNumber { x: 8; y: 7; width: 92; height: 85; label: "ТЕКУЩИЕ HP"; value: page.hpValue; minimum: -999; maximum: 9999; onEdited: value => page.hpValue = value }
                            PaperNumber { x: 108; y: 7; width: 78; height: 85; label: "МАКС. HP"; value: page.hpMaxValue; minimum: 0; maximum: 9999; onEdited: value => page.hpMaxValue = value }
                            PaperNumber { x: 194; y: 7; width: 82; height: 85; label: "ВРЕМ. HP"; value: page.hpTempValue; minimum: 0; maximum: 9999; onEdited: value => page.hpTempValue = value }
                        }

                        Rectangle {
                            x: 12; y: 220; width: 135; height: 86; color: "#fbf8f0"; border.width: 1.5; border.color: page.inkColor; radius: 5
                            PaperField { x: 8; y: 7; width: 119; height: 56; label: "КОСТЬ ХИТОВ"; value: page.hitDieValue; textSize: 18; onEdited: value => page.hitDieValue = value }
                            Text { anchors.horizontalCenter: parent.horizontalCenter; anchors.bottom: parent.bottom; anchors.bottomMargin: 5; text: "КОСТИ ХИТОВ"; color: page.inkColor; font.pixelSize: 9; font.weight: Font.DemiBold }
                        }

                        Rectangle {
                            x: 159; y: 220; width: 139; height: 86; color: "#fbf8f0"; border.width: 1.5; border.color: page.inkColor; radius: 5
                            Text { x: 8; y: 9; text: "УСПЕХИ"; color: page.inkColor; font.pixelSize: 9 }
                            Repeater {
                                model: 3
                                delegate: Rectangle {
                                    required property int index
                                    x: 66 + index * 21; y: 7; width: 15; height: 15; radius: 8
                                    color: page.deathSuccess[index] ? page.inkColor : "transparent"; border.width: 1.1; border.color: page.inkColor
                                    MouseArea { anchors.fill: parent; onClicked: page.toggleDeath("success", index) }
                                }
                            }
                            Text { x: 8; y: 35; text: "ПРОВАЛЫ"; color: page.inkColor; font.pixelSize: 9 }
                            Repeater {
                                model: 3
                                delegate: Rectangle {
                                    required property int index
                                    x: 66 + index * 21; y: 33; width: 15; height: 15; radius: 8
                                    color: page.deathFail[index] ? page.inkColor : "transparent"; border.width: 1.1; border.color: page.inkColor
                                    MouseArea { anchors.fill: parent; onClicked: page.toggleDeath("fail", index) }
                                }
                            }
                            Text { anchors.horizontalCenter: parent.horizontalCenter; anchors.bottom: parent.bottom; anchors.bottomMargin: 5; text: "СПАСБРОСКИ ОТ СМЕРТИ"; color: page.inkColor; font.pixelSize: 8; font.weight: Font.DemiBold }
                        }

                        Rectangle {
                            x: 12; y: 320; width: 286; height: 374; color: "#fbf8f0"; border.width: 1.5; border.color: page.inkColor; radius: 5
                            Text { x: 10; y: 7; text: "НАЗВАНИЕ"; color: page.faintInk; font.pixelSize: 8 }
                            Text { x: 151; y: 7; text: "БОНУС"; color: page.faintInk; font.pixelSize: 8 }
                            Text { x: 204; y: 7; text: "УРОН / ТИП"; color: page.faintInk; font.pixelSize: 8 }

                            Repeater {
                                model: 3
                                delegate: Item {
                                    required property int index
                                    property var weapon: page.weaponAt(index)
                                    x: 8; y: 24 + index * 45; width: 270; height: 39
                                    TextField {
                                        x: 0; y: 0; width: 137; height: 35
                                        text: weapon.name || ""
                                        color: page.inkColor; font.pixelSize: 11; background: Rectangle { color: "#eee9df"; radius: 2 }
                                        onTextEdited: page.setWeaponField(index, "name", text)
                                    }
                                    Text {
                                        x: 143; y: 8; width: 48
                                        text: page.signed(page.weaponHit(weapon))
                                        color: page.inkColor; font.pixelSize: 12; font.weight: Font.DemiBold; horizontalAlignment: Text.AlignHCenter
                                    }
                                    TextField {
                                        x: 197; y: 0; width: 73; height: 35
                                        text: weapon.damage || ""
                                        color: page.inkColor; font.pixelSize: 10; background: Rectangle { color: "#eee9df"; radius: 2 }
                                        onTextEdited: page.setWeaponField(index, "damage", text)
                                    }
                                }
                            }

                            TextArea {
                                x: 8; y: 162; width: 270; height: 178
                                text: page.weapons.length > 3 ? "Ещё оружие: " + (page.weapons.length - 3) + "\nОткрой интерактивный вид для расширенного редактирования списка." : ""
                                readOnly: true
                                color: page.faintInk
                                font.pixelSize: 10
                                wrapMode: TextEdit.Wrap
                                background: Item {}
                            }
                            Text { anchors.horizontalCenter: parent.horizontalCenter; anchors.bottom: parent.bottom; anchors.bottomMargin: 6; text: "АТАКИ И ЗАКЛИНАНИЯ"; color: page.inkColor; font.pixelSize: 10; font.weight: Font.DemiBold }
                        }
                    }

                    PaperArea {
                        x: 374; y: 866; width: 310; height: 382
                        label: "СНАРЯЖЕНИЕ"
                        value: page.equipmentValue
                        onEdited: value => page.equipmentValue = value
                    }

                    PaperArea { x: 704; y: 140; width: 228; height: 148; label: "ЧЕРТЫ ХАРАКТЕРА"; value: page.personalityValue; onEdited: value => page.personalityValue = value }
                    PaperArea { x: 704; y: 300; width: 228; height: 126; label: "ИДЕАЛЫ"; value: page.idealsValue; onEdited: value => page.idealsValue = value }
                    PaperArea { x: 704; y: 438; width: 228; height: 126; label: "ПРИВЯЗАННОСТИ"; value: page.bondsValue; onEdited: value => page.bondsValue = value }
                    PaperArea { x: 704; y: 576; width: 228; height: 126; label: "СЛАБОСТИ"; value: page.flawsValue; onEdited: value => page.flawsValue = value }
                    PaperArea { x: 704; y: 718; width: 228; height: 530; label: "УМЕНИЯ И ОСОБЕННОСТИ"; value: page.featuresValue; onEdited: value => page.featuresValue = value }

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: 7
                        text: "DnD Tracker · классический вид"
                        color: "#aaa195"
                        font.pixelSize: 8
                    }
                }
            }
        }
    }
}
