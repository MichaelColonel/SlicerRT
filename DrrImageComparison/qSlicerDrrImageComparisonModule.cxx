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

// DrrImageComparison Logic includes
#include <vtkSlicerDrrImageComparisonLogic.h>

// DrrImageComparison includes
#include "qSlicerDrrImageComparisonModule.h"
#include "qSlicerDrrImageComparisonModuleWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerDrrImageComparisonModulePrivate
{
public:
  qSlicerDrrImageComparisonModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerDrrImageComparisonModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerDrrImageComparisonModulePrivate::qSlicerDrrImageComparisonModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerDrrImageComparisonModule methods

//-----------------------------------------------------------------------------
qSlicerDrrImageComparisonModule::qSlicerDrrImageComparisonModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerDrrImageComparisonModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerDrrImageComparisonModule::~qSlicerDrrImageComparisonModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerDrrImageComparisonModule::helpText() const
{
  return "This is a loadable module that can be bundled in an extension";
}

//-----------------------------------------------------------------------------
QString qSlicerDrrImageComparisonModule::acknowledgementText() const
{
  return "This work was partially funded by NIH grant NXNNXXNNNNNN-NNXN";
}

//-----------------------------------------------------------------------------
QStringList qSlicerDrrImageComparisonModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("John Doe (AnyWare Corp.)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerDrrImageComparisonModule::icon() const
{
  return QIcon(":/Icons/DrrImageComparison.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerDrrImageComparisonModule::categories() const
{
  return QStringList() << "Radiotherapy";
}

//-----------------------------------------------------------------------------
QStringList qSlicerDrrImageComparisonModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerDrrImageComparisonModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerDrrImageComparisonModule
::createWidgetRepresentation()
{
  return new qSlicerDrrImageComparisonModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerDrrImageComparisonModule::createLogic()
{
  return vtkSlicerDrrImageComparisonLogic::New();
}
