#include "CharacterDocument.h"
#include "JsonUtils.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QtTest>

class CoreTests : public QObject {
    Q_OBJECT

private slots:
    void parsesNumericStrings();
    void lssRoundTripPreservesUnknownData();
    void rejectsBrokenNestedLssData();
};

void CoreTests::parsesNumericStrings() {
    const QJsonObject data{
        {"vitality", QJsonObject{
            {"initiative", QJsonObject{{"value", "+3"}}},
            {"negative", QJsonObject{{"value", " -2 "}}},
            {"bad", QJsonObject{{"value", "nope"}}}
        }}
    };

    QCOMPARE(JsonUtils::safeGetInt(data, {"vitality", "initiative"}), 3);
    QCOMPARE(JsonUtils::safeGetInt(data, {"vitality", "negative"}), -2);
    QCOMPARE(JsonUtils::safeGetInt(data, {"vitality", "bad"}, 17), 17);
}

void CoreTests::lssRoundTripPreservesUnknownData() {
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath("character.json");

    const QJsonObject inner{
        {"name", QJsonObject{{"value", "Тестовый герой"}}},
        {"vitality", QJsonObject{
            {"hp-current", QJsonObject{{"value", 10}}},
            {"hp-max", QJsonObject{{"value", 15}}},
            {"initiative", QJsonObject{{"value", "+3"}}}
        }},
        {"unknownSection", QJsonObject{{"keepMe", 42}}}
    };
    const QJsonObject root{
        {"data", QString::fromUtf8(QJsonDocument(inner).toJson(QJsonDocument::Compact))},
        {"metadata", QJsonObject{{"source", "test"}}}
    };

    QFile initial(path);
    QVERIFY(initial.open(QIODevice::WriteOnly));
    initial.write(QJsonDocument(root).toJson());
    initial.close();

    CharacterDocument document;
    QVERIFY(document.load(path));
    QCOMPARE(document.getName(), QStringLiteral("Тестовый герой"));
    QCOMPARE(document.getInitiative(), 3);
    QCOMPARE(document.getHp(), 10);

    document.setHp(7);
    QVERIFY(document.save());

    QFile saved(path);
    QVERIFY(saved.open(QIODevice::ReadOnly));
    const QJsonDocument savedRootDocument = QJsonDocument::fromJson(saved.readAll());
    QVERIFY(savedRootDocument.isObject());
    const QJsonObject savedRoot = savedRootDocument.object();
    QCOMPARE(savedRoot.value("metadata").toObject().value("source").toString(),
             QStringLiteral("test"));

    const QJsonDocument savedInnerDocument =
        QJsonDocument::fromJson(savedRoot.value("data").toString().toUtf8());
    QVERIFY(savedInnerDocument.isObject());
    const QJsonObject savedInner = savedInnerDocument.object();
    QCOMPARE(savedInner.value("unknownSection").toObject().value("keepMe").toInt(), 42);
    QCOMPARE(JsonUtils::safeGetInt(savedInner, {"vitality", "hp-current"}), 7);
    QCOMPARE(savedInner.value("vitality").toObject()
                 .value("initiative").toObject().value("value").toString(),
             QStringLiteral("+3"));
}

void CoreTests::rejectsBrokenNestedLssData() {
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath("broken.json");

    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(QJsonDocument(QJsonObject{
        {"data", QStringLiteral("{this is not json")},
        {"metadata", QJsonObject{{"important", true}}}
    }).toJson());
    file.close();

    CharacterDocument document;
    QVERIFY(!document.load(path));
}

QTEST_MAIN(CoreTests)
#include "CoreTests.moc"
