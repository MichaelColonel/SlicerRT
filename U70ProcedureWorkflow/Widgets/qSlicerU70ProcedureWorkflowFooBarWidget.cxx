/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// FooBar Widgets includes
#include "qSlicerU70ProcedureWorkflowFooBarWidget.h"
#include "ui_qSlicerU70ProcedureWorkflowFooBarWidget.h"

//-----------------------------------------------------------------------------
class qSlicerU70ProcedureWorkflowFooBarWidgetPrivate : public Ui_qSlicerU70ProcedureWorkflowFooBarWidget
{
  Q_DECLARE_PUBLIC(qSlicerU70ProcedureWorkflowFooBarWidget);

protected:
  qSlicerU70ProcedureWorkflowFooBarWidget* const q_ptr;

public:
  qSlicerU70ProcedureWorkflowFooBarWidgetPrivate(qSlicerU70ProcedureWorkflowFooBarWidget& object);
  virtual void setupUi(qSlicerU70ProcedureWorkflowFooBarWidget*);
};

// --------------------------------------------------------------------------
qSlicerU70ProcedureWorkflowFooBarWidgetPrivate::qSlicerU70ProcedureWorkflowFooBarWidgetPrivate(qSlicerU70ProcedureWorkflowFooBarWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerU70ProcedureWorkflowFooBarWidgetPrivate::setupUi(qSlicerU70ProcedureWorkflowFooBarWidget* widget)
{
  this->Ui_qSlicerU70ProcedureWorkflowFooBarWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerU70ProcedureWorkflowFooBarWidget methods

//-----------------------------------------------------------------------------
qSlicerU70ProcedureWorkflowFooBarWidget::qSlicerU70ProcedureWorkflowFooBarWidget(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qSlicerU70ProcedureWorkflowFooBarWidgetPrivate(*this))
{
  Q_D(qSlicerU70ProcedureWorkflowFooBarWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerU70ProcedureWorkflowFooBarWidget ::~qSlicerU70ProcedureWorkflowFooBarWidget() {}
