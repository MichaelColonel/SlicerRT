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

// RtImage Logic includes
#include "vtkSlicerRtImageLogic.h"

// Slicer includes
#include <vtkSlicerCLIModuleLogic.h>

// SlicerRT PlanarImage includes
#include <vtkSlicerPlanarImageModuleLogic.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLScalarVolumeDisplayNode.h>
#include <vtkMRMLLinearTransformNode.h>

//#include <vtkMRMLMarkupsPlaneNode.h>
#include <vtkMRMLMarkupsClosedCurveNode.h>
#include <vtkMRMLMarkupsFiducialNode.h>
#include <vtkMRMLMarkupsLineNode.h>

// SlicerRT MRML includes
#include <vtkMRMLRTBeamNode.h>
#include <vtkMRMLRTPlanNode.h>
#include "vtkMRMLRTImageNode.h"

// SubjectHierarchy includes
#include <vtkMRMLSubjectHierarchyConstants.h>
#include <vtkMRMLSubjectHierarchyNode.h>
#include <vtkSlicerSubjectHierarchyModuleLogic.h>

// VTK includes
#include <vtkTransform.h>
#include <vtkGeneralTransform.h>
#include <vtkMatrix4x4.h>
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// SlicerRT includes
#include <vtkSlicerRtCommon.h>

const char* vtkSlicerRtImageLogic::IMAGER_BOUNDARY_MARKUPS_NODE_NAME = "ImagerBoundary"; // curve
const char* vtkSlicerRtImageLogic::IMAGE_WINDOW_MARKUPS_NODE_NAME = "ImageWindow"; // curve
const char* vtkSlicerRtImageLogic::FIDUCIALS_MARKUPS_NODE_NAME = "FiducialPoints"; // fiducial
const char* vtkSlicerRtImageLogic::NORMAL_VECTOR_MARKUPS_NODE_NAME = "NormalVector"; // line
const char* vtkSlicerRtImageLogic::VUP_VECTOR_MARKUPS_NODE_NAME = "VupVector"; // line

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerRtImageLogic);

//----------------------------------------------------------------------------
vtkSlicerRtImageLogic::vtkSlicerRtImageLogic()
  :
  PlanarImageLogic(nullptr),
  PlastimatchDRRComputationLogic(nullptr),
  PlastimatchDRRImageIndex(0)
{
}

//----------------------------------------------------------------------------
vtkSlicerRtImageLogic::~vtkSlicerRtImageLogic()
{
}

//----------------------------------------------------------------------------
void vtkSlicerRtImageLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerRtImageLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerRtImageLogic::RegisterNodes()
{
  vtkMRMLScene* scene = this->GetMRMLScene(); 
  if (!scene)
  {
    vtkErrorMacro("RegisterNodes: Invalid MRML scene");
    return;
  }
  if (!scene->IsNodeClassRegistered("vtkMRMLRTPlanNode"))
  {
    scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLRTPlanNode>::New());
  }
  if (!scene->IsNodeClassRegistered("vtkMRMLRTBeamNode"))
  {
    scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLRTBeamNode>::New());
  }
  if (!scene->IsNodeClassRegistered("vtkMRMLRTImageNode"))
  {
    scene->RegisterNodeClass(vtkSmartPointer<vtkMRMLRTImageNode>::New());
  }
}

//---------------------------------------------------------------------------
void vtkSlicerRtImageLogic::UpdateFromMRMLScene()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("UpdateFromMRMLScene: Invalid MRML scene");
    return;
  }
}

//---------------------------------------------------------------------------
void vtkSlicerRtImageLogic::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  if (!node || !this->GetMRMLScene())
  {
    vtkErrorMacro("OnMRMLSceneNodeAdded: Invalid MRML scene or input node");
    return;
  }

  if (node->IsA("vtkMRMLRTImageNode"))
  {
    vtkNew<vtkIntArray> events;
    events->InsertNextValue(vtkCommand::ModifiedEvent);
    vtkObserveMRMLNodeEventsMacro(node, events);
  }
}

//---------------------------------------------------------------------------
void vtkSlicerRtImageLogic::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
}

//----------------------------------------------------------------------------
void vtkSlicerRtImageLogic::ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData)
{
  Superclass::ProcessMRMLNodesEvents(caller, event, callData);

  vtkMRMLScene* mrmlScene = this->GetMRMLScene();
  if (!mrmlScene)
  {
    vtkErrorMacro("ProcessMRMLNodesEvents: Invalid MRML scene");
    return;
  }
  if (mrmlScene->IsBatchProcessing())
  {
    return;
  }

  if (caller->IsA("vtkMRMLRTImageNode"))
  {
    vtkMRMLRTImageNode* parameterNode = vtkMRMLRTImageNode::SafeDownCast(caller);

    if (parameterNode && event == vtkCommand::ModifiedEvent)
    {
      this->UpdateMarkupsNodes(parameterNode);
    }
  }
}

//----------------------------------------------------------------------------
void vtkSlicerRtImageLogic::CreateMarkupsNodes(vtkMRMLRTImageNode* rtImageNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene(); 
  if (!scene)
  {
    vtkErrorMacro("CreateMarkupsNodes: Invalid MRML scene");
    return;
  }
  if (!rtImageNode)
  {
    vtkErrorMacro("CreateMarkupsNodes: Invalid RT Image node");
    return;
  }

  vtkMRMLRTBeamNode* beamNode = rtImageNode->GetBeamNode();
  vtkMRMLTransformNode* beamTransformNode = nullptr;
  if (beamNode)
  {
    beamTransformNode = beamNode->GetParentTransformNode();
  }

  // Create markups nodes if they don't exist

  // Imager boundary markups node
  vtkSmartPointer<vtkMRMLMarkupsClosedCurveNode> imagerMarkupsNode;
  if (!scene->GetFirstNodeByName(IMAGER_BOUNDARY_MARKUPS_NODE_NAME))
  {
    imagerMarkupsNode = this->CreateImagerBoundary(rtImageNode);
  }
  else
  {
    imagerMarkupsNode = vtkMRMLMarkupsClosedCurveNode::SafeDownCast(
      scene->GetFirstNodeByName(IMAGER_BOUNDARY_MARKUPS_NODE_NAME));
    // Update imager points using RTImage node data
    if (beamTransformNode)
    {
      imagerMarkupsNode->SetAndObserveTransformNodeID(beamTransformNode->GetID());
    }
  }

  // Image window markups node
  vtkSmartPointer<vtkMRMLMarkupsClosedCurveNode> imageWindowMarkupsNode;
  if (!scene->GetFirstNodeByName(IMAGE_WINDOW_MARKUPS_NODE_NAME))
  {
    imageWindowMarkupsNode = this->CreateImageWindow(rtImageNode);
  }
  else
  {
    imageWindowMarkupsNode = vtkMRMLMarkupsClosedCurveNode::SafeDownCast(
      scene->GetFirstNodeByName(IMAGE_WINDOW_MARKUPS_NODE_NAME));
    // Update image window points using RTImage node data
    if (beamTransformNode)
    {
      imageWindowMarkupsNode->SetAndObserveTransformNodeID(beamTransformNode->GetID());
    }
  }

  // Imager normal vector markups node
  vtkSmartPointer<vtkMRMLMarkupsLineNode> normalVectorMarkupsNode;
  if (!scene->GetFirstNodeByName(NORMAL_VECTOR_MARKUPS_NODE_NAME))
  {
    normalVectorMarkupsNode = this->CreateImagerNormal(rtImageNode);
  }
  else
  {
    normalVectorMarkupsNode = vtkMRMLMarkupsLineNode::SafeDownCast(
      scene->GetFirstNodeByName(NORMAL_VECTOR_MARKUPS_NODE_NAME));
    // Update Normal vector points using RTImage node data
    if (beamTransformNode)
    {
      normalVectorMarkupsNode->SetAndObserveTransformNodeID(beamTransformNode->GetID());
    }
  }

  // Imager vup vector markups node
  vtkSmartPointer<vtkMRMLMarkupsLineNode> vupVectorMarkupsNode;
  if (!scene->GetFirstNodeByName(VUP_VECTOR_MARKUPS_NODE_NAME))
  {
    vupVectorMarkupsNode = this->CreateImagerVUP(rtImageNode);
  }
  else
  {
    vupVectorMarkupsNode = vtkMRMLMarkupsLineNode::SafeDownCast(
      scene->GetFirstNodeByName(VUP_VECTOR_MARKUPS_NODE_NAME));
    // Update VUP vector points using RTImage node data
    if (beamTransformNode)
    {
      vupVectorMarkupsNode->SetAndObserveTransformNodeID(beamTransformNode->GetID());
    }
  }

  // Fiducial markups node
  vtkSmartPointer<vtkMRMLMarkupsFiducialNode> pointsMarkupsNode;
  if (!scene->GetFirstNodeByName(FIDUCIALS_MARKUPS_NODE_NAME))
  {
    pointsMarkupsNode = this->CreateFiducials(rtImageNode);
  }
  else
  {
    pointsMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(
      scene->GetFirstNodeByName(FIDUCIALS_MARKUPS_NODE_NAME));
    // Update fiducial points using RTImage node data
    if (beamTransformNode)
    {
      pointsMarkupsNode->SetAndObserveTransformNodeID(beamTransformNode->GetID());
    }
  }
}

//----------------------------------------------------------------------------
void vtkSlicerRtImageLogic::UpdateMarkupsNodes(vtkMRMLRTImageNode* rtImageNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene(); 
  if (!scene)
  {
    vtkErrorMacro("UpdateMarkupsNodes: Invalid MRML scene");
    return;
  }
  if (!rtImageNode)
  {
    vtkErrorMacro("UpdateMarkupsNodes: Invalid RT Image node");
    return;
  }

  vtkMRMLRTBeamNode* beamNode = rtImageNode->GetBeamNode();
  if (!beamNode)
  {
    vtkErrorMacro("UpdateMarkupsNodes: Invalid beam node");
    return;
  }

  vtkMRMLTransformNode* beamTransformNode = beamNode->GetParentTransformNode();
  if (!beamTransformNode)
  {
    vtkErrorMacro("UpdateMarkupsNodes: Beam transform node is invalid");
    return;
  }

  double distance = rtImageNode->GetIsocenterImagerDistance();
    
  double spacing[2] = {};
  rtImageNode->GetImagerSpacing(spacing);

  int resolution[2] = {};
  rtImageNode->GetImagerResolution(resolution);

  double offset[2] = {};
  rtImageNode->GetImagerCenterOffset(offset);

  int imageWindow[4] = {};
  rtImageNode->GetImageWindow(imageWindow);

  double imagerHalfWidth = spacing[0] * resolution[0] / 2.; // columns
  double imagerHalfHeight = spacing[1] * resolution[1] / 2.; // rows
  double& x = imagerHalfWidth;
  double& y = imagerHalfHeight;

  // Imager boundary markups node
  if (scene->GetFirstNodeByName(IMAGER_BOUNDARY_MARKUPS_NODE_NAME))
  {
    vtkMRMLMarkupsClosedCurveNode* imagerMarkupsNode = vtkMRMLMarkupsClosedCurveNode::SafeDownCast(
      scene->GetFirstNodeByName(IMAGER_BOUNDARY_MARKUPS_NODE_NAME));

    // add points
    vtkVector3d imagerP0( -1. * y + offset[0], x + offset[1], -distance);
    vtkVector3d imagerP1( y + offset[0], x + offset[1], -distance);
    vtkVector3d imagerP2( y + offset[0], -1. * x + offset[1], -distance);
    vtkVector3d imagerP3( -1. * y + offset[0], -1. * x + offset[1], -distance);

    double p[3];
    imagerMarkupsNode->GetNthControlPointPosition( 0, p);
    p[0] = imagerP0.GetX();
    p[1] = imagerP0.GetY();
    p[2] = imagerP0.GetZ();
    imagerMarkupsNode->SetNthControlPointPosition( 0, p[0], p[1], p[2]);

    imagerMarkupsNode->GetNthControlPointPosition( 1, p);
    p[0] = imagerP1.GetX();
    p[1] = imagerP1.GetY();
    p[2] = imagerP1.GetZ();
    imagerMarkupsNode->SetNthControlPointPosition( 1, p[0], p[1], p[2]);

    imagerMarkupsNode->GetNthControlPointPosition( 2, p);
    p[0] = imagerP2.GetX();
    p[1] = imagerP2.GetY();
    p[2] = imagerP2.GetZ();
    imagerMarkupsNode->SetNthControlPointPosition( 2, p[0], p[1], p[2]);

    imagerMarkupsNode->GetNthControlPointPosition( 3, p);
    p[0] = imagerP3.GetX();
    p[1] = imagerP3.GetY();
    p[2] = imagerP3.GetZ();
    imagerMarkupsNode->SetNthControlPointPosition( 3, p[0], p[1], p[2]);

    // Update imager boundary markups transform node if it's changed    
    vtkMRMLTransformNode* markupsTransformNode = imagerMarkupsNode->GetParentTransformNode();

    if (markupsTransformNode)
    {
      imagerMarkupsNode->SetAndObserveTransformNodeID(beamTransformNode->GetID());
    }
  }

  // Image window markups node
  if (scene->GetFirstNodeByName(IMAGE_WINDOW_MARKUPS_NODE_NAME))
  {
    vtkMRMLMarkupsClosedCurveNode* imageWindowMarkupsNode = vtkMRMLMarkupsClosedCurveNode::SafeDownCast(
      scene->GetFirstNodeByName(IMAGE_WINDOW_MARKUPS_NODE_NAME));

    imageWindowMarkupsNode->SetDisplayVisibility(rtImageNode->GetImageWindowFlag());

    // Imager upper left point
    vtkVector3d imagerP0( -1. * y + offset[0], -1. * x + offset[1], -distance);

    double r1 = imagerP0.GetX() + imageWindow[1] * spacing[1];
    double c1 = imagerP0.GetY() + imageWindow[0] * spacing[0];
    double r2 = imagerP0.GetX() + imageWindow[3] * spacing[1];
    double c2 = imagerP0.GetY() + imageWindow[2] * spacing[0];

    // update points
    vtkVector3d imageP0( r1, c1, -distance);
    vtkVector3d imageP1( r1, c2, -distance);
    vtkVector3d imageP2( r2, c2, -distance);
    vtkVector3d imageP3( r2, c1, -distance);

    double p[3];
    imageWindowMarkupsNode->GetNthControlPointPosition( 0, p);
    p[0] = imageP0.GetX();
    p[1] = imageP0.GetY();
    p[2] = imageP0.GetZ();
    imageWindowMarkupsNode->SetNthControlPointPosition( 0, p[0], p[1], p[2]);

    imageWindowMarkupsNode->GetNthControlPointPosition( 1, p);
    p[0] = imageP1.GetX();
    p[1] = imageP1.GetY();
    p[2] = imageP1.GetZ();
    imageWindowMarkupsNode->SetNthControlPointPosition( 1, p[0], p[1], p[2]);

    imageWindowMarkupsNode->GetNthControlPointPosition( 2, p);
    p[0] = imageP2.GetX();
    p[1] = imageP2.GetY();
    p[2] = imageP2.GetZ();
    imageWindowMarkupsNode->SetNthControlPointPosition( 2, p[0], p[1], p[2]);

    imageWindowMarkupsNode->GetNthControlPointPosition( 3, p);
    p[0] = imageP3.GetX();
    p[1] = imageP3.GetY();
    p[2] = imageP3.GetZ();
    imageWindowMarkupsNode->SetNthControlPointPosition( 3, p[0], p[1], p[2]);

    // Update image window markups transform node if it's changed    
    vtkMRMLTransformNode* markupsTransformNode = imageWindowMarkupsNode->GetParentTransformNode();

    if (markupsTransformNode)
    {
      imageWindowMarkupsNode->SetAndObserveTransformNodeID(beamTransformNode->GetID());
    }
  }

  // normal vector markups line node
  if (scene->GetFirstNodeByName(NORMAL_VECTOR_MARKUPS_NODE_NAME))
  {
    vtkMRMLMarkupsLineNode* vectorMarkupsNode = vtkMRMLMarkupsLineNode::SafeDownCast(
      scene->GetFirstNodeByName(NORMAL_VECTOR_MARKUPS_NODE_NAME));

    // update points
    vtkVector3d p0( offset[0], offset[1], -distance);
    vtkVector3d p1( offset[0], offset[1], -distance + 100.);

    double p[3];
    vectorMarkupsNode->GetNthControlPointPosition( 0, p);
    p[0] = p0.GetX();
    p[1] = p0.GetY();
    p[2] = p0.GetZ();
    vectorMarkupsNode->SetNthControlPointPosition( 0, p[0], p[1], p[2]);

    vectorMarkupsNode->GetNthControlPointPosition( 1, p);
    p[0] = p1.GetX();
    p[1] = p1.GetY();
    p[2] = p1.GetZ();
    vectorMarkupsNode->SetNthControlPointPosition( 1, p[0], p[1], p[2]);

    // Update imager normal vector markups transform node if it's changed    
    vtkMRMLTransformNode* markupsTransformNode = vectorMarkupsNode->GetParentTransformNode();

    if (markupsTransformNode)
    {
      vectorMarkupsNode->SetAndObserveTransformNodeID(beamTransformNode->GetID());
    }
  }

  // vup vector markups line node
  if (scene->GetFirstNodeByName(VUP_VECTOR_MARKUPS_NODE_NAME))
  {
    vtkMRMLMarkupsLineNode* vectorMarkupsNode = vtkMRMLMarkupsLineNode::SafeDownCast(
      scene->GetFirstNodeByName(VUP_VECTOR_MARKUPS_NODE_NAME));

    // update points
    vtkVector3d p0( offset[0], offset[1], -distance);
    vtkVector3d p1( -1. * y + offset[0], 0. + offset[1], -distance); // vup

    double p[3];
    vectorMarkupsNode->GetNthControlPointPosition( 0, p);
    p[0] = p0.GetX();
    p[1] = p0.GetY();
    p[2] = p0.GetZ();
    vectorMarkupsNode->SetNthControlPointPosition( 0, p[0], p[1], p[2]);

    vectorMarkupsNode->GetNthControlPointPosition( 1, p);
    p[0] = p1.GetX();
    p[1] = p1.GetY();
    p[2] = p1.GetZ();
    vectorMarkupsNode->SetNthControlPointPosition( 1, p[0], p[1], p[2]);

    // Update VUP VECTOR markups transform node if it's changed    
    vtkMRMLTransformNode* markupsTransformNode = vectorMarkupsNode->GetParentTransformNode();

    if (markupsTransformNode)
    {
      vectorMarkupsNode->SetAndObserveTransformNodeID(beamTransformNode->GetID());
    }
  }

  // fiducial markups line node
  if (scene->GetFirstNodeByName(FIDUCIALS_MARKUPS_NODE_NAME))
  {
    vtkMRMLMarkupsFiducialNode* pointsMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(
      scene->GetFirstNodeByName(FIDUCIALS_MARKUPS_NODE_NAME));

    // update points
    vtkVector3d p0( 0., 0., -distance); // imager center
    vtkVector3d p1( 0., 0., -distance + 100.); // n
    vtkVector3d p2( -1. * y + offset[0], -1. * x + offset[1], -distance); // (0,0)
    vtkVector3d p3( -1. * y + offset[0], 0.0, -distance); // vup

    double p[3];
    pointsMarkupsNode->GetNthControlPointPosition( 0, p);
    p[0] = p0.GetX();
    p[1] = p0.GetY();
    p[2] = p0.GetZ();
    pointsMarkupsNode->SetNthControlPointPosition( 0, p[0], p[1], p[2]);

    pointsMarkupsNode->GetNthControlPointPosition( 1, p);
    p[0] = p1.GetX();
    p[1] = p1.GetY();
    p[2] = p1.GetZ();
    pointsMarkupsNode->SetNthControlPointPosition( 1, p[0], p[1], p[2]);

    pointsMarkupsNode->GetNthControlPointPosition( 2, p);
    p[0] = p2.GetX();
    p[1] = p2.GetY();
    p[2] = p2.GetZ();
    pointsMarkupsNode->SetNthControlPointPosition( 2, p[0], p[1], p[2]);

    pointsMarkupsNode->GetNthControlPointPosition( 3, p);
    p[0] = p3.GetX();
    p[1] = p3.GetY();
    p[2] = p3.GetZ();
    pointsMarkupsNode->SetNthControlPointPosition( 3, p[0], p[1], p[2]);

    // Update fiducials markups transform node if it's changed    
    vtkMRMLTransformNode* markupsTransformNode = pointsMarkupsNode->GetParentTransformNode();

    if (markupsTransformNode)
    {
      pointsMarkupsNode->SetAndObserveTransformNodeID(beamTransformNode->GetID());
    }
  }

}

//----------------------------------------------------------------------------
void vtkSlicerRtImageLogic::ShowMarkupsNodes(bool toggled)
{
  vtkMRMLScene* scene = this->GetMRMLScene(); 
  if (!scene)
  {
    vtkErrorMacro("UpdateMarkupsNodes: Invalid MRML scene");
    return;
  }

  // Imager boundary markups node
  if (scene->GetFirstNodeByName(IMAGER_BOUNDARY_MARKUPS_NODE_NAME))
  {
    vtkMRMLMarkupsClosedCurveNode* imagerMarkupsNode = vtkMRMLMarkupsClosedCurveNode::SafeDownCast(
      scene->GetFirstNodeByName(IMAGER_BOUNDARY_MARKUPS_NODE_NAME));
    imagerMarkupsNode->SetDisplayVisibility(int(toggled));
  }

  // Image window markups node
  if (scene->GetFirstNodeByName(IMAGE_WINDOW_MARKUPS_NODE_NAME))
  {
    vtkMRMLMarkupsClosedCurveNode* imageWindowMarkupsNode = vtkMRMLMarkupsClosedCurveNode::SafeDownCast(
      scene->GetFirstNodeByName(IMAGE_WINDOW_MARKUPS_NODE_NAME));
    imageWindowMarkupsNode->SetDisplayVisibility(int(toggled));
  }

  // normal vector markups line node
  if (scene->GetFirstNodeByName(NORMAL_VECTOR_MARKUPS_NODE_NAME))
  {
    vtkMRMLMarkupsLineNode* vectorMarkupsNode = vtkMRMLMarkupsLineNode::SafeDownCast(
      scene->GetFirstNodeByName(NORMAL_VECTOR_MARKUPS_NODE_NAME));
    vectorMarkupsNode->SetDisplayVisibility(int(toggled));
  }

  // vup vector markups line node
  if (scene->GetFirstNodeByName(VUP_VECTOR_MARKUPS_NODE_NAME))
  {
    vtkMRMLMarkupsLineNode* vectorMarkupsNode = vtkMRMLMarkupsLineNode::SafeDownCast(
      scene->GetFirstNodeByName(VUP_VECTOR_MARKUPS_NODE_NAME));
    vectorMarkupsNode->SetDisplayVisibility(int(toggled));
  }

  // fiducial markups line node
  if (scene->GetFirstNodeByName(FIDUCIALS_MARKUPS_NODE_NAME))
  {
    vtkMRMLMarkupsFiducialNode* pointsMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(
      scene->GetFirstNodeByName(FIDUCIALS_MARKUPS_NODE_NAME));
    pointsMarkupsNode->SetDisplayVisibility(int(toggled));
  }
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsClosedCurveNode* vtkSlicerRtImageLogic::CreateImagerBoundary(vtkMRMLRTImageNode* rtImageNode)
{
  vtkNew<vtkMRMLMarkupsClosedCurveNode> imagerMarkupsNode;
  this->GetMRMLScene()->AddNode(imagerMarkupsNode);
  imagerMarkupsNode->SetName(IMAGER_BOUNDARY_MARKUPS_NODE_NAME);
  imagerMarkupsNode->SetCurveTypeToLinear();
  imagerMarkupsNode->SetHideFromEditors(1);
  std::string singletonTag = std::string("RTIMAGE_") + IMAGER_BOUNDARY_MARKUPS_NODE_NAME;
  imagerMarkupsNode->SetSingletonTag(singletonTag.c_str());

  if (rtImageNode)
  {
    double distance = rtImageNode->GetIsocenterImagerDistance();
     
    double spacing[2] = {};
    rtImageNode->GetImagerSpacing(spacing);

    int resolution[2] = {};
    rtImageNode->GetImagerResolution(resolution);

    double offset[2] = {};
    rtImageNode->GetImagerCenterOffset(offset);

    double imagerHalfWidth = spacing[0] * resolution[0] / 2.; // columns
    double imagerHalfHeight = spacing[1] * resolution[1] / 2.; // rows

    double& x = imagerHalfWidth;
    double& y = imagerHalfHeight;

    // add points
    vtkVector3d imagerP0( -1. * y + offset[0], x + offset[1], -distance);
    vtkVector3d imagerP1( y + offset[0], x + offset[1], -distance);
    vtkVector3d imagerP2( y + offset[0], -1. * x + offset[1], -distance);
    vtkVector3d imagerP3( -1. * y + offset[0], -1. * x + offset[1], -distance);

    imagerMarkupsNode->AddControlPoint(imagerP0); // "Upper Left", "-x,y"
    imagerMarkupsNode->AddControlPoint(imagerP1); // "Upper Right", "x,y"
    imagerMarkupsNode->AddControlPoint(imagerP2); // "Lower Right", "x,-y"
    imagerMarkupsNode->AddControlPoint(imagerP3); // "Lower Left", "-x,-y"

    if (vtkMRMLRTBeamNode* beamNode = rtImageNode->GetBeamNode())
    {
      vtkMRMLTransformNode* beamTransformNode = beamNode->GetParentTransformNode();
      if (beamTransformNode)
      {
        imagerMarkupsNode->SetAndObserveTransformNodeID(beamTransformNode->GetID());
      }
    }
  }
  return imagerMarkupsNode;
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsClosedCurveNode* vtkSlicerRtImageLogic::CreateImageWindow(vtkMRMLRTImageNode* rtImageNode)
{
  vtkNew<vtkMRMLMarkupsClosedCurveNode> imageWindowMarkupsNode;
  this->GetMRMLScene()->AddNode(imageWindowMarkupsNode);
  imageWindowMarkupsNode->SetName(IMAGE_WINDOW_MARKUPS_NODE_NAME);
  imageWindowMarkupsNode->SetCurveTypeToLinear();
  imageWindowMarkupsNode->SetHideFromEditors(1);
  std::string singletonTag = std::string("RTIMAGE_") + IMAGE_WINDOW_MARKUPS_NODE_NAME;
  imageWindowMarkupsNode->SetSingletonTag(singletonTag.c_str());

  if (rtImageNode)
  {
    double distance = rtImageNode->GetIsocenterImagerDistance();
     
    double spacing[2] = {};
    rtImageNode->GetImagerSpacing(spacing);

    int resolution[2] = {};
    rtImageNode->GetImagerResolution(resolution);

    double offset[2] = {};
    rtImageNode->GetImagerCenterOffset(offset);

    int imageWindow[4] = {};
    rtImageNode->GetImageWindow(imageWindow);

    double imagerHalfWidth = spacing[0] * resolution[0] / 2.; // columns
    double imagerHalfHeight = spacing[1] * resolution[1] / 2.; // rows

    double& x = imagerHalfWidth;
    double& y = imagerHalfHeight;

    // Imager upper left point
    vtkVector3d imagerP0( -1. * y + offset[0], -1. * x + offset[1], -distance);

    double r1 = imagerP0.GetX() + imageWindow[1] * spacing[1];
    double c1 = imagerP0.GetY() + imageWindow[0] * spacing[0];
    double r2 = imagerP0.GetX() + imageWindow[3] * spacing[1];
    double c2 = imagerP0.GetY() + imageWindow[2] * spacing[0];

    // add points
    vtkVector3d imageP0( r1, c1, -distance);
    vtkVector3d imageP1( r1, c2, -distance);
    vtkVector3d imageP2( r2, c2, -distance);
    vtkVector3d imageP3( r2, c1, -distance);

    imageWindowMarkupsNode->AddControlPoint(imageP0); // r1, c1
    imageWindowMarkupsNode->AddControlPoint(imageP1); // r2, c1
    imageWindowMarkupsNode->AddControlPoint(imageP2); // r2, c2
    imageWindowMarkupsNode->AddControlPoint(imageP3); // r1, c2

    if (vtkMRMLRTBeamNode* beamNode = rtImageNode->GetBeamNode())
    {
      vtkMRMLTransformNode* beamTransformNode = beamNode->GetParentTransformNode();
      if (beamTransformNode)
      {
        imageWindowMarkupsNode->SetAndObserveTransformNodeID(beamTransformNode->GetID());
      }
    }
  }
  return imageWindowMarkupsNode;
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsLineNode* vtkSlicerRtImageLogic::CreateImagerNormal(vtkMRMLRTImageNode* rtImageNode)
{
  vtkNew<vtkMRMLMarkupsLineNode> vectorMarkupsNode;
  this->GetMRMLScene()->AddNode(vectorMarkupsNode);
  vectorMarkupsNode->SetName(NORMAL_VECTOR_MARKUPS_NODE_NAME);
  vectorMarkupsNode->SetHideFromEditors(1);
  std::string singletonTag = std::string("RTIMAGE_") + NORMAL_VECTOR_MARKUPS_NODE_NAME;
  vectorMarkupsNode->SetSingletonTag(singletonTag.c_str());

  if (rtImageNode)
  {
    double distance = rtImageNode->GetIsocenterImagerDistance();

    // add points
    vtkVector3d p0( 0, 0, -distance);
    vtkVector3d p1( 0, 0, -distance + 100.);

    vectorMarkupsNode->AddControlPoint(p0);
    vectorMarkupsNode->AddControlPoint(p1);

    if (vtkMRMLRTBeamNode* beamNode = rtImageNode->GetBeamNode())
    {
      vtkMRMLTransformNode* beamTransformNode = beamNode->GetParentTransformNode();
      if (beamTransformNode)
      {
        vectorMarkupsNode->SetAndObserveTransformNodeID(beamTransformNode->GetID());
      }
    }
  }
  return vectorMarkupsNode;
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsLineNode* vtkSlicerRtImageLogic::CreateImagerVUP(vtkMRMLRTImageNode* rtImageNode)
{
  vtkNew<vtkMRMLMarkupsLineNode> vectorMarkupsNode;
  this->GetMRMLScene()->AddNode(vectorMarkupsNode);
  vectorMarkupsNode->SetName(VUP_VECTOR_MARKUPS_NODE_NAME);
  vectorMarkupsNode->SetHideFromEditors(1);
  std::string singletonTag = std::string("RTIMAGE_") + VUP_VECTOR_MARKUPS_NODE_NAME;
  vectorMarkupsNode->SetSingletonTag(singletonTag.c_str());

  if (rtImageNode)
  {
    double distance = rtImageNode->GetIsocenterImagerDistance();
     
    double spacing[2] = {};
    rtImageNode->GetImagerSpacing(spacing);

    int resolution[2] = {};
    rtImageNode->GetImagerResolution(resolution);

    double offset[2] = {};
    rtImageNode->GetImagerCenterOffset(offset);

    double imagerHalfHeight = spacing[1] * resolution[1] / 2.; // rows
    double& y = imagerHalfHeight;

    // add points
    vtkVector3d p0( 0. + offset[0], 0. + offset[1], -distance);
    vtkVector3d p1( -1. * y + offset[0], 0. + offset[1], -distance); // vup

    vectorMarkupsNode->AddControlPoint(p0);
    vectorMarkupsNode->AddControlPoint(p1);

    if (vtkMRMLRTBeamNode* beamNode = rtImageNode->GetBeamNode())
    {
      vtkMRMLTransformNode* beamTransformNode = beamNode->GetParentTransformNode();
      if (beamTransformNode)
      {
        vectorMarkupsNode->SetAndObserveTransformNodeID(beamTransformNode->GetID());
      }
    }
  }
  return vectorMarkupsNode;
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsFiducialNode* vtkSlicerRtImageLogic::CreateFiducials(vtkMRMLRTImageNode* rtImageNode)
{
  vtkNew<vtkMRMLMarkupsFiducialNode> pointsMarkupsNode;
  this->GetMRMLScene()->AddNode(pointsMarkupsNode);
  pointsMarkupsNode->SetName(FIDUCIALS_MARKUPS_NODE_NAME);
  pointsMarkupsNode->SetHideFromEditors(1);
  std::string singletonTag = std::string("RTIMAGE_") + FIDUCIALS_MARKUPS_NODE_NAME;
  pointsMarkupsNode->SetSingletonTag(singletonTag.c_str());

  if (rtImageNode)
  {
    double distance = rtImageNode->GetIsocenterImagerDistance();
     
    double spacing[2] = {};
    rtImageNode->GetImagerSpacing(spacing);

    int resolution[2] = {};
    rtImageNode->GetImagerResolution(resolution);

    double offset[2] = {};
    rtImageNode->GetImagerCenterOffset(offset);

    double imagerHalfWidth = spacing[0] * resolution[0] / 2.; // columns
    double imagerHalfHeight = spacing[1] * resolution[1] / 2.; // rows

    double& x = imagerHalfWidth;
    double& y = imagerHalfHeight;

    // add points
    vtkVector3d p0( 0., 0., -distance); // imager center
    vtkVector3d p1( 0., 0., -distance + 100.); // n
    vtkVector3d p2( -1. * y + offset[0], -1. * x + offset[1], -distance); // (0,0)
    vtkVector3d p3( -1. * y + offset[0], 0.0, -distance); // vup

    pointsMarkupsNode->AddControlPoint( p0, "Imager center");
    pointsMarkupsNode->AddControlPoint( p1, "n");
    pointsMarkupsNode->AddControlPoint( p2, "(0,0)");
    pointsMarkupsNode->AddControlPoint( p3, "VUP");

    if (vtkMRMLRTBeamNode* beamNode = rtImageNode->GetBeamNode())
    {
      vtkMRMLTransformNode* beamTransformNode = beamNode->GetParentTransformNode();
      if (beamTransformNode)
      {
        pointsMarkupsNode->SetAndObserveTransformNodeID(beamTransformNode->GetID());
      }
    }
  }
  return pointsMarkupsNode;
}

//------------------------------------------------------------------------------
bool vtkSlicerRtImageLogic::ComputePlastimatchDRR( vtkMRMLRTImageNode* parameterNode, 
  vtkMRMLScalarVolumeNode* ctVolumeNode)
{
  vtkMRMLScene* scene = this->GetMRMLScene(); 
  if (!scene)
  {
    vtkErrorMacro("ComputePlastimatchDRR: Invalid MRML scene");
    return false;
  }

  if (!parameterNode)
  {
    vtkErrorMacro("ComputePlastimatchDRR: Invalid parameter node");
    return false;
  }

  vtkMRMLRTBeamNode* beamNode = parameterNode->GetBeamNode();
  if (!beamNode)
  {
    vtkErrorMacro("ComputePlastimatchDRR: Invalid RT Beam node");
    return false;
  }

  if (!ctVolumeNode)
  {
    vtkErrorMacro("ComputePlastimatchDRR: Invalid input CT volume node");
    return false;
  }

  if (!this->PlastimatchDRRComputationLogic)
  {
    vtkErrorMacro("ComputePlastimatchDRR: slicer_plastimatch_drr logic is not set");
    return false;
  }

  vtkMRMLCommandLineModuleNode* cmdNode = this->PlastimatchDRRComputationLogic->CreateNodeInScene();
  if (!cmdNode)
  {
    vtkErrorMacro("ComputePlastimatchDRR: failed to create CLI module node");
    return false;
  }

  // Create node for the DRR image volume
  vtkNew<vtkMRMLScalarVolumeNode> drrVolumeNode;
  scene->AddNode(drrVolumeNode);

  cmdNode->SetParameterAsString( "inputVolume", ctVolumeNode->GetID());
  cmdNode->SetParameterAsString( "outputVolume", drrVolumeNode->GetID());
  cmdNode->SetParameterAsDouble( "sourceAxisDistance", beamNode->GetSAD());
  cmdNode->SetParameterAsDouble( "sourceImagerDistance", beamNode->GetSAD() + parameterNode->GetIsocenterImagerDistance());

  // Fill CLI cmd node data
  std::stringstream vupStream;
  double vup[4] = { -1., 0., 0., 0 };
  parameterNode->GetViewUpVector(vup);
  vupStream << vup[0] << "," << vup[1] << "," << vup[2];
  cmdNode->SetParameterAsString( "viewUpVector", vupStream.str());

  std::stringstream normalStream;
  double n[4] = { 0., 0., 1., 0 };
  parameterNode->GetNormalVector(n);
  normalStream << n[0] << "," << n[1] << "," << n[2];
  cmdNode->SetParameterAsString( "normalVector", normalStream.str());

  std::stringstream isocenterStream;
  double isocenter[3] = {};
  parameterNode->GetIsocenterPositionLPS(isocenter);
  isocenterStream << isocenter[0] << "," << isocenter[1] << "," << isocenter[2];
  cmdNode->SetParameterAsString( "isocenterPosition", isocenterStream.str());
  
  std::stringstream imagerResolutionStream;
  int imagerResolution[2] = { 1024, 768 };
  parameterNode->GetImagerResolution(imagerResolution);
  imagerResolutionStream << imagerResolution[0] << "," << imagerResolution[1];
  cmdNode->SetParameterAsString( "imagerResolution", imagerResolutionStream.str());

  std::stringstream imagerSpacingStream;
  double imagerSpacing[2] = { 0.25, 0.25 };
  parameterNode->GetImagerSpacing(imagerSpacing);
  imagerSpacingStream << imagerSpacing[0] << "," << imagerSpacing[1];
  cmdNode->SetParameterAsString( "imagerSpacing", imagerSpacingStream.str());

  cmdNode->SetParameterAsBool( "useImageWindow", parameterNode->GetImageWindowFlag());
  if (parameterNode->GetImageWindowFlag())
  {
    std::stringstream imageWindowStream;
    int imageWindow[4] = { 0, 0, 1023, 767 };
    parameterNode->GetImageWindow(imageWindow);
    imageWindowStream << imageWindow[0] << "," << imageWindow[1] << "," << imageWindow[2] << "," << imageWindow[3];
    cmdNode->SetParameterAsString( "imageWindow", imageWindowStream.str());
  }

  cmdNode->SetParameterAsBool( "autoscale", parameterNode->GetAutoscaleFlag());
  
  std::stringstream autoscaleRangeStream;
  float autoscaleRange[2] = { 0., 255. };
  parameterNode->GetAutoscaleRange(autoscaleRange);
  autoscaleRangeStream << autoscaleRange[0] << "," << autoscaleRange[1];
  cmdNode->SetParameterAsString( "autoscaleRange", autoscaleRangeStream.str());

  cmdNode->SetParameterAsBool( "exponentialMapping", parameterNode->GetExponentialMappingFlag());
  
  std::string threadingString = "cpu";
  switch (parameterNode->GetThreading())
  {
  case vtkMRMLRTImageNode::PlastimatchThreadingType::CPU:
    threadingString = "cpu";
    break;
  case vtkMRMLRTImageNode::PlastimatchThreadingType::CUDA:
    threadingString = "cuda";
    break;
  case vtkMRMLRTImageNode::PlastimatchThreadingType::OPENCL:
    threadingString = "opencl";
    break;
  default:
    break;
  }
  cmdNode->SetParameterAsString( "threading", threadingString);

  std::string huconversionString = "preprocess";
  switch (parameterNode->GetHUConversion())
  {
  case vtkMRMLRTImageNode::PlastimatchHounsfieldUnitsConversionType::INLINE:
    huconversionString = "inline";
    break;
  case vtkMRMLRTImageNode::PlastimatchHounsfieldUnitsConversionType::PREPROCESS:
    huconversionString = "preprocess";
    break;
  case vtkMRMLRTImageNode::PlastimatchHounsfieldUnitsConversionType::NONE:
    huconversionString = "none";
    break;
  default:
    break;
  }
  cmdNode->SetParameterAsString( "huconversion", huconversionString);

  std::string algorithmString = "exact";
  switch (parameterNode->GetAlgorithmReconstuction())
  {
  case vtkMRMLRTImageNode::PlastimatchAlgorithmReconstuctionType::EXACT:
    algorithmString = "exact";
    break;
  case vtkMRMLRTImageNode::PlastimatchAlgorithmReconstuctionType::UNIFORM:
    algorithmString = "uniform";
    break;
  default:
    break;
  }
  cmdNode->SetParameterAsString( "algorithm", algorithmString);
  cmdNode->SetParameterAsBool( "invertIntensity", true);
  cmdNode->SetParameterAsString( "outputFormat", "raw");

  this->PlastimatchDRRComputationLogic->ApplyAndWait( cmdNode, false);

  scene->RemoveNode(cmdNode);
  // success?

  return this->SetupDisplayAndSubjectHierarchyNodes( parameterNode, drrVolumeNode);
}

//------------------------------------------------------------------------------
bool vtkSlicerRtImageLogic::SetupDisplayAndSubjectHierarchyNodes( vtkMRMLRTImageNode* parameterNode, 
  vtkMRMLScalarVolumeNode* drrVolumeNode)
{
  vtkMRMLRTBeamNode* beamNode = parameterNode->GetBeamNode();

  vtkMRMLSubjectHierarchyNode* shNode = vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode(this->GetMRMLScene());
  if (!shNode)
  {
    vtkErrorMacro("SetupDisplayAndSubjectHierarchyNodes: Failed to access subject hierarchy node");
    return false;
  }

  // Create display node for the volume
  vtkNew<vtkMRMLScalarVolumeDisplayNode> volumeDisplayNode;
  this->GetMRMLScene()->AddNode(volumeDisplayNode);
  volumeDisplayNode->SetDefaultColorMap();

  float autoscaleRange[2] = { 0.f, 255.f };
  parameterNode->GetAutoscaleRange(autoscaleRange);
  
  // TODO: add manual level setting
  volumeDisplayNode->AutoWindowLevelOn();

  drrVolumeNode->SetAndObserveDisplayNodeID(volumeDisplayNode->GetID());

  // Set up subject hierarchy item
  vtkIdType rtImageVolumeShItemID = shNode->CreateItem( shNode->GetSceneItemID(), drrVolumeNode);

  double sid = beamNode->GetSAD() + parameterNode->GetIsocenterImagerDistance();
  // Set RT image specific attributes
  shNode->SetItemAttribute(rtImageVolumeShItemID, vtkSlicerRtCommon::DICOMRTIMPORT_RTIMAGE_IDENTIFIER_ATTRIBUTE_NAME, "1");
  shNode->SetItemAttribute(rtImageVolumeShItemID, vtkMRMLSubjectHierarchyConstants::GetDICOMReferencedInstanceUIDsAttributeName(), "");
  shNode->SetItemAttribute(rtImageVolumeShItemID, vtkSlicerRtCommon::DICOMRTIMPORT_SOURCE_AXIS_DISTANCE_ATTRIBUTE_NAME, std::to_string(beamNode->GetSAD()));
  shNode->SetItemAttribute(rtImageVolumeShItemID, vtkSlicerRtCommon::DICOMRTIMPORT_GANTRY_ANGLE_ATTRIBUTE_NAME, std::to_string(beamNode->GetGantryAngle()));
  shNode->SetItemAttribute(rtImageVolumeShItemID, vtkSlicerRtCommon::DICOMRTIMPORT_COUCH_ANGLE_ATTRIBUTE_NAME, std::to_string(beamNode->GetCouchAngle()));
  shNode->SetItemAttribute(rtImageVolumeShItemID, vtkSlicerRtCommon::DICOMRTIMPORT_COLLIMATOR_ANGLE_ATTRIBUTE_NAME, std::to_string(beamNode->GetCollimatorAngle()));
  shNode->SetItemAttribute(rtImageVolumeShItemID, vtkSlicerRtCommon::DICOMRTIMPORT_BEAM_NUMBER_ATTRIBUTE_NAME, std::to_string(beamNode->GetBeamNumber()));
  shNode->SetItemAttribute(rtImageVolumeShItemID, vtkSlicerRtCommon::DICOMRTIMPORT_RTIMAGE_SID_ATTRIBUTE_NAME, std::to_string(sid));

  double rtImagePosition[2] = {};
  parameterNode->GetRTImagePosition(rtImagePosition);
  std::string rtImagePositionString = std::to_string(rtImagePosition[0]) + std::string(" ") + std::to_string(rtImagePosition[1]);
  shNode->SetItemAttribute(rtImageVolumeShItemID, vtkSlicerRtCommon::DICOMRTIMPORT_RTIMAGE_POSITION_ATTRIBUTE_NAME, rtImagePositionString);

  // Compute and set RT image geometry. Uses the referenced beam 
  return this->SetupGeometry( parameterNode, drrVolumeNode);
}

//------------------------------------------------------------------------------
bool vtkSlicerRtImageLogic::SetupGeometry( vtkMRMLRTImageNode* parameterNode, vtkMRMLScalarVolumeNode* drrVolumeNode)
{
  vtkMRMLRTBeamNode* beamNode = parameterNode->GetBeamNode();

  vtkMRMLSubjectHierarchyNode* shNode = vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode(this->GetMRMLScene());

  // Get RT plan for beam
  vtkMRMLRTPlanNode *planNode = beamNode->GetParentPlanNode();
  if (!planNode)
  {
    vtkErrorMacro("SetupGeometry: Failed to retrieve valid plan node for beam '" << beamNode->GetName() << "'");
    return false;
  }
  vtkIdType planShItemID = planNode->GetPlanSubjectHierarchyItemID();
  if (planShItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
  {
    vtkErrorMacro("SetupGeometry: Failed to retrieve valid plan subject hierarchy item for beam '" << beamNode->GetName() << "'");
    return false;
  }
  std::string rtPlanSopInstanceUid = shNode->GetItemUID(planShItemID, vtkMRMLSubjectHierarchyConstants::GetDICOMInstanceUIDName());
  if (rtPlanSopInstanceUid.empty())
  {
    vtkWarningMacro("SetupGeometry: Failed to get RT Plan DICOM UID for beam '" << beamNode->GetName() << "'");
  }

  // Return if a referenced displayed model is present for the RT image, because it means that the geometry has been set up successfully before
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(
    drrVolumeNode->GetNodeReference(vtkMRMLPlanarImageNode::PLANARIMAGE_DISPLAYED_MODEL_REFERENCE_ROLE.c_str()) );
  if (modelNode)
  {
    vtkWarningMacro("SetupGeometry: RT image '" << drrVolumeNode->GetName() << "' belonging to beam '" << beamNode->GetName() << "' seems to have been set up already.");
    return false;
  }

  // Set more user friendly DRR image name
  std::string drrName = std::string("DRR ") + std::to_string(PlastimatchDRRImageIndex++) + std::string(" : ") + std::string(beamNode->GetName());
  drrVolumeNode->SetName(drrName.c_str());

  // Extract beam-related parameters needed to compute RT image coordinate system
  double sourceAxisDistance = beamNode->GetSAD();
  double gantryAngle = beamNode->GetGantryAngle();
  double couchAngle = beamNode->GetCouchAngle();

  // Source to RT image plane distance (along beam axis)
  double rtImageSid = sourceAxisDistance + parameterNode->GetIsocenterImagerDistance();

  // RT image position (the x and y coordinates (in mm) of the upper left hand corner of the image, in the IEC X-RAY IMAGE RECEPTOR coordinate system)
  double rtImagePosition[2] = {};
  parameterNode->GetRTImagePosition(rtImagePosition);

//  int window[4];
//  parameterNode->GetImageWindow(window);
//  double imagerSpacing[2];
//  parameterNode->GetImagerSpacing(imagerSpacing);

  // Get isocenter coordinates
  double isocenterWorldCoordinates[3] = {};
  if (!beamNode->GetPlanIsocenterPosition(isocenterWorldCoordinates))
  {
    vtkErrorMacro("SetupGeometry: Failed to get plan isocenter position");
    return false;
  }

  // Assemble transform from isocenter IEC to RT image RAS
  vtkNew<vtkTransform> fixedToIsocenterTransform;
  fixedToIsocenterTransform->Identity();
  fixedToIsocenterTransform->Translate(isocenterWorldCoordinates);

  vtkNew<vtkTransform> couchToFixedTransform;
  couchToFixedTransform->Identity();
  couchToFixedTransform->RotateWXYZ(couchAngle, 0.0, 1.0, 0.0);

  vtkNew<vtkTransform> gantryToCouchTransform;
  gantryToCouchTransform->Identity();
  gantryToCouchTransform->RotateWXYZ(gantryAngle, 0.0, 0.0, 1.0);

  vtkNew<vtkTransform> sourceToGantryTransform;
  sourceToGantryTransform->Identity();
  sourceToGantryTransform->Translate(0.0, sourceAxisDistance, 0.0);

  vtkNew<vtkTransform> rtImageToSourceTransform;
  rtImageToSourceTransform->Identity();
  rtImageToSourceTransform->Translate(0.0, -1. * rtImageSid, 0.0);

  vtkNew<vtkTransform> rtImageCenterToCornerTransform;
  rtImageCenterToCornerTransform->Identity();
  rtImageCenterToCornerTransform->Translate( -1. * rtImagePosition[0], 0.0, rtImagePosition[1]);

  // Create isocenter to RAS transform
  // The transformation below is based section C.8.8 in DICOM standard volume 3:
  // "Note: IEC document 62C/269/CDV 'Amendment to IEC 61217: Radiotherapy Equipment -
  //  Coordinates, movements and scales' also defines a patient-based coordinate system, and
  //  specifies the relationship between the DICOM Patient Coordinate System (see Section
  //  C.7.6.2.1.1) and the IEC PATIENT Coordinate System. Rotating the IEC PATIENT Coordinate
  //  System described in IEC 62C/269/CDV (1999) by 90 degrees counter-clockwise (in the negative
  //  direction) about the x-axis yields the DICOM Patient Coordinate System, i.e. (XDICOM, YDICOM,
  //  ZDICOM) = (XIEC, -ZIEC, YIEC). Refer to the latest IEC documentation for the current definition of the
  //  IEC PATIENT Coordinate System."
  // The IJK to RAS transform already contains the LPS to RAS conversion, so we only need to consider this rotation
  vtkNew<vtkTransform> iecToLpsTransform;
  iecToLpsTransform->Identity();
  iecToLpsTransform->RotateX(90.0);
  iecToLpsTransform->RotateZ(-90.0);

  // Get RT image IJK to RAS matrix (containing the spacing and the LPS-RAS conversion)
  vtkNew<vtkMatrix4x4> rtImageIjkToRtImageRasTransformMatrix;
  drrVolumeNode->GetIJKToRASMatrix(rtImageIjkToRtImageRasTransformMatrix);
  vtkNew<vtkTransform> rtImageIjkToRtImageRasTransform;
  rtImageIjkToRtImageRasTransform->SetMatrix(rtImageIjkToRtImageRasTransformMatrix);

  // Concatenate the transform components
  vtkNew<vtkTransform> isocenterToRtImageRas;
  isocenterToRtImageRas->Identity();
  isocenterToRtImageRas->PreMultiply();
  isocenterToRtImageRas->Concatenate(fixedToIsocenterTransform);
  isocenterToRtImageRas->Concatenate(couchToFixedTransform);
  isocenterToRtImageRas->Concatenate(gantryToCouchTransform);
  isocenterToRtImageRas->Concatenate(sourceToGantryTransform);
  isocenterToRtImageRas->Concatenate(rtImageToSourceTransform);
  isocenterToRtImageRas->Concatenate(rtImageCenterToCornerTransform);
  isocenterToRtImageRas->Concatenate(iecToLpsTransform); // LPS = IJK
  isocenterToRtImageRas->Concatenate(rtImageIjkToRtImageRasTransformMatrix);

  // Transform RT image to proper position and orientation
  drrVolumeNode->SetIJKToRASMatrix(isocenterToRtImageRas->GetMatrix());

  // Set up outputs for the planar image display
  vtkNew<vtkMRMLModelNode> displayedModelNode;
  this->GetMRMLScene()->AddNode(displayedModelNode);
  std::string displayedModelNodeName = vtkMRMLPlanarImageNode::PLANARIMAGE_MODEL_NODE_NAME_PREFIX + std::string(drrVolumeNode->GetName());
  displayedModelNode->SetName(displayedModelNodeName.c_str());
  displayedModelNode->SetAttribute(vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyExcludeFromTreeAttributeName().c_str(), "1");

  // Create PlanarImage parameter set node
  std::string planarImageParameterSetNodeName;
  planarImageParameterSetNodeName = this->GetMRMLScene()->GenerateUniqueName(
    vtkMRMLPlanarImageNode::PLANARIMAGE_PARAMETER_SET_BASE_NAME_PREFIX + std::string(drrVolumeNode->GetName()) );
  vtkNew<vtkMRMLPlanarImageNode> planarImageParameterSetNode;
  planarImageParameterSetNode->SetName(planarImageParameterSetNodeName.c_str());
  this->GetMRMLScene()->AddNode(planarImageParameterSetNode);
  planarImageParameterSetNode->SetAndObserveRtImageVolumeNode(drrVolumeNode);
  planarImageParameterSetNode->SetAndObserveDisplayedModelNode(displayedModelNode);

  // Create planar image model for the RT image
  this->PlanarImageLogic->CreateModelForPlanarImage(planarImageParameterSetNode);

  // Show the displayed planar image model by default
  displayedModelNode->SetDisplayVisibility(1);

  return true;
}

//------------------------------------------------------------------------------
void vtkSlicerRtImageLogic::SetPlanarImageLogic(vtkSlicerPlanarImageModuleLogic* planarImageLogic)
{
  this->PlanarImageLogic = planarImageLogic;
}

//------------------------------------------------------------------------------
void vtkSlicerRtImageLogic::SetDRRComputationLogic(vtkSlicerCLIModuleLogic* plastimatchDrrLogic)
{
  this->PlastimatchDRRComputationLogic = plastimatchDrrLogic;
}
