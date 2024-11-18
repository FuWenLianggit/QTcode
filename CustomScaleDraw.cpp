#include "CustomScaleDraw.h"
#include <qwt_text.h>
// 设置原始数据文件的显示横轴格式
QwtText CustomScaleDraw::label(double value) const {
    QDateTime dateTime = QDateTime::fromSecsSinceEpoch(static_cast<qint64>(value));
    QString formattedDate = dateTime.toString("HH:mm:ss"); // Format as needed
    return QwtText(formattedDate);
}
