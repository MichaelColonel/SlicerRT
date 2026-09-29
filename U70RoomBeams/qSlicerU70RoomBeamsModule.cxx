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

// U70RoomBeams Logic includes
#include <vtkSlicerU70RoomBeamsLogic.h>

// U70RoomBeams includes
#include "qSlicerU70RoomBeamsModule.h"
#include "qSlicerU70RoomBeamsModuleWidget.h"

//-----------------------------------------------------------------------------
class qSlicerU70RoomBeamsModulePrivate
{
public:
  qSlicerU70RoomBeamsModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerU70RoomBeamsModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerU70RoomBeamsModulePrivate::qSlicerU70RoomBeamsModulePrivate() {}

//-----------------------------------------------------------------------------
// qSlicerU70RoomBeamsModule methods

//-----------------------------------------------------------------------------
qSlicerU70RoomBeamsModule::qSlicerU70RoomBeamsModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerU70RoomBeamsModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerU70RoomBeamsModule::~qSlicerU70RoomBeamsModule() {}

//-----------------------------------------------------------------------------
QString qSlicerU70RoomBeamsModule::helpText() const
{
  return "This is a loadable module that can be bundled in an extension";
}

//-----------------------------------------------------------------------------
QString qSlicerU70RoomBeamsModule::acknowledgementText() const
{
  return "This work was partially funded by NIH grant NXNNXXNNNNNN-NNXN";
}

//-----------------------------------------------------------------------------
QStringList qSlicerU70RoomBeamsModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("John Doe (AnyWare Corp.)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerU70RoomBeamsModule::icon() const
{
  return QIcon(":/Icons/U70RoomBeams.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerU70RoomBeamsModule::categories() const
{
  return QStringList() << "Examples";
}

//-----------------------------------------------------------------------------
QStringList qSlicerU70RoomBeamsModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomBeamsModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerU70RoomBeamsModule::createWidgetRepresentation()
{
  return new qSlicerU70RoomBeamsModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerU70RoomBeamsModule::createLogic()
{
  return vtkSlicerU70RoomBeamsLogic::New();
}
