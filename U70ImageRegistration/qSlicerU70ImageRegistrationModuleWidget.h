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

#ifndef __qSlicerU70ImageRegistrationModuleWidget_h
#define __qSlicerU70ImageRegistrationModuleWidget_h

// Slicer includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerU70ImageRegistrationModuleExport.h"

class qSlicerU70ImageRegistrationModuleWidgetPrivate;
class vtkMRMLNode;

class Q_SLICER_QTMODULES_U70IMAGEREGISTRATION_EXPORT qSlicerU70ImageRegistrationModuleWidget : public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:
  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerU70ImageRegistrationModuleWidget(QWidget* parent = 0);
  virtual ~qSlicerU70ImageRegistrationModuleWidget();

public slots:

protected:
  QScopedPointer<qSlicerU70ImageRegistrationModuleWidgetPrivate> d_ptr;

  void setup() override;

private:
  Q_DECLARE_PRIVATE(qSlicerU70ImageRegistrationModuleWidget);
  Q_DISABLE_COPY(qSlicerU70ImageRegistrationModuleWidget);
};

#endif
