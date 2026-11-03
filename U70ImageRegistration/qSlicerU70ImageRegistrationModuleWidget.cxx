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

// U70ImageRegistration widgets includes
#include "ctkExampleDerivedWorkflowWidgetStep.h"
#include "ChooseModeWidget.h"
#include "MoveToImmoWidget.h"
#include "MoveToIsoWidget.h"

//-----------------------------------------------------------------------------
class qSlicerU70ImageRegistrationModuleWidgetPrivate : public Ui_qSlicerU70ImageRegistrationModuleWidget
{
public:
  qSlicerU70ImageRegistrationModuleWidgetPrivate();
  ctkWorkflow* Workflow{ nullptr };
};

//-----------------------------------------------------------------------------
// qSlicerU70ImageRegistrationModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerU70ImageRegistrationModuleWidgetPrivate::qSlicerU70ImageRegistrationModuleWidgetPrivate() {}

//-----------------------------------------------------------------------------
// qSlicerU70ImageRegistrationModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerU70ImageRegistrationModuleWidget::qSlicerU70ImageRegistrationModuleWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerU70ImageRegistrationModuleWidgetPrivate)
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
