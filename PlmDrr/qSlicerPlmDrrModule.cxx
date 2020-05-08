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

// PlmDrr Logic includes
#include <vtkSlicerPlmDrrLogic.h>

// PlmDrr includes
#include "qSlicerPlmDrrModule.h"
#include "qSlicerPlmDrrModuleWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerPlmDrrModulePrivate
{
public:
  qSlicerPlmDrrModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerPlmDrrModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerPlmDrrModulePrivate::qSlicerPlmDrrModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerPlmDrrModule methods

//-----------------------------------------------------------------------------
qSlicerPlmDrrModule::qSlicerPlmDrrModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerPlmDrrModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerPlmDrrModule::~qSlicerPlmDrrModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerPlmDrrModule::helpText() const
{
  return "This is a loadable module that can be bundled in an extension";
}

//-----------------------------------------------------------------------------
QString qSlicerPlmDrrModule::acknowledgementText() const
{
  return "This work was partially funded by NIH grant NXNNXXNNNNNN-NNXN";
}

//-----------------------------------------------------------------------------
QStringList qSlicerPlmDrrModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("John Doe (AnyWare Corp.)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerPlmDrrModule::icon() const
{
  return QIcon(":/Icons/PlmDrr.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerPlmDrrModule::categories() const
{
  return QStringList() << "Radiotherapy";
}

//-----------------------------------------------------------------------------
QStringList qSlicerPlmDrrModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerPlmDrrModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerPlmDrrModule
::createWidgetRepresentation()
{
  return new qSlicerPlmDrrModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerPlmDrrModule::createLogic()
{
  return vtkSlicerPlmDrrLogic::New();
}
