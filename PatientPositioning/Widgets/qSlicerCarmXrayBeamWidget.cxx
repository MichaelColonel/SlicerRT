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

// Qt includes
#include <QDebug>

// CarmXrayBeam Widgets includes
#include "qSlicerCarmXrayBeamWidget.h"
#include "ui_qSlicerCarmXrayBeamWidget.h"

#include <qSlicerLayoutManager.h>
#include <qSlicerApplication.h>
#include <qMRMLSliceWidget.h>
#include <qSlicerSubjectHierarchyFolderPlugin.h>
#include <qSlicerSubjectHierarchyPluginHandler.h>
#include <qMRMLThreeDWidget.h>
#include <qMRMLThreeDView.h>

// MRML includes
#include <vtkMRMLChannel26GeometryNode.h>
#include <vtkMRMLDrrImageComputationNode.h>

#include <vtkMRMLRTPlanNode.h>
#include <vtkMRMLRTChannel26Cabin3BeamNode.h>

#include <vtkMRMLScalarVolumeNode.h>


#include <vtkMRMLScene.h>
#include <vtkMRMLLayoutNode.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLLinearTransformNode.h>

// Beams inlcudes
#include <vtkMRMLRTBeamNode.h>

// Logic includes
#include <vtkSlicerPatientPositioningLogic.h>
#include <vtkSlicerDrrImageComputationLogic.h>

#include <vtkMRMLSliceLogic.h>

// VTK includes
#include <vtkVector.h>
#include <vtkTransform.h>

//-----------------------------------------------------------------------------
class qSlicerCarmXrayBeamWidgetPrivate : public Ui_qSlicerCarmXrayBeamWidget
{
  Q_DECLARE_PUBLIC(qSlicerCarmXrayBeamWidget);
protected:
  qSlicerCarmXrayBeamWidget* const q_ptr;

public:
  qSlicerCarmXrayBeamWidgetPrivate(
    qSlicerCarmXrayBeamWidget& object);
  virtual void setupUi(qSlicerCarmXrayBeamWidget*);
  vtkMRMLPatientPositioningNode::CarmProjectionOrientation getCurrentOrientation();

  void init();

  vtkWeakPointer< vtkMRMLPatientPositioningNode > ParameterNode;
  vtkWeakPointer< vtkMRMLChannel26GeometryNode > Channel26GeoNode;
  vtkWeakPointer< vtkSlicerPatientPositioningLogic > PatientPositioningLogic;
};

// --------------------------------------------------------------------------
qSlicerCarmXrayBeamWidgetPrivate::qSlicerCarmXrayBeamWidgetPrivate(
  qSlicerCarmXrayBeamWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerCarmXrayBeamWidgetPrivate
::setupUi(qSlicerCarmXrayBeamWidget* widget)
{
  this->Ui_qSlicerCarmXrayBeamWidget::setupUi(widget);
}

// --------------------------------------------------------------------------
void qSlicerCarmXrayBeamWidgetPrivate::init()
{
  Q_Q(qSlicerCarmXrayBeamWidget);

  QObject::connect( this->PushButton_ComputeCarmXrayDrr, SIGNAL(clicked()), q, SLOT(onComputeDrrClicked()));
  QObject::connect( this->MRMLNodeComboBox_DrrImageNode, SIGNAL(currentNodeChanged(vtkMRMLNode*)), 
    q, SLOT(onDrrImageNodeChanged(vtkMRMLNode*)));
  QObject::connect( this->MRMLNodeComboBox_CarmXrayImageNode, SIGNAL(currentNodeChanged(vtkMRMLNode*)), 
    q, SLOT(onCarmXrayImageNodeChanged(vtkMRMLNode*)));
  QObject::connect( this->CheckBox_ShowRtImageView, SIGNAL(toggled(bool)), q, SLOT(onSetImagesToSliceViewToggled(bool)));
  
  QObject::connect( this->PushButton_TransformCarmRawImage, SIGNAL(clicked()), q, SLOT(onTransformCarmRawImageClicked()));
}

// --------------------------------------------------------------------------
vtkMRMLPatientPositioningNode::CarmProjectionOrientation qSlicerCarmXrayBeamWidgetPrivate::getCurrentOrientation()
{
  Q_Q(qSlicerCarmXrayBeamWidget);
  vtkMRMLPatientPositioningNode::CarmProjectionOrientation projType =
    vtkMRMLPatientPositioningNode::CarmProjectionOrientation_Last;

  if (this->RadioButton_OrientationVertical->isChecked())
  {
    projType = vtkMRMLPatientPositioningNode::ORIENTATION_VERTICAL;
  }
  else if (this->RadioButton_OrientationHorizontal->isChecked())
  {
    projType = vtkMRMLPatientPositioningNode::ORIENTATION_HORIZONTAL;
  }
  else if (this->RadioButton_OrientationAngle->isChecked())
  {
    projType = vtkMRMLPatientPositioningNode::ORIENTATION_ANGLE;
  }
  return projType;
}

//-----------------------------------------------------------------------------
// qSlicerCarmXrayBeamWidget methods

//-----------------------------------------------------------------------------
qSlicerCarmXrayBeamWidget::qSlicerCarmXrayBeamWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerCarmXrayBeamWidgetPrivate(*this) )
{
  Q_D(qSlicerCarmXrayBeamWidget);
  d->setupUi(this);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerCarmXrayBeamWidget::~qSlicerCarmXrayBeamWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerCarmXrayBeamWidget::setParameterNode(vtkMRMLNode* node)
{
  Q_D(qSlicerCarmXrayBeamWidget);

  vtkMRMLPatientPositioningNode* parameterNode = vtkMRMLPatientPositioningNode::SafeDownCast(node);
  // Each time the node is modified, the UI widgets are updated
  qvtkReconnect( d->ParameterNode, parameterNode, vtkCommand::ModifiedEvent, 
    this, SLOT( updateWidgetFromMRML() ) );
  qvtkReconnect( d->Channel26GeoNode, parameterNode->GetChannel26GeometryNode(), vtkCommand::ModifiedEvent, 
    this, SLOT( updateWidgetFromMRML() ) );

  d->ParameterNode = parameterNode;
  if (parameterNode)
  {
    d->Channel26GeoNode = parameterNode->GetChannel26GeometryNode();
  }

  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerCarmXrayBeamWidget::setPatientPositioningLogic(vtkSlicerPatientPositioningLogic* logic)
{
  Q_D(qSlicerCarmXrayBeamWidget);
  d->PatientPositioningLogic = logic;
}

//-----------------------------------------------------------------------------
/*
void qSlicerCarmXrayBeamWidget::setDrrImageComputationNode(vtkMRMLDrrImageComputationNode* node)
{
  Q_D(qSlicerCarmXrayBeamWidget);
  if (!node)
  {
    return;
  }
  d->MRMLNodeComboBox_DrrNode->setCurrentNode(node);
  this->updateWidgetFromMRML();
}
*/
//-----------------------------------------------------------------------------
void qSlicerCarmXrayBeamWidget::onComputeDrrClicked()
{
  Q_D(qSlicerCarmXrayBeamWidget);
  if (!d->PatientPositioningLogic)
  {
    qWarning() << Q_FUNC_INFO << "PatientPositioning logic is invalid";
    return;
  }

  if (!d->ParameterNode)
  {
    qWarning() << Q_FUNC_INFO << "Parameter node is invalid";
    return;
  }

  vtkMRMLScalarVolumeNode* ctInputVolumeNode = nullptr;
  vtkMRMLRTBeamNode* beamNode = d->ParameterNode->GetBeamNode(); // ion beam node
  if (beamNode)
  {
    vtkMRMLRTPlanNode* planNode = beamNode->GetParentPlanNode();
    if (planNode)
    {
      ctInputVolumeNode = planNode->GetReferenceVolumeNode();
    }
  }
//  vtkMRMLScene* scene = d->PatientPositioningLogic->GetMRMLScene();
  vtkMRMLNode* node = d->MRMLNodeComboBox_DrrNode->currentNode();
  vtkMRMLDrrImageComputationNode* drrNode = nullptr;
  if (!node)
  {
    qCritical() << Q_FUNC_INFO << "DRR image computation node is invalid";
    return;
  }
  if (node)
  {
    drrNode = vtkMRMLDrrImageComputationNode::SafeDownCast(node);
    if (drrNode && ctInputVolumeNode)
    {
      vtkMRMLRTBeamNode* carmXrayBeamNode = d->ParameterNode->GetCarmXrayBeamNode();
      drrNode->SetAndObserveBeamNode(carmXrayBeamNode);
    }
  }

  if (drrNode && ctInputVolumeNode)
  {
    // compute DRR image here
    vtkSlicerDrrImageComputationLogic* drrLogic = d->PatientPositioningLogic->GetDrrImageComputationLogic();
    drrLogic->UpdateMarkupsNodes(drrNode);
    drrLogic->UpdateNormalAndVupVectors(drrNode);
    QApplication::setOverrideCursor(Qt::WaitCursor);

    vtkMRMLScalarVolumeNode* drrImageNode = drrLogic->ComputePlastimatchDRR( drrNode, ctInputVolumeNode, true);
    if (drrImageNode)
    {
      // node is OK
      QApplication::restoreOverrideCursor();
      return;
    }
    QApplication::restoreOverrideCursor();
  }
}

//-----------------------------------------------------------------------------
void qSlicerCarmXrayBeamWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerCarmXrayBeamWidget);

  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  if (!d->PatientPositioningLogic)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid PatientPositioning logic";
    return;
  }
  vtkMRMLDrrImageComputationNode* drrNode = d->ParameterNode->GetDrrComputationNode();
  if (drrNode)
  {
    d->MRMLNodeComboBox_DrrNode->setCurrentNode(drrNode);
    int res[2] = { -1, -1 };
    drrNode->GetImagerResolution(res);
    double spacing[2] = { -0.1, -0.1 };
    drrNode->GetImagerSpacing(spacing);
    double sad = drrNode->GetBeamNode()->GetSAD();
    double isoImagerDistance = drrNode->GetIsocenterImagerDistance();
    d->Label_DrrResolution->setText(tr("%1 x %2").arg(res[0]).arg(res[1]));
    d->Label_DrrSpacing->setText(tr("%1 x %2").arg(spacing[0]).arg(spacing[1]));
    d->Label_DrrSAD->setText(tr("%1").arg(sad));
    d->Label_DrrSID->setText(tr("%1").arg(sad + isoImagerDistance));
  }
  else
  {
    d->Label_DrrResolution->setText("");
    d->Label_DrrSpacing->setText("");
    d->Label_DrrSAD->setText("");
    d->Label_DrrSID->setText("");
  }
  vtkMRMLPatientPositioningNode::CarmProjectionOrientation proj = d->getCurrentOrientation();
  vtkTransform* registrationTransform = d->ParameterNode->GetRegistrationTransform(proj);
  vtkMRMLLinearTransformNode* transformNode = d->PatientPositioningLogic->GetDefaultRegistrationTransformNode();
  if (transformNode)
  {
    transformNode->SetAndObserveTransformToParent(registrationTransform);
    d->MRMLTransformSliders_RegistrationTranslate->setMRMLTransformNode(transformNode);
  }
}

//-----------------------------------------------------------------------------
void qSlicerCarmXrayBeamWidget::onSetImagesToSliceViewToggled(bool maximize)
{
  Q_D(qSlicerCarmXrayBeamWidget);

  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  vtkMRMLPatientPositioningNode::CarmProjectionOrientation projType = d->getCurrentOrientation();

  vtkMRMLScalarVolumeNode* carmDrrImageNode = vtkMRMLScalarVolumeNode::SafeDownCast(
    d->MRMLNodeComboBox_DrrImageNode->currentNode()); // moved image
  vtkMRMLScalarVolumeNode* carmXrayImageNode = vtkMRMLScalarVolumeNode::SafeDownCast(
    d->MRMLNodeComboBox_CarmXrayImageNode->currentNode()); // static image

  // Manage "Red" slice for DRR
  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  qMRMLSliceWidget* sliceWidget = slicerApplication->layoutManager()->sliceWidget("Red");

  if (!sliceWidget)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid slice widget";
    return;
  }

  if (!carmDrrImageNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid DRR image node";
    return;
  }

  bool isMaximized = false;
  bool canBeMaximized = false;
  vtkMRMLSliceNode* sliceNode = sliceWidget->mrmlSliceNode();
  vtkMRMLLayoutNode* layoutNode = sliceNode->GetMaximizedState(isMaximized, canBeMaximized);

  // When toggled, set DRR image as background and maximize slice
  if (maximize && layoutNode)
  {
    vtkMRMLSliceLogic* sliceLogic = sliceWidget->sliceLogic();
    sliceLogic->GetSliceCompositeNode()->SetBackgroundVolumeID(carmDrrImageNode->GetID());
    sliceLogic->RotateSliceToLowestVolumeAxes(); // Reformat

    sliceLogic->FitSliceToAll();
    sliceNode->UpdateMatrices();
    if (canBeMaximized && !isMaximized)
    {
      layoutNode->AddMaximizedViewNode(sliceNode);
    }
  }
  else if (!maximize && layoutNode && isMaximized) // When released, restore view layout
  {
    layoutNode->RemoveMaximizedViewNode(sliceNode);
  }

  emit registrationRtImagePairChanged(projType, carmXrayImageNode, carmDrrImageNode);
}

//-----------------------------------------------------------------------------
void qSlicerCarmXrayBeamWidget::onDrrImageNodeChanged(vtkMRMLNode* drrNode)
{
  Q_D(qSlicerCarmXrayBeamWidget);

  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  vtkMRMLPatientPositioningNode::CarmProjectionOrientation proj = d->getCurrentOrientation();
  vtkMRMLScalarVolumeNode* drrImageNode = vtkMRMLScalarVolumeNode::SafeDownCast(drrNode);
  if (drrImageNode)
  {
    qDebug() << Q_FUNC_INFO << "C-arm x-ray image is valid: " << drrImageNode->GetName();
    d->ParameterNode->SetRegistrationImages(proj, nullptr, drrImageNode);
  }
}

//-----------------------------------------------------------------------------
void qSlicerCarmXrayBeamWidget::onCarmXrayImageNodeChanged(vtkMRMLNode* xrayNode)
{
  Q_D(qSlicerCarmXrayBeamWidget);
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  vtkMRMLPatientPositioningNode::CarmProjectionOrientation proj = d->getCurrentOrientation();
  vtkMRMLScalarVolumeNode* xrayImageNode = vtkMRMLScalarVolumeNode::SafeDownCast(xrayNode);
  if (xrayImageNode)
  {
    qDebug() << Q_FUNC_INFO << "C-arm x-ray image is valid: " << xrayImageNode->GetName();
    d->ParameterNode->SetRegistrationImages(proj, xrayImageNode, nullptr);
  }
}

//-----------------------------------------------------------------------------
void qSlicerCarmXrayBeamWidget::onTransformCarmRawImageClicked()
{
  Q_D(qSlicerCarmXrayBeamWidget);
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  vtkMRMLNode* node = d->MRMLNodeComboBox_CarmRawVolume->currentNode();
  vtkMRMLScalarVolumeNode* imageNode = vtkMRMLScalarVolumeNode::SafeDownCast(node);
  if (!imageNode)
  {
    return;
  }
  d->ParameterNode->GetDrrComputationNode()->SetAndObserveRtImageVolumeNode(imageNode);
  if (d->PatientPositioningLogic->ApplyCarmXrayDetectorTransformToXrayImage(d->ParameterNode, imageNode))
  {
    d->MRMLNodeComboBox_CarmXrayImageNode->setCurrentNode(imageNode);
  }
}

//-----------------------------------------------------------------------------
void qSlicerCarmXrayBeamWidget::onMoveUpClicked()
{
  Q_D(qSlicerCarmXrayBeamWidget);
}

//-----------------------------------------------------------------------------
void qSlicerCarmXrayBeamWidget::onMoveDownClicked()
{
  Q_D(qSlicerCarmXrayBeamWidget);
}

//-----------------------------------------------------------------------------
void qSlicerCarmXrayBeamWidget::onMoveLeftClicked()
{
  Q_D(qSlicerCarmXrayBeamWidget);
}

//-----------------------------------------------------------------------------
void qSlicerCarmXrayBeamWidget::onMoveRightClicked()
{
  Q_D(qSlicerCarmXrayBeamWidget);
}
