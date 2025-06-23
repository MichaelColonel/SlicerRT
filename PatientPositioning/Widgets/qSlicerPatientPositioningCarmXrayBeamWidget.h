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

#ifndef __qSlicerPatientPositioningCarmXrayBeamWidget_h
#define __qSlicerPatientPositioningCarmXrayBeamWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// CarmXrayBeam Widgets includes
#include "qSlicerPatientPositioningModuleWidgetsExport.h"

class vtkMRMLNode;
class vtkSlicerPatientPositioningLogic;

class qSlicerPatientPositioningCarmXrayBeamWidgetPrivate;

class QAbstractButton;

class Q_SLICER_MODULE_PATIENTPOSITIONING_WIDGETS_EXPORT qSlicerPatientPositioningCarmXrayBeamWidget
  : public QWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef QWidget Superclass;
  qSlicerPatientPositioningCarmXrayBeamWidget(QWidget *parent=0);
  ~qSlicerPatientPositioningCarmXrayBeamWidget() override;

public slots:
  /// Set PatientPositioning MRML node (Parameter node)
  void setParameterNode(vtkMRMLNode* node);
  /// Set PatientPositioning logic
  void setPatientPositioningLogic(vtkSlicerPatientPositioningLogic* logic);
  /// Update widget GUI from RT Image parameters node
  void updateWidgetFromMRML();

  void onBeamsEyeViewPlusXButtonClicked();
  void onBeamsEyeViewMinusXButtonClicked();
  void onBeamsEyeViewPlusYButtonClicked();
  void onBeamsEyeViewMinusYButtonClicked();
  void onComputeDrrClicked();

signals:
  void bevOrientationChanged(const std::array< double, 3 >& vup);

protected slots:

protected:
  QScopedPointer<qSlicerPatientPositioningCarmXrayBeamWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerPatientPositioningCarmXrayBeamWidget);
  Q_DISABLE_COPY(qSlicerPatientPositioningCarmXrayBeamWidget);
};

#endif
