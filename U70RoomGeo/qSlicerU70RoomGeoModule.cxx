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

// U70RoomGeo Logic includes
#include <vtkSlicerU70RoomGeoLogic.h>

// U70RoomGeo includes
#include "qSlicerU70RoomGeoModule.h"
#include "qSlicerU70RoomGeoModuleWidget.h"

//-----------------------------------------------------------------------------
class qSlicerU70RoomGeoModulePrivate
{
public:
  qSlicerU70RoomGeoModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerU70RoomGeoModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerU70RoomGeoModulePrivate::qSlicerU70RoomGeoModulePrivate() {}

//-----------------------------------------------------------------------------
// qSlicerU70RoomGeoModule methods

//-----------------------------------------------------------------------------
qSlicerU70RoomGeoModule::qSlicerU70RoomGeoModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerU70RoomGeoModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerU70RoomGeoModule::~qSlicerU70RoomGeoModule() {}

//-----------------------------------------------------------------------------
QString qSlicerU70RoomGeoModule::helpText() const
{
  return tr("Geometry of the treatment rooms: Channel-26 (Cabin-1 and Cabin-2), Channe-26A (Cabin-3).");
}

//-----------------------------------------------------------------------------
QString qSlicerU70RoomGeoModule::acknowledgementText() const
{
  return tr("This work was partially funded by NIH grant NXNNXXNNNNNN-NNXN");
}

//-----------------------------------------------------------------------------
QStringList qSlicerU70RoomGeoModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString(tr("Mikhail Polkovnikov"));
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerU70RoomGeoModule::icon() const
{
  return QIcon(":/Icons/U70RoomGeo.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerU70RoomGeoModule::categories() const
{
  return QStringList() << tr("Lutch U-70");
}

//-----------------------------------------------------------------------------
QStringList qSlicerU70RoomGeoModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomGeoModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerU70RoomGeoModule::createWidgetRepresentation()
{
  return new qSlicerU70RoomGeoModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerU70RoomGeoModule::createLogic()
{
  return vtkSlicerU70RoomGeoLogic::New();
}
