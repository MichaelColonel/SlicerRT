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

#ifndef __qSlicerPlmDrrModuleWidget_h
#define __qSlicerPlmDrrModuleWidget_h

// Qt includes
#include <QProcess>

// SlicerQt includes
#include <qSlicerAbstractModuleWidget.h>

#include "qSlicerPlmDrrModuleExport.h"

class qSlicerPlmDrrModuleWidgetPrivate;
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class Q_SLICER_QTMODULES_PLMDRR_EXPORT qSlicerPlmDrrModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerPlmDrrModuleWidget(QWidget *parent=0);
  ~qSlicerPlmDrrModuleWidget() override;

  void exit() override;
  void enter() override;

public slots:
  void setMRMLScene(vtkMRMLScene*) override;
  void setParameterNode(vtkMRMLNode*);
  void onSceneImportedEvent();
  void onSceneClosedEvent();

  void onRTBeamNodeChanged(vtkMRMLNode*);
  void onParameterNodeChanged(vtkMRMLNode*);
  void onReferenceVolumeNodeChanged(vtkMRMLNode*);

  /// Update widget GUI from drr parameters node
  void updateWidgetFromMRML();

protected slots:
  void onLogicModified();
  void onIsocenterImagerDistanceValueChanged(double);
  void onImagerCenterOffsetCoordinatesChanged(double*);
  void onImageWindowCoordinatesChanged(double*);
  void onImageSpacingChanged(double*);
  void onImageDimentionChanged(double*);
  void onComputeDrrClicked();
  void onUpdateImageWindowFromBeamJaws();
  void onRotateAroundNormal(double);
  void onUpdatePlmDrrArgs();
  void onPlatimatchDrrProcessStarted();
  void onPlatimatchDrrProcessFinished( int, QProcess::ExitStatus);
  void onSelectPlastimatchAppPathClicked();
  void onShowMarkupsToggled(bool);
  void onUseImageWindowToggled(bool);
  void onUseExponentialMappingToggled(bool);
  void onReconstructionAlgorithmChanged(int);
  void onThreadingChanged(int);
  void onHUConversionChanged(int);

protected:
  QScopedPointer<qSlicerPlmDrrModuleWidgetPrivate> d_ptr;

  void setup() override;
  void onEnter();

private:
  Q_DECLARE_PRIVATE(qSlicerPlmDrrModuleWidget);
  Q_DISABLE_COPY(qSlicerPlmDrrModuleWidget);
};

#endif