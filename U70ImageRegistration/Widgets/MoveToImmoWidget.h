#ifndef MOVETOIMMOWIDGET_H
#define MOVETOIMMOWIDGET_H

#include <QWidget>

namespace Ui {
  class MoveToImmoWidget;
}

class MoveToImmoWidget : public QWidget
{
  Q_OBJECT

public:
  explicit MoveToImmoWidget(QWidget *parent = nullptr);
  ~MoveToImmoWidget();

private:
  Ui::MoveToImmoWidget *ui;
};

#endif // MOVETOIMMOWIDGET_H
