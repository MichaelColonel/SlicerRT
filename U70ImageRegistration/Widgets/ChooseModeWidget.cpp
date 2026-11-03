#include "ChooseModeWidget.h"
#include "ui_ChooseModeWidget.h"

ChooseModeWidget::ChooseModeWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::ChooseModeWidget)
{
  ui->setupUi(this);
}

ChooseModeWidget::~ChooseModeWidget()
{
  delete ui;
}
