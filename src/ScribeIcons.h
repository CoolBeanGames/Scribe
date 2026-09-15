#pragma once
#include <QIcon>
#include <QPixmap>
#include <QPainter>
#include <QPen>
#include <QBrush>

namespace ScribeIcons {

inline QIcon makeIcon(void (*drawFunc)(QPainter&, int, int), int size = 20) {
    QIcon icon;
    auto renderPixmap = [&](const QColor& color) -> QPixmap {
        QPixmap pm(size, size);
        pm.fill(Qt::transparent);
        QPainter p(&pm);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setPen(QPen(color, 1.6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        p.setBrush(Qt::NoBrush);
        drawFunc(p, size, size);
        return pm;
    };

    icon.addPixmap(renderPixmap(QColor("#8A95A8")), QIcon::Normal, QIcon::Off);
    icon.addPixmap(renderPixmap(QColor("#8B7CFF")), QIcon::Normal, QIcon::On);
    icon.addPixmap(renderPixmap(QColor("#8B7CFF")), QIcon::Active, QIcon::On);
    icon.addPixmap(renderPixmap(QColor("#F4F6FA")), QIcon::Active, QIcon::Off);
    icon.addPixmap(renderPixmap(QColor("#4A5568")), QIcon::Disabled, QIcon::Off);
    return icon;
}

// ── Icons ───────────────────────────────────────────────────────────────────

inline QIcon openIcon() {
    return makeIcon([](QPainter& p, int w, int h) {
        // Folder
        p.drawRoundedRect(3, 6, 14, 10, 1.5, 1.5);
        p.drawLine(3, 6, 7, 6);
        p.drawLine(7, 6, 9, 8);
        p.drawLine(9, 8, 17, 8);
    });
}

inline QIcon saveIcon() {
    return makeIcon([](QPainter& p, int w, int h) {
        // Diskette
        p.drawRoundedRect(3, 3, 14, 14, 2, 2);
        p.drawRect(6, 3, 8, 5);
        p.drawRect(5, 11, 10, 6);
    });
}

inline QIcon printIcon() {
    return makeIcon([](QPainter& p, int w, int h) {
        // Top paper sheet
        p.drawRect(6, 3, 8, 4);
        // Printer body
        p.drawRoundedRect(3, 7, 14, 6, 1.5, 1.5);
        // Bottom output paper
        p.drawRect(5, 11, 10, 6);
        // Text lines on output paper
        p.drawLine(7, 13, 13, 13);
        p.drawLine(7, 15, 11, 15);
    });
}

inline QIcon undoIcon() {
    return makeIcon([](QPainter& p, int w, int h) {
        // Undo curved arrow
        p.drawLine(4, 9, 8, 5);
        p.drawLine(4, 9, 8, 13);
        p.drawLine(4, 9, 11, 9);
        p.drawArc(7, 9, 9, 8, 90 * 16, -180 * 16);
    });
}

inline QIcon redoIcon() {
    return makeIcon([](QPainter& p, int w, int h) {
        // Redo curved arrow
        p.drawLine(16, 9, 12, 5);
        p.drawLine(16, 9, 12, 13);
        p.drawLine(16, 9, 9, 9);
        p.drawArc(4, 9, 9, 8, 90 * 16, 180 * 16);
    });
}

inline QIcon runIcon() {
    return makeIcon([](QPainter& p, int w, int h) {
        // Play triangle
        QPolygon pts;
        pts << QPoint(6, 4) << QPoint(16, 10) << QPoint(6, 16);
        p.setBrush(p.pen().color());
        p.drawPolygon(pts);
    });
}

inline QIcon boldIcon() {
    return makeIcon([](QPainter& p, int w, int h) {
        QFont f = p.font();
        f.setBold(true);
        f.setPointSize(11);
        f.setFamily("Segoe UI");
        p.setFont(f);
        p.drawText(QRect(0, 0, w, h), Qt::AlignCenter, "B");
    });
}

inline QIcon italicIcon() {
    return makeIcon([](QPainter& p, int w, int h) {
        QFont f = p.font();
        f.setItalic(true);
        f.setPointSize(11);
        f.setFamily("Segoe UI");
        p.setFont(f);
        p.drawText(QRect(0, 0, w, h), Qt::AlignCenter, "I");
    });
}

inline QIcon underlineIcon() {
    return makeIcon([](QPainter& p, int w, int h) {
        QFont f = p.font();
        f.setUnderline(true);
        f.setPointSize(11);
        f.setFamily("Segoe UI");
        p.setFont(f);
        p.drawText(QRect(0, 0, w, h), Qt::AlignCenter, "U");
    });
}

inline QIcon bulletListIcon() {
    return makeIcon([](QPainter& p, int w, int h) {
        // 3 bullet dots and lines
        p.setBrush(p.pen().color());
        p.drawEllipse(3, 5, 2, 2);
        p.drawEllipse(3, 9, 2, 2);
        p.drawEllipse(3, 13, 2, 2);

        p.setBrush(Qt::NoBrush);
        p.drawLine(7, 6, 17, 6);
        p.drawLine(7, 10, 17, 10);
        p.drawLine(7, 14, 17, 14);
    });
}

inline QIcon numberedListIcon() {
    return makeIcon([](QPainter& p, int w, int h) {
        // 1 2 3 lines
        QFont f = p.font();
        f.setPointSize(7);
        f.setBold(true);
        f.setFamily("Segoe UI");
        p.setFont(f);
        p.drawText(QRect(2, 2, 6, 7), Qt::AlignLeft | Qt::AlignVCenter, "1");
        p.drawText(QRect(2, 7, 6, 7), Qt::AlignLeft | Qt::AlignVCenter, "2");
        p.drawText(QRect(2, 12, 6, 7), Qt::AlignLeft | Qt::AlignVCenter, "3");

        p.drawLine(8, 5, 17, 5);
        p.drawLine(8, 10, 17, 10);
        p.drawLine(8, 15, 15, 15);
    });
}

inline QIcon fontColorIcon() {
    return makeIcon([](QPainter& p, int w, int h) {
        QFont f = p.font();
        f.setBold(true);
        f.setPointSize(10);
        f.setFamily("Segoe UI");
        p.setFont(f);
        p.drawText(QRect(0, 0, w, h - 3), Qt::AlignCenter, "A");
        p.setPen(QPen(QColor("#8B7CFF"), 2.2));
        p.drawLine(4, h - 3, w - 4, h - 3);
    });
}

inline QIcon alignLeftIcon() {
    return makeIcon([](QPainter& p, int w, int h) {
        p.drawLine(3, 5, 17, 5);
        p.drawLine(3, 8, 12, 8);
        p.drawLine(3, 11, 17, 11);
        p.drawLine(3, 14, 10, 14);
    });
}

inline QIcon alignCenterIcon() {
    return makeIcon([](QPainter& p, int w, int h) {
        p.drawLine(3, 5, 17, 5);
        p.drawLine(5, 8, 15, 8);
        p.drawLine(3, 11, 17, 11);
        p.drawLine(6, 14, 14, 14);
    });
}

inline QIcon alignRightIcon() {
    return makeIcon([](QPainter& p, int w, int h) {
        p.drawLine(3, 5, 17, 5);
        p.drawLine(8, 8, 17, 8);
        p.drawLine(3, 11, 17, 11);
        p.drawLine(10, 14, 17, 14);
    });
}

inline QIcon alignJustifyIcon() {
    return makeIcon([](QPainter& p, int w, int h) {
        p.drawLine(3, 5, 17, 5);
        p.drawLine(3, 8, 17, 8);
        p.drawLine(3, 11, 17, 11);
        p.drawLine(3, 14, 17, 14);
    });
}

inline QIcon tableIcon() {
    return makeIcon([](QPainter& p, int w, int h) {
        p.drawRoundedRect(3, 4, 14, 12, 1.5, 1.5);
        p.drawLine(3, 8, 17, 8);
        p.drawLine(10, 4, 10, 16);
    });
}

inline QIcon imageIcon() {
    return makeIcon([](QPainter& p, int w, int h) {
        p.drawRoundedRect(3, 4, 14, 12, 1.5, 1.5);
        // Sun
        p.drawEllipse(5, 6, 2, 2);
        // Mountain lines
        QPolygon pts;
        pts << QPoint(4, 14) << QPoint(8, 9) << QPoint(12, 14);
        p.drawPolyline(pts);
    });
}

inline QIcon linkIcon() {
    return makeIcon([](QPainter& p, int w, int h) {
        // Link chain
        p.drawEllipse(4, 8, 6, 6);
        p.drawEllipse(10, 6, 6, 6);
    });
}

inline QIcon sectionBreakIcon() {
    return makeIcon([](QPainter& p, int w, int h) {
        p.drawLine(3, 4, 17, 4);
        p.drawLine(3, 7, 13, 7);
        p.drawLine(2, 10, 18, 10);
        p.drawLine(2, 12, 18, 12);
        p.drawLine(3, 15, 17, 15);
    });
}

inline QIcon pageBreakIcon() {
    return makeIcon([](QPainter& p, int w, int h) {
        p.drawRect(4, 2, 12, 5);
        p.drawLine(2, 10, 18, 10);
        p.drawRect(4, 13, 12, 5);
    });
}

inline QIcon columnsIcon() {
    return makeIcon([](QPainter& p, int w, int h) {
        p.drawRect(3, 3, 6, 14);
        p.drawRect(11, 3, 6, 14);
    });
}

inline QIcon addRowIcon() {
    return makeIcon([](QPainter& p, int w, int h) {
        // Row grid + plus
        p.drawRect(3, 8, 14, 6);
        p.drawLine(10, 2, 10, 6);
        p.drawLine(8, 4, 12, 4);
    });
}

inline QIcon addColIcon() {
    return makeIcon([](QPainter& p, int w, int h) {
        // Column grid + plus
        p.drawRect(7, 4, 6, 13);
        p.drawLine(2, 8, 6, 8);
        p.drawLine(4, 6, 4, 10);
    });
}

inline QIcon delRowIcon() {
    return makeIcon([](QPainter& p, int w, int h) {
        p.drawRect(3, 8, 14, 6);
        p.drawLine(8, 4, 12, 4);
    });
}

inline QIcon delColIcon() {
    return makeIcon([](QPainter& p, int w, int h) {
        p.drawRect(7, 4, 6, 13);
        p.drawLine(2, 8, 6, 8);
    });
}

inline QIcon cellColorIcon() {
    return makeIcon([](QPainter& p, int w, int h) {
        // Paint bucket
        p.drawRect(4, 5, 8, 9);
        p.drawLine(12, 10, 16, 14);
        p.drawEllipse(15, 15, 1, 1);
    });
}

inline QIcon lineNumbersIcon() {
    return makeIcon([](QPainter& p, int w, int h) {
        // Line numbers toggle: vertical separator + line number hashes + text lines
        p.drawLine(7, 3, 7, 17);
        // Numbers on left (dots / small dashes)
        p.drawLine(3, 6, 5, 6);
        p.drawLine(3, 10, 5, 10);
        p.drawLine(3, 14, 5, 14);
        // Text lines on right
        p.drawLine(9, 6, 17, 6);
        p.drawLine(9, 10, 15, 10);
        p.drawLine(9, 14, 17, 14);
    });
}

} // namespace ScribeIcons
