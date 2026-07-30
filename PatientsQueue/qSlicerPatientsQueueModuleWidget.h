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

#ifndef __qSlicerPatientsQueueModuleWidget_h
#define __qSlicerPatientsQueueModuleWidget_h

// Slicer includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerPatientsQueueModuleExport.h"

class qSlicerPatientsQueueModuleWidgetPrivate;
class vtkMRMLNode;

class Q_SLICER_QTMODULES_PATIENTSQUEUE_EXPORT qSlicerPatientsQueueModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerPatientsQueueModuleWidget(QWidget *parent=0);
  virtual ~qSlicerPatientsQueueModuleWidget();

public slots:
  void onSetCustomLayoutClicked();
  void onCheckConnectionClicked();

protected:
  QScopedPointer<qSlicerPatientsQueueModuleWidgetPrivate> d_ptr;

  void setup() override;

private:
  Q_DECLARE_PRIVATE(qSlicerPatientsQueueModuleWidget);
  Q_DISABLE_COPY(qSlicerPatientsQueueModuleWidget);
};

#endif
