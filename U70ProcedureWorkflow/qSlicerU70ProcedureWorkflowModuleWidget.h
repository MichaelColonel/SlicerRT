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

#ifndef __qSlicerU70ProcedureWorkflowModuleWidget_h
#define __qSlicerU70ProcedureWorkflowModuleWidget_h

// Slicer includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerU70ProcedureWorkflowModuleExport.h"

class qSlicerU70ProcedureWorkflowModuleWidgetPrivate;
class vtkMRMLNode;

class Q_SLICER_QTMODULES_U70PROCEDUREWORKFLOW_EXPORT qSlicerU70ProcedureWorkflowModuleWidget : public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:
  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerU70ProcedureWorkflowModuleWidget(QWidget* parent = 0);
  virtual ~qSlicerU70ProcedureWorkflowModuleWidget();

public slots:

protected:
  QScopedPointer<qSlicerU70ProcedureWorkflowModuleWidgetPrivate> d_ptr;

  void setup() override;

private:
  Q_DECLARE_PRIVATE(qSlicerU70ProcedureWorkflowModuleWidget);
  Q_DISABLE_COPY(qSlicerU70ProcedureWorkflowModuleWidget);
};

#endif
