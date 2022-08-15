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

// IhepMlcControl Logic includes
#include <vtkSlicerIhepMlcControlLogic.h>

// IhepMlcControl includes
#include "qSlicerIhepMlcControlModule.h"
#include "qSlicerIhepMlcControlModuleWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerIhepMlcControlModulePrivate
{
public:
  qSlicerIhepMlcControlModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerIhepMlcControlModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerIhepMlcControlModulePrivate::qSlicerIhepMlcControlModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerIhepMlcControlModule methods

//-----------------------------------------------------------------------------
qSlicerIhepMlcControlModule::qSlicerIhepMlcControlModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerIhepMlcControlModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerIhepMlcControlModule::~qSlicerIhepMlcControlModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerIhepMlcControlModule::helpText() const
{
  return "This is a loadable module that can be bundled in an extension";
}

//-----------------------------------------------------------------------------
QString qSlicerIhepMlcControlModule::acknowledgementText() const
{
  return "This work was partially funded by NIH grant NXNNXXNNNNNN-NNXN";
}

//-----------------------------------------------------------------------------
QStringList qSlicerIhepMlcControlModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("John Doe (AnyWare Corp.)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerIhepMlcControlModule::icon() const
{
  return QIcon(":/Icons/IhepMlcControl.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerIhepMlcControlModule::categories() const
{
  return QStringList() << "Radiotherapy";
}

//-----------------------------------------------------------------------------
QStringList qSlicerIhepMlcControlModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerIhepMlcControlModule
::createWidgetRepresentation()
{
  return new qSlicerIhepMlcControlModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerIhepMlcControlModule::createLogic()
{
  return vtkSlicerIhepMlcControlLogic::New();
}
