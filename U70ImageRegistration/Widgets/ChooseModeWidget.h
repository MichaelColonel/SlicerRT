#ifndef CHOOSEMODEWIDGET_H
#define CHOOSEMODEWIDGET_H

#include <QWidget>

namespace Ui {
  class ChooseModeWidget;
}

class ChooseModeWidget : public QWidget
{
  Q_OBJECT

public:
  explicit ChooseModeWidget(QWidget *parent = nullptr);
  ~ChooseModeWidget();
private:
  Ui::ChooseModeWidget *ui;
};

#endif // CHOOSEMODEWIDGET_H
