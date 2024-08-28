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

#ifndef __qSlicerPatientPositioningModuleWidget_h
#define __qSlicerPatientPositioningModuleWidget_h

// Slicer includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerPatientPositioningModuleExport.h"

#include <vtkMRMLPatientPositioningNode.h>

class qSlicerPatientPositioningModuleWidgetPrivate;
class vtkMRMLNode;

class QAbstractButton;

class Q_SLICER_QTMODULES_PATIENTPOSITIONING_EXPORT qSlicerPatientPositioningModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerPatientPositioningModuleWidget(QWidget *parent=0);
  virtual ~qSlicerPatientPositioningModuleWidget();

  void exit() override;
  void enter() override;

public slots:
  void setMRMLScene(vtkMRMLScene*) override;
  void setParameterNode(vtkMRMLNode*);
  void onSceneImportedEvent();
  void onSceneClosedEvent();

  void onParameterNodeChanged(vtkMRMLNode* parameterNode);
  void onLoadTreatmentMachineButtonClicked();
  void onPatientSupportRotationAngleChanged(double angle);
  void onPatientTableTopTranslationChanged(double* position);
  void onBaseFixedToFixedReferenceTranslationChanged(double* position);
  void onTableTopRobotAnglesChanged(double* a);
  void onTableTopRobotA1Changed(double a1);
  void onTableTopRobotA2Changed(double a2);
  void onTableTopRobotA3Changed(double a3);
  void onTableTopRobotA4Changed(double a4);
  void onTableTopRobotA5Changed(double a5);
  void onTableTopRobotA6Changed(double a6);
  void onRotatePatientHeadFeetToggled(bool toggled);

  /// Update widget GUI from PatientPositioning parameters node
  void updateWidgetFromMRML();

protected:
  QScopedPointer<qSlicerPatientPositioningModuleWidgetPrivate> d_ptr;

  void setup() override;
  void onEnter();

private:
  Q_DECLARE_PRIVATE(qSlicerPatientPositioningModuleWidget);
  Q_DISABLE_COPY(qSlicerPatientPositioningModuleWidget);
};

#endif
