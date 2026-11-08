/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// Qt includes
#include <QDebug>
#include <QDateTime>

// CTK includes
#include <ctkPushButton.h>
#include <ctkWorkflow.h>
#include <ctkWorkflowStep.h>
#include <ctkWorkflowWidget.h>
#include <ctkWorkflowStackedWidget.h>
#include <ctkWorkflowTabWidget.h>
#include <ctkWorkflowGroupBox.h>
#include <ctkWorkflowButtonBoxWidget.h>

// Slicer includes
#include "qSlicerU70ImageRegistrationModuleWidget.h"
#include "ui_qSlicerU70ImageRegistrationModuleWidget.h"

// Logic includes
#include "vtkSlicerU70ImageRegistrationLogic.h"

// U70RoomOpcUa includes
#include <vtkSlicerU70RoomOpcUaLogic.h>
#include <vtkSlicerU70RoomScadaOpcUaLogic.h>
#include "vtkSlicerU70RoomScadaOpcUaNodesNames.h"

// U70ImageRegistration widgets includes
#include "ctkExampleDerivedWorkflowWidgetStep.h"
#include "ChooseModeWidget.h"
#include "MoveToImmoWidget.h"
#include "MoveToIsoWidget.h"

//-----------------------------------------------------------------------------
class qSlicerU70ImageRegistrationModuleWidgetPrivate : public Ui_qSlicerU70ImageRegistrationModuleWidget
{
  Q_DECLARE_PUBLIC(qSlicerU70ImageRegistrationModuleWidget);
protected:
  qSlicerU70ImageRegistrationModuleWidget* const q_ptr;
public:
  qSlicerU70ImageRegistrationModuleWidgetPrivate(qSlicerU70ImageRegistrationModuleWidget& object);
  ~qSlicerU70ImageRegistrationModuleWidgetPrivate() = default;

  vtkSlicerU70ImageRegistrationLogic* logic() const;
  vtkSlicerU70RoomOpcUaLogic* opcUaLogic() const;
  vtkSlicerU70RoomScadaOpcUaLogic* scadaOpcUaLogic() const;

  ctkWorkflow* Workflow{ nullptr };
};

//-----------------------------------------------------------------------------
// qSlicerU70ImageRegistrationModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerU70ImageRegistrationModuleWidgetPrivate::qSlicerU70ImageRegistrationModuleWidgetPrivate(
  qSlicerU70ImageRegistrationModuleWidget &object)
  :
  q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
vtkSlicerU70ImageRegistrationLogic* qSlicerU70ImageRegistrationModuleWidgetPrivate::logic() const
{
  Q_Q(const qSlicerU70ImageRegistrationModuleWidget);
  return vtkSlicerU70ImageRegistrationLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
vtkSlicerU70RoomOpcUaLogic* qSlicerU70ImageRegistrationModuleWidgetPrivate::opcUaLogic() const
{
  vtkSlicerU70RoomOpcUaLogic* opcUaLogic = nullptr;
  if (this->logic())
  {
    opcUaLogic = this->logic()->GetOpcUaLogic();
  }
  return opcUaLogic;
}

//-----------------------------------------------------------------------------
vtkSlicerU70RoomScadaOpcUaLogic* qSlicerU70ImageRegistrationModuleWidgetPrivate::scadaOpcUaLogic() const
{
  vtkSlicerU70RoomScadaOpcUaLogic* scadaOpcUaLogic = nullptr;
  if (this->opcUaLogic())
  {
    scadaOpcUaLogic = this->opcUaLogic()->GetScadaOpcUaLogic();
  }
  return scadaOpcUaLogic;
}

//-----------------------------------------------------------------------------
// qSlicerU70ImageRegistrationModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerU70ImageRegistrationModuleWidget::qSlicerU70ImageRegistrationModuleWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerU70ImageRegistrationModuleWidgetPrivate(*this))
{
}

//-----------------------------------------------------------------------------
qSlicerU70ImageRegistrationModuleWidget::~qSlicerU70ImageRegistrationModuleWidget() {}

//-----------------------------------------------------------------------------
void qSlicerU70ImageRegistrationModuleWidget::setup()
{
  Q_D(qSlicerU70ImageRegistrationModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  QObject::connect(d->PushButton_SetModPosSysTime, SIGNAL(clicked()), this,
    SLOT(onSetModPosLocalTimeClicked()));
  QObject::connect(d->PushButton_SetModPosEventString, SIGNAL(clicked()), this,
    SLOT(onSetEventStringClicked()));
  QObject::connect(d->PushButton_SetModPosState, SIGNAL(clicked()), this,
    SLOT(onSetStateClicked()));

  d->Workflow = new ctkWorkflow(this);
  QObject::connect(d->Workflow, &ctkWorkflow::currentStepChanged, this, &qSlicerU70ImageRegistrationModuleWidget::onWorkflowCurrentStepChanged);

  d->WorkflowWidget->setWorkflow(d->Workflow);

  ctkWorkflowGroupBox* groupBox = d->WorkflowWidget->workflowGroupBox();
  groupBox->setPreText("I am some pre-text");
  groupBox->setPostText("I am some post-text");
  groupBox->setHideWidgetsOfNonCurrentSteps(false);

  // create and add the first workflow step (depends on workflowWidget
  // type)
  ctkExampleDerivedWorkflowWidgetStep* step1 = new ctkExampleDerivedWorkflowWidgetStep("Step 1");
  step1->setName("Choose Mode");
  step1->setDescription("I am in step 1");
  if (ctkWorkflowTabWidget* tabWidget = qobject_cast<ctkWorkflowTabWidget*>(d->WorkflowWidget))
  {
    tabWidget->associateStepWithLabel(step1, "tab1");
  }
  QWidget* stepAreaWidget = step1->stepArea();
  if (stepAreaWidget)
  {
    QWidget* widget = new ChooseModeWidget(stepAreaWidget);
    stepAreaWidget->setLayout(widget->layout());
  }
  // step1 is the initial step
  d->Workflow->setInitialStep(step1);

  // create and add the second workflow step (depends on
  // workflowWidget type)
  ctkExampleDerivedWorkflowWidgetStep* step2 = new ctkExampleDerivedWorkflowWidgetStep("Step 2");
  step2->setName("HOME position");
  step2->setDescription("I am in step 2");
  if (ctkWorkflowTabWidget* tabWidget = qobject_cast<ctkWorkflowTabWidget*>(d->WorkflowWidget))
  {
    tabWidget->associateStepWithLabel(step2, "tab2");
  }
  QWidget* step2AreaWidget = step2->stepArea();
  if (step2AreaWidget)
  {
    QWidget* widget = new MoveToImmoWidget(step2AreaWidget);
    step2AreaWidget->setLayout(widget->layout());
  }
  // add the steps to the workflow
  d->Workflow->addTransition(step1, step2);
  qDebug() << Q_FUNC_INFO << "CurrentStepCount = " << d->Workflow->steps().count();

  // create and add the third workflow step (depends on
  // workflowWidget type)
  ctkExampleDerivedWorkflowWidgetStep* step3 = new ctkExampleDerivedWorkflowWidgetStep("Step 3");
  step3->setName("IMMO position");
  step3->setDescription("I am in step 3");
  if (ctkWorkflowTabWidget* tabWidget = qobject_cast<ctkWorkflowTabWidget*>(d->WorkflowWidget))
  {
    tabWidget->associateStepWithLabel(step3, "tab3");
  }
  QWidget* step3AreaWidget = step3->stepArea();
  if (step3AreaWidget)
  {
    QWidget* widget = new MoveToIsoWidget(step3AreaWidget);
    step3AreaWidget->setLayout(widget->layout());
  }
  // add the steps to the workflow
  d->Workflow->addTransition(step2, step3);
  qDebug() << Q_FUNC_INFO << "CurrentStepCount = " << d->Workflow->steps().count();

  // create and add the forth workflow step (depends on
  // workflowWidget type)
  ctkExampleDerivedWorkflowWidgetStep* step4 = new ctkExampleDerivedWorkflowWidgetStep("Step 4");
  step4->setName("ISO position");
  step4->setDescription("I am in step 4");
  if (ctkWorkflowTabWidget* tabWidget = qobject_cast<ctkWorkflowTabWidget*>(d->WorkflowWidget))
  {
    tabWidget->associateStepWithLabel(step4, "tab4");
  }
  // add the steps to the workflow
  d->Workflow->addTransition(step3, step4);

  qDebug() << Q_FUNC_INFO << "CurrentStepCount = " << d->Workflow->steps().count();

  // start the workflow
  d->Workflow->start();

  if (d->scadaOpcUaLogic())
  {
    qDebug() << Q_FUNC_INFO << "SCADA OPC UA logic is OK";
  }
}

void qSlicerU70ImageRegistrationModuleWidget::onWorkflowCurrentStepChanged(ctkWorkflowStep* currentStep)
{
  Q_D(qSlicerU70ImageRegistrationModuleWidget);

  if (currentStep && (currentStep->id() == QString("Step 1")))
  {
    d->WorkflowWidget->buttonBoxWidget()->setNextButtonFormat("{nextButtonText|\"To HOME\"}(next:description)[->]");
    d->WorkflowWidget->buttonBoxWidget()->setBackButtonFormat("{backButtonText|\"Mode\"}(back:description)[<-]");
  }
  if (currentStep && (currentStep->id() == QString("Step 2")))
  {
    d->WorkflowWidget->buttonBoxWidget()->setNextButtonFormat("{nextButtonText|\"To IMMO\"}(next:description)[->]");
    d->WorkflowWidget->buttonBoxWidget()->setBackButtonFormat("{backButtonText|\"To HOME\"}(back:description)[<-]");
  }
  if (currentStep && (currentStep->id() == QString("Step 3")))
  {
    d->WorkflowWidget->buttonBoxWidget()->setNextButtonFormat("{nextButtonText|\"To ISO\"}(next:description)[->]");
    d->WorkflowWidget->buttonBoxWidget()->setBackButtonFormat("{backButtonText|\"To IMMO\"}(back:description)[<-]");
  }
}

void qSlicerU70ImageRegistrationModuleWidget::onSetModPosLocalTimeClicked()
{
  Q_D(qSlicerU70ImageRegistrationModuleWidget);
  qint64 currDateTime = QDateTime::currentMSecsSinceEpoch();
  vtkSlicerU70RoomScadaOpcUaLogic* logic = d->scadaOpcUaLogic();
  if (logic)
  {
    bool res = logic->WriteSysTimeModPos(static_cast< uint64_t >(currDateTime));
    if (res)
    {
      qDebug() << Q_FUNC_INFO << "Value is written: " << currDateTime;
    }
  }
}

void qSlicerU70ImageRegistrationModuleWidget::onSetEventStringClicked()
{
  Q_D(qSlicerU70ImageRegistrationModuleWidget);
  QDateTime currDateTime = QDateTime::currentDateTime();
  QString text = currDateTime.toString("[dd.MM.yyyy hh:mm:ss.zzz]: ") + d->lineEdit->text();

  vtkSlicerU70RoomScadaOpcUaLogic* logic = d->scadaOpcUaLogic();
  if (logic)
  {
    bool res = logic->WriteEventString(text.toStdString());
    if (res)
    {
      qDebug() << Q_FUNC_INFO << "Value is written: " << text;
    }
  }
}

void qSlicerU70ImageRegistrationModuleWidget::onSetStateClicked()
{
  Q_D(qSlicerU70ImageRegistrationModuleWidget);
  QDateTime currDateTime = QDateTime::currentDateTime();
  int st = d->SliderWidget_ModPosState->value();

  vtkSlicerU70RoomScadaOpcUaLogic* logic = d->scadaOpcUaLogic();
  if (logic)
  {
    bool res = logic->WriteStateModPos(st);
    if (res)
    {
      qDebug() << Q_FUNC_INFO << "Value is written: " << st;
    }
  }
}
