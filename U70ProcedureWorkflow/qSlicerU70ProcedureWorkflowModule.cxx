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

// U70ProcedureWorkflow Logic includes
#include <vtkSlicerU70ProcedureWorkflowLogic.h>

// U70ProcedureWorkflow includes
#include "qSlicerU70ProcedureWorkflowModule.h"
#include "qSlicerU70ProcedureWorkflowModuleWidget.h"

//-----------------------------------------------------------------------------
class qSlicerU70ProcedureWorkflowModulePrivate
{
public:
  qSlicerU70ProcedureWorkflowModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerU70ProcedureWorkflowModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerU70ProcedureWorkflowModulePrivate::qSlicerU70ProcedureWorkflowModulePrivate() {}

//-----------------------------------------------------------------------------
// qSlicerU70ProcedureWorkflowModule methods

//-----------------------------------------------------------------------------
qSlicerU70ProcedureWorkflowModule::qSlicerU70ProcedureWorkflowModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerU70ProcedureWorkflowModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerU70ProcedureWorkflowModule::~qSlicerU70ProcedureWorkflowModule() {}

//-----------------------------------------------------------------------------
QString qSlicerU70ProcedureWorkflowModule::helpText() const
{
  return "This is a loadable module that can be bundled in an extension";
}

//-----------------------------------------------------------------------------
QString qSlicerU70ProcedureWorkflowModule::acknowledgementText() const
{
  return "This work was partially funded by NIH grant NXNNXXNNNNNN-NNXN";
}

//-----------------------------------------------------------------------------
QStringList qSlicerU70ProcedureWorkflowModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("John Doe (AnyWare Corp.)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerU70ProcedureWorkflowModule::icon() const
{
  return QIcon(":/Icons/U70ProcedureWorkflow.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerU70ProcedureWorkflowModule::categories() const
{
  return QStringList() << "Examples";
}

//-----------------------------------------------------------------------------
QStringList qSlicerU70ProcedureWorkflowModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerU70ProcedureWorkflowModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerU70ProcedureWorkflowModule::createWidgetRepresentation()
{
  return new qSlicerU70ProcedureWorkflowModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerU70ProcedureWorkflowModule::createLogic()
{
  return vtkSlicerU70ProcedureWorkflowLogic::New();
}
