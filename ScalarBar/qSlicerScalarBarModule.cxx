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

// ScalarBar Logic includes
#include <vtkSlicerScalarBarLogic.h>

// ScalarBar includes
#include "qSlicerScalarBarModule.h"
#include "qSlicerScalarBarModuleWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerScalarBarModulePrivate
{
public:
  qSlicerScalarBarModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerScalarBarModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerScalarBarModulePrivate::qSlicerScalarBarModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerScalarBarModule methods

//-----------------------------------------------------------------------------
qSlicerScalarBarModule::qSlicerScalarBarModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerScalarBarModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerScalarBarModule::~qSlicerScalarBarModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerScalarBarModule::helpText() const
{
  return "This is a loadable module that can be bundled in an extension";
}

//-----------------------------------------------------------------------------
QString qSlicerScalarBarModule::acknowledgementText() const
{
  return "This work was partially funded by NIH grant NXNNXXNNNNNN-NNXN";
}

//-----------------------------------------------------------------------------
QStringList qSlicerScalarBarModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("John Doe (AnyWare Corp.)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerScalarBarModule::icon() const
{
  return QIcon(":/Icons/ScalarBar.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerScalarBarModule::categories() const
{
  return QStringList() << "Examples";
}

//-----------------------------------------------------------------------------
QStringList qSlicerScalarBarModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerScalarBarModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerScalarBarModule
::createWidgetRepresentation()
{
  return new qSlicerScalarBarModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerScalarBarModule::createLogic()
{
  return vtkSlicerScalarBarLogic::New();
}
