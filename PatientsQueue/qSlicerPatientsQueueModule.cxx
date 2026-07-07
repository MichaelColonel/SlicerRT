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

// PatientsQueue Logic includes
#include <vtkSlicerPatientsQueueLogic.h>

// PatientsQueue includes
#include "qSlicerPatientsQueueModule.h"
#include "qSlicerPatientsQueueModuleWidget.h"

//-----------------------------------------------------------------------------
class qSlicerPatientsQueueModulePrivate
{
public:
  qSlicerPatientsQueueModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerPatientsQueueModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerPatientsQueueModulePrivate::qSlicerPatientsQueueModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerPatientsQueueModule methods

//-----------------------------------------------------------------------------
qSlicerPatientsQueueModule::qSlicerPatientsQueueModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerPatientsQueueModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerPatientsQueueModule::~qSlicerPatientsQueueModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerPatientsQueueModule::helpText() const
{
  return "This is a loadable module that can be bundled in an extension";
}

//-----------------------------------------------------------------------------
QString qSlicerPatientsQueueModule::acknowledgementText() const
{
  return "This work was partially funded by NIH grant NXNNXXNNNNNN-NNXN";
}

//-----------------------------------------------------------------------------
QStringList qSlicerPatientsQueueModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("John Doe (AnyWare Corp.)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerPatientsQueueModule::icon() const
{
  return QIcon(":/Icons/PatientsQueue.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerPatientsQueueModule::categories() const
{
  return QStringList() << "Examples";
}

//-----------------------------------------------------------------------------
QStringList qSlicerPatientsQueueModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerPatientsQueueModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerPatientsQueueModule
::createWidgetRepresentation()
{
  return new qSlicerPatientsQueueModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerPatientsQueueModule::createLogic()
{
  return vtkSlicerPatientsQueueLogic::New();
}
