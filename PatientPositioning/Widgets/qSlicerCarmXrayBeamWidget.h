/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerCarmXrayBeamWidget_h
#define __qSlicerCarmXrayBeamWidget_h

// Qt includes
#include <qSlicerWidget.h>

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// CarmXrayBeam Widgets includes
#include "qSlicerPatientPositioningModuleWidgetsExport.h"

// MRML includes
#include <vtkMRMLPatientPositioningNode.h>

class vtkMRMLNode;
class vtkSlicerPatientPositioningLogic;
class vtkMRMLDrrImageComputationNode;
class vtkMRMLScalarVolumeNode;

class qSlicerCarmXrayBeamWidgetPrivate;

class QAbstractButton;

class Q_SLICER_MODULE_PATIENTPOSITIONING_WIDGETS_EXPORT qSlicerCarmXrayBeamWidget
  : public qSlicerWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef qSlicerWidget Superclass;
  qSlicerCarmXrayBeamWidget(QWidget *parent=0);
  ~qSlicerCarmXrayBeamWidget() override;

public slots:
  /// Set PatientPositioning MRML node (Parameter node)
  void setParameterNode(vtkMRMLNode* node);
  /// Set PatientPositioning logic
  void setPatientPositioningLogic(vtkSlicerPatientPositioningLogic* logic);
  /// Update widget GUI from PatientPositioning parameters node
  void updateWidgetFromMRML();

  void onComputeDrrClicked();
  void onSetImagesToSliceViewToggled(bool);
  void onDrrImageNodeChanged(vtkMRMLNode* drrNode);
  void onCarmXrayImageNodeChanged(vtkMRMLNode* xrayImageNode);
  void onTransformCarmRawImageClicked();
  void onMoveUpClicked();
  void onMoveDownClicked();
  void onMoveLeftClicked();
  void onMoveRightClicked();

signals:
  void registrationRtImagePairChanged(vtkMRMLPatientPositioningNode::CarmProjectionOrientation,
    vtkMRMLScalarVolumeNode* drrImage, vtkMRMLScalarVolumeNode* carmXrayRtImage);
  void registrationRtImagePairOffsetChanged(vtkMRMLPatientPositioningNode::CarmProjectionOrientation,
    double offsetX, double offsetY, double offsetZ);

protected:
  QScopedPointer<qSlicerCarmXrayBeamWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerCarmXrayBeamWidget);
  Q_DISABLE_COPY(qSlicerCarmXrayBeamWidget);
};

#endif
