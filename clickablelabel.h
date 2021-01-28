#ifndef CLICKABLELABEL_H
#define CLICKABLELABEL_H

#include <QWidget>
#include <qlabel.h>

class ClickableLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ClickableLabel(QWidget *parent = nullptr);
    ~ClickableLabel();
    bool isClicked() { return mClicked; }

signals:
    void clicked();
    void released();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *ev);

private:
    bool mClicked = true;
};

#endif // CLICKABLELABEL_H
