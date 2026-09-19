import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import DndTracker

Item {
    id: book
    required property string filePath
    signal backRequested()

    readonly property int sheetWidth: 960
    readonly property int sheetHeight: 1280
    readonly property color paperColor: "#f8f3e8"
    readonly property color inkColor: "#2d2924"
    readonly property color faintInk: "#756f66"
    readonly property color panelColor: "#efe9dd"
    readonly property var pageTitles: ["Основное", "Биография", "Заклинания"]
    readonly property var statCodes: ["str", "dex", "con", "int", "wis", "cha"]
    readonly property var statNames: ["СИЛА", "ЛОВКОСТЬ", "ТЕЛОСЛОЖЕНИЕ", "ИНТЕЛЛЕКТ", "МУДРОСТЬ", "ХАРИЗМА"]

    property int currentPage: 0
    property var details: ({})
    property var saves: ({})
    property var skills: []
    property var deathSuccess: [false, false, false]
    property var deathFail: [false, false, false]
    property var weapons: []
    property var spellSlots: [0,0,0,0,0,0,0,0,0,0]
    property var expendedSlots: [[],[],[],[],[],[],[],[],[],[]]
    property var spellTexts: ["","","","","","","","","",""]
    property var preparedSpells: [[],[],[],[],[],[],[],[],[],[]]
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
    property string hitDiceTotalValue: ""
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

    property string proficienciesLanguagesValue: ""
    property string attacksNotesValue: ""
    property int cpValue: 0
    property int spValue: 0
    property int epValue: 0
    property int gpValue: 0
    property int ppValue: 0

    property string ageValue: ""
    property string heightValue: ""
    property string weightValue: ""
    property string eyesValue: ""
    property string skinValue: ""
    property string hairValue: ""
    property string appearanceValue: ""
    property string alliesOrganizationsValue: ""
    property string alliesSymbolValue: ""
    property string backstoryValue: ""
    property string additionalFeaturesValue: ""
    property string treasureValue: ""

    property real fitScale: Math.min(1.0, Math.max(0.28, (sheetFlick.width - 24) / sheetWidth))
    property real zoom: fitScale
    property bool manualZoom: false

    function clone(value, fallback) {
        const source = value === undefined || value === null ? fallback : value
        return JSON.parse(JSON.stringify(source))
    }

    function asInt(value, fallback) {
        const n = Number(value)
        return Number.isFinite(n) ? Math.trunc(n) : fallback
    }

    function reload() {
        const d = App.characterDetails(filePath)
        const e = ClassicStore.load(filePath)
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
        saves = clone(d.saves, {})
        skills = clone(d.skills, [])
        deathSuccess = clone(d.deathSuccess, [false, false, false])
        deathFail = clone(d.deathFail, [false, false, false])
        personalityValue = d.personality || ""
        idealsValue = d.ideals || ""
        bondsValue = d.bonds || ""
        flawsValue = d.flaws || ""
        featuresValue = d.features || ""
        equipmentValue = d.equipment || ""
        weapons = clone(d.weapons, [])
        casterClassValue = d.casterClass || ""
        spellAbility = d.spellAbility || "int"
        spellSlots = clone(d.spellSlots, [0,0,0,0,0,0,0,0,0,0])
        expendedSlots = clone(d.expendedSlots, [[],[],[],[],[],[],[],[],[],[]])
        spellTexts = clone(d.spellTexts, ["","","","","","","","","",""])
        passivePerceptionOverride = d.passivePerceptionOverride === undefined
                ? -1 : asInt(d.passivePerceptionOverride, -1)

        proficienciesLanguagesValue = e.proficienciesLanguages || ""
        attacksNotesValue = e.attacksNotes || ""
        hitDiceTotalValue = e.hitDiceTotal || ""
        cpValue = asInt(e.cp, 0)
        spValue = asInt(e.sp, 0)
        epValue = asInt(e.ep, 0)
        gpValue = asInt(e.gp, 0)
        ppValue = asInt(e.pp, 0)
        ageValue = e.age || ""
        heightValue = e.height || ""
        weightValue = e.weight || ""
        eyesValue = e.eyes || ""
        skinValue = e.skin || ""
        hairValue = e.hair || ""
        appearanceValue = e.appearance || ""
        alliesOrganizationsValue = e.alliesOrganizations || ""
        alliesSymbolValue = e.alliesSymbol || ""
        backstoryValue = e.backstory || ""
        additionalFeaturesValue = e.additionalFeatures || ""
        treasureValue = e.treasure || ""
        preparedSpells = clone(e.preparedSpells, [[],[],[],[],[],[],[],[],[],[]])
    }

    function save() {
        const baseValues = {
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
        const extraValues = {
            proficienciesLanguages: proficienciesLanguagesValue,
            attacksNotes: attacksNotesValue,
            hitDiceTotal: hitDiceTotalValue,
            cp: cpValue,
            sp: spValue,
            ep: epValue,
            gp: gpValue,
            pp: ppValue,
            age: ageValue,
            height: heightValue,
            weight: weightValue,
            eyes: eyesValue,
            skin: skinValue,
            hair: hairValue,
            appearance: appearanceValue,
            alliesOrganizations: alliesOrganizationsValue,
            alliesSymbol: alliesSymbolValue,
            backstory: backstoryValue,
            additionalFeatures: additionalFeaturesValue,
            treasure: treasureValue,
            preparedSpells: preparedSpells
        }
        const ok = App.saveCharacterBasics(filePath, baseValues)
                && ClassicStore.save(filePath, extraValues)
        if (ok) {
            saveState.text = "Сохранено ✓"
            saveState.opacity = 1
            saveFlash.restart()
        } else {
            saveState.text = "Ошибка сохранения"
            saveState.opacity = 1
        }
        return ok
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

    function statModifier(code) { return Math.floor((statValue(code) - 10) / 2) }
    function signed(value) { return (value >= 0 ? "+" : "") + value }

    function setSave(code, checked) {
        let copy = clone(saves, {})
        copy[code] = checked
        saves = copy
    }

    function skillName(skill) {
        if (!skill) return ""
        const labels = {
            "acrobatics": "Акробатика", "animalHandling": "Уход за животными",
            "animal-handling": "Уход за животными", "arcana": "Магия",
            "athletics": "Атлетика", "deception": "Обман", "history": "История",
            "insight": "Проницательность", "intimidation": "Запугивание",
            "investigation": "Расследование", "medicine": "Медицина", "nature": "Природа",
            "perception": "Внимательность", "performance": "Выступление",
            "persuasion": "Убеждение", "religion": "Религия",
            "sleightOfHand": "Ловкость рук", "sleight-of-hand": "Ловкость рук",
            "stealth": "Скрытность", "survival": "Выживание"
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
        let copy = clone(skills, [])
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
        let copy = clone(which === "success" ? deathSuccess : deathFail, [false, false, false])
        while (copy.length < 3) copy.push(false)
        copy[index] = !copy[index]
        if (which === "success") deathSuccess = copy
        else deathFail = copy
    }

    function weaponAt(index) { return index >= 0 && index < weapons.length ? weapons[index] : ({}) }

    function setWeaponField(index, key, value) {
        let copy = clone(weapons, [])
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

    function spellAbilityName() {
        if (spellAbility === "wis") return "Мудрость"
        if (spellAbility === "cha") return "Харизма"
        return "Интеллект"
    }

    function setSpellAbilityFromText(value) {
        const text = String(value).toLowerCase()
        if (text.indexOf("муд") === 0 || text === "wis") spellAbility = "wis"
        else if (text.indexOf("хар") === 0 || text === "cha") spellAbility = "cha"
        else spellAbility = "int"
    }

    function spellSaveDc() { return 8 + proficiencyValue + statModifier(spellAbility) }
    function spellAttackBonus() { return proficiencyValue + statModifier(spellAbility) }

    function spellLine(level, index) {
        const lines = String(spellTexts[level] || "").split("\n")
        return index < lines.length ? lines[index] : ""
    }

    function setSpellLine(level, index, value) {
        let texts = clone(spellTexts, ["","","","","","","","","",""])
        while (texts.length < 10) texts.push("")
        let lines = String(texts[level] || "").split("\n")
        if (lines.length === 1 && lines[0] === "") lines = []
        while (lines.length <= index) lines.push("")
        lines[index] = value
        while (lines.length > 0 && lines[lines.length - 1] === "") lines.pop()
        texts[level] = lines.join("\n")
        spellTexts = texts
    }

    function spellPrepared(level, index) {
        return preparedSpells[level] && preparedSpells[level][index] === true
    }

    function toggleSpellPrepared(level, index) {
        let all = clone(preparedSpells, [[],[],[],[],[],[],[],[],[],[]])
        while (all.length < 10) all.push([])
        let levelFlags = all[level] || []
        while (levelFlags.length <= index) levelFlags.push(false)
        levelFlags[index] = !levelFlags[index]
        all[level] = levelFlags
        preparedSpells = all
    }

    function spellSlotCount(level) { return asInt(spellSlots[level], 0) }

    function setSpellSlotCount(level, count) {
        let slots = clone(spellSlots, [0,0,0,0,0,0,0,0,0,0])
        while (slots.length < 10) slots.push(0)
        slots[level] = Math.max(0, count)
        spellSlots = slots
        if (spellExpendedCount(level) > count) setSpellExpendedCount(level, count)
    }

    function spellExpendedCount(level) {
        const flags = expendedSlots[level] || []
        let count = 0
        for (let i = 0; i < flags.length; ++i) if (flags[i]) ++count
        return count
    }

    function setSpellExpendedCount(level, count) {
        let all = clone(expendedSlots, [[],[],[],[],[],[],[],[],[],[]])
        while (all.length < 10) all.push([])
        const total = Math.max(spellSlotCount(level), count)
        let flags = []
        for (let i = 0; i < total; ++i) flags.push(i < count)
        all[level] = flags
        expendedSlots = all
    }

    function changeZoom(delta) {
        manualZoom = true
        zoom = Math.max(0.28, Math.min(1.8, zoom + delta))
    }

    function fitToWidth() { manualZoom = false; zoom = fitScale }
    function previousPage() { currentPage = (currentPage + 2) % 3; sheetFlick.contentX = 0; sheetFlick.contentY = 0 }
    function nextPage() { currentPage = (currentPage + 1) % 3; sheetFlick.contentX = 0; sheetFlick.contentY = 0 }

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
                        book.save()
                        book.backRequested()
                    }
                }
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 0
                    Label {
                        Layout.fillWidth: true
                        text: book.nameValue.length ? book.nameValue : "Персонаж"
                        color: Theme.text
                        font.pixelSize: 18
                        font.weight: Font.Bold
                        elide: Text.ElideRight
                    }
                    Label {
                        text: "Классический вид · " + book.pageTitles[book.currentPage]
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
                    onClicked: book.save()
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 52
            color: Theme.surface
            border.width: 1
            border.color: Theme.border

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 10
                anchors.rightMargin: 10
                spacing: 6

                AppButton { text: "‹"; implicitWidth: 42; onClicked: book.previousPage() }
                Label {
                    Layout.fillWidth: true
                    text: "Лист " + (book.currentPage + 1) + "/3 · " + book.pageTitles[book.currentPage]
                    color: Theme.textMuted
                    elide: Text.ElideRight
                }
                AppButton { text: "›"; implicitWidth: 42; onClicked: book.nextPage() }
                AppButton { text: "−"; implicitWidth: 42; onClicked: book.changeZoom(-0.1) }
                AppButton {
                    text: Math.round(book.zoom * 100) + "%"
                    implicitWidth: 68
                    onClicked: book.fitToWidth()
                    ToolTip.visible: hovered
                    ToolTip.text: "По ширине"
                }
                AppButton { text: "+"; implicitWidth: 42; onClicked: book.changeZoom(0.1) }
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
                width: Math.max(sheetFlick.width, book.sheetWidth * book.zoom + 24)
                height: book.sheetHeight * book.zoom + 24

                Item {
                    id: scaledSheet
                    x: Math.max(12, (paperHost.width - book.sheetWidth * book.zoom) / 2)
                    y: 12
                    width: book.sheetWidth
                    height: book.sheetHeight
                    scale: book.zoom
                    transformOrigin: Item.TopLeft

                    Loader {
                        anchors.fill: parent
                        sourceComponent: book.currentPage === 0 ? pageOneComponent
                                       : book.currentPage === 1 ? pageTwoComponent
                                       : pageThreeComponent
                    }
                }
            }
        }
    }

    Component { id: pageOneComponent; ClassicSheetPageOne { book: book } }
    Component { id: pageTwoComponent; ClassicSheetPageTwo { book: book } }
    Component { id: pageThreeComponent; ClassicSheetPageThree { book: book } }
}
