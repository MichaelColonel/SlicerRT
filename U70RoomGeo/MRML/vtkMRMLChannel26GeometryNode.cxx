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

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

#include "vtkMRMLChannel26GeometryNode.h"
#include "vtkSlicerChannel26RoomRobotsGeometryCommon.h"
#include "vtkSlicerChannel26Room1RobotsGeometryCommon.h"
#include "vtkSlicerChannel26Room2RobotsGeometryCommon.h"
#include "vtkSlicerChannel26Room3RobotsGeometryCommon.h"

#include <cstring>

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLChannel26GeometryNode);

//----------------------------------------------------------------------------
vtkMRMLChannel26GeometryNode::vtkMRMLChannel26GeometryNode()
{
}

//----------------------------------------------------------------------------
vtkMRMLChannel26GeometryNode::~vtkMRMLChannel26GeometryNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLChannel26GeometryNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  // Write all MRML node attributes into output stream
  vtkMRMLWriteXMLBeginMacro(of);

  vtkMRMLWriteXMLVectorMacro(carmRobotAngles, CarmRobotAngles, double, 6);
  vtkMRMLWriteXMLVectorMacro(tableRobotAngles, TableRobotAngles, double, 6);
  vtkMRMLWriteXMLVectorMacro(tableBaseFixedToFixedReferenceTranslation, TableBaseFixedToFixedReferenceTranslation, double, 3);
  vtkMRMLWriteXMLVectorMacro(carmBaseFixedToTableTopBaseFixedOffset, CarmBaseFixedToTableBaseFixedOffset, double, 3);
  vtkMRMLWriteXMLVectorMacro(patientToTableTopTranslation, PatientToTableTopTranslation, double, 3);
  vtkMRMLWriteXMLVectorMacro(anglesABC, AnglesABC, double, 3);
  vtkMRMLWriteXMLBooleanMacro(patientHeadFeetRotation, PatientHeadFeetRotation);

  // add new parameters here
  vtkMRMLWriteXMLEndMacro();

}

//----------------------------------------------------------------------------
void vtkMRMLChannel26GeometryNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();
  vtkMRMLNode::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);

  vtkMRMLReadXMLVectorMacro(carmRobotAngles, CarmRobotAngles, double, 6);
  vtkMRMLReadXMLVectorMacro(tableRobotAngles, TableRobotAngles, double, 6);
  vtkMRMLReadXMLVectorMacro(tableBaseFixedToFixedReferenceTranslation, TableBaseFixedToFixedReferenceTranslation, double, 3);
  vtkMRMLReadXMLVectorMacro(carmBaseFixedToTableBaseFixedOffset, CarmBaseFixedToTableBaseFixedOffset, double, 3);
  vtkMRMLReadXMLVectorMacro(patientToTableTopTranslation, PatientToTableTopTranslation, double, 3);
  vtkMRMLReadXMLVectorMacro(anglesABC, AnglesABC, double, 3);
  vtkMRMLReadXMLBooleanMacro(patientHeadFeetRotation, PatientHeadFeetRotation);

  // add new parameters here
  vtkMRMLReadXMLEndMacro();

  this->EndModify(disabledModify);

  // Note: ReportString is not read from XML, it is a strictly temporary value
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
void vtkMRMLChannel26GeometryNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);

  vtkMRMLChannel26GeometryNode* node = vtkMRMLChannel26GeometryNode::SafeDownCast(anode);
  if (!node)
  {
    return;
  }

  // Copy beam parameters
  this->DisableModifiedEventOn();

  vtkMRMLCopyBeginMacro(node);

  vtkMRMLCopyVectorMacro(CarmRobotAngles, double, 6);
  vtkMRMLCopyVectorMacro(TableRobotAngles, double, 6);
  vtkMRMLCopyVectorMacro(TableBaseFixedToFixedReferenceTranslation, double, 6);
  vtkMRMLCopyVectorMacro(CarmBaseFixedToTableBaseFixedOffset, double, 6);
  vtkMRMLCopyVectorMacro(PatientToTableTopTranslation, double, 3);
  vtkMRMLCopyVectorMacro(AnglesABC, double, 3);
  vtkMRMLCopyBooleanMacro(PatientHeadFeetRotation);

  // add new parameters here
  vtkMRMLCopyEndMacro(); 

  this->EndModify(disabledModify);

  this->InvokePendingModifiedEvent();
}

//----------------------------------------------------------------------------
void vtkMRMLChannel26GeometryNode::CopyContent(vtkMRMLNode *anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkMRMLChannel26GeometryNode* node = vtkMRMLChannel26GeometryNode::SafeDownCast(anode);
  if (!node)
  {
    return;
  }

  vtkMRMLCopyBeginMacro(node);

  vtkMRMLCopyVectorMacro(CarmRobotAngles, double, 6);
  vtkMRMLCopyVectorMacro(TableRobotAngles, double, 6);
  vtkMRMLCopyVectorMacro(TableBaseFixedToFixedReferenceTranslation, double, 6);
  vtkMRMLCopyVectorMacro(CarmBaseFixedToTableBaseFixedOffset, double, 6);
  vtkMRMLCopyVectorMacro(PatientToTableTopTranslation, double, 3);
  vtkMRMLCopyVectorMacro(AnglesABC, double, 3);
  vtkMRMLCopyBooleanMacro(PatientHeadFeetRotation);

  // add new parameters here
  vtkMRMLCopyEndMacro();

}

//----------------------------------------------------------------------------
void vtkMRMLChannel26GeometryNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  vtkMRMLPrintBeginMacro(os, indent);

  vtkMRMLPrintVectorMacro(CarmRobotAngles, double, 6);
  vtkMRMLPrintVectorMacro(TableRobotAngles, double, 6);
  vtkMRMLPrintVectorMacro(TableBaseFixedToFixedReferenceTranslation, double, 6);
  vtkMRMLPrintVectorMacro(CarmBaseFixedToTableBaseFixedOffset, double, 6);
  vtkMRMLPrintVectorMacro(PatientToTableTopTranslation, double, 3);
  vtkMRMLPrintVectorMacro(AnglesABC, double, 3);
  vtkMRMLPrintBooleanMacro(PatientHeadFeetRotation);

  // add new parameters here
  vtkMRMLPrintEndMacro(); 

}

//----------------------------------------------------------------------------
void vtkMRMLChannel26GeometryNode::ProcessMRMLEvents(vtkObject *caller, unsigned long eventID, void *callData)
{
  Superclass::ProcessMRMLEvents(caller, eventID, callData);

  if (!this->Scene)
  {
    vtkErrorMacro("ProcessMRMLEvents: Invalid MRML scene");
    return;
  }
  if (this->Scene->IsBatchProcessing())
  {
    return;
  }
}

//----------------------------------------------------------------------------
void vtkMRMLChannel26GeometryNode::GetTableRobotAnglesModel(double a[6])
{
  a[0] = this->TableRobotAngles[0];
  a[1] = this->TableRobotAngles[1] + 90.;
  a[2] = this->TableRobotAngles[2] - 90.;
  a[3] = this->TableRobotAngles[3];
  a[4] = this->TableRobotAngles[4] + 90.;
  a[5] = this->TableRobotAngles[5] + 90.;
}

//----------------------------------------------------------------------------
void vtkMRMLChannel26GeometryNode::GetCarmRobotAnglesModel(double a[6])
{
  a[0] = this->CarmRobotAngles[0];
  a[1] = this->CarmRobotAngles[1] + 90.;
  a[2] = this->CarmRobotAngles[2] - 90.;
  a[3] = this->CarmRobotAngles[3];
  a[4] = this->CarmRobotAngles[4];
  a[5] = this->CarmRobotAngles[5] + 180.;
}

//----------------------------------------------------------------------------
void vtkMRMLChannel26GeometryNode::GetTableBaseFixedToFixedReferenceTranslation(double trans[3])
{
  double* ptr = this->TableBaseFixedToFixedReferenceTranslation;
  switch (this->U70RoomGeo)
  {
  case vtkMRMLChannel26GeometryNode::R1:
    {
      using Room1CoordPos = vtkSlicerChannel26Room1RobotsGeometryCommon;
      ptr[0] = Room1CoordPos::D_NORTH_WALL_ISOCENTER - Room1CoordPos::D_NORTH_WALL_TABLE_TOP_ROBOT_BASE;
      ptr[1] = Room1CoordPos::D_WEST_WALL_TABLE_TOP_ROBOT_BASE - Room1CoordPos::D_WEST_WALL_ISOCENTER;
      ptr[2] = -1. * Room1CoordPos::D_BASEMENT_ISOCENTER;
    }
    break;
  case vtkMRMLChannel26GeometryNode::R2:
    {
      using Room2CoordPos = vtkSlicerChannel26Room2RobotsGeometryCommon;
      ptr[0] = Room2CoordPos::D_NORTH_WALL_ISOCENTER - Room2CoordPos::D_NORTH_WALL_TABLE_TOP_ROBOT_BASE;//-1360.;//-4300. + 2940.;
      ptr[1] = Room2CoordPos::D_WEST_WALL_TABLE_TOP_ROBOT_BASE - Room2CoordPos::D_WEST_WALL_ISOCENTER;//710.;//2780. - 2080.;
      ptr[2] = -1. * Room2CoordPos::D_BASEMENT_ISOCENTER;
    }
    break;
  case vtkMRMLChannel26GeometryNode::R3:
    {
      using Room3CoordPos = vtkSlicerChannel26Room3RobotsGeometryCommon;
      ptr[0] = Room3CoordPos::D_NORTH_WALL_ISOCENTER - Room3CoordPos::D_NORTH_WALL_TABLE_TOP_ROBOT_BASE,
      ptr[1] = Room3CoordPos::D_EAST_WALL_ISOCENTER - Room3CoordPos::D_EAST_WALL_TABLE_TOP_ROBOT_BASE,
      ptr[2] = -1. * Room3CoordPos::D_BASEMENT_ISOCENTER;
    }
    break;
  default:
    break;
  }
  trans[0] = this->TableBaseFixedToFixedReferenceTranslation[0];
  trans[1] = this->TableBaseFixedToFixedReferenceTranslation[1];
  trans[2] = this->TableBaseFixedToFixedReferenceTranslation[2];
}

//----------------------------------------------------------------------------
double* vtkMRMLChannel26GeometryNode::GetTableBaseFixedToFixedReferenceTranslation()
{
  double* ptr = this->TableBaseFixedToFixedReferenceTranslation;
  switch (this->U70RoomGeo)
  {
  case vtkMRMLChannel26GeometryNode::R1:
    {
      using Room1CoordPos = vtkSlicerChannel26Room1RobotsGeometryCommon;
      ptr[0] = Room1CoordPos::D_NORTH_WALL_ISOCENTER - Room1CoordPos::D_NORTH_WALL_TABLE_TOP_ROBOT_BASE;
      ptr[1] = Room1CoordPos::D_WEST_WALL_TABLE_TOP_ROBOT_BASE - Room1CoordPos::D_WEST_WALL_ISOCENTER;
      ptr[2] = -1. * Room1CoordPos::D_BASEMENT_ISOCENTER;
    }
    break;
  case vtkMRMLChannel26GeometryNode::R2:
    {
      using Room2CoordPos = vtkSlicerChannel26Room2RobotsGeometryCommon;
      ptr[0] = Room2CoordPos::D_NORTH_WALL_ISOCENTER - Room2CoordPos::D_NORTH_WALL_TABLE_TOP_ROBOT_BASE;//-1360.;//-4300. + 2940.;
      ptr[1] = Room2CoordPos::D_WEST_WALL_TABLE_TOP_ROBOT_BASE - Room2CoordPos::D_WEST_WALL_ISOCENTER;//710.;//2780. - 2080.;
      ptr[2] = -1. * Room2CoordPos::D_BASEMENT_ISOCENTER;
    }
    break;
  case vtkMRMLChannel26GeometryNode::R3:
    {
      using Room3CoordPos = vtkSlicerChannel26Room3RobotsGeometryCommon;
      ptr[0] = Room3CoordPos::D_NORTH_WALL_ISOCENTER - Room3CoordPos::D_NORTH_WALL_TABLE_TOP_ROBOT_BASE;
      ptr[1] = Room3CoordPos::D_EAST_WALL_ISOCENTER - Room3CoordPos::D_EAST_WALL_TABLE_TOP_ROBOT_BASE;
      ptr[2] = -1. * Room3CoordPos::D_BASEMENT_ISOCENTER;
    }
    break;
  default:
    break;
  }
  return this->TableBaseFixedToFixedReferenceTranslation;
}

//----------------------------------------------------------------------------
void vtkMRMLChannel26GeometryNode::GetCarmBaseFixedToTableBaseFixedOffset(double trans[3])
{
  double* ptr = this->CarmBaseFixedToTableBaseFixedOffset;
  switch (this->U70RoomGeo)
  {
  case vtkMRMLChannel26GeometryNode::R1:
    {
      using Room1CoordPos = vtkSlicerChannel26Room1RobotsGeometryCommon;
      ptr[0] = Room1CoordPos::D_WEST_WALL_CARM_XRAY_ROBOT_BASE - Room1CoordPos::D_WEST_WALL_ISOCENTER - 80;
      ptr[1] = Room1CoordPos::D_BASEMENT_CARM_ROBOT_BASE;
      ptr[2] = Room1CoordPos::D_NORTH_WALL_ISOCENTER - Room1CoordPos::D_NORTH_WALL_CARM_XRAY_ROBOT_BASE + 590;
    }
    break;
  case vtkMRMLChannel26GeometryNode::R2:
    {
      using Room2CoordPos = vtkSlicerChannel26Room2RobotsGeometryCommon;
      ptr[0] = Room2CoordPos::D_NORTH_WALL_TABLE_TOP_ROBOT_BASE - Room2CoordPos::D_NORTH_WALL_CARM_XRAY_ROBOT_BASE + 2720.;
      ptr[1] = Room2CoordPos::D_BASEMENT_CARM_ROBOT_BASE;
      ptr[2] = Room2CoordPos::D_WEST_WALL_CARM_XRAY_ROBOT_BASE - Room2CoordPos::D_WEST_WALL_TABLE_TOP_ROBOT_BASE;//-2780. + 2080. - (2880 - 1300);
    }
    break;
  case vtkMRMLChannel26GeometryNode::R3:
    {
      using Room3CoordPos = vtkSlicerChannel26Room3RobotsGeometryCommon;
      ptr[0] = Room3CoordPos::D_NORTH_WALL_TABLE_TOP_ROBOT_BASE - Room3CoordPos::D_NORTH_WALL_CARM_XRAY_ROBOT_BASE;
      ptr[1] = Room3CoordPos::D_BASEMENT_CARM_ROBOT_BASE;
      ptr[2] = Room3CoordPos::D_EAST_WALL_CARM_XRAY_ROBOT_BASE - Room3CoordPos::D_EAST_WALL_TABLE_TOP_ROBOT_BASE;
    }
    break;
  default:
    break;
  }
  trans[0] = this->CarmBaseFixedToTableBaseFixedOffset[0];
  trans[1] = this->CarmBaseFixedToTableBaseFixedOffset[1];
  trans[2] = this->CarmBaseFixedToTableBaseFixedOffset[2];
}

//----------------------------------------------------------------------------
double* vtkMRMLChannel26GeometryNode::GetCarmBaseFixedToTableBaseFixedOffset()
{
  double* ptr = this->CarmBaseFixedToTableBaseFixedOffset;
  switch (this->U70RoomGeo)
  {
  case vtkMRMLChannel26GeometryNode::R1:
    {
      using Room1CoordPos = vtkSlicerChannel26Room1RobotsGeometryCommon;
      ptr[0] = Room1CoordPos::D_WEST_WALL_CARM_XRAY_ROBOT_BASE - Room1CoordPos::D_WEST_WALL_ISOCENTER - 80;
      ptr[1] = Room1CoordPos::D_BASEMENT_CARM_ROBOT_BASE;
      ptr[2] = Room1CoordPos::D_NORTH_WALL_ISOCENTER - Room1CoordPos::D_NORTH_WALL_CARM_XRAY_ROBOT_BASE + 590;
    }
    break;
  case vtkMRMLChannel26GeometryNode::R2:
    {
      using Room2CoordPos = vtkSlicerChannel26Room2RobotsGeometryCommon;
      ptr[0] = Room2CoordPos::D_NORTH_WALL_TABLE_TOP_ROBOT_BASE - Room2CoordPos::D_NORTH_WALL_CARM_XRAY_ROBOT_BASE + 2720.;
      ptr[1] = Room2CoordPos::D_BASEMENT_CARM_ROBOT_BASE;
      ptr[2] = Room2CoordPos::D_WEST_WALL_CARM_XRAY_ROBOT_BASE - Room2CoordPos::D_WEST_WALL_TABLE_TOP_ROBOT_BASE;//-2290.;//-2780. + 2080. - (2880 - 1300);
    }
    break;
  case vtkMRMLChannel26GeometryNode::R3:
    {
      using Room3CoordPos = vtkSlicerChannel26Room3RobotsGeometryCommon;
      ptr[0] = Room3CoordPos::D_NORTH_WALL_TABLE_TOP_ROBOT_BASE - Room3CoordPos::D_NORTH_WALL_CARM_XRAY_ROBOT_BASE;
      ptr[1] = Room3CoordPos::D_BASEMENT_CARM_ROBOT_BASE;
      ptr[2] = Room3CoordPos::D_EAST_WALL_CARM_XRAY_ROBOT_BASE - Room3CoordPos::D_EAST_WALL_TABLE_TOP_ROBOT_BASE;
    }
    break;
  default:
    break;
  }
  return this->CarmBaseFixedToTableBaseFixedOffset;
}


//---------------------------------------------------------------------------
void vtkMRMLChannel26GeometryNode::SetU70RoomGeo(int id)
{
  switch (id)
  {
    case 1: this->SetU70RoomGeo(vtkMRMLChannel26GeometryNode::R1); break;
    case 2: this->SetU70RoomGeo(vtkMRMLChannel26GeometryNode::R2); break;
    case 3: this->SetU70RoomGeo(vtkMRMLChannel26GeometryNode::R3); break;
    default: this->SetU70RoomGeo(vtkMRMLChannel26GeometryNode::U70RoomGeoType_Last); break;
  }
}

//---------------------------------------------------------------------------
const char* vtkMRMLChannel26GeometryNode::GetU70RoomGeoAsString(int id)
{
  switch (id)
  {
    case vtkMRMLChannel26GeometryNode::R1: return "R1";
    case vtkMRMLChannel26GeometryNode::R2: return "R2";
    case vtkMRMLChannel26GeometryNode::R3: return "R2";
    default: return "UNKNOWN";
  }
}

//---------------------------------------------------------------------------
int vtkMRMLChannel26GeometryNode::GetU70RoomGeoFromString(const char* name)
{
  if (name == nullptr)
  {
    // invalid name
    return -1;
  }
  for (int i = 0; i < vtkMRMLChannel26GeometryNode::U70RoomGeoType_Last; i++)
  {
    if (std::strcmp(name, vtkMRMLChannel26GeometryNode::GetU70RoomGeoAsString(i)) == 0)
    {
      // found a matching name
      return i;
    }
  }
  // unknown name
  return -1;
}
