#include "MoveToImmoWidget.h"
#include "ui_MoveToImmoWidget.h"

MoveToImmoWidget::MoveToImmoWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::MoveToImmoWidget)
{
  ui->setupUi(this);
}

MoveToImmoWidget::~MoveToImmoWidget()
{
  delete ui;
}
