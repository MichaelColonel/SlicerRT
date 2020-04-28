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
#include "qSlicerLoadableModuleTemplateModuleWidget.h"
#include "ui_qSlicerLoadableModuleTemplateModuleWidget.h"

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
#include "vtkSlicerLoadableModuleTemplateLogic.h"

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
class qSlicerLoadableModuleTemplateModuleWidgetPrivate : public Ui_qSlicerLoadableModuleTemplateModuleWidget {
  Q_DECLARE_PUBLIC(qSlicerLoadableModuleTemplateModuleWidget);
protected:
  qSlicerLoadableModuleTemplateModuleWidget* const q_ptr;
public:
  qSlicerLoadableModuleTemplateModuleWidgetPrivate(qSlicerLoadableModuleTemplateModuleWidget& object);
  virtual ~qSlicerLoadableModuleTemplateModuleWidgetPrivate();
  vtkSlicerLoadableModuleTemplateLogic* logic() const;

public:
  vtkMRMLVolumeNode* ReferenceVolumeNode;
  vtkMRMLSegmentationNode* StructureSetSegmentationNode;
  vtkSlicerSegmentationsModuleLogic* SegmentationsLogic;
  vtkSlicerBeamsModuleLogic* BeamsLogic;
  vtkSlicerModelsLogic* ModelsLogic;
  vtkMRMLRTBeamNode* BeamNode;
  QString m_TargetVolumeName;
};

//-----------------------------------------------------------------------------
// qSlicerLoadableModuleTemplateModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerLoadableModuleTemplateModuleWidgetPrivate::qSlicerLoadableModuleTemplateModuleWidgetPrivate(qSlicerLoadableModuleTemplateModuleWidget &object)
  :
  q_ptr(&object),
  ReferenceVolumeNode(nullptr),
  StructureSetSegmentationNode(nullptr),
  SegmentationsLogic(nullptr),
  BeamsLogic(nullptr),
  ModelsLogic(nullptr),
  BeamNode(nullptr)
{
}

//-----------------------------------------------------------------------------
qSlicerLoadableModuleTemplateModuleWidgetPrivate::~qSlicerLoadableModuleTemplateModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
vtkSlicerLoadableModuleTemplateLogic*
qSlicerLoadableModuleTemplateModuleWidgetPrivate::logic() const
{
  Q_Q(const qSlicerLoadableModuleTemplateModuleWidget);
  return vtkSlicerLoadableModuleTemplateLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
// qSlicerLoadableModuleTemplateModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerLoadableModuleTemplateModuleWidget::qSlicerLoadableModuleTemplateModuleWidget(QWidget* parent)
  :
  Superclass(parent),
  d_ptr(new qSlicerLoadableModuleTemplateModuleWidgetPrivate(*this))
{
  // Q_D(qSlicerLoadableModuleTemplateModuleWidget);
}

//-----------------------------------------------------------------------------
qSlicerLoadableModuleTemplateModuleWidget::~qSlicerLoadableModuleTemplateModuleWidget()
{
}

//-----------------------------------------------------------------------------
void
qSlicerLoadableModuleTemplateModuleWidget::setup()
{
  Q_D(qSlicerLoadableModuleTemplateModuleWidget);
  this->Superclass::setup();

  d->setupUi(this);

  QStringList planNodes;
  planNodes.push_back("vtkMRMLRTPlanNode");
  d->RTPlanNodeComboBox->setNodeTypes(planNodes);

  QStringList rtBeamNodes;
  rtBeamNodes.push_back("vtkMRMLRTBeamNode");
  d->RTBeamNodeComboBox->setNodeTypes(rtBeamNodes);

  QStringList volumeNodes;
  volumeNodes.push_back("vtkMRMLVolumeNode");
  d->ReferenceVolumeNodeComboBox->setNodeTypes(volumeNodes);

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
  connect( d->RTPlanNodeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), 
    this, SLOT(onRTPlanNodeChanged(vtkMRMLNode*)));

  // Make connections for the checkboxes and buttons
//  connect( d->SubjectHierarchyDisplayDataNodeIDsCheckBox, SIGNAL(toggled(bool)),
//    this, SLOT(setMRMLIDsVisible(bool)));

  connect( d->CalculateOpeningPushButton, SIGNAL(clicked()),
    this, SLOT(onCalculateOpeningButtonClicked()));
  connect( d->CalculateOpeningPushButton1, SIGNAL(clicked()),
    this, SLOT(onCalculateOpeningButtonClicked1()));

  qDebug() << Q_FUNC_INFO;
}

//-----------------------------------------------------------------------------
void
qSlicerLoadableModuleTemplateModuleWidget::enter()
{
  Q_D(qSlicerLoadableModuleTemplateModuleWidget);
  this->Superclass::enter();

  qDebug() << Q_FUNC_INFO;
}

//-----------------------------------------------------------------------------
void
qSlicerLoadableModuleTemplateModuleWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qSlicerLoadableModuleTemplateModuleWidget);
  this->Superclass::setMRMLScene(scene);
  
//  this->setMRMLIDsVisible(d->SubjectHierarchyDisplayDataNodeIDsCheckBox->isChecked());

  qDebug() << Q_FUNC_INFO;
}

void
qSlicerLoadableModuleTemplateModuleWidget::onCalculateOpeningButtonClicked()
{
  Q_D(qSlicerLoadableModuleTemplateModuleWidget);
  if (!d->StructureSetSegmentationNode || d->m_TargetVolumeName.isEmpty())
  {
    return;
  }

  vtkSlicerLoadableModuleTemplateLogic* moduleLogic = vtkSlicerLoadableModuleTemplateLogic::SafeDownCast(this->logic());

  std::string targetId = d->m_TargetVolumeName.toStdString();

  if (moduleLogic && !targetId.empty())
  {
    vtkPolyData* targetPoly = d->StructureSetSegmentationNode->GetClosedSurfaceInternalRepresentation(targetId);
    if (targetPoly)
    {
      vtkMRMLMarkupsCurveNode* positionCurve = moduleLogic->CalculatePositionCurve( 
        d->BeamNode, targetPoly);
      if (positionCurve)
      {
        vtkMRMLTransformNode* beamTransformNode = d->BeamNode->GetParentTransformNode();
        positionCurve->SetAndObserveTransformNodeID(beamTransformNode->GetID());
        
        vtkMRMLDoubleArrayNode* mlcBoundaryNode = moduleLogic->CreateMultiLeafCollimatorDoubleArrayNode();
        vtkMRMLTableNode* mlcPositionNode = moduleLogic->CalculateMultiLeafCollimatorPosition( positionCurve, mlcBoundaryNode);
        if (mlcBoundaryNode && mlcPositionNode)
        {
          d->BeamNode->SetAndObserveMLCBoundaryDoubleArrayNode(mlcBoundaryNode);
          d->BeamNode->SetAndObserveMLCPositionTableNode(mlcPositionNode);
        }

        double area = moduleLogic->CalculateMultiLeafCollimatorPositionArea( mlcBoundaryNode, mlcPositionNode);
        qDebug() << Q_FUNC_INFO << ": Position area (mm^2) = " << area;

        area = moduleLogic->CalculateCurvePolygonArea(positionCurve);
        qDebug() << Q_FUNC_INFO << ": Polygon area (mm^2) = " << area;
        moduleLogic->SetParentForMultiLeafCollimatorPosition( d->BeamNode, mlcPositionNode);
      }
    }
  }
}

void
qSlicerLoadableModuleTemplateModuleWidget::onCalculateOpeningButtonClicked1()
{
  Q_D(qSlicerLoadableModuleTemplateModuleWidget);

  vtkSlicerLoadableModuleTemplateLogic* moduleLogic = vtkSlicerLoadableModuleTemplateLogic::SafeDownCast(this->logic());

  std::string targetId = d->m_TargetVolumeName.toStdString();

  if (moduleLogic && !targetId.empty())
  {
//    vtkSlicerLoadableModuleTemplateLogic* moduleLogic = vtkSlicerLoadableModuleTemplateLogic::SafeDownCast(this->logic());
//    moduleLogic->SetModelsLogic(d->ModelsLogic);
    moduleLogic->CreateMultiLeafCollimatorModelPolyData(d->BeamNode);
        
    vtkMRMLDoubleArrayNode* mlcBoundaryNode = d->BeamNode->GetMLCBoundaryDoubleArrayNode();
    vtkMRMLTableNode* mlcPositionNode = d->BeamNode->GetMLCPositionTableNode();

    double area = moduleLogic->CalculateMultiLeafCollimatorPositionArea( mlcBoundaryNode, mlcPositionNode);
    qDebug() << Q_FUNC_INFO << ": Position area (mm^2) = " << area;
  }
}

//-----------------------------------------------------------------------------
void
qSlicerLoadableModuleTemplateModuleWidget::setMRMLIDsVisible(bool visible)
{
  Q_D(qSlicerLoadableModuleTemplateModuleWidget);

  // Subject hierarchy view
//  d->SubjectHierarchyTreeView->setColumnHidden( d->SubjectHierarchyTreeView->model()->idColumn(), !visible);

  qDebug() << Q_FUNC_INFO << ": state = " << visible;
}

/// RTPlan Node changed
void
qSlicerLoadableModuleTemplateModuleWidget::onRTPlanNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerLoadableModuleTemplateModuleWidget);
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

/// RTBeam Node (RTBeam or RTIonBeam) changed
void
qSlicerLoadableModuleTemplateModuleWidget::onRTBeamNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerLoadableModuleTemplateModuleWidget);
  vtkMRMLRTBeamNode* beamNode = vtkMRMLRTBeamNode::SafeDownCast(node);
  vtkMRMLRTIonBeamNode* ionBeamNode = vtkMRMLRTIonBeamNode::SafeDownCast(node);
  d->BeamNode = beamNode;
  Q_UNUSED(ionBeamNode);
}

/// Segmentation node changed
void
qSlicerLoadableModuleTemplateModuleWidget::onSegmentationNodeChanged(vtkMRMLNode* segmentationNode)
{
  Q_D(qSlicerLoadableModuleTemplateModuleWidget);
  d->StructureSetSegmentationNode = vtkMRMLSegmentationNode::SafeDownCast(segmentationNode);
}

/// Segmentation node changed
void
qSlicerLoadableModuleTemplateModuleWidget::onVolumeNodeChanged(vtkMRMLNode* volumeNode)
{
  Q_D(qSlicerLoadableModuleTemplateModuleWidget);
  d->ReferenceVolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(volumeNode);
}

void
qSlicerLoadableModuleTemplateModuleWidget::setSegmentationsLogic(vtkSlicerSegmentationsModuleLogic* logic)
{
  Q_D(qSlicerLoadableModuleTemplateModuleWidget);
  if (logic)
  {
    qDebug() << "Segmentations logic is valid";
    d->SegmentationsLogic = logic;
  }
  else
  {
    qCritical() << "Segmentations logic is invalid";
    d->SegmentationsLogic = nullptr;
  }
}

void
qSlicerLoadableModuleTemplateModuleWidget::setBeamsLogic(vtkSlicerBeamsModuleLogic* logic)
{
  Q_D(qSlicerLoadableModuleTemplateModuleWidget);
  if (logic)
  {
    qDebug() << "Beams logic is valid";
    d->BeamsLogic = logic;
  }
  else
  {
    qCritical() << "Beams logic is invalid";
    d->BeamsLogic = nullptr;
  }
}

void
qSlicerLoadableModuleTemplateModuleWidget::setModelsLogic(vtkSlicerModelsLogic* logic)
{
  Q_D(qSlicerLoadableModuleTemplateModuleWidget);
  if (logic)
  {
    qDebug() << "Models logic is valid";
    d->ModelsLogic = logic;
  }
  else
  {
    qCritical() << "Models logic is invalid";
    d->ModelsLogic = nullptr;
  }
}

void
qSlicerLoadableModuleTemplateModuleWidget::onTargetSegmentChanged(const QString& text)
{
  Q_D(qSlicerLoadableModuleTemplateModuleWidget);
  d->m_TargetVolumeName = text;
}

