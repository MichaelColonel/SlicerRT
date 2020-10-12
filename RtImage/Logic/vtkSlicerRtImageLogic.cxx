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

#include <vtkMRMLMarkupsPlaneNode.h>
#include <vtkMRMLMarkupsClosedCurveNode.h>
#include <vtkMRMLMarkupsFiducialNode.h>
#include <vtkMRMLMarkupsLineNode.h>

// SlicerRT MRML includes
#include <vtkMRMLRTBeamNode.h>
#include <vtkMRMLRTPlanNode.h>
#include <vtkMRMLRTImageNode.h>

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

const char* vtkSlicerRtImageLogic::IMAGER_BOUNDARY_MARKUPS_NODE_NAME = "ImagerBoundary"; // plane
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
  PlastimatchDRRComputationLogic(nullptr)
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

  if (node->IsA("vtkMRMLRTBeamNode"))
  {
    // Observe beam events
    vtkSmartPointer<vtkIntArray> events = vtkSmartPointer<vtkIntArray>::New();
    events->InsertNextValue(vtkMRMLRTBeamNode::BeamTransformModified);
    vtkObserveMRMLNodeEventsMacro(node, events);
  }
  else if (node->IsA("vtkMRMLRTPlanNode"))
  {
    vtkSmartPointer<vtkIntArray> events = vtkSmartPointer<vtkIntArray>::New();
    events->InsertNextValue(vtkMRMLRTPlanNode::IsocenterModifiedEvent);
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

  if (caller->IsA("vtkMRMLRTBeamNode"))
  {
    vtkMRMLRTBeamNode* beamNode = vtkMRMLRTBeamNode::SafeDownCast(caller);
    if (event == vtkMRMLRTBeamNode::BeamTransformModified)
    {
      // Make sure transform node exists
//      beamNode->CreateDefaultTransformNode();
      // Calculate transform from beam parameters and isocenter from plan
//      this->UpdateTransformForBeam(beamNode);
    }
  }
  else if (caller->IsA("vtkMRMLRTPlanNode"))
  {
    vtkMRMLRTPlanNode* planNode = vtkMRMLRTPlanNode::SafeDownCast(caller);

    if (event == vtkMRMLRTPlanNode::IsocenterModifiedEvent)
    {
      // Get added beam node
/*      char* beamNodeID = reinterpret_cast<char*>(callData);
      if (!beamNodeID)
      {
        vtkErrorMacro("ProcessMRMLNodesEvents: No beam node ID for beam added event in plan " << planNode->GetName());
        return;
      }
      vtkMRMLRTBeamNode* beamNode = vtkMRMLRTBeamNode::SafeDownCast(mrmlScene->GetNodeByID(beamNodeID));
      if (!beamNode)
      {
        vtkErrorMacro("ProcessMRMLNodesEvents: Failed to get added beam node by ID " << beamNodeID);
        return;
      }

      // Make sure transform node exists
      beamNode->CreateDefaultTransformNode();
      // Calculate transform from beam parameters and isocenter from plan
      this->UpdateTransformForBeam(beamNode);

      // Make sure display is set up
      beamNode->UpdateGeometry();
*/
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

  vtkMRMLRTBeamNode* beamNode = rtImageNode->GetBeamNode();
  vtkMRMLTransformNode* beamTransformNode = nullptr;
  if (beamNode)
  {
    beamTransformNode = beamNode->GetParentTransformNode();
  }

  // Create markups nodes if they don't exist

  // Imager boundary markups node
  vtkSmartPointer<vtkMRMLMarkupsPlaneNode> imagerMarkupsNode;
  if (!scene->GetFirstNodeByName(IMAGER_BOUNDARY_MARKUPS_NODE_NAME))
  {
    imagerMarkupsNode = this->CreateImagerBoundary(rtImageNode);
  }
  else
  {
    imagerMarkupsNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(
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
    vtkErrorMacro("UpdateMarkupsNodes: Invalid RTImage node");
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

  // Imager boundary markups node
  if (scene->GetFirstNodeByName(IMAGER_BOUNDARY_MARKUPS_NODE_NAME))
  {
    vtkMRMLMarkupsPlaneNode* imagerMarkupsNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(
      scene->GetFirstNodeByName(IMAGER_BOUNDARY_MARKUPS_NODE_NAME));

    double distance = rtImageNode->GetIsocenterImagerDistance();
    
    double spacing[2] = {};
    rtImageNode->GetImageSpacing(spacing);

    int dimention[2] = {};
    rtImageNode->GetImageDimention(dimention);

    double offset[2] = {};
    rtImageNode->GetImagerCenterOffset(offset);

    double imagerHalfWidth = spacing[0] * dimention[0] / 2.; // columns
    double imagerHalfHeight = spacing[1] * dimention[1] / 2.; // rows

    // update points
    vtkVector3d imagerP0( -1. * imagerHalfWidth + offset[0], imagerHalfHeight + offset[1], -distance);
    vtkVector3d imagerP1( imagerHalfWidth + offset[0], imagerHalfHeight + offset[1], -distance);
    vtkVector3d imagerP2( imagerHalfWidth + offset[0], -1. * imagerHalfHeight + offset[1], -distance);
    vtkVector3d imagerP3( -1. * imagerHalfWidth + offset[0], -1. * imagerHalfHeight + offset[1], -distance);

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

    if (markupsTransformNode/* && beamTransformNode->GetID() != markupsTransformNode->GetID()*/)
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

    double distance = rtImageNode->GetIsocenterImagerDistance();
     
    double spacing[2] = {};
    rtImageNode->GetImageSpacing(spacing);

    int dimention[2] = {};
    rtImageNode->GetImageDimention(dimention);

    double offset[2] = {};
    rtImageNode->GetImagerCenterOffset(offset);

    int imageWindow[4] = {};
    rtImageNode->GetImageWindow(imageWindow);

    double imagerHalfWidth = spacing[0] * dimention[0] / 2.; // columns
    double imagerHalfHeight = spacing[1] * dimention[1] / 2.; // rows

    // imager top left corner
    vtkVector3d imagerP0( -1. * imagerHalfWidth + offset[0], imagerHalfHeight + offset[1], -distance);

    double r1 = -1. * imagerP0.GetY() + imageWindow[1] * spacing[1];
    double c1 = imagerP0.GetX() + imageWindow[0] * spacing[0];
    double r2 = -1. * imagerP0.GetY() + imageWindow[3] * spacing[1];
    double c2 = imagerP0.GetX() + imageWindow[2] * spacing[0];

    // update points
    vtkVector3d imageP0( c1, r1, -distance);
    vtkVector3d imageP1( c1, r2, -distance);
    vtkVector3d imageP2( c2, r2, -distance);
    vtkVector3d imageP3( c2, r1, -distance);

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

    if (markupsTransformNode/* && beamTransformNode->GetID() != markupsTransformNode->GetID()*/)
    {
      imageWindowMarkupsNode->SetAndObserveTransformNodeID(beamTransformNode->GetID());
    }
  }

  // normal vector markups line node
  if (scene->GetFirstNodeByName(NORMAL_VECTOR_MARKUPS_NODE_NAME))
  {
    vtkMRMLMarkupsLineNode* vectorMarkupsNode = vtkMRMLMarkupsLineNode::SafeDownCast(
      scene->GetFirstNodeByName(NORMAL_VECTOR_MARKUPS_NODE_NAME));

    double offset[2] = {};
    rtImageNode->GetImagerCenterOffset(offset);

    double distance = rtImageNode->GetIsocenterImagerDistance();

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

    if (markupsTransformNode/* && beamTransformNode->GetID() != markupsTransformNode->GetID()*/)
    {
      vectorMarkupsNode->SetAndObserveTransformNodeID(beamTransformNode->GetID());
    }
  }

  // vup vector markups line node
  if (scene->GetFirstNodeByName(VUP_VECTOR_MARKUPS_NODE_NAME))
  {
    vtkMRMLMarkupsLineNode* vectorMarkupsNode = vtkMRMLMarkupsLineNode::SafeDownCast(
      scene->GetFirstNodeByName(VUP_VECTOR_MARKUPS_NODE_NAME));

    double distance = rtImageNode->GetIsocenterImagerDistance();

    double spacing[2] = {};
    rtImageNode->GetImageSpacing(spacing);

    int dimention[2] = {};
    rtImageNode->GetImageDimention(dimention);

    double offset[2] = {};
    rtImageNode->GetImagerCenterOffset(offset);

    double x = spacing[0] * dimention[0] / 2.; // center imager column

    // update points
    vtkVector3d p0( offset[0], offset[1], -distance);
    vtkVector3d p1( -x + offset[0], offset[1], -distance);

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

    if (markupsTransformNode/* && beamTransformNode->GetID() != markupsTransformNode->GetID()*/)
    {
      vectorMarkupsNode->SetAndObserveTransformNodeID(beamTransformNode->GetID());
    }
  }

  // fiducial markups line node
  if (scene->GetFirstNodeByName(FIDUCIALS_MARKUPS_NODE_NAME))
  {
    vtkMRMLMarkupsFiducialNode* pointsMarkupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(
      scene->GetFirstNodeByName(FIDUCIALS_MARKUPS_NODE_NAME));

    double distance = rtImageNode->GetIsocenterImagerDistance();
     
    double spacing[2] = {};
    rtImageNode->GetImageSpacing(spacing);

    int dimention[2] = {};
    rtImageNode->GetImageDimention(dimention);

    double offset[2] = {};
    rtImageNode->GetImagerCenterOffset(offset);

    double x = spacing[0] * dimention[0] / 2.; // imager center column
    double y = spacing[1] * dimention[1] / 2.; // imager center row

    // update points
    vtkVector3d p0( 0., 0., -distance); // imager center
    vtkVector3d p1( 0., 0., -distance + 100.); // imager normal vector
    vtkVector3d p2( -x + offset[0], -y + offset[1], -distance); // (0,0)
    vtkVector3d p3( -x + offset[0], 0., -distance); // vup vector

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

    if (markupsTransformNode/* && beamTransformNode->GetID() != markupsTransformNode->GetID()*/)
    {
      pointsMarkupsNode->SetAndObserveTransformNodeID(beamTransformNode->GetID());
    }
  }
}

//----------------------------------------------------------------------------
void vtkSlicerRtImageLogic::ShowMarkupsNodes(vtkMRMLRTImageNode* rtImageNode, bool toggled)
{
  vtkMRMLScene* scene = this->GetMRMLScene(); 
  if (!scene)
  {
    vtkErrorMacro("UpdateMarkupsNodes: Invalid MRML scene");
    return;
  }
  if (!rtImageNode)
  {
    vtkErrorMacro("UpdateMarkupsNodes: Invalid RTImage node");
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
vtkMRMLMarkupsPlaneNode* vtkSlicerRtImageLogic::CreateImagerBoundary(vtkMRMLRTImageNode* rtImageNode)
{
  auto imagerMarkupsNode = vtkSmartPointer<vtkMRMLMarkupsPlaneNode>::New();
  this->GetMRMLScene()->AddNode(imagerMarkupsNode);
  imagerMarkupsNode->SetName(IMAGER_BOUNDARY_MARKUPS_NODE_NAME);
//  imagerMarkupsNode->SetCurveTypeToLinear();
  imagerMarkupsNode->SetHideFromEditors(1);
  std::string singletonTag = std::string("RTIMAGE_") + IMAGER_BOUNDARY_MARKUPS_NODE_NAME;
  imagerMarkupsNode->SetSingletonTag(singletonTag.c_str());

  if (rtImageNode)
  {
    double distance = rtImageNode->GetIsocenterImagerDistance();
     
    double spacing[2] = {};
    rtImageNode->GetImageSpacing(spacing);

    int dimention[2] = {};
    rtImageNode->GetImageDimention(dimention);

    double offset[2] = {};
    rtImageNode->GetImagerCenterOffset(offset);

    double imagerHalfWidth = spacing[0] * dimention[0] / 2.; // columns
    double imagerHalfHeight = spacing[1] * dimention[1] / 2.; // rows

    // add points
    vtkVector3d imagerP0( -1. * imagerHalfWidth + offset[0], imagerHalfHeight + offset[1], -distance);
    vtkVector3d imagerP1( imagerHalfWidth + offset[0], imagerHalfHeight + offset[1], -distance);
    vtkVector3d imagerP2( imagerHalfWidth + offset[0], -1. * imagerHalfHeight + offset[1], -distance);
    vtkVector3d imagerP3( -1. * imagerHalfWidth + offset[0], -1. * imagerHalfHeight + offset[1], -distance);

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
  auto imageWindowMarkupsNode = vtkSmartPointer<vtkMRMLMarkupsClosedCurveNode>::New();
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
    rtImageNode->GetImageSpacing(spacing);

    int dimention[2] = {};
    rtImageNode->GetImageDimention(dimention);

    double offset[2] = {};
    rtImageNode->GetImagerCenterOffset(offset);

    int imageWindow[4] = {};
    rtImageNode->GetImageWindow(imageWindow);

    double imagerHalfWidth = spacing[0] * dimention[0] / 2.; // columns
    double imagerHalfHeight = spacing[1] * dimention[1] / 2.; // rows

    // Imager upper left point
    vtkVector3d imagerP0( -1. * imagerHalfWidth + offset[0], imagerHalfHeight + offset[1], -distance);

    double r1 = -1. * imagerP0.GetY() + imageWindow[1] * spacing[1];
    double c1 = imagerP0.GetX() + imageWindow[0] * spacing[0];
    double r2 = -1. * imagerP0.GetY() + imageWindow[3] * spacing[1];
    double c2 = imagerP0.GetX() + imageWindow[2] * spacing[0];

    // add points
    vtkVector3d imageP0( c1, r1, -distance);
    vtkVector3d imageP1( c1, r2, -distance);
    vtkVector3d imageP2( c2, r2, -distance);
    vtkVector3d imageP3( c2, r1, -distance);

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
  auto vectorMarkupsNode = vtkSmartPointer<vtkMRMLMarkupsLineNode>::New();
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
  auto vectorMarkupsNode = vtkSmartPointer<vtkMRMLMarkupsLineNode>::New();
  this->GetMRMLScene()->AddNode(vectorMarkupsNode);
  vectorMarkupsNode->SetName(VUP_VECTOR_MARKUPS_NODE_NAME);
  vectorMarkupsNode->SetHideFromEditors(1);
  std::string singletonTag = std::string("RTIMAGE_") + VUP_VECTOR_MARKUPS_NODE_NAME;
  vectorMarkupsNode->SetSingletonTag(singletonTag.c_str());

  if (rtImageNode)
  {
    double distance = rtImageNode->GetIsocenterImagerDistance();
     
    double spacing[2] = {};
    rtImageNode->GetImageSpacing(spacing);

    int dimention[2] = {};
    rtImageNode->GetImageDimention(dimention);

    double offset[2] = {};
    rtImageNode->GetImagerCenterOffset(offset);

    double x = spacing[0] * dimention[0] / 2.; // center column
//    double y = spacing[1] * dimention[1] / 2.; // center row

    // add points
    vtkVector3d p0( 0 + offset[0], 0 + offset[1], -distance);
    vtkVector3d p1( -x + offset[0], 0 + offset[1], -distance);

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
  auto pointsMarkupsNode = vtkSmartPointer<vtkMRMLMarkupsFiducialNode>::New();
  this->GetMRMLScene()->AddNode(pointsMarkupsNode);
  pointsMarkupsNode->SetName(FIDUCIALS_MARKUPS_NODE_NAME);
  pointsMarkupsNode->SetHideFromEditors(1);
  std::string singletonTag = std::string("RTIMAGE_") + FIDUCIALS_MARKUPS_NODE_NAME;
  pointsMarkupsNode->SetSingletonTag(singletonTag.c_str());

  if (rtImageNode)
  {
    double distance = rtImageNode->GetIsocenterImagerDistance();
     
    double spacing[2] = {};
    rtImageNode->GetImageSpacing(spacing);

    int dimention[2] = {};
    rtImageNode->GetImageDimention(dimention);

    double offset[2] = {};
    rtImageNode->GetImagerCenterOffset(offset);

    double x = spacing[0] * dimention[0] / 2.; // columns
    double y = spacing[1] * dimention[1] / 2.; // rows

    // add points
    vtkVector3d p0( 0, 0, -distance); // imager center
    vtkVector3d p1( 0, 0, -distance + 100.); // n
    vtkVector3d p2( -x + offset[0], -y + offset[1], -distance); // (0,0)
    vtkVector3d p3( -x + offset[0], 0.0, -distance); // vup

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
void vtkSlicerRtImageLogic::SetPlanarImageLogic(vtkSlicerPlanarImageModuleLogic* planarImageLogic)
{
  this->PlanarImageLogic = planarImageLogic;
}

//------------------------------------------------------------------------------
void vtkSlicerRtImageLogic::SetDRRComputationLogic(vtkSlicerCLIModuleLogic* plastimatchDrrLogic)
{
  this->PlastimatchDRRComputationLogic = plastimatchDrrLogic;
}
