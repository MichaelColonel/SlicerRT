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

#ifndef __qSlicerTestMe2ModuleWidget_h
#define __qSlicerTestMe2ModuleWidget_h

// Slicer includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerTestMe2ModuleExport.h"

class qSlicerTestMe2ModuleWidgetPrivate;
class vtkMRMLNode;

class Q_SLICER_QTMODULES_TESTME2_EXPORT qSlicerTestMe2ModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerTestMe2ModuleWidget(QWidget *parent=0);
  virtual ~qSlicerTestMe2ModuleWidget();

public slots:
  void onFiducialNodeChanged(vtkMRMLNode*);
  void onTransformNodeChanged(vtkMRMLNode*);
  void onCheckNodesButtonClicked();

protected:
  QScopedPointer<qSlicerTestMe2ModuleWidgetPrivate> d_ptr;

  void setup() override;

private:
  Q_DECLARE_PRIVATE(qSlicerTestMe2ModuleWidget);
  Q_DISABLE_COPY(qSlicerTestMe2ModuleWidget);
};

#endif
