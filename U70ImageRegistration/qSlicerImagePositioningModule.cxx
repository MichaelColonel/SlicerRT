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

// ImagePositioning Logic includes
#include <vtkSlicerImagePositioningLogic.h>

// ImagePositioning includes
#include "qSlicerImagePositioningModule.h"
#include "qSlicerImagePositioningModuleWidget.h"

//-----------------------------------------------------------------------------
class qSlicerImagePositioningModulePrivate
{
public:
  qSlicerImagePositioningModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerImagePositioningModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerImagePositioningModulePrivate::qSlicerImagePositioningModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerImagePositioningModule methods

//-----------------------------------------------------------------------------
qSlicerImagePositioningModule::qSlicerImagePositioningModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerImagePositioningModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerImagePositioningModule::~qSlicerImagePositioningModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerImagePositioningModule::helpText() const
{
  return "This is a loadable module that can be bundled in an extension";
}

//-----------------------------------------------------------------------------
QString qSlicerImagePositioningModule::acknowledgementText() const
{
  return "This work was partially funded by NIH grant NXNNXXNNNNNN-NNXN";
}

//-----------------------------------------------------------------------------
QStringList qSlicerImagePositioningModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("John Doe (AnyWare Corp.)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerImagePositioningModule::icon() const
{
  return QIcon(":/Icons/ImagePositioning.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerImagePositioningModule::categories() const
{
  return QStringList() << "Radiotherapy";
}

//-----------------------------------------------------------------------------
QStringList qSlicerImagePositioningModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerImagePositioningModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerImagePositioningModule
::createWidgetRepresentation()
{
  return new qSlicerImagePositioningModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerImagePositioningModule::createLogic()
{
  return vtkSlicerImagePositioningLogic::New();
}
