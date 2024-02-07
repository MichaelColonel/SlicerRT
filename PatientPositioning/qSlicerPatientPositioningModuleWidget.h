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

class qSlicerPatientPositioningModuleWidgetPrivate;
class vtkMRMLNode;

class Q_SLICER_QTMODULES_PATIENTPOSITIONING_EXPORT qSlicerPatientPositioningModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerPatientPositioningModuleWidget(QWidget *parent=0);
  virtual ~qSlicerPatientPositioningModuleWidget();

public slots:


protected:
  QScopedPointer<qSlicerPatientPositioningModuleWidgetPrivate> d_ptr;

  void setup() override;

private:
  Q_DECLARE_PRIVATE(qSlicerPatientPositioningModuleWidget);
  Q_DISABLE_COPY(qSlicerPatientPositioningModuleWidget);
};

#endif
