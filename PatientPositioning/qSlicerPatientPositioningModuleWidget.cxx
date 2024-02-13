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

// Qt includes
#include <QDebug>

// Slicer includes
#include "qSlicerPatientPositioningModuleWidget.h"
#include "ui_qSlicerPatientPositioningModuleWidget.h"

//-----------------------------------------------------------------------------
class qSlicerPatientPositioningModuleWidgetPrivate: public Ui_qSlicerPatientPositioningModuleWidget
{
public:
  qSlicerPatientPositioningModuleWidgetPrivate();
};

//-----------------------------------------------------------------------------
// qSlicerPatientPositioningModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerPatientPositioningModuleWidgetPrivate::qSlicerPatientPositioningModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerPatientPositioningModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerPatientPositioningModuleWidget::qSlicerPatientPositioningModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerPatientPositioningModuleWidgetPrivate )
{
}

//-----------------------------------------------------------------------------
qSlicerPatientPositioningModuleWidget::~qSlicerPatientPositioningModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::setup()
{
  Q_D(qSlicerPatientPositioningModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  connect( d->PushButton_SetImagesToSliceView, SIGNAL(clicked()), this, SLOT(onSetImagesToSliceViewClicked()));
  connect( d->MRMLNodeComboBox_DRR, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(onDrrNodeChanged(vtkMRMLNode*)));
  connect( d->MRMLNodeComboBox_Xray, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(onXrayImageNodeChanged(vtkMRMLNode*)));
  connect( d->ButtonGroup_XrayProjection, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(onXrayProjectionButtonGroupChanged(QAbstractButton*)));
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onSetImagesToSliceViewClicked()
{
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onDrrNodeChanged(vtkMRMLNode*)
{
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onXrayImageNodeChanged(vtkMRMLNode*)
{
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningModuleWidget::onXrayProjectionButtonGroupChanged(QAbstractButton* but)
{
  Q_D(qSlicerPatientPositioningModuleWidget);

  QRadioButton* rButton = qobject_cast<QRadioButton*>(but);
  if (rButton == d->RadioButton_Vertical)
  {
    qDebug() << Q_FUNC_INFO << ": Vertical";
  }
  else if (rButton == d->RadioButton_Horizontal)
  {
    qDebug() << Q_FUNC_INFO << ": Horizontal";
  }
  else if (rButton == d->RadioButton_Angle)
  {
    qDebug() << Q_FUNC_INFO << ": Angle";
  }
}
