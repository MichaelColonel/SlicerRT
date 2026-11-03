#ifndef MOVETOISOWIDGET_H
#define MOVETOISOWIDGET_H

#include <QWidget>

namespace Ui {
  class MoveToIsoWidget;
}

class MoveToIsoWidget : public QWidget
{
  Q_OBJECT

public:
  explicit MoveToIsoWidget(QWidget *parent = nullptr);
  ~MoveToIsoWidget();

private:
  Ui::MoveToIsoWidget *ui;
};

#endif // MOVETOISOWIDGET_H
