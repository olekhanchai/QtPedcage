#include "clickablelabel.h"
#include "QTimer"

ClickableLabel::ClickableLabel(QWidget *parent) : QLabel(parent)
{
}

ClickableLabel::~ClickableLabel(){}

void ClickableLabel::mousePressEvent(QMouseEvent *event)
{
    mClicked = !mClicked;
    emit clicked();
}

void ClickableLabel::mouseReleaseEvent(QMouseEvent *ev)
{
    emit released();
}
