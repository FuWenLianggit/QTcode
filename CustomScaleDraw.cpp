#include "CustomScaleDraw.h"
#include <qwt_text.h>

QwtText CustomScaleDraw::label(double value) const {
    QDateTime dateTime = QDateTime::fromSecsSinceEpoch(static_cast<qint64>(value));
    QString formattedDate = dateTime.toString("yy MM:dd HH:mm:ss"); // Format as needed
    return QwtText(formattedDate);
}
