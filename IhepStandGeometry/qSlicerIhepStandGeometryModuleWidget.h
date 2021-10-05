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

#ifndef __qSlicerIhepStandGeometryModuleWidget_h
#define __qSlicerIhepStandGeometryModuleWidget_h

// Slicer includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerIhepStandGeometryModuleExport.h"

class qSlicerIhepStandGeometryModuleWidgetPrivate;
class vtkMRMLNode;
class vtkMRMLCameraNode;

/// \ingroup Slicer_QtModules_IhepStandGeometry
class Q_SLICER_QTMODULES_IHEPSTANDGEOMETRY_EXPORT qSlicerIhepStandGeometryModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerIhepStandGeometryModuleWidget(QWidget *parent=0);
  ~qSlicerIhepStandGeometryModuleWidget() override;

  void exit() override;
  void enter() override;

public slots:
  void setMRMLScene(vtkMRMLScene*) override;
  void setParameterNode(vtkMRMLNode*);
  void onSceneImportedEvent();
  void onSceneClosedEvent();
  void onParameterNodeChanged(vtkMRMLNode*);
  void onRTBeamNodeChanged(vtkMRMLNode*);
  void onReferenceVolumeNodeChanged(vtkMRMLNode*);
  void onPatientBodySegmentationNodeChanged(vtkMRMLNode*);
  void onPatientBodySegmentNameChanged(const QString&);
  void onLogicModified();
  void onBeamsEyeViewPlusXButtonClicked();
  void onBeamsEyeViewMinusXButtonClicked();
  void onBeamsEyeViewPlusYButtonClicked();
  void onBeamsEyeViewMinusYButtonClicked();

  void onLoadStandModelsButtonClicked();
  void onResetToInitialPositionButtonClicked();
//  void onTableTopVerticalPositionChanged(double);
//  void onTableTopVerticalMirrorPositionChanged(double);
  void onTableTopOriginPositionChanged(double);
//  void onTableTopVerticalMiddlePositionChanged(double);
  void onTableTopPlatformLongitudinalPositionChanged(double);
  void onTableTopStandLateralPositionChanged(double);
  void onPatientSupportFixedReferenceAngleChanged(double);

  void onPatientTableTopTranslationChanged(double*);

  /// Update widget GUI from RT Image parameters node
  void updateWidgetFromMRML();
  /// Update fixed reference camera in 3D view
  void updateFixedReferenceCamera(bool update = true);

protected:
  QScopedPointer<qSlicerIhepStandGeometryModuleWidgetPrivate> d_ptr;

  void setup() override;
  void onEnter();

private:
  void onBeamsEyeViewButtonClicked(const double viewUpVector[4]);
  vtkMRMLCameraNode* Get3DViewCameraNode();

  Q_DECLARE_PRIVATE(qSlicerIhepStandGeometryModuleWidget);
  Q_DISABLE_COPY(qSlicerIhepStandGeometryModuleWidget);
};

#endif
