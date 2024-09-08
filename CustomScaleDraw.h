#ifndef CUSTOMSCALEDRAW_H
#define CUSTOMSCALEDRAW_H

#include <Qwt/qwt_scale_draw.h>

#include <QDateTime>

class CustomScaleDraw : public QwtScaleDraw {
public:
    QwtText label(double value) const override;
};

#endif // CUSTOMSCALEDRAW_H
