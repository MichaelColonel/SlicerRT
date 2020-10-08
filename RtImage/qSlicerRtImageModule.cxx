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

// RtImage Logic includes
#include <vtkSlicerRtImageLogic.h>

// RtImage includes
#include "qSlicerRtImageModule.h"
#include "qSlicerRtImageModuleWidget.h"

// SlicerQt includes
#include <qSlicerCoreApplication.h>
#include <qSlicerModuleManager.h>

// SlicerRT includes
#include <vtkSlicerPlanarImageModuleLogic.h>
#include <vtkSlicerBeamsModuleLogic.h>

// Slicer includes
#include <vtkSlicerCLIModuleLogic.h>

// Qt includes
#include <QDebug>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_RtImage
class qSlicerRtImageModulePrivate
{
public:
  qSlicerRtImageModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerRtImageModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerRtImageModulePrivate::qSlicerRtImageModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerRtImageModule methods

//-----------------------------------------------------------------------------
qSlicerRtImageModule::qSlicerRtImageModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerRtImageModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerRtImageModule::~qSlicerRtImageModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerRtImageModule::helpText() const
{
  return "This is a loadable module that calculated a digitally " \
  "reconstructed radiograph (DRR) using the plastimatch reconstruct library.";
}

//-----------------------------------------------------------------------------
QString qSlicerRtImageModule::acknowledgementText() const
{
  return "This work was supported by Slicer Community.";
}

//-----------------------------------------------------------------------------
QStringList qSlicerRtImageModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("Mikhail Polkovnikov (NRC \"Kurchatov Institute\" - IHEP)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerRtImageModule::icon() const
{
  return QIcon(":/Icons/RtImage.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerRtImageModule::categories() const
{
  return QStringList() << "Radiotherapy";
}

//-----------------------------------------------------------------------------
QStringList qSlicerRtImageModule::dependencies() const
{
  return QStringList() << "Beams" << "PlanarImage" << "plastimatch_slicer_drr";
}

//-----------------------------------------------------------------------------
void qSlicerRtImageModule::setup()
{
  this->Superclass::setup();

  vtkSlicerRtImageLogic* rtImageLogic = vtkSlicerRtImageLogic::SafeDownCast(this->logic());

  // Set planar image logic to the logic
  qSlicerAbstractCoreModule* planarImageModule = qSlicerCoreApplication::application()->moduleManager()->module("PlanarImage");
  if (planarImageModule && rtImageLogic)
  {
    vtkSlicerPlanarImageModuleLogic* planarImageLogic = vtkSlicerPlanarImageModuleLogic::SafeDownCast(planarImageModule->logic());
    rtImageLogic->SetPlanarImageLogic(planarImageLogic);
  }
  else
  {
    qCritical() << Q_FUNC_INFO << ": Planar Image module is not found";
  }

  // Set plastimatch DRR computation logic to the logic
  qSlicerAbstractCoreModule* plastimatchDrrModule = qSlicerCoreApplication::application()->moduleManager()->module("plastimatch_slicer_drr");
  if (plastimatchDrrModule && rtImageLogic)
  {
    vtkSlicerCLIModuleLogic* plastimatchDrrLogic = vtkSlicerCLIModuleLogic::SafeDownCast(plastimatchDrrModule->logic());
    rtImageLogic->SetDRRComputationLogic(plastimatchDrrLogic);
  }
  else
  {
    qCritical() << Q_FUNC_INFO << ": Plastimatch DRR module is not found";
  }

  // Set beams logic to the logic
  qSlicerAbstractCoreModule* beamsModule = qSlicerCoreApplication::application()->moduleManager()->module("Beams");
  if (beamsModule && rtImageLogic)
  {
    vtkSlicerBeamsModuleLogic* beamsLogic = vtkSlicerBeamsModuleLogic::SafeDownCast(beamsModule->logic());
    rtImageLogic->SetBeamsLogic(beamsLogic);
  }
  else
  {
    qCritical() << Q_FUNC_INFO << ": Beams module is not found";
  }
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerRtImageModule
::createWidgetRepresentation()
{
  return new qSlicerRtImageModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerRtImageModule::createLogic()
{
  return vtkSlicerRtImageLogic::New();
}
