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

// Slicer includes
#include "qSlicerU70ProcedureWorkflowModuleWidget.h"
#include "ui_qSlicerU70ProcedureWorkflowModuleWidget.h"

//-----------------------------------------------------------------------------
class qSlicerU70ProcedureWorkflowModuleWidgetPrivate : public Ui_qSlicerU70ProcedureWorkflowModuleWidget
{
public:
  qSlicerU70ProcedureWorkflowModuleWidgetPrivate();
};

//-----------------------------------------------------------------------------
// qSlicerU70ProcedureWorkflowModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerU70ProcedureWorkflowModuleWidgetPrivate::qSlicerU70ProcedureWorkflowModuleWidgetPrivate() {}

//-----------------------------------------------------------------------------
// qSlicerU70ProcedureWorkflowModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerU70ProcedureWorkflowModuleWidget::qSlicerU70ProcedureWorkflowModuleWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerU70ProcedureWorkflowModuleWidgetPrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerU70ProcedureWorkflowModuleWidget::~qSlicerU70ProcedureWorkflowModuleWidget() {}

//-----------------------------------------------------------------------------
void qSlicerU70ProcedureWorkflowModuleWidget::setup()
{
  Q_D(qSlicerU70ProcedureWorkflowModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
}
