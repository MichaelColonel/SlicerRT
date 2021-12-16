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

// Qt includes
#include <QDebug>

// IhepStandGeometry Logic includes
#include <vtkSlicerIhepStandGeometryLogic.h>

// SlicerQt includes
#include <qSlicerCoreApplication.h>
#include <qSlicerModuleManager.h>

// SlicerRT includes
#include <vtkSlicerBeamsModuleLogic.h>

// StandGeo includes
#include "qSlicerIhepStandGeometryModule.h"
#include "qSlicerIhepStandGeometryModuleWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_IhepStandGeometry
class qSlicerIhepStandGeometryModulePrivate
{
public:
  qSlicerIhepStandGeometryModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerIhepStandGeometryModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerIhepStandGeometryModulePrivate::qSlicerIhepStandGeometryModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerIhepStandGeometryModule methods

//-----------------------------------------------------------------------------
qSlicerIhepStandGeometryModule::qSlicerIhepStandGeometryModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerIhepStandGeometryModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerIhepStandGeometryModule::~qSlicerIhepStandGeometryModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerIhepStandGeometryModule::helpText() const
{
  return "This module displays and handles IHEP Channel25 stand geometry models created from the loaded isocenter and source fiducials.";
}

//-----------------------------------------------------------------------------
QString qSlicerIhepStandGeometryModule::acknowledgementText() const
{
  return "This work was supported by Slicer Community.";
}

//-----------------------------------------------------------------------------
QStringList qSlicerIhepStandGeometryModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("Mikhail Polkovnikov (NRC \"Kurchatov Institute\" - IHEP)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerIhepStandGeometryModule::icon() const
{
  return QIcon(":/Icons/IhepStandGeometry.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerIhepStandGeometryModule::categories() const
{
  return QStringList() << "Radiotherapy";
}

//-----------------------------------------------------------------------------
QStringList qSlicerIhepStandGeometryModule::dependencies() const
{
  return QStringList() << "Beams";
}

//-----------------------------------------------------------------------------
void qSlicerIhepStandGeometryModule::setup()
{
  this->Superclass::setup();
 
  vtkSlicerIhepStandGeometryLogic* standGeometryLogic = vtkSlicerIhepStandGeometryLogic::SafeDownCast(this->logic());

  // Set beams logic to the logic
  qSlicerAbstractCoreModule* beamsModule = qSlicerCoreApplication::application()->moduleManager()->module("Beams");
  if (beamsModule && standGeometryLogic)
  {
    vtkSlicerBeamsModuleLogic* beamsLogic = vtkSlicerBeamsModuleLogic::SafeDownCast(beamsModule->logic());
//    standGeometryLogic->SetBeamsLogic(beamsLogic);
  }
  else
  {
    qCritical() << Q_FUNC_INFO << ": Beams module is not found";
  }
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerIhepStandGeometryModule
::createWidgetRepresentation()
{
  return new qSlicerIhepStandGeometryModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerIhepStandGeometryModule::createLogic()
{
  return vtkSlicerIhepStandGeometryLogic::New();
}
