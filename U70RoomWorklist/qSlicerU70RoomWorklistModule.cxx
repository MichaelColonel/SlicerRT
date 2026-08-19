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

// U70RoomWorklist Logic includes
#include <vtkSlicerU70RoomWorklistLogic.h>

// U70RoomWorklist includes
#include "qSlicerU70RoomWorklistModule.h"
#include "qSlicerU70RoomWorklistModuleWidget.h"

//-----------------------------------------------------------------------------
class qSlicerU70RoomWorklistModulePrivate
{
public:
  qSlicerU70RoomWorklistModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerU70RoomWorklistModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerU70RoomWorklistModulePrivate::qSlicerU70RoomWorklistModulePrivate() {}

//-----------------------------------------------------------------------------
// qSlicerU70RoomWorklistModule methods

//-----------------------------------------------------------------------------
qSlicerU70RoomWorklistModule::qSlicerU70RoomWorklistModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerU70RoomWorklistModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerU70RoomWorklistModule::~qSlicerU70RoomWorklistModule() {}

//-----------------------------------------------------------------------------
QString qSlicerU70RoomWorklistModule::helpText() const
{
  return "This is a loadable module that can be bundled in an extension";
}

//-----------------------------------------------------------------------------
QString qSlicerU70RoomWorklistModule::acknowledgementText() const
{
  return "This work was partially funded by NIH grant NXNNXXNNNNNN-NNXN";
}

//-----------------------------------------------------------------------------
QStringList qSlicerU70RoomWorklistModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString(tr("Mikhail Polkovnikov"));
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerU70RoomWorklistModule::icon() const
{
  return QIcon(":/Icons/U70RoomWorklist.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerU70RoomWorklistModule::categories() const
{
  return QStringList() << tr("Lutch U-70");
}

//-----------------------------------------------------------------------------
QStringList qSlicerU70RoomWorklistModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomWorklistModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerU70RoomWorklistModule::createWidgetRepresentation()
{
  return new qSlicerU70RoomWorklistModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerU70RoomWorklistModule::createLogic()
{
  return vtkSlicerU70RoomWorklistLogic::New();
}
