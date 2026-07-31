#ifndef TOUCHUTILS_H
#define TOUCHUTILS_H

#include <QScroller>
#include <QWidget>

namespace TouchUtils {
// Включает кинетический скроллинг пальцем (Touch gesture) для любого виджета или области скролла.
inline void enableTouchScroll(QWidget *w) {
    if (!w) return;
    QScroller::grabGesture(w, QScroller::TouchGesture);
}
}

#endif // TOUCHUTILS_H
