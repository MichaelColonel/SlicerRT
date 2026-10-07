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

#include "qSlicerImagePositioningModuleWidget.h"
#include "ui_qSlicerImagePositioningModuleWidget.h"


#include <qSlicerPythonManager.h> // include Python Manager first

// Qt includes
#include <QDebug>
#include <QSignalBlocker>
#include <QVariantMap>
#include <QString>

// CTK includes
#include <ctkDICOMQuery.h>
#include <ctkDICOMRetrieve.h>
#include <ctkDICOMDatabase.h>

// MRML includes
#include <vtkMRMLImagePositioningNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLLayoutNode.h>
#include <vtkMRMLLayoutLogic.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLSliceLogic.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLScalarVolumeDisplayNode.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLSliceCompositeNode.h>

// Slicer includes
#include <qSlicerLayoutManager.h>
#include <qSlicerApplication.h>
#include <qMRMLSliceWidget.h>

#include <vtkMatrix4x4.h>

// Logic includes
#include <vtkSlicerImagePositioningLogic.h>

class QAbstractButton;

//-----------------------------------------------------------------------------
class qSlicerImagePositioningModuleWidgetPrivate: public Ui_qSlicerImagePositioningModuleWidget
{
    Q_DECLARE_PUBLIC(qSlicerImagePositioningModuleWidget);
protected:
    qSlicerImagePositioningModuleWidget* const q_ptr;
public:
  qSlicerImagePositioningModuleWidgetPrivate(qSlicerImagePositioningModuleWidget& object);
  virtual ~qSlicerImagePositioningModuleWidgetPrivate();
  struct Pose2D
  {
    double HorizontalOffset = 0.0;
    double VerticalOffset = 0.0;
    double RotationDeg = 0.0;
  };

  const char* GENERIC_LAYOUT_DESCRIPTION = \
    "<layout type=\"vertical\" split=\"true\" >"\
    "  <item splitSize=\"500\"> "\
    "    <layout type=\"horizontal\">"\
    "    <item>" \
    "     <view class=\"vtkMRMLViewNode\" singletontag=\"1\">" \
    "       <property name=\"viewlabel\" action=\"default\">1</property>" \
    "     </view>" \
    "    </item>" \
    "     <item>" \
    "       <view class=\"vtkMRMLSliceNode\" singletontag=\"XrayDetectorSlice\">" \
    "         <property name=\"orientation\" action=\"default\">Axial</property>" \
    "         <property name=\"viewlabel\" action=\"default\">C</property>" \
    "         <property name=\"viewcolor\" action=\"default\">#C3B1E1</property>" \
    "         <property name=\"viewgroup\" action=\"default\">101</property>" \
    "       </view>" \
    "     </item>" \
    "    </layout>" \
    "  </item>" \
    "  <item splitSize=\"350\">" \
    "    <layout type=\"horizontal\">" \
    "      <item>" \
    "         <view class=\"vtkMRMLSliceNode\" singletontag=\"Red\">" \
    "           <property name=\"orientation\" action=\"default\">Axial</property>" \
    "           <property name=\"viewlabel\" action=\"default\">R</property>" \
    "           <property name=\"viewcolor\" action=\"default\">#F34A33</property>" \
    "        </view>" \
    "      </item>" \
    "      <item>" \
    "         <view class=\"vtkMRMLSliceNode\" singletontag=\"Green\">" \
    "           <property name=\"orientation\" action=\"default\">Coronal</property>" \
    "           <property name=\"viewlabel\" action=\"default\">G</property>" \
    "           <property name=\"viewcolor\" action=\"default\">#6EB04B</property>" \
    "        </view>" \
    "      </item>" \
    "      <item>" \
    "         <view class=\"vtkMRMLSliceNode\" singletontag=\"Yellow\">" \
    "           <property name=\"orientation\" action=\"default\">Sagittal</property>" \
    "           <property name=\"viewlabel\" action=\"default\">Y</property>" \
    "           <property name=\"viewcolor\" action=\"default\">#EDD54C</property>" \
    "        </view>" \
    "      </item>" \
    "    </layout>" \
    "  </item>" \
    "  <item splitSize=\"0\">" \
    "    <layout type=\"horizontal\">" \
    "      <item>" \
    "        <view class=\"vtkMRMLViewNode\" singletontag=\"XrayDetectorSlice\">" \
    "           <property name=\"viewlabel\" action=\"default\">F</property>" \
    "           <property name=\"viewcolor\" action=\"default\">#C3B1E1</property>" \
    "           <property name=\"viewgroup\" action=\"default\">100</property>" \
    "        </view>" \
    "       </item>" \
    "    </layout>" \
    "  </item>" \
    "</layout>";
  const int GENERIC_LAYOUT_ID = 1020;
  int PreviousLayoutId = -1;
  Pose2D HorizontalPose;
  Pose2D VerticalPose;
  vtkSmartPointer<vtkMRMLImagePositioningNode> ParameterNode;
  vtkSlicerImagePositioningLogic* logic() const;
};

//-----------------------------------------------------------------------------
// qSlicerImagePositioningModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerImagePositioningModuleWidgetPrivate::qSlicerImagePositioningModuleWidgetPrivate(qSlicerImagePositioningModuleWidget& object)
    :
    q_ptr(&object)
{
}

qSlicerImagePositioningModuleWidgetPrivate::~qSlicerImagePositioningModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------

vtkSlicerImagePositioningLogic* qSlicerImagePositioningModuleWidgetPrivate::logic() const
{
    Q_Q(const qSlicerImagePositioningModuleWidget);
    return vtkSlicerImagePositioningLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
// qSlicerImagePositioningModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerImagePositioningModuleWidget::qSlicerImagePositioningModuleWidget(QWidget* _parent)
  : Superclass( _parent )
    , d_ptr(new qSlicerImagePositioningModuleWidgetPrivate(*this))
{
}

//-----------------------------------------------------------------------------
qSlicerImagePositioningModuleWidget::~qSlicerImagePositioningModuleWidget()
{
}



//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::setup()
{
  Q_D(qSlicerImagePositioningModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  // Get layout manager
  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();

  // Save previous layout
  d->PreviousLayoutId = layoutManager->layout();

  vtkMRMLLayoutNode* layoutNode = layoutManager->layoutLogic()->GetLayoutNode();
  if (layoutNode)
  {
    if (!layoutNode->SetLayoutDescription(d->GENERIC_LAYOUT_ID, d->GENERIC_LAYOUT_DESCRIPTION))
    {
      layoutNode->AddLayoutDescription(d->GENERIC_LAYOUT_ID, d->GENERIC_LAYOUT_DESCRIPTION);
    }
  }
  // Nodes
  connect(d->MRMLNodeComboBox_DrrImage, SIGNAL(currentNodeChanged(vtkMRMLNode*)), 
    this, SLOT(onDrrImageNodeChanged(vtkMRMLNode*)));
  connect(d->MRMLNodeComboBox_XrayImage, SIGNAL(currentNodeChanged(vtkMRMLNode*)), 
    this, SLOT(onXrayImageNodeChanged(vtkMRMLNode*)));
  connect(d->MRMLSliderWidget_XrayOpacity, SIGNAL(valueChanged(double)),
      this, SLOT(onXrayOpacityChanged(double)));


  // Buttons
  QObject::connect(d->PushButton_RetrieveDICOM, SIGNAL(clicked()), this, SLOT(onRetrieveDICOMClicked()));
  QObject::connect(d->PushButton_SetView, SIGNAL(clicked()), this, SLOT(onSetViewClicked()));
  QObject::connect(d->PushButton_CustomLayout, SIGNAL(clicked()), this, SLOT(onSetCustomLayoutClicked()));
  QObject::connect(d->PushButton_ScaleXrayImage, SIGNAL(clicked()), this, SLOT(onScaleXrayImageClicked()));
  QObject::connect(d->PushButton_Up, SIGNAL(clicked()), this, SLOT(onMoveUpClicked()));
  QObject::connect(d->PushButton_Down, SIGNAL(clicked()), this, SLOT(onMoveDownClicked()));
  QObject::connect(d->PushButton_Left, SIGNAL(clicked()), this, SLOT(onMoveLeftClicked()));
  QObject::connect(d->PushButton_Right, SIGNAL(clicked()), this, SLOT(onMoveRightClicked()));
  QObject::connect(d->PushButton_Clockwise, SIGNAL(clicked()), this, SLOT(onRotateClockwiseClicked()));
  QObject::connect(d->PushButton_CounterClockwise, SIGNAL(clicked()), this, SLOT(onRotateCounterClockwiseClicked()));
  QObject::connect(d->PushButton_Reset, SIGNAL(clicked()), this, SLOT(onResetTransformClicked()));

  // Checkboxes
  QObject::connect(d->checkBox_ShowEdgesXray, SIGNAL(toggled(bool)), this, SLOT(onShowEdgesXrayToggled(bool)));
  QObject::connect(d->checkBox_ShowEdgesDrr, SIGNAL(toggled(bool)), this, SLOT(onShowEdgesDrrToggled(bool)));

  // Sliders
  QObject::connect(d->MRMLSliderWidget_HorizontalTransform, SIGNAL(valueChanged(double)), this, SLOT(onHorizontalTransformChanged(double)));
  QObject::connect(d->MRMLSliderWidget_VerticalTransform, SIGNAL(valueChanged(double)), this, SLOT(onVerticalTransformChanged(double)));
  QObject::connect(d->MRMLSliderWidget_Rotation, SIGNAL(valueChanged(double)), this, SLOT(onRotationTransformChanged(double)));


  this->setXrayNode(nullptr);
  this->sync2DControlsFromActiveOrientation();
}


//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qSlicerImagePositioningModuleWidget);
  this->Superclass::setMRMLScene(scene);

  qvtkReconnect(d->logic(), scene, vtkMRMLScene::EndImportEvent, this, SLOT(onSceneImportedEvent()));
  qvtkReconnect(d->logic(), scene, vtkMRMLScene::EndCloseEvent, this, SLOT(onSceneClosedEvent()));

  // Find parameters node or create it if there is none in the scene
  if (scene)
  {
    if (d->MRMLNodeComboBox_ParameterSet->currentNode())
    {
        this->setParameterNode(d->MRMLNodeComboBox_ParameterSet->currentNode());
    }
    else if (vtkMRMLNode* node = scene->GetNthNodeByClass(0, "vtkMRMLImagePositioningNode"))
    {
        this->setParameterNode(node);
    }
    else
    {
        vtkMRMLNode* newNode = scene->AddNewNodeByClass("vtkMRMLImagePositioningNode");
        this->setParameterNode(newNode);
    }
  }
}
//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::setParameterNode(vtkMRMLNode* node)
{
  Q_D(qSlicerImagePositioningModuleWidget);

  vtkMRMLImagePositioningNode* parameterNode = vtkMRMLImagePositioningNode::SafeDownCast(node);

  // Make sure the parameter set node is selected (in case the function was not called by the selector combobox signal)
  d->MRMLNodeComboBox_ParameterSet->setCurrentNode(node);

  // Each time the node is modified, the UI widgets are updated
  //qvtkReconnect(d->ParameterNode, parameterNode, vtkCommand::ModifiedEvent,
  //    this, SLOT(updateWidgetFromMRML()));

  d->ParameterNode = parameterNode;



  // Set selected MRML nodes in comboboxes in the parameter set if it was nullptr there
  // (then in the meantime the comboboxes selected the first one from the scene and we have to set that)
  //if (d->ParameterNode)
  //{
  //    vtkMRMLRTBeamNode* beamNode = vtkMRMLRTBeamNode::SafeDownCast(d->MRMLNodeComboBox_Beam->currentNode());
  //    d->ParameterNode->SetAndObserveBeamNode(beamNode);
  //    vtkMRMLRTBeamNode* planNode = vtkMRMLRTBeamNode::SafeDownCast(d->MRMLNodeComboBox_Plan->currentNode());
  //}
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerImagePositioningModuleWidget);

  vtkMRMLImagePositioningNode* parameterNode = vtkMRMLImagePositioningNode::SafeDownCast(d->MRMLNodeComboBox_ParameterSet->currentNode());

  if (!this->mrmlScene())
  {
      qCritical() << Q_FUNC_INFO << ": Invalid scene";
      return;
  }

  if (!parameterNode)
  {
      qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
      return;
  }
  // TODO
}

//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::onRetrieveDICOMClicked()
{
  Q_D(qSlicerImagePositioningModuleWidget);

  QString patientID = d->LineEdit_PatientID->text();
  QString aeTitle = d->LineEdit_AETitle->text();
  QString host = d->LineEdit_Host->text();
  int port = d->SpinBox_Port->value();
  if (patientID.isEmpty() || aeTitle.isEmpty() || host.isEmpty())
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameters. Fill in the fields.";
    return;
  }

  QApplication::setOverrideCursor(Qt::WaitCursor);

  // -------- Query ----------

  ctkDICOMQuery* dicomQuery = new ctkDICOMQuery();
  dicomQuery->setCallingAETitle("SLICER");
  dicomQuery->setCalledAETitle(aeTitle);
  dicomQuery->setHost(host);
  dicomQuery->setPort(port);

  QMap<QString, QVariant> filters;
  filters["ID"] = patientID;
  dicomQuery->setFilters(filters);

  // temporary in-memory database for storing query results
  ctkDICOMDatabase* tempDb = new ctkDICOMDatabase();
  tempDb->openDatabase("");
  dicomQuery->query(*tempDb);

  // Get the main DICOM database of Slicer
  ctkDICOMDatabase* slicerDb = qSlicerCoreApplication::application()->dicomDatabase();

  // ---------- Retrieve -----------

  bool useCGET = true;
  ctkDICOMRetrieve* dicomRetrieve = new ctkDICOMRetrieve();
  dicomRetrieve->setCallingAETitle(dicomQuery->callingAETitle());
  dicomRetrieve->setCalledAETitle(dicomQuery->calledAETitle());
  dicomRetrieve->setHost(dicomQuery->host());
  dicomRetrieve->setPort(dicomQuery->port());
  dicomRetrieve->setDatabase(*slicerDb);

  QList<QPair<QString, QString>> studySeriesList = dicomQuery->studyAndSeriesInstanceUIDQueried();
  for (const auto& studySeries : studySeriesList)
  {
      QString study = studySeries.first;
      QString series = studySeries.second;
      qDebug() << "ctkDICOMRetrieve: Retrieving Study:" << study << "- Series:" << series;
      QCoreApplication::processEvents();

      bool success;
      if (useCGET)
      {
          success = dicomRetrieve->getSeries(study, series);
      }
      else
      {
          dicomRetrieve->setMoveDestinationAETitle(dicomQuery->callingAETitle());
          success = dicomRetrieve->moveSeries(study, series);
      }
      if (!success)
      {
          qCritical() << "Failed";
          QApplication::restoreOverrideCursor();
          return;
      }
      else
      {
          qDebug() << "Success";
      }
  }

  slicerDb->updateDisplayedFields();

  // --------- Load into scene ----------

  qSlicerPythonManager* pythonManager = qSlicerApplication::application()->pythonManager();

  QString pythonCode = QString(
      "from DICOMLib import DICOMUtils\n"
      "loadedNodes = DICOMUtils.loadPatientByPatientID('%1')\n"
      "print(f'DICOMUtils: Loaded {len(loadedNodes)} nodes')\n"
  ).arg(patientID);

  pythonManager->executeString(pythonCode);
  QApplication::restoreOverrideCursor();
}

//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::onSetCustomLayoutClicked()
{
  Q_D(qSlicerImagePositioningModuleWidget);

  // Get layout manager
  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();

  if (d->PushButton_CustomLayout->isChecked())
  {
    if (layoutManager)
    {
      layoutManager->setLayout(d->GENERIC_LAYOUT_ID);
      qDebug() << Q_FUNC_INFO << layoutManager->layout();
      d->MRMLSliderWidget_XrayOpacity->setEnabled(true);
    }
  }
  else
  {
    if (layoutManager)
    {
      layoutManager->setLayout(d->PreviousLayoutId);
      qDebug() << Q_FUNC_INFO << layoutManager->layout();
      d->MRMLSliderWidget_XrayOpacity->setEnabled(false);
    }
  }
  slicerApplication->processEvents();
}

//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::onSetViewClicked()
{
  Q_D(qSlicerImagePositioningModuleWidget);
  if (!d->ParameterNode)
  {
      qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
      return;
  }
  if (!d->ParameterNode->GetXrayNode() || !d->ParameterNode->GetDrrNode())
  {
    qCritical() << Q_FUNC_INFO << ": Invalid Xray or DRR image node";
    return;
  }

  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();

  // Set images
  // DRR - background, XRay - foreground
  if (layoutManager->layout() == 1020)
  {
    vtkMRMLScalarVolumeNode* xrayImageNode = d->ParameterNode->GetXrayNode();
    vtkMRMLScalarVolumeNode* drrImageNode = d->ParameterNode->GetDrrNode();

    // Set active orientation based on the button checked
    if (d->PushButton_VerticalOrientation->isChecked())
    {
      d->logic()->SetActiveOrientation(vtkMRMLImagePositioningNode::OrientationVertical);
    }
    else
    {
      d->logic()->SetActiveOrientation(vtkMRMLImagePositioningNode::OrientationHorizontal);
    }



    this->setXrayNode(xrayImageNode);

    qMRMLSliceWidget* sliceWidget = layoutManager->sliceWidget("XrayDetectorSlice");
    vtkMRMLSliceNode* sliceNode = sliceWidget->mrmlSliceNode();
    vtkMRMLSliceLogic* sliceLogic = sliceWidget->sliceLogic();

    // Set slice to show DRR first
    sliceLogic->GetSliceCompositeNode()->SetForegroundVolumeID(nullptr);
    sliceLogic->GetSliceCompositeNode()->SetBackgroundVolumeID(drrImageNode->GetID());
    sliceLogic->RotateSliceToLowestVolumeAxes(); // Reformat 
    sliceLogic->FitSliceToAll();
    sliceNode->UpdateMatrices();
    this->setSliceOrientation();

    sliceLogic->GetSliceCompositeNode()->SetForegroundVolumeID(xrayImageNode->GetID());
    sliceLogic->GetSliceCompositeNode()->SetForegroundOpacity(d->MRMLSliderWidget_XrayOpacity->value());
    sliceLogic->GetSliceCompositeNode()->SetClipToBackgroundVolume(false);

    // Change color of images
    vtkMRMLScalarVolumeDisplayNode* drrDisplayNode = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(drrImageNode->GetDisplayNode());
    vtkMRMLScalarVolumeDisplayNode* xrayDisplayNode = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(xrayImageNode->GetDisplayNode());
    drrDisplayNode->SetAndObserveColorNodeID("vtkMRMLColorTableNodeGreen");
    xrayDisplayNode->SetAndObserveColorNodeID("vtkMRMLColorTableNodeRed");

    // Set compositing mode
    // TODO: let user choose compositing mode
    // alpha blend = 0
    // add = 2
    // subtract = 3
    //sliceLogic->GetSliceCompositeNode()->SetCompositing(2); // add

    d->checkBox_ShowEdgesXray->setChecked(false);
    d->checkBox_ShowEdgesDrr->setChecked(false);

    this->sync2DControlsFromActiveOrientation();
    this->updateXrayTransformFrom2DControls();
  }
  else
  {
      qCritical() << Q_FUNC_INFO << ": Wrong layout, set layout to custom";
      return;
  }
}

//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::onScaleXrayImageClicked()
{
  Q_D(qSlicerImagePositioningModuleWidget);
  double sourceToImagerMm = d->MRMLSliderWidget_SID->value();

  d->logic()->AlignAndScaleXrayToDrr(sourceToImagerMm);
}


// void qSlicerImagePositioningModuleWidget::onHorizontalOrientationClicked()
// {
//   Q_D(qSlicerImagePositioningModuleWidget);
//   if (!d->ParameterNode)
//   {
//       qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
//       return;
//   }

//   d->logic()->SetActiveOrientation(vtkMRMLImagePositioningNode::OrientationHorizontal);
//  // this->setXrayNode(d->ParameterNode->GetXrayNode());
//  // this->sync2DControlsFromActiveOrientation();
//  // this->updateXrayTransformFrom2DControls();
// }

// void qSlicerImagePositioningModuleWidget::onVerticalOrientationClicked()
// {
//   Q_D(qSlicerImagePositioningModuleWidget);
//   if (!d->ParameterNode)
//   {
//       qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
//       return;
//   }

  
//  // this->setXrayNode(d->ParameterNode->GetXrayNode());
//  // this->sync2DControlsFromActiveOrientation();
//   this->updateXrayTransformFrom2DControls();
// }

//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::onDrrImageNodeChanged(vtkMRMLNode* drrImageNode)
{
  Q_D(qSlicerImagePositioningModuleWidget);
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  if (drrImageNode)
  {
    d->ParameterNode->SetAndObserveDrrNode(vtkMRMLScalarVolumeNode::SafeDownCast(drrImageNode));
  }
  else 
  {
    qCritical() << Q_FUNC_INFO << ": Invalid DRR image node";
    d->PushButton_SetView->setEnabled(false);
    return;
  }

  if (d->ParameterNode->GetXrayNode() && d->ParameterNode->GetDrrNode())
  {
    d->PushButton_SetView->setEnabled(true);
  //  d->PushButton_ScaleXrayImage->setEnabled(true);
    d->MRMLSliderWidget_SID->setEnabled(true);
  }
}

//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::onXrayImageNodeChanged(vtkMRMLNode* xrayImageNode)
{
  Q_D(qSlicerImagePositioningModuleWidget);
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  //vtkMRMLScalarVolumeNode* xrayVolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(xrayImageNode);


  // if (d->ParameterNode->GetActiveOrientation() != vtkMRMLImagePositioningNode::OrientationVertical)
  // {
  //   d->logic()->SetActiveOrientation(vtkMRMLImagePositioningNode::OrientationHorizontal);
  // }
  //this->setXrayNode(xrayVolumeNode);
  //this->sync2DControlsFromActiveOrientation();
  //this->updateXrayTransformFrom2DControls();

  if (xrayImageNode)
  {
    d->ParameterNode->SetAndObserveXrayNode(vtkMRMLScalarVolumeNode::SafeDownCast(xrayImageNode));
  }
  else
  {
    qCritical() << Q_FUNC_INFO << ": Invalid Xray image node";
    d->PushButton_SetView->setEnabled(false);
    return;
  }
  if (d->ParameterNode->GetXrayNode() && d->ParameterNode->GetDrrNode())
  {
    d->PushButton_SetView->setEnabled(true);
    d->MRMLSliderWidget_SID->setEnabled(true);
  }
}

//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::setXrayNode(vtkMRMLScalarVolumeNode* xrayNode)
{
  Q_D(qSlicerImagePositioningModuleWidget);

  vtkMRMLLinearTransformNode* activeTransformNode = nullptr;
  if (xrayNode)
  {
    d->logic()->SetXrayNode(xrayNode);
    activeTransformNode = d->logic()->GetActiveXrayTransformNode();
  }


  d->MRMLMatrixWidget_TransformMatrix->setMRMLTransformNode(activeTransformNode);
  const bool hasTransform = (activeTransformNode != nullptr);
  d->PushButton_ScaleXrayImage->setEnabled(hasTransform);
  d->MRMLMatrixWidget_TransformMatrix->setEnabled(hasTransform);
  d->MRMLSliderWidget_HorizontalTransform->setEnabled(hasTransform);
  d->MRMLSliderWidget_VerticalTransform->setEnabled(hasTransform);
  d->MRMLSliderWidget_Rotation->setEnabled(hasTransform);
  // TODO: Uncheck edges
  d->PushButton_Up->setEnabled(hasTransform);
  d->PushButton_Down->setEnabled(hasTransform);
  d->PushButton_Left->setEnabled(hasTransform);
  d->PushButton_Right->setEnabled(hasTransform);
  d->PushButton_Clockwise->setEnabled(hasTransform);
  d->PushButton_CounterClockwise->setEnabled(hasTransform);
  d->PushButton_Reset->setEnabled(hasTransform);
 // d->MRMLCoordinatesWidget_TranslatePosition->setEnabled(hasTransform);
}

//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::sync2DControlsFromActiveOrientation()
{
  Q_D(qSlicerImagePositioningModuleWidget);
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  int orientation = d->ParameterNode->GetActiveOrientation();
  qSlicerImagePositioningModuleWidgetPrivate::Pose2D* pose = &d->HorizontalPose;
  if (orientation == vtkMRMLImagePositioningNode::OrientationVertical)
  {
    pose = &d->VerticalPose;
  }
  QSignalBlocker horizontalBlocker(d->MRMLSliderWidget_HorizontalTransform);
  QSignalBlocker verticalBlocker(d->MRMLSliderWidget_VerticalTransform);
  QSignalBlocker rotationBlocker(d->MRMLSliderWidget_Rotation);
  d->MRMLSliderWidget_HorizontalTransform->setValue(pose->HorizontalOffset);
  d->MRMLSliderWidget_VerticalTransform->setValue(pose->VerticalOffset);
  d->MRMLSliderWidget_Rotation->setValue(pose->RotationDeg);
}

//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::updateXrayTransformFrom2DControls()
{
    // TODO: Clean up
  Q_D(qSlicerImagePositioningModuleWidget);
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  qSlicerImagePositioningModuleWidgetPrivate::Pose2D* pose = &d->HorizontalPose;
  int orientation = d->ParameterNode->GetActiveOrientation();
  if (orientation == vtkMRMLImagePositioningNode::OrientationVertical)
  {
    pose = &d->VerticalPose;
  }

  d->logic()->ApplyXray2DTransform(
    pose->HorizontalOffset, pose->VerticalOffset, pose->RotationDeg, orientation);
  //d->MRMLCoordinatesWidget_TranslatePosition->setCoordinates(translationRAS);
  d->MRMLMatrixWidget_TransformMatrix->setMRMLTransformNode(d->logic()->GetActiveXrayTransformNode());
}

//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::onMoveUpClicked()
{
  Q_D(qSlicerImagePositioningModuleWidget);
  d->MRMLSliderWidget_VerticalTransform->setValue(
    d->MRMLSliderWidget_VerticalTransform->value() + d->MRMLSliderWidget_VerticalTransform->singleStep());
}

//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::onMoveDownClicked()
{
  Q_D(qSlicerImagePositioningModuleWidget);
  d->MRMLSliderWidget_VerticalTransform->setValue(
    d->MRMLSliderWidget_VerticalTransform->value() - d->MRMLSliderWidget_VerticalTransform->singleStep());
}

//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::onMoveLeftClicked()
{
  Q_D(qSlicerImagePositioningModuleWidget);
  d->MRMLSliderWidget_HorizontalTransform->setValue(
    d->MRMLSliderWidget_HorizontalTransform->value() - d->MRMLSliderWidget_HorizontalTransform->singleStep());
}

//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::onMoveRightClicked()
{
  Q_D(qSlicerImagePositioningModuleWidget);
  d->MRMLSliderWidget_HorizontalTransform->setValue(
    d->MRMLSliderWidget_HorizontalTransform->value() + d->MRMLSliderWidget_HorizontalTransform->singleStep());
}

//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::onRotateClockwiseClicked()
{
  Q_D(qSlicerImagePositioningModuleWidget);
  d->MRMLSliderWidget_Rotation->setValue(
    d->MRMLSliderWidget_Rotation->value() - d->MRMLSliderWidget_Rotation->singleStep());
}

//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::onRotateCounterClockwiseClicked()
{
  Q_D(qSlicerImagePositioningModuleWidget);
  d->MRMLSliderWidget_Rotation->setValue(
    d->MRMLSliderWidget_Rotation->value() + d->MRMLSliderWidget_Rotation->singleStep());
}

//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::onResetTransformClicked()
{
  Q_D(qSlicerImagePositioningModuleWidget);
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  int orientation = d->ParameterNode->GetActiveOrientation();
  if (orientation == vtkMRMLImagePositioningNode::OrientationVertical)
  {
    d->VerticalPose = qSlicerImagePositioningModuleWidgetPrivate::Pose2D();
  }
  else
  {
    d->HorizontalPose = qSlicerImagePositioningModuleWidgetPrivate::Pose2D();
  }
  this->sync2DControlsFromActiveOrientation();
  this->updateXrayTransformFrom2DControls();
}

//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::onHorizontalTransformChanged(double value)
{
  Q_D(qSlicerImagePositioningModuleWidget);
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  int orientation = d->ParameterNode->GetActiveOrientation();
  if (orientation == vtkMRMLImagePositioningNode::OrientationVertical)
  {
    d->VerticalPose.HorizontalOffset = value;
  }
  else
  {
    d->HorizontalPose.HorizontalOffset = value;
  }
  this->updateXrayTransformFrom2DControls();
}

//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::onVerticalTransformChanged(double value)
{
  Q_D(qSlicerImagePositioningModuleWidget);
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  int orientation = d->ParameterNode->GetActiveOrientation();
  if (orientation == vtkMRMLImagePositioningNode::OrientationVertical)
  {
    d->VerticalPose.VerticalOffset = value;
  }
  else
  {
    d->HorizontalPose.VerticalOffset = value;
  }
  this->updateXrayTransformFrom2DControls();
}

//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::onRotationTransformChanged(double value)
{
  Q_D(qSlicerImagePositioningModuleWidget);
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  int orientation = d->ParameterNode->GetActiveOrientation();
  if (orientation == vtkMRMLImagePositioningNode::OrientationVertical)
  {
    d->VerticalPose.RotationDeg = value;
  }
  else
  {
    d->HorizontalPose.RotationDeg = value;
  }
  this->updateXrayTransformFrom2DControls();
}

//void qSlicerImagePositioningModuleWidget::onDrrOpacityChanged(double* opacity)
//{
//    Q_D(qSlicerImagePositioningModuleWidget);
//    qSlicerApplication* slicerApplication = qSlicerApplication::application();
//    qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();
//
//    if (layoutManager->layout() == 1020)
//    {
//        qMRMLSliceWidget* sliceWidget = slicerApplication->layoutManager()->sliceWidget("XrayDetectorSlice");
//        vtkMRMLSliceLogic* sliceLogic = sliceWidget->sliceLogic();
//        sliceLogic->GetSliceCompositeNode()->SetBackgroundOpacity(opacity);
//    }
//    else
//    {
//        qCritical() << Q_FUNC_INFO << ": Wrong layout, set layout to custom";
//        return;
//    }
//}

//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::onXrayOpacityChanged(double opacity)
{
  Q_D(qSlicerImagePositioningModuleWidget);
  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();

  if (layoutManager->layout() == 1020)
  {
    qMRMLSliceWidget* sliceWidget = slicerApplication->layoutManager()->sliceWidget("XrayDetectorSlice");
    vtkMRMLSliceLogic* sliceLogic = sliceWidget->sliceLogic();
    sliceLogic->GetSliceCompositeNode()->SetForegroundOpacity(opacity);
  }
  else
  {
    qCritical() << Q_FUNC_INFO << ": Wrong layout, set layout to custom";
    return;
  }
}

//-----------------------------------------------------------------------------
void qSlicerImagePositioningModuleWidget::setSliceOrientation()
{
  Q_D(qSlicerImagePositioningModuleWidget);
  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();
  qMRMLSliceWidget* sliceWidget = layoutManager->sliceWidget("XrayDetectorSlice");
  vtkMRMLSliceNode* sliceNode = sliceWidget->mrmlSliceNode();

  double N[3];
  double T[3];

  if (d->ParameterNode->GetActiveOrientation() == vtkMRMLImagePositioningNode::OrientationHorizontal)
  {
    // Horizontal
    N[0] = -1.0; N[1] = 0.0; N[2] = 0.0; // Normal: Left -> Right
    T[0] = 0.0; T[1] = 0.0; T[2] = 1.0; // Up: Anterior
  }
  else if (d->ParameterNode->GetActiveOrientation() == vtkMRMLImagePositioningNode::OrientationVertical)
  {
    // Vertical
    N[0] = 0.0; N[1] = -1.0; N[2] = 0.0; // Normal: Anterior -> Posterior
    T[0] = 1.0; T[1] = 0.0; T[2] = 0.0; // Up: Superior
  }
  else
  {
    qCritical() << Q_FUNC_INFO << ": Set a proper beam orientation";
    return;
  }

  vtkMatrix4x4* SliceToRASMatrix = sliceNode->GetSliceToRAS();

  double originRAS[3];
  originRAS[0] = SliceToRASMatrix->GetElement(0,3);
  originRAS[1] = SliceToRASMatrix->GetElement(1,3);
  originRAS[2] = SliceToRASMatrix->GetElement(2,3);

  sliceNode->SetSliceToRASByNTP(
    N[0], N[1], N[2],
    T[0], T[1], T[2],
    originRAS[0], originRAS[1], originRAS[2],
    0                   // Orientation index (unused)
  );
  sliceNode->UpdateMatrices();
}

void qSlicerImagePositioningModuleWidget::onShowEdgesXrayToggled(bool checked)
{
  Q_D(qSlicerImagePositioningModuleWidget);

  vtkMRMLScalarVolumeNode* xrayImageNode = d->ParameterNode->GetXrayNode();

  // Get slice logic
  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();
  qMRMLSliceWidget* sliceWidget = layoutManager->sliceWidget("XrayDetectorSlice");
  vtkMRMLSliceLogic* sliceLogic = sliceWidget->sliceLogic();

  if (checked)
  {
    vtkMRMLScalarVolumeNode* gradientImageNode = d->logic()->GetOrCreateImageWithGradientFilter(xrayImageNode);
    sliceLogic->GetSliceCompositeNode()->SetForegroundVolumeID(gradientImageNode->GetID());

    // Set color
    vtkMRMLScalarVolumeDisplayNode* xrayDisplayNode = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(gradientImageNode->GetDisplayNode());
    xrayDisplayNode->SetAndObserveColorNodeID("vtkMRMLColorTableNodeRed");
  }
  else
  {
    sliceLogic->GetSliceCompositeNode()->SetForegroundVolumeID(xrayImageNode->GetID());
  }
}

void qSlicerImagePositioningModuleWidget::onShowEdgesDrrToggled(bool checked)
{
  Q_D(qSlicerImagePositioningModuleWidget);

  vtkMRMLScalarVolumeNode* drrImageNode = d->ParameterNode->GetDrrNode();

  // Get slice logic
  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();
  qMRMLSliceWidget* sliceWidget = layoutManager->sliceWidget("XrayDetectorSlice");
  vtkMRMLSliceLogic* sliceLogic = sliceWidget->sliceLogic();

  if (checked)
  {
    vtkMRMLScalarVolumeNode* gradientImageNode = d->logic()->GetOrCreateImageWithGradientFilter(drrImageNode);
    sliceLogic->GetSliceCompositeNode()->SetBackgroundVolumeID(gradientImageNode->GetID());

    // Set color
    vtkMRMLScalarVolumeDisplayNode* drrDisplayNode = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(gradientImageNode->GetDisplayNode());
    drrDisplayNode->SetAndObserveColorNodeID("vtkMRMLColorTableNodeGreen");
  }
  else
  {
    sliceLogic->GetSliceCompositeNode()->SetBackgroundVolumeID(drrImageNode->GetID());
  }
}