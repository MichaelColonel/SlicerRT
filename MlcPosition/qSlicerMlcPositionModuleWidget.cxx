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

// SlicerQt includes
#include "qSlicerMlcPositionModuleWidget.h"
#include "ui_qSlicerMlcPositionModuleWidget.h"

#include <qSlicerApplication.h>
#include <qSlicerLayoutManager.h>
#include <qSlicerIOManager.h>

// Segmentations Module Logic includes
#include <vtkSlicerSegmentationsModuleLogic.h>

// Beams Module Logic includes
#include <vtkSlicerBeamsModuleLogic.h>
#include <vtkSlicerIECTransformLogic.h>
#include <vtkMRMLRTBeamNode.h>
#include <vtkMRMLRTIonBeamNode.h>
#include <vtkMRMLRTPlanNode.h>

// Slicer Models includes
#include <vtkSlicerModelsLogic.h>

// Logic includes
#include "vtkSlicerMlcPositionLogic.h"

// MRMLWidgets includes
#include <qMRMLSceneModel.h>
#include <qMRMLSliceWidget.h>
#include <qMRMLSliceView.h>
#include <qMRMLNodeComboBox.h>

// MRML includes
#include <vtkMRMLVolumeNode.h>
#include <vtkMRMLScalarVolumeNode.h> // image volume data (images)
#include <vtkMRMLSegmentationNode.h> // RTSTRUCT sermentation data
#include <vtkMRMLLabelMapVolumeNode.h> // Label Map
#include <vtkMRMLMarkupsCurveNode.h>
#include <vtkMRMLSubjectHierarchyNode.h>
#include <vtkMRMLLinearTransformNode.h> // beam transformation
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkSegmentation.h>
#include <vtkOrientedImageData.h>
#include <vtkGeneralTransform.h>
#include <vtkTransform.h>
#include <vtkCallbackCommand.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRendererCollection.h>
#include <vtkRenderer.h>
#include <vtkImageMapper3D.h>
#include <vtkImageProperty.h>
#include <vtkImageActor.h>
#include <vtkImageMapToWindowLevelColors.h>
#include <vtkLookupTable.h>

// Qt includes
#include <QAction>
#include <QDebug>
#include <QSettings>
#include <QTimer>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerMlcPositionModuleWidgetPrivate : public Ui_qSlicerMlcPositionModuleWidget {
  Q_DECLARE_PUBLIC(qSlicerMlcPositionModuleWidget);
protected:
  qSlicerMlcPositionModuleWidget* const q_ptr;
public:
  qSlicerMlcPositionModuleWidgetPrivate(qSlicerMlcPositionModuleWidget& object);
  virtual ~qSlicerMlcPositionModuleWidgetPrivate();
  vtkSlicerMlcPositionLogic* logic() const;

public:
  vtkMRMLSegmentationNode* StructureSetSegmentationNode;
  vtkMRMLRTBeamNode* BeamNode;
  QString m_TargetVolumeName;
};

//-----------------------------------------------------------------------------
// qSlicerMlcPositionModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerMlcPositionModuleWidgetPrivate::qSlicerMlcPositionModuleWidgetPrivate(qSlicerMlcPositionModuleWidget &object)
  :
  q_ptr(&object),
  StructureSetSegmentationNode(nullptr),
  BeamNode(nullptr)
{
}

//-----------------------------------------------------------------------------
qSlicerMlcPositionModuleWidgetPrivate::~qSlicerMlcPositionModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
vtkSlicerMlcPositionLogic*
qSlicerMlcPositionModuleWidgetPrivate::logic() const
{
  Q_Q(const qSlicerMlcPositionModuleWidget);
  return vtkSlicerMlcPositionLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
// qSlicerMlcPositionModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerMlcPositionModuleWidget::qSlicerMlcPositionModuleWidget(QWidget* parent)
  :
  Superclass(parent),
  d_ptr(new qSlicerMlcPositionModuleWidgetPrivate(*this))
{
  // Q_D(qSlicerMlcPositionModuleWidget);
}

//-----------------------------------------------------------------------------
qSlicerMlcPositionModuleWidget::~qSlicerMlcPositionModuleWidget()
{
}

//-----------------------------------------------------------------------------
void
qSlicerMlcPositionModuleWidget::setup()
{
  Q_D(qSlicerMlcPositionModuleWidget);
  this->Superclass::setup();

  d->setupUi(this);

  QStringList rtBeamNodes;
  rtBeamNodes.push_back("vtkMRMLRTBeamNode");
  d->RTBeamNodeComboBox->setNodeTypes(rtBeamNodes);

  // Make connections for scene
//  connect( this, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), 
//    d->SubjectHierarchyTreeView, SLOT(setMRMLScene(vtkMRMLScene*)));

  // Make connections for Combo Box
  connect( d->TargetVolumeSegmentSelectorWidget, SIGNAL(currentNodeChanged(vtkMRMLNode*)), 
    this, SLOT(onSegmentationNodeChanged(vtkMRMLNode*)));
  connect( d->TargetVolumeSegmentSelectorWidget, SIGNAL(currentSegmentChanged(QString)), 
    this, SLOT(onTargetSegmentChanged(QString)));    
  connect( d->RTBeamNodeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), 
    this, SLOT(onRTBeamNodeChanged(vtkMRMLNode*)));

  connect( d->CalculateMlcPositionPushButton, SIGNAL(clicked()),
    this, SLOT(onCalculateMultiLeafCollimatorPositionButtonClicked()));
  connect( d->ShowMlcModelPushButton, SIGNAL(clicked()),
    this, SLOT(onShowMultiLeafCollimatorModelButtonClicked()));

  qDebug() << Q_FUNC_INFO;
}

//-----------------------------------------------------------------------------
void
qSlicerMlcPositionModuleWidget::enter()
{
  Q_D(qSlicerMlcPositionModuleWidget);
  this->Superclass::enter();

  qDebug() << Q_FUNC_INFO;
}

//-----------------------------------------------------------------------------
void
qSlicerMlcPositionModuleWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qSlicerMlcPositionModuleWidget);
  this->Superclass::setMRMLScene(scene);
  
//  this->setMRMLIDsVisible(d->SubjectHierarchyDisplayDataNodeIDsCheckBox->isChecked());

  qDebug() << Q_FUNC_INFO;
}

void
qSlicerMlcPositionModuleWidget::onCalculateMultiLeafCollimatorPositionButtonClicked()
{
  Q_D(qSlicerMlcPositionModuleWidget);
  if (!d->StructureSetSegmentationNode || d->m_TargetVolumeName.isEmpty())
  {
    return;
  }

  std::string targetId = d->m_TargetVolumeName.toStdString();

  if (d->logic() && !targetId.empty())
  {
    vtkPolyData* targetPoly = d->StructureSetSegmentationNode->GetClosedSurfaceInternalRepresentation(targetId);
    if (targetPoly)
    {
      vtkMRMLMarkupsCurveNode* convexHullCurve = d->logic()->CalculatePositionConvexHullCurve( d->BeamNode, targetPoly);
      if (convexHullCurve)
      {
        vtkMRMLTransformNode* beamTransformNode = d->BeamNode->GetParentTransformNode();
        
        vtkMRMLTableNode* mlcTableNode = d->logic()->CreateMultiLeafCollimatorTableNodeBoundaryData();

        if (mlcTableNode && d->logic()->CalculateMultiLeafCollimatorPosition( mlcTableNode, convexHullCurve))
        {
          d->BeamNode->SetAndObserveMultiLeafCollimatorTableNode(mlcTableNode);
          d->logic()->SetParentForMultiLeafCollimatorTableNode(d->BeamNode);

          double area = d->logic()->CalculateMultiLeafCollimatorPositionArea(d->BeamNode);
          qDebug() << Q_FUNC_INFO << ": Position area (mm^2) = " << area;

          area = d->logic()->CalculateCurvePolygonArea(convexHullCurve);
          qDebug() << Q_FUNC_INFO << ": Polygon area (mm^2) = " << area;

          d->logic()->SetParentForMultiLeafCollimatorCurve( d->BeamNode, convexHullCurve);

          convexHullCurve->SetAndObserveTransformNodeID(beamTransformNode->GetID());
        }
/*
        vtkMRMLTableNode* mlcPositionNode = moduleLogic->CalculateMultiLeafCollimatorPosition( d->BeamNode, positionCurve);
        if (moduleLogic->CalculateMultiLeafCollimatorPosition( d->BeamNode, positionCurve);)
        {
          d->BeamNode->SetAndObserveMLCPositionTableNode(mlcPositionNode);
        }

        double area = moduleLogic->CalculateMultiLeafCollimatorPositionArea(d->BeamNode);
        qDebug() << Q_FUNC_INFO << ": Position area (mm^2) = " << area;

        area = moduleLogic->CalculateCurvePolygonArea(positionCurve);
        qDebug() << Q_FUNC_INFO << ": Polygon area (mm^2) = " << area;
        moduleLogic->SetParentForMultiLeafCollimatorPosition(d->BeamNode);        

//        positionCurve->ApplyTransform(beamTransformNode->GetTransformToParent());
        moduleLogic->SetParentForMultiLeafCollimatorCurve( d->BeamNode, positionCurve);

        positionCurve->SetAndObserveTransformNodeID(beamTransformNode->GetID());
*/
      }
    }
  }
}

void
qSlicerMlcPositionModuleWidget::onShowMultiLeafCollimatorModelButtonClicked()
{
  Q_D(qSlicerMlcPositionModuleWidget);
/*
  vtkSlicerLoadableModuleTemplateLogic* moduleLogic = vtkSlicerLoadableModuleTemplateLogic::SafeDownCast(this->logic());

  std::string targetId = d->m_TargetVolumeName.toStdString();

  if (moduleLogic && !targetId.empty())
  {
//    vtkSlicerLoadableModuleTemplateLogic* moduleLogic = vtkSlicerLoadableModuleTemplateLogic::SafeDownCast(this->logic());
//    moduleLogic->SetModelsLogic(d->ModelsLogic);
    moduleLogic->CreateMultiLeafCollimatorModelPolyData(d->BeamNode);
    double area = moduleLogic->CalculateMultiLeafCollimatorPositionArea(d->BeamNode);
    qDebug() << Q_FUNC_INFO << ": Position area (mm^2) = " << area;
  }
  */
}

//-----------------------------------------------------------------------------
void
qSlicerMlcPositionModuleWidget::setMRMLIDsVisible(bool visible)
{
  Q_D(qSlicerMlcPositionModuleWidget);

  // Subject hierarchy view
//  d->SubjectHierarchyTreeView->setColumnHidden( d->SubjectHierarchyTreeView->model()->idColumn(), !visible);

  qDebug() << Q_FUNC_INFO << ": state = " << visible;
}

/// RTPlan Node changed
/*
void
qSlicerMlcPositionModuleWidget::onRTPlanNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerMlcPositionModuleWidget);
  vtkMRMLRTPlanNode* planNode = vtkMRMLRTPlanNode::SafeDownCast(node);
  if (planNode)
  {
    d->ReferenceVolumeNodeComboBox->setCurrentNode(planNode->GetReferenceVolumeNode());
    d->TargetVolumeSegmentSelectorWidget->setCurrentNode(planNode->GetSegmentationNode());
    if (vtkMRMLNode* beamNode = planNode->GetBeamByNumber(0))
    {
      d->RTBeamNodeComboBox->setCurrentNode(beamNode);
    }
  }
}
*/
/// RTBeam Node (RTBeam or RTIonBeam) changed
void
qSlicerMlcPositionModuleWidget::onRTBeamNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerMlcPositionModuleWidget);
  vtkMRMLRTBeamNode* beamNode = vtkMRMLRTBeamNode::SafeDownCast(node);
  vtkMRMLRTIonBeamNode* ionBeamNode = vtkMRMLRTIonBeamNode::SafeDownCast(node);
  d->BeamNode = beamNode;
  Q_UNUSED(ionBeamNode);
}

/// Segmentation node changed
void
qSlicerMlcPositionModuleWidget::onSegmentationNodeChanged(vtkMRMLNode* segmentationNode)
{
  Q_D(qSlicerMlcPositionModuleWidget);
  d->StructureSetSegmentationNode = vtkMRMLSegmentationNode::SafeDownCast(segmentationNode);
}

/// Segmentation node changed
/*
void
qSlicerMlcPositionModuleWidget::onVolumeNodeChanged(vtkMRMLNode* volumeNode)
{
  Q_D(qSlicerMlcPositionModuleWidget);
  d->ReferenceVolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(volumeNode);
}
*/
void
qSlicerMlcPositionModuleWidget::setSegmentationsLogic(vtkSlicerSegmentationsModuleLogic* logic)
{
  Q_D(qSlicerMlcPositionModuleWidget);
  if (logic)
  {
    qDebug() << "Segmentations logic is valid";
//    d->SegmentationsLogic = logic;
  }
  else
  {
    qCritical() << "Segmentations logic is invalid";
//    d->SegmentationsLogic = nullptr;
  }
}

void
qSlicerMlcPositionModuleWidget::setBeamsLogic(vtkSlicerBeamsModuleLogic* logic)
{
  Q_D(qSlicerMlcPositionModuleWidget);
  if (logic)
  {
    qDebug() << "Beams logic is valid";
//    d->BeamsLogic = logic;
  }
  else
  {
    qCritical() << "Beams logic is invalid";
//    d->BeamsLogic = nullptr;
  }
}

void
qSlicerMlcPositionModuleWidget::setModelsLogic(vtkSlicerModelsLogic* logic)
{
  Q_D(qSlicerMlcPositionModuleWidget);
  if (logic)
  {
    qDebug() << "Models logic is valid";
//    d->ModelsLogic = logic;
  }
  else
  {
    qCritical() << "Models logic is invalid";
//    d->ModelsLogic = nullptr;
  }
}

void
qSlicerMlcPositionModuleWidget::onTargetSegmentChanged(const QString& text)
{
  Q_D(qSlicerMlcPositionModuleWidget);
  d->m_TargetVolumeName = text;
}

