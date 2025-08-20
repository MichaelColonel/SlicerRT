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

// Beams inlcudes
#include <vtkMRMLRTBeamNode.h>

// Logic includes
#include <vtkSlicerPatientPositioningLogic.h>
#include <vtkSlicerDrrImageComputationLogic.h>

// VTK includes
#include <vtkVector.h>

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
  vtkMRMLPatientPositioningNode::OrientationRtImagePairMap RtImagePairMap;
  std::map< vtkMRMLPatientPositioningNode::CarmProjectionOrientation,
    std::array< double, 3 > > RtImagePairOffsetMap;
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
    q, SLOT(onDrrNodeChanged(vtkMRMLNode*)));
  QObject::connect( this->MRMLNodeComboBox_CarmXrayImageNode, SIGNAL(currentNodeChanged(vtkMRMLNode*)), 
    q, SLOT(onCarmXrayImageNodeChanged(vtkMRMLNode*)));
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
  vtkMRMLNode* node = d->MRMLNodeComboBox_DrrNode->currentNode();
  vtkMRMLDrrImageComputationNode* drrNode = vtkMRMLDrrImageComputationNode::SafeDownCast(node);
  if (drrNode)
  {
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
}

//-----------------------------------------------------------------------------
void qSlicerCarmXrayBeamWidget::onSetImagesToSliceViewClicked()
{
  Q_D(qSlicerCarmXrayBeamWidget);

  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  vtkMRMLPatientPositioningNode::CarmProjectionOrientation projType = vtkMRMLPatientPositioningNode::CarmProjectionOrientation_Last;

  if (d->RadioButton_OrientationVertical->isChecked())
  {
    projType = vtkMRMLPatientPositioningNode::ORIENTATION_VERTICAL;
  }
  else if (d->RadioButton_OrientationHorizontal->isChecked())
  {
    projType = vtkMRMLPatientPositioningNode::ORIENTATION_HORIZONTAL;
  }
  else if (d->RadioButton_OrientationAngle->isChecked())
  {
    projType = vtkMRMLPatientPositioningNode::ORIENTATION_ANGLE;
  }
  emit registrationRtImagePairChanged(projType, nullptr, nullptr);
}

//-----------------------------------------------------------------------------
void qSlicerCarmXrayBeamWidget::onDrrNodeChanged(vtkMRMLNode* drrNode)
{
  Q_D(qSlicerCarmXrayBeamWidget);

  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  vtkMRMLPatientPositioningNode::CarmProjectionOrientation proj = d->getCurrentOrientation();

  vtkMRMLPatientPositioningNode::RtImagePair& pair = d->RtImagePairMap[proj];
  pair.first = vtkMRMLScalarVolumeNode::SafeDownCast(drrNode);
}

//-----------------------------------------------------------------------------
void qSlicerCarmXrayBeamWidget::onCarmXrayImageNodeChanged(vtkMRMLNode* xrayImageNode)
{
  Q_D(qSlicerCarmXrayBeamWidget);
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  vtkMRMLPatientPositioningNode::CarmProjectionOrientation proj = d->getCurrentOrientation();

  vtkMRMLPatientPositioningNode::RtImagePair& pair = d->RtImagePairMap[proj];
  pair.second = vtkMRMLScalarVolumeNode::SafeDownCast(xrayImageNode);
}
