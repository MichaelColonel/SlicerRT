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

// PatientPositioning Logic includes
#include <vtkSlicerPatientPositioningLogic.h>

// PatientPositioning includes
#include "qSlicerPatientPositioningModule.h"
#include "qSlicerPatientPositioningModuleWidget.h"

// Slicer includes
#include "qSlicerApplication.h"
#include "qSlicerModuleManager.h"

// Modules logic includes
#include <vtkSlicerDrrImageComputationLogic.h>
#include <vtkSlicerPlanarImageModuleLogic.h>

#include <QDebug>

//-----------------------------------------------------------------------------
class qSlicerPatientPositioningModulePrivate
{
public:
  qSlicerPatientPositioningModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerPatientPositioningModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerPatientPositioningModulePrivate::qSlicerPatientPositioningModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerPatientPositioningModule methods

//-----------------------------------------------------------------------------
qSlicerPatientPositioningModule::qSlicerPatientPositioningModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerPatientPositioningModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerPatientPositioningModule::~qSlicerPatientPositioningModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerPatientPositioningModule::helpText() const
{
  return "This is a loadable module that can be bundled in an extension";
}

//-----------------------------------------------------------------------------
QString qSlicerPatientPositioningModule::acknowledgementText() const
{
  return "This work was partially funded by NIH grant NXNNXXNNNNNN-NNXN";
}

//-----------------------------------------------------------------------------
QStringList qSlicerPatientPositioningModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("John Doe (AnyWare Corp.)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerPatientPositioningModule::icon() const
{
  return QIcon(":/Icons/PatientPositioning.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerPatientPositioningModule::categories() const
{
  return QStringList() << "Radiotherapy";
}

//-----------------------------------------------------------------------------
QStringList qSlicerPatientPositioningModule::dependencies() const
{
  return QStringList() << "Beams" << "DrrImageComputation" << "PlanarImage";
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModule::setup()
{
  this->Superclass::setup();

  vtkSlicerPatientPositioningLogic* patPosLogic = vtkSlicerPatientPositioningLogic::SafeDownCast(this->logic());

  // Set planar image logic to the logic
  qSlicerAbstractCoreModule* planarImageModule = qSlicerCoreApplication::application()->moduleManager()->module("PlanarImage");
  if (planarImageModule && patPosLogic)
  {
    vtkSlicerPlanarImageModuleLogic* planarImageLogic = vtkSlicerPlanarImageModuleLogic::SafeDownCast(planarImageModule->logic());
    patPosLogic->SetPlanarImageLogic(planarImageLogic);
  }
  else
  {
    qCritical() << Q_FUNC_INFO << ": \"Planar Image\" module is not found";
  }

  // Set DRR image computation logic to the logic
  qSlicerAbstractCoreModule* drrImageComputationModule = qSlicerCoreApplication::application()->moduleManager()->module("DrrImageComputation");
  if (drrImageComputationModule && patPosLogic)
  {
    vtkSlicerDrrImageComputationLogic* drrImageComputationLogic = vtkSlicerDrrImageComputationLogic::SafeDownCast(drrImageComputationModule->logic());
    patPosLogic->SetDrrImageComputationLogic(drrImageComputationLogic);
  }
  else
  {
    qCritical() << Q_FUNC_INFO << ": \"DRR Image Computation\" module is not found";
  }
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerPatientPositioningModule
::createWidgetRepresentation()
{
  return new qSlicerPatientPositioningModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerPatientPositioningModule::createLogic()
{
  return vtkSlicerPatientPositioningLogic::New();
}
