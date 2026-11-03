#include "MoveToIsoWidget.h"
#include "ui_MoveToIsoWidget.h"

MoveToIsoWidget::MoveToIsoWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::MoveToIsoWidget)
{
  ui->setupUi(this);
}

MoveToIsoWidget::~MoveToIsoWidget()
{
  delete ui;
}
