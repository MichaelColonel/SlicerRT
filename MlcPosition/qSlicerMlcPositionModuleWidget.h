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

#ifndef __qSlicerMlcPositionModuleWidget_h
#define __qSlicerMlcPositionModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerMlcPositionModuleExport.h"

class qSlicerMlcPositionModuleWidgetPrivate;

class vtkMRMLNode;
class vtkMRMLVolumeNode;
class vtkMRMLSegmentationNode;

class vtkSlicerSegmentationsModuleLogic;
class vtkSlicerBeamsModuleLogic;
class vtkSlicerModelsLogic;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class Q_SLICER_QTMODULES_MLCPOSITION_EXPORT qSlicerMlcPositionModuleWidget : public qSlicerAbstractModuleWidget {

  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerMlcPositionModuleWidget(QWidget *parent = nullptr);
  virtual ~qSlicerMlcPositionModuleWidget();
  virtual void enter();
  void setSegmentationsLogic(vtkSlicerSegmentationsModuleLogic*);
  void setBeamsLogic(vtkSlicerBeamsModuleLogic*);
  void setModelsLogic(vtkSlicerModelsLogic*);

public slots:
  /// Reimplemented for internal reasons
  virtual void setMRMLScene(vtkMRMLScene* scene);

  /// RTBeam or RTIonBeam changed
  void onRTBeamNodeChanged(vtkMRMLNode* beamNode); 
  /// Segmentation node (RTSTRUCT contours) changed
  void onSegmentationNodeChanged(vtkMRMLNode* segmentationNode);
  /// Target volume name changed
  void onTargetSegmentChanged(const QString& text);

  /// Process Volume and Segmentation Nodes
  void onCalculateMultiLeafCollimatorPositionButtonClicked();
  void onShowMultiLeafCollimatorModelButtonClicked();

  /// Change visibility of the MRML node ID columns
  void setMRMLIDsVisible(bool visible);

protected:
  QScopedPointer<qSlicerMlcPositionModuleWidgetPrivate> d_ptr;

  virtual void setup();

private:
  Q_DECLARE_PRIVATE(qSlicerMlcPositionModuleWidget);
  Q_DISABLE_COPY(qSlicerMlcPositionModuleWidget);
};

#endif
