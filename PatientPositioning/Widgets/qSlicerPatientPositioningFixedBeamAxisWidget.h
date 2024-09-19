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

#ifndef __qSlicerPatientPositioningFixedBeamAxisWidget_h
#define __qSlicerPatientPositioningFixedBeamAxisWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// FixedBeamAxis Widgets includes
#include "qSlicerPatientPositioningModuleWidgetsExport.h"

class vtkMRMLNode;
class vtkSlicerPatientPositioningLogic;

class qSlicerPatientPositioningFixedBeamAxisWidgetPrivate;

class Q_SLICER_MODULE_PATIENTPOSITIONING_WIDGETS_EXPORT qSlicerPatientPositioningFixedBeamAxisWidget
  : public QWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef QWidget Superclass;
  qSlicerPatientPositioningFixedBeamAxisWidget(QWidget *parent=0);
  ~qSlicerPatientPositioningFixedBeamAxisWidget() override;

public slots:
  /// Set PatientPositioning MRML node (Parameter node)
  void setParameterNode(vtkMRMLNode* node);
  /// Set PatientPositioning logic
  void setPatientPositioningLogic(vtkSlicerPatientPositioningLogic* logic);

  /// Update widget GUI from RT Image parameters node
  void updateWidgetFromMRML();

  void onButtonUpClicked();
  void onButtonDownClicked();
  void onButtonLeftClicked();
  void onButtonRightClicked();

protected slots:

protected:
  QScopedPointer<qSlicerPatientPositioningFixedBeamAxisWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerPatientPositioningFixedBeamAxisWidget);
  Q_DISABLE_COPY(qSlicerPatientPositioningFixedBeamAxisWidget);
};

#endif
