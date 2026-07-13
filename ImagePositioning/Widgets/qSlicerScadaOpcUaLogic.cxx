/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// MLC device includes
#include "qSlicerScadaOpcUaLogic.h"

// SlicerRT ScadaOpcUa MRML includes
#include <vtkMRMLScadaOpcUaNode.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLScalarVolumeDisplayNode.h>
#include <vtkMRMLSubjectHierarchyNode.h>
#include <vtkMRMLSubjectHierarchyConstants.h>
#include <vtkMRMLColorTableNode.h>
#include <vtkMRMLSliceCompositeNode.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLTableNode.h>

// Slicer includes
#include "qSlicerCoreApplication.h"
#include "vtkSlicerApplicationLogic.h"

// OPC UA headers
#include <QOpcUaProvider>

// VTK includes
#include <vtkSmartPointer.h>

// Qt includes
#include <QDebug>

// STD includes
#include <cstring>
#include <bitset>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy
class qSlicerScadaOpcUaLogicPrivate
{
  Q_DECLARE_PUBLIC(qSlicerScadaOpcUaLogic);
protected:
  qSlicerScadaOpcUaLogic* const q_ptr;
public:
  qSlicerScadaOpcUaLogicPrivate(qSlicerScadaOpcUaLogic& object);
  ~qSlicerScadaOpcUaLogicPrivate();
  void loadApplicationSettings();
//  bool parseExtendedObject(QOpcUaExtensionObject& extObject);

  const std::string SCADA_PATPOS_NODE_ID = "ns=1;s=Модуль Позиционирования";

  const std::string SCADA_TEST_READ_NODE_ID = SCADA_PATPOS_NODE_ID + ".SysTimeScada";
  const std::string SCADA_TEST_WRITE_NODE_ID  = SCADA_PATPOS_NODE_ID + ".SysTimeModPos";
  const std::string SCADA_ERROR_WRITE_NODE_ID  = SCADA_PATPOS_NODE_ID + ".ErrStr";
  const std::string SCADA_EVENT_WRITE_NODE_ID  = SCADA_PATPOS_NODE_ID + ".EventStr";

  const std::string SCADA_TOASU_NODE_ID = SCADA_PATPOS_NODE_ID + ".TO_ASU";
  const std::string SCADA_TOASU_ERROR_MES_NODE_ID = SCADA_TOASU_NODE_ID + ".ErrorMes";
  const std::string SCADA_TOASU_SERV_MES_NODE_ID = SCADA_TOASU_NODE_ID + ".ServMes";
  const std::string SCADA_TOASU_MESSAGES_NODE_ID = SCADA_TOASU_NODE_ID + ".Messages";
  const std::string SCADA_TOASU_MODE_NODE_ID = SCADA_TOASU_NODE_ID + ".Mode";
  const std::string SCADA_TOASU_STATUS_RTK_NODE_ID = SCADA_TOASU_NODE_ID + ".Status_RTK";
  const std::string SCADA_TOASU_STATUS_R1_DEKA_NODE_ID = SCADA_TOASU_NODE_ID + ".Status_R1_deka";
  const std::string SCADA_TOASU_STATUS_R2_C_DUGA_NODE_ID = SCADA_TOASU_NODE_ID + ".Status_R2_C_Duga";
  const std::string SCADA_TOASU_COORDFROMASU_NODE_ID = SCADA_TOASU_NODE_ID + ".CoordFromASU";
  const std::string SCADA_TOASU_COORDFROMASU_X_NODE_ID = SCADA_TOASU_COORDFROMASU_NODE_ID + ".X";
  const std::string SCADA_TOASU_COORDFROMASU_Y_NODE_ID = SCADA_TOASU_COORDFROMASU_NODE_ID + ".Y";
  const std::string SCADA_TOASU_COORDFROMASU_Z_NODE_ID = SCADA_TOASU_COORDFROMASU_NODE_ID + ".Z";
  const std::string SCADA_TOASU_COORDFROMASU_XRAY_Z_CORRECTION_NODE_ID = SCADA_TOASU_COORDFROMASU_NODE_ID + ".XRAY_Z_correction";
  const std::string SCADA_TOASU_COORDFROMASU_ANGLEA_R1_NODE_ID = SCADA_TOASU_COORDFROMASU_NODE_ID + ".AngleA_R1";
  const std::string SCADA_TOASU_COORDFROMASU_ANGLEB_R1_NODE_ID = SCADA_TOASU_COORDFROMASU_NODE_ID + ".AngleB_R1";
  const std::string SCADA_TOASU_COORDFROMASU_ANGLEC_R1_NODE_ID = SCADA_TOASU_COORDFROMASU_NODE_ID + ".AngleC_R1";
  const std::string SCADA_TOASU_BUTTONS_NODE_ID = SCADA_TOASU_NODE_ID + ".Buttons";
  const std::string SCADA_TOASU_BUTTONS_AM_NODE_ID = SCADA_TOASU_BUTTONS_NODE_ID + ".AM";

  const std::string IS_PRESSED = ".isPressed";
  const std::string IS_ENABLE = ".isEnable";
  const std::string SCADA_TOASU_BUTTONS_AM_R1_LOADTOISO_NODE_ID = SCADA_TOASU_BUTTONS_AM_NODE_ID + ".R1_LoadToIso";
  const std::string SCADA_TOASU_BUTTONS_AM_R1_LOADTOISO_ISPRESSED_NODE_ID = SCADA_TOASU_BUTTONS_AM_R1_LOADTOISO_NODE_ID + IS_PRESSED;
  const std::string SCADA_TOASU_BUTTONS_AM_R1_LOADTOISO_ISENABLE_NODE_ID = SCADA_TOASU_BUTTONS_AM_R1_LOADTOISO_NODE_ID + IS_ENABLE;

  const std::string SCADA_TOASU_BUTTONS_AM_R1_TONEWCOORDS_NODE_ID = SCADA_TOASU_BUTTONS_AM_NODE_ID + ".R1_ToNewCoords";
  const std::string SCADA_TOASU_BUTTONS_AM_R1_TONEWCOORDS_ISPRESSED_NODE_ID = SCADA_TOASU_BUTTONS_AM_R1_TONEWCOORDS_NODE_ID + IS_PRESSED;
  const std::string SCADA_TOASU_BUTTONS_AM_R1_TONEWCOORDS_ISENABLE_NODE_ID = SCADA_TOASU_BUTTONS_AM_R1_TONEWCOORDS_NODE_ID + IS_ENABLE;

  const std::string SCADA_TOASU_BUTTONS_AM_R1_TOHOME_NODE_ID = SCADA_TOASU_BUTTONS_AM_NODE_ID + ".R1_ToHome";
  const std::string SCADA_TOASU_BUTTONS_AM_R1_TOHOME_ISPRESSED_NODE_ID = SCADA_TOASU_BUTTONS_AM_R1_TOHOME_NODE_ID + IS_PRESSED;
  const std::string SCADA_TOASU_BUTTONS_AM_R1_TOHOME_ISENABLE_NODE_ID = SCADA_TOASU_BUTTONS_AM_R1_TOHOME_NODE_ID + IS_ENABLE;

  const std::string SCADA_TOASU_BUTTONS_AM_R1_TOLOAD_NODE_ID = SCADA_TOASU_BUTTONS_AM_NODE_ID + ".R1_ToLoad";
  const std::string SCADA_TOASU_BUTTONS_AM_R1_TOLOAD_ISPRESSED_NODE_ID = SCADA_TOASU_BUTTONS_AM_R1_TOLOAD_NODE_ID + IS_PRESSED;
  const std::string SCADA_TOASU_BUTTONS_AM_R1_TOLOAD_ISENABLE_NODE_ID = SCADA_TOASU_BUTTONS_AM_R1_TOLOAD_NODE_ID + IS_ENABLE;

  const std::string SCADA_TOASU_BUTTONS_AM_R2_TOISO_NODE_ID = SCADA_TOASU_BUTTONS_AM_NODE_ID + ".R2_ToIso";
  const std::string SCADA_TOASU_BUTTONS_AM_R2_TOISO_ISPRESSED_NODE_ID = SCADA_TOASU_BUTTONS_AM_R2_TOISO_NODE_ID + IS_PRESSED;
  const std::string SCADA_TOASU_BUTTONS_AM_R2_TOISO_ISENABLE_NODE_ID = SCADA_TOASU_BUTTONS_AM_R2_TOISO_NODE_ID + IS_ENABLE;

  const std::string SCADA_TOASU_BUTTONS_AM_R2_TO2NDPL_NODE_ID = SCADA_TOASU_BUTTONS_AM_NODE_ID + ".R2_To2ndPl";
  const std::string SCADA_TOASU_BUTTONS_AM_R2_TO2NDPL_ISPRESSED_NODE_ID = SCADA_TOASU_BUTTONS_AM_R2_TO2NDPL_NODE_ID + IS_PRESSED;
  const std::string SCADA_TOASU_BUTTONS_AM_R2_TO2NDPL_ISENABLE_NODE_ID = SCADA_TOASU_BUTTONS_AM_R2_TO2NDPL_NODE_ID + IS_ENABLE;

  const std::string SCADA_TOASU_BUTTONS_AM_MAKEXRAY_NODE_ID = SCADA_TOASU_BUTTONS_AM_NODE_ID + ".MakeXRay";
  const std::string SCADA_TOASU_BUTTONS_AM_MAKEXRAY_ISPRESSED_NODE_ID = SCADA_TOASU_BUTTONS_AM_MAKEXRAY_NODE_ID + IS_PRESSED;
  const std::string SCADA_TOASU_BUTTONS_AM_MAKEXRAY_ISENABLE_NODE_ID = SCADA_TOASU_BUTTONS_AM_MAKEXRAY_NODE_ID + IS_ENABLE;

  const std::string SCADA_TOASU_BUTTONS_AM_R2_TOHOME_NODE_ID = SCADA_TOASU_BUTTONS_AM_NODE_ID + ".R2_ToHome";
  const std::string SCADA_TOASU_BUTTONS_AM_R2_TOHOME_ISPRESSED_NODE_ID = SCADA_TOASU_BUTTONS_AM_R2_TOHOME_NODE_ID + IS_PRESSED;
  const std::string SCADA_TOASU_BUTTONS_AM_R2_TOHOME_ISENABLE_NODE_ID = SCADA_TOASU_BUTTONS_AM_R2_TOHOME_NODE_ID + IS_ENABLE;

  const std::string SCADA_TOASU_BUTTONS_AM_R2_SET_NEW_Z_NODE_ID = SCADA_TOASU_BUTTONS_AM_NODE_ID + ".R2_Set_New_Z";
  const std::string SCADA_TOASU_BUTTONS_AM_R2_SET_NEW_Z_ISPRESSED_NODE_ID = SCADA_TOASU_BUTTONS_AM_R2_SET_NEW_Z_NODE_ID + IS_PRESSED;
  const std::string SCADA_TOASU_BUTTONS_AM_R2_SET_NEW_Z_ISENABLE_NODE_ID = SCADA_TOASU_BUTTONS_AM_R2_SET_NEW_Z_NODE_ID + IS_ENABLE;

  const std::string SCADA_TOASU_BUTTONS_MM_NODE_ID = SCADA_TOASU_BUTTONS_NODE_ID + ".MM";
  const std::string SCADA_TOASU_BUTTONS_MM_R1_LOADTOISO_NODE_ID = SCADA_TOASU_BUTTONS_MM_NODE_ID + ".R1_LoadToIso";
  const std::string SCADA_TOASU_BUTTONS_MM_R1_LOADTOISO_ISPRESSED_NODE_ID = SCADA_TOASU_BUTTONS_MM_R1_LOADTOISO_NODE_ID + IS_PRESSED;
  const std::string SCADA_TOASU_BUTTONS_MM_R1_LOADTOISO_ISENABLE_NODE_ID = SCADA_TOASU_BUTTONS_MM_R1_LOADTOISO_NODE_ID + IS_ENABLE;

  const std::string SCADA_TOASU_BUTTONS_MM_R1_TONEWCOORDS_NODE_ID = SCADA_TOASU_BUTTONS_MM_NODE_ID + ".R1_ToNewCoords";
  const std::string SCADA_TOASU_BUTTONS_MM_R1_TONEWCOORDS_ISPRESSED_NODE_ID = SCADA_TOASU_BUTTONS_MM_R1_TONEWCOORDS_NODE_ID + IS_PRESSED;
  const std::string SCADA_TOASU_BUTTONS_MM_R1_TONEWCOORDS_ISENABLE_NODE_ID = SCADA_TOASU_BUTTONS_MM_R1_TONEWCOORDS_NODE_ID + IS_ENABLE;

  const std::string SCADA_TOASU_BUTTONS_MM_R1_TOHOME_NODE_ID = SCADA_TOASU_BUTTONS_MM_NODE_ID + ".R1_ToHome";
  const std::string SCADA_TOASU_BUTTONS_MM_R1_TOHOME_ISPRESSED_NODE_ID = SCADA_TOASU_BUTTONS_MM_R1_TOHOME_NODE_ID + IS_PRESSED;
  const std::string SCADA_TOASU_BUTTONS_MM_R1_TOHOME_ISENABLE_NODE_ID = SCADA_TOASU_BUTTONS_MM_R1_TOHOME_NODE_ID + IS_ENABLE;

  const std::string SCADA_TOASU_BUTTONS_MM_R1_TOLOAD_NODE_ID = SCADA_TOASU_BUTTONS_MM_NODE_ID + ".R1_ToLoad";
  const std::string SCADA_TOASU_BUTTONS_MM_R1_TOLOAD_ISPRESSED_NODE_ID = SCADA_TOASU_BUTTONS_MM_R1_TOLOAD_NODE_ID + IS_PRESSED;
  const std::string SCADA_TOASU_BUTTONS_MM_R1_TOLOAD_ISENABLE_NODE_ID = SCADA_TOASU_BUTTONS_MM_R1_TOLOAD_NODE_ID + IS_ENABLE;

  const std::string SCADA_TOASU_BUTTONS_MM_R2_TOISO_NODE_ID = SCADA_TOASU_BUTTONS_MM_NODE_ID + ".R2_ToIso";
  const std::string SCADA_TOASU_BUTTONS_MM_R2_TOISO_ISPRESSED_NODE_ID = SCADA_TOASU_BUTTONS_MM_R2_TOISO_NODE_ID + IS_PRESSED;
  const std::string SCADA_TOASU_BUTTONS_MM_R2_TOISO_ISENABLE_NODE_ID = SCADA_TOASU_BUTTONS_MM_R2_TOISO_NODE_ID + IS_ENABLE;

  const std::string SCADA_TOASU_BUTTONS_MM_R2_TO2NDPL_NODE_ID = SCADA_TOASU_BUTTONS_MM_NODE_ID + ".R2_To2ndPl";
  const std::string SCADA_TOASU_BUTTONS_MM_R2_TO2NDPL_ISPRESSED_NODE_ID = SCADA_TOASU_BUTTONS_MM_R2_TO2NDPL_NODE_ID + IS_PRESSED;
  const std::string SCADA_TOASU_BUTTONS_MM_R2_TO2NDPL_ISENABLE_NODE_ID = SCADA_TOASU_BUTTONS_MM_R2_TO2NDPL_NODE_ID + IS_ENABLE;

  const std::string SCADA_TOASU_BUTTONS_MM_MAKEXRAY_NODE_ID = SCADA_TOASU_BUTTONS_MM_NODE_ID + ".MakeXRay";
  const std::string SCADA_TOASU_BUTTONS_MM_MAKEXRAY_ISPRESSED_NODE_ID = SCADA_TOASU_BUTTONS_MM_MAKEXRAY_NODE_ID + IS_PRESSED;
  const std::string SCADA_TOASU_BUTTONS_MM_MAKEXRAY_ISENABLE_NODE_ID = SCADA_TOASU_BUTTONS_MM_MAKEXRAY_NODE_ID + IS_ENABLE;

  const std::string SCADA_TOASU_BUTTONS_MM_R2_TOHOME_NODE_ID = SCADA_TOASU_BUTTONS_MM_NODE_ID + ".R2_ToHome";
  const std::string SCADA_TOASU_BUTTONS_MM_R2_TOHOME_ISPRESSED_NODE_ID = SCADA_TOASU_BUTTONS_MM_R2_TOHOME_NODE_ID + IS_PRESSED;
  const std::string SCADA_TOASU_BUTTONS_MM_R2_TOHOME_ISENABLE_NODE_ID = SCADA_TOASU_BUTTONS_MM_R2_TOHOME_NODE_ID + IS_ENABLE;

  const std::string SCADA_TOASU_BUTTONS_MM_R2_SET_NEW_Z_NODE_ID = SCADA_TOASU_BUTTONS_MM_NODE_ID + ".R2_Set_New_Z";
  const std::string SCADA_TOASU_BUTTONS_MM_R2_SET_NEW_Z_ISPRESSED_NODE_ID = SCADA_TOASU_BUTTONS_MM_R2_SET_NEW_Z_NODE_ID + IS_PRESSED;
  const std::string SCADA_TOASU_BUTTONS_MM_R2_SET_NEW_Z_ISENABLE_NODE_ID = SCADA_TOASU_BUTTONS_MM_R2_SET_NEW_Z_NODE_ID + IS_ENABLE;

  const std::string SCADA_TOASU_BUTTONS_RESETERRORS_NODE_ID = SCADA_TOASU_BUTTONS_NODE_ID + ".ResetErrors";
  const std::string SCADA_TOASU_BUTTONS_RESETERRORS_ISPRESSED_NODE_ID = SCADA_TOASU_BUTTONS_RESETERRORS_NODE_ID + IS_PRESSED;
  const std::string SCADA_TOASU_BUTTONS_RESETERRORS_ISENABLE_NODE_ID = SCADA_TOASU_BUTTONS_RESETERRORS_NODE_ID + IS_ENABLE;

  const std::string SCADA_TOASU_BUTTONS_SM_NODE_ID = SCADA_TOASU_BUTTONS_NODE_ID + ".SM";
  const std::string SCADA_TOASU_BUTTONS_SM_BRAKETESTR1_NODE_ID = SCADA_TOASU_BUTTONS_SM_NODE_ID + ".BreakTestR1";
  const std::string SCADA_TOASU_BUTTONS_SM_BRAKETESTR1_ISPRESSED_NODE_ID = SCADA_TOASU_BUTTONS_SM_BRAKETESTR1_NODE_ID + IS_PRESSED;
  const std::string SCADA_TOASU_BUTTONS_SM_BRAKETESTR1_ISENABLE_NODE_ID = SCADA_TOASU_BUTTONS_SM_BRAKETESTR1_NODE_ID + IS_ENABLE;

  const std::string SCADA_TOASU_BUTTONS_SM_MASRESTESTR1_NODE_ID = SCADA_TOASU_BUTTONS_SM_NODE_ID + ".MasResTestR1";
  const std::string SCADA_TOASU_BUTTONS_SM_MASRESTESTR1_ISPRESSED_NODE_ID = SCADA_TOASU_BUTTONS_SM_MASRESTESTR1_NODE_ID + IS_PRESSED;
  const std::string SCADA_TOASU_BUTTONS_SM_MASRESTESTR1_ISENABLE_NODE_ID = SCADA_TOASU_BUTTONS_SM_MASRESTESTR1_NODE_ID + IS_ENABLE;

  const std::string SCADA_TOASU_BUTTONS_SM_HOMER1_NODE_ID = SCADA_TOASU_BUTTONS_SM_NODE_ID + ".HomeR1";
  const std::string SCADA_TOASU_BUTTONS_SM_HOMER1_ISPRESSED_NODE_ID = SCADA_TOASU_BUTTONS_SM_HOMER1_NODE_ID + IS_PRESSED;
  const std::string SCADA_TOASU_BUTTONS_SM_HOMER1_ISENABLE_NODE_ID = SCADA_TOASU_BUTTONS_SM_HOMER1_NODE_ID + IS_ENABLE;

  const std::string SCADA_TOASU_BUTTONS_SM_ISOR1_NODE_ID = SCADA_TOASU_BUTTONS_SM_NODE_ID + ".IsoR1";
  const std::string SCADA_TOASU_BUTTONS_SM_ISOR1_ISPRESSED_NODE_ID = SCADA_TOASU_BUTTONS_SM_ISOR1_NODE_ID + IS_PRESSED;
  const std::string SCADA_TOASU_BUTTONS_SM_ISOR1_ISENABLE_NODE_ID = SCADA_TOASU_BUTTONS_SM_ISOR1_NODE_ID + IS_ENABLE;

  const std::string SCADA_TOASU_BUTTONS_SM_SERVICEPOSR1_NODE_ID = SCADA_TOASU_BUTTONS_SM_NODE_ID + ".ServicePosR1";
  const std::string SCADA_TOASU_BUTTONS_SM_SERVICEPOSR1_ISPRESSED_NODE_ID = SCADA_TOASU_BUTTONS_SM_SERVICEPOSR1_NODE_ID + IS_PRESSED;
  const std::string SCADA_TOASU_BUTTONS_SM_SERVICEPOSR1_ISENABLE_NODE_ID = SCADA_TOASU_BUTTONS_SM_SERVICEPOSR1_NODE_ID + IS_ENABLE;

  const std::string SCADA_TOASU_BUTTONS_SM_BRAKETESTR2_NODE_ID = SCADA_TOASU_BUTTONS_SM_NODE_ID + ".BreakTestR2";
  const std::string SCADA_TOASU_BUTTONS_SM_BRAKETESTR2_ISPRESSED_NODE_ID = SCADA_TOASU_BUTTONS_SM_BRAKETESTR2_NODE_ID + IS_PRESSED;
  const std::string SCADA_TOASU_BUTTONS_SM_BRAKETESTR2_ISENABLE_NODE_ID = SCADA_TOASU_BUTTONS_SM_BRAKETESTR2_NODE_ID + IS_ENABLE;

  const std::string SCADA_TOASU_BUTTONS_SM_MASRESTESTR2_NODE_ID = SCADA_TOASU_BUTTONS_SM_NODE_ID + ".MasResTestR2";
  const std::string SCADA_TOASU_BUTTONS_SM_MASRESTESTR2_ISPRESSED_NODE_ID = SCADA_TOASU_BUTTONS_SM_MASRESTESTR2_NODE_ID + IS_PRESSED;
  const std::string SCADA_TOASU_BUTTONS_SM_MASRESTESTR2_ISENABLE_NODE_ID = SCADA_TOASU_BUTTONS_SM_MASRESTESTR2_NODE_ID + IS_ENABLE;

  const std::string SCADA_TOASU_BUTTONS_SM_HOMER2_NODE_ID = SCADA_TOASU_BUTTONS_SM_NODE_ID + ".HomeR2";
  const std::string SCADA_TOASU_BUTTONS_SM_HOMER2_ISPRESSED_NODE_ID = SCADA_TOASU_BUTTONS_SM_HOMER2_NODE_ID + IS_PRESSED;
  const std::string SCADA_TOASU_BUTTONS_SM_HOMER2_ISENABLE_NODE_ID = SCADA_TOASU_BUTTONS_SM_HOMER2_NODE_ID + IS_ENABLE;

  const std::string SCADA_TOASU_BUTTONS_SM_ISOR2_NODE_ID = SCADA_TOASU_BUTTONS_SM_NODE_ID + ".IsoR2";
  const std::string SCADA_TOASU_BUTTONS_SM_ISOR2_ISPRESSED_NODE_ID = SCADA_TOASU_BUTTONS_SM_ISOR2_NODE_ID + IS_PRESSED;
  const std::string SCADA_TOASU_BUTTONS_SM_ISOR2_ISENABLE_NODE_ID = SCADA_TOASU_BUTTONS_SM_ISOR2_NODE_ID + IS_ENABLE;

  const std::string SCADA_TOASU_BUTTONS_SM_SERVICEPOSR2_NODE_ID = SCADA_TOASU_BUTTONS_SM_NODE_ID + ".ServicePosR2";
  const std::string SCADA_TOASU_BUTTONS_SM_SERVICEPOSR2_ISPRESSED_NODE_ID = SCADA_TOASU_BUTTONS_SM_SERVICEPOSR2_NODE_ID + IS_PRESSED;
  const std::string SCADA_TOASU_BUTTONS_SM_SERVICEPOSR2_ISENABLE_NODE_ID = SCADA_TOASU_BUTTONS_SM_SERVICEPOSR2_NODE_ID + IS_ENABLE;

  const std::string SCADA_TOASU_BUTTONS_SM_RESTARTSM_NODE_ID = SCADA_TOASU_BUTTONS_SM_NODE_ID + ".RestartSM";
  const std::string SCADA_TOASU_BUTTONS_SM_RESTARTSM_ISPRESSED_NODE_ID = SCADA_TOASU_BUTTONS_SM_RESTARTSM_NODE_ID + IS_PRESSED;
  const std::string SCADA_TOASU_BUTTONS_SM_RESTARTSM_ISENABLE_NODE_ID = SCADA_TOASU_BUTTONS_SM_RESTARTSM_NODE_ID + IS_ENABLE;

  const std::string SCADA_TOASU_BUTTONS_KSM_NODE_ID = SCADA_TOASU_BUTTONS_NODE_ID + ".KSM";
  const std::string SCADA_TOASU_BUTTONS_KSM_ALLOWT1_NODE_ID = SCADA_TOASU_BUTTONS_KSM_NODE_ID + ".AllowT1";
  const std::string SCADA_TOASU_BUTTONS_KSM_ALLOWT1_ISPRESSED_NODE_ID = SCADA_TOASU_BUTTONS_KSM_ALLOWT1_NODE_ID + IS_PRESSED;
  const std::string SCADA_TOASU_BUTTONS_KSM_ALLOWT1_ISENABLE_NODE_ID = SCADA_TOASU_BUTTONS_KSM_ALLOWT1_NODE_ID + IS_ENABLE;

  const std::string SCADA_TOASU_BUTTONS_KSM_ALLOWXRAY_NODE_ID = SCADA_TOASU_BUTTONS_KSM_NODE_ID + ".AllowXRay";
  const std::string SCADA_TOASU_BUTTONS_KSM_ALLOWXRAY_ISPRESSED_NODE_ID = SCADA_TOASU_BUTTONS_KSM_ALLOWXRAY_NODE_ID + IS_PRESSED;
  const std::string SCADA_TOASU_BUTTONS_KSM_ALLOWXRAY_ISENABLE_NODE_ID = SCADA_TOASU_BUTTONS_KSM_ALLOWXRAY_NODE_ID + IS_ENABLE;

  const std::string SCADA_TOASU_BUTTONS_KSM_MAKEXRAY_NODE_ID = SCADA_TOASU_BUTTONS_KSM_NODE_ID + ".MakeXRay";
  const std::string SCADA_TOASU_BUTTONS_KSM_MAKEXRAY_ISPRESSED_NODE_ID = SCADA_TOASU_BUTTONS_KSM_MAKEXRAY_NODE_ID + IS_PRESSED;
  const std::string SCADA_TOASU_BUTTONS_KSM_MAKEXRAY_ISENABLE_NODE_ID = SCADA_TOASU_BUTTONS_KSM_MAKEXRAY_NODE_ID + IS_ENABLE;

  const std::string SCADA_TOASU_BUTTONS_KSM_ALLOWBEAM_NODE_ID = SCADA_TOASU_BUTTONS_KSM_NODE_ID + ".AllowBeam";
  const std::string SCADA_TOASU_BUTTONS_KSM_ALLOWBEAM_ISPRESSED_NODE_ID = SCADA_TOASU_BUTTONS_KSM_ALLOWBEAM_NODE_ID + IS_PRESSED;
  const std::string SCADA_TOASU_BUTTONS_KSM_ALLOWBEAM_ISENABLE_NODE_ID = SCADA_TOASU_BUTTONS_KSM_ALLOWBEAM_NODE_ID + IS_ENABLE;

  const std::string SCADA_TOASU_ROBOTSREADYFORXRAY1_NODE_ID = SCADA_TOASU_NODE_ID + ".RobotsReadyForXray1";
  const std::string SCADA_TOASU_ROBOTSREADYFORXRAY2_NODE_ID = SCADA_TOASU_NODE_ID + ".RobotsReadyForXray2";
  const std::string SCADA_TOASU_ROBOTSREADYFORBEAM_NODE_ID = SCADA_TOASU_NODE_ID + ".RobotsReadyForBeam";
  const std::string SCADA_TOASU_HUMANONDEKA_NODE_ID = SCADA_TOASU_NODE_ID + ".HUMAN_ON_DEKA";

  const std::string SCADA_KUKA_X_COORDINATE_TO_TCS_R1_NODE_ID = SCADA_TOASU_NODE_ID + ".KUKA_X_coordinate_to_TCS_R1";
  const std::string SCADA_KUKA_Y_COORDINATE_TO_TCS_R1_NODE_ID = SCADA_TOASU_NODE_ID + ".KUKA_Y_coordinate_to_TCS_R1";
  const std::string SCADA_KUKA_Z_COORDINATE_TO_TCS_R1_NODE_ID = SCADA_TOASU_NODE_ID + ".KUKA_Z_coordinate_to_TCS_R1";

  const std::string SCADA_AXIS_CORD_A1_R1_NODE_ID = SCADA_TOASU_NODE_ID + ".AXIS_cord_A1_R1";
  const std::string SCADA_AXIS_CORD_A2_R1_NODE_ID = SCADA_TOASU_NODE_ID + ".AXIS_cord_A2_R1";
  const std::string SCADA_AXIS_CORD_A3_R1_NODE_ID = SCADA_TOASU_NODE_ID + ".AXIS_cord_A3_R1";
  const std::string SCADA_AXIS_CORD_A4_R1_NODE_ID = SCADA_TOASU_NODE_ID + ".AXIS_cord_A4_R1";
  const std::string SCADA_AXIS_CORD_A5_R1_NODE_ID = SCADA_TOASU_NODE_ID + ".AXIS_cord_A5_R1";
  const std::string SCADA_AXIS_CORD_A6_R1_NODE_ID = SCADA_TOASU_NODE_ID + ".AXIS_cord_A6_R1";

  const std::string SCADA_AXIS_CORD_A1_R2_NODE_ID = SCADA_TOASU_NODE_ID + ".AXIS_cord_A1_R2";
  const std::string SCADA_AXIS_CORD_A2_R2_NODE_ID = SCADA_TOASU_NODE_ID + ".AXIS_cord_A2_R2";
  const std::string SCADA_AXIS_CORD_A3_R2_NODE_ID = SCADA_TOASU_NODE_ID + ".AXIS_cord_A3_R2";
  const std::string SCADA_AXIS_CORD_A4_R2_NODE_ID = SCADA_TOASU_NODE_ID + ".AXIS_cord_A4_R2";
  const std::string SCADA_AXIS_CORD_A5_R2_NODE_ID = SCADA_TOASU_NODE_ID + ".AXIS_cord_A5_R2";
  const std::string SCADA_AXIS_CORD_A6_R2_NODE_ID = SCADA_TOASU_NODE_ID + ".AXIS_cord_A6_R2";
  
  vtkWeakPointer<vtkMRMLScadaOpcUaNode> ParameterNode;

  QScopedPointer<QOpcUaProvider> OpcUaProvider;
  QScopedPointer<QOpcUaClient> OpcUaClient;
  QScopedPointer<QOpcUaNode> OpcUaScadaLocalTimeNode;

  // Modes
  QScopedPointer<QOpcUaNode> OpcUaScadaModeNode;
  QScopedPointer<QOpcUaNode> OpcUaScadaStatusRtkNode;
  QScopedPointer<QOpcUaNode> OpcUaScadaStatusR1RobotNode;
  QScopedPointer<QOpcUaNode> OpcUaScadaStatusR2CarmNode;

  // Coords from ASUTP
  QScopedPointer<QOpcUaNode> OpcUaScadaCoordAsuXNode;
  QScopedPointer<QOpcUaNode> OpcUaScadaCoordAsuYNode;
  QScopedPointer<QOpcUaNode> OpcUaScadaCoordAsuZNode;
  QScopedPointer<QOpcUaNode> OpcUaScadaCoordAsuXrayZNode;
  QScopedPointer<QOpcUaNode> OpcUaScadaCoordAsuAngleAR1Node;
  QScopedPointer<QOpcUaNode> OpcUaScadaCoordAsuAngleBR1Node;
  QScopedPointer<QOpcUaNode> OpcUaScadaCoordAsuAngleCR1Node;

  // Automatic Movement (AM) robots mode
  QScopedPointer<QOpcUaNode> OpcUaScadaAmR1LoadToIsoIsEnabledNode;
  QScopedPointer<QOpcUaNode> OpcUaScadaAmR1LoadToIsoIsPressedNode;

  QScopedPointer<QOpcUaNode> OpcUaScadaAmR1ToNewCoordsIsEnabledNode;
  QScopedPointer<QOpcUaNode> OpcUaScadaAmR1ToNewCoordsIsPressedNode;

  QScopedPointer<QOpcUaNode> OpcUaScadaAmR1ToHomeIsEnabledNode;
  QScopedPointer<QOpcUaNode> OpcUaScadaAmR1ToHomeIsPressedNode;

  QScopedPointer<QOpcUaNode> OpcUaScadaAmR1ToLoadIsEnabledNode;
  QScopedPointer<QOpcUaNode> OpcUaScadaAmR1ToLoadIsPressedNode;

  QScopedPointer<QOpcUaNode> OpcUaScadaAmR2ToIsoIsEnabledNode;
  QScopedPointer<QOpcUaNode> OpcUaScadaAmR2ToIsoIsPressedNode;

  QScopedPointer<QOpcUaNode> OpcUaScadaAmR2ToSecondPlaneIsEnabledNode;
  QScopedPointer<QOpcUaNode> OpcUaScadaAmR2ToSecondPlaneIsPressedNode;

  QScopedPointer<QOpcUaNode> OpcUaScadaAmR2MakeXrayIsEnabledNode;
  QScopedPointer<QOpcUaNode> OpcUaScadaAmR2MakeXrayIsPressedNode;

  QScopedPointer<QOpcUaNode> OpcUaScadaAmR2ToHomeIsEnabledNode;
  QScopedPointer<QOpcUaNode> OpcUaScadaAmR2ToHomeIsPressedNode;

  QScopedPointer<QOpcUaNode> OpcUaScadaAmR2SetNewZIsEnabledNode;
  QScopedPointer<QOpcUaNode> OpcUaScadaAmR2SetNewZIsPressedNode;

  QScopedPointer<QOpcUaNode> OpcUaPatPosLocalTimeNode;
  QScopedPointer<QOpcUaNode> OpcUaPatPosErrorMessageNode;
  QScopedPointer<QOpcUaNode> OpcUaPatPosEventMessageNode;

  bool connectStatusNodes();
  bool connectCoordFromAsuNodes();
  bool connectAutomaticMovementNodes();

  bool ClientConnectedFlag{ false };
};

//-----------------------------------------------------------------------------
// qSlicerIhepMlcDeviceLogicPrivate methods

//-----------------------------------------------------------------------------
qSlicerScadaOpcUaLogicPrivate::qSlicerScadaOpcUaLogicPrivate(qSlicerScadaOpcUaLogic& object)
  : q_ptr(&object),
  OpcUaProvider(new QOpcUaProvider(&object))
{
  qDebug() << Q_FUNC_INFO << this->OpcUaProvider->availableBackends();
}

//-----------------------------------------------------------------------------
qSlicerScadaOpcUaLogicPrivate::~qSlicerScadaOpcUaLogicPrivate()
{
}

//-----------------------------------------------------------------------------
void qSlicerScadaOpcUaLogicPrivate::loadApplicationSettings()
{
  //TODO: Implement if there are application settings (such as default dose engine)
  //      See qSlicerSubjectHierarchyPluginLogicPrivate::loadApplicationSettings
}

//-----------------------------------------------------------------------------
bool qSlicerScadaOpcUaLogicPrivate::connectStatusNodes()
{
  if (!this->OpcUaClient || !this->ParameterNode)
  {
    return false;
  }
  vtkMRMLScadaOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();
  // write values
  // Mode
  QString nodeIdStr = QString::fromStdString(this->SCADA_TOASU_MODE_NODE_ID);
  this->OpcUaScadaModeNode.reset(this->OpcUaClient->node(nodeIdStr));
  // Status RTK
  nodeIdStr = QString::fromStdString(this->SCADA_TOASU_STATUS_RTK_NODE_ID);
  this->OpcUaScadaStatusRtkNode.reset(this->OpcUaClient->node(nodeIdStr));
  // Status R1
  nodeIdStr = QString::fromStdString(this->SCADA_TOASU_STATUS_R1_DEKA_NODE_ID);
  this->OpcUaScadaStatusR1RobotNode.reset(this->OpcUaClient->node(nodeIdStr));
  // Status R2
  nodeIdStr = QString::fromStdString(this->SCADA_TOASU_STATUS_R2_C_DUGA_NODE_ID);
  this->OpcUaScadaStatusR2CarmNode.reset(this->OpcUaClient->node(nodeIdStr));

  // Connect signal handlers for subscribed values
  // Mode
  if (!this->OpcUaScadaModeNode)
  {
    return false;
  }
  QOpcUaNode* opcNode = this->OpcUaScadaModeNode.get();
  QObject::connect(opcNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value)
      {
        bool ok = false;
        uint8_t statusMode = value.toUInt(&ok);
        if (ok && mrmlNode)
        {
          mrmlNode->SetMode(statusMode);
          qDebug() << Q_FUNC_INFO << "PatPos module status mode: " << statusMode;
        }
      }
    }
  );
  // Subscribe to data changes
  opcNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(100));

  // Status RTK
  if (!this->OpcUaScadaStatusRtkNode)
  {
    return false;
  }
  opcNode = this->OpcUaScadaStatusRtkNode.get();
  QObject::connect(opcNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value)
      {
        bool ok = false;
        uint32_t statusRtk = value.toUInt(&ok);
        if (ok && mrmlNode)
        {
          mrmlNode->SetStatus_RTK(statusRtk);
          qDebug() << Q_FUNC_INFO << "PatPos module status RTK: " << statusRtk;
        }
      }
    }
  );
  // Subscribe to data changes
  opcNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(100));

  // Status R1 TableTop
  if (!this->OpcUaScadaStatusR1RobotNode)
  {
    return false;
  }
  opcNode = this->OpcUaScadaStatusR1RobotNode.get();
  QObject::connect(opcNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value)
      {
        bool ok = false;
        uint32_t statusR1 = value.toUInt(&ok);
        if (ok && mrmlNode)
        {
          mrmlNode->SetStatus_R1_deka(statusR1);
          qDebug() << Q_FUNC_INFO << "PatPos module status R1 TableTop: " << statusR1;
        }
      }
    }
  );
  // Subscribe to data changes
  opcNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(100));

  // Status R2 C-Arm
  if (!this->OpcUaScadaStatusR2CarmNode)
  {
    return false;
  }
  opcNode = this->OpcUaScadaStatusR2CarmNode.get();
  QObject::connect(opcNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value)
      {
        bool ok = false;
        uint32_t statusR2 = value.toUInt(&ok);
        if (ok && mrmlNode)
        {
          mrmlNode->SetStatus_R2_C_Duga(statusR2);
          qDebug() << Q_FUNC_INFO << "PatPos module status R2 C-Arm: " << statusR2;
        }
      }
    }
  );
  // Subscribe to data changes
  opcNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(100));

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerScadaOpcUaLogicPrivate::connectCoordFromAsuNodes()
{
  if (!this->OpcUaClient || !this->ParameterNode)
  {
    return false;
  }
  vtkMRMLScadaOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();
  // write values
  // Coord from ASU coord X in TableTop frame
  QString nodeIdStr = QString::fromStdString(this->SCADA_TOASU_COORDFROMASU_X_NODE_ID);
  this->OpcUaScadaCoordAsuXNode.reset(this->OpcUaClient->node(nodeIdStr));

  // Coord from ASU coord Y in TableTop frame
  nodeIdStr = QString::fromStdString(this->SCADA_TOASU_COORDFROMASU_Y_NODE_ID);
  this->OpcUaScadaCoordAsuYNode.reset(this->OpcUaClient->node(nodeIdStr));

  // Coord from ASU coord Z in TableTop frame
  nodeIdStr = QString::fromStdString(this->SCADA_TOASU_COORDFROMASU_Z_NODE_ID);
  this->OpcUaScadaCoordAsuZNode.reset(this->OpcUaClient->node(nodeIdStr));

  // Coord from ASU coord Z in C-arm frame (Z correction in C-arm frame)
  nodeIdStr = QString::fromStdString(this->SCADA_TOASU_COORDFROMASU_XRAY_Z_CORRECTION_NODE_ID);
  this->OpcUaScadaCoordAsuXrayZNode.reset(this->OpcUaClient->node(nodeIdStr));

  // Coord from ASU angleA of TableTop robot
  nodeIdStr = QString::fromStdString(this->SCADA_TOASU_COORDFROMASU_ANGLEA_R1_NODE_ID);
  this->OpcUaScadaCoordAsuAngleAR1Node.reset(this->OpcUaClient->node(nodeIdStr));

  // Coord from ASU angleB of TableTop robot
  nodeIdStr = QString::fromStdString(this->SCADA_TOASU_COORDFROMASU_ANGLEB_R1_NODE_ID);
  this->OpcUaScadaCoordAsuAngleBR1Node.reset(this->OpcUaClient->node(nodeIdStr));

  // Coord from ASU angleB of TableTop robot
  nodeIdStr = QString::fromStdString(this->SCADA_TOASU_COORDFROMASU_ANGLEC_R1_NODE_ID);
  this->OpcUaScadaCoordAsuAngleCR1Node.reset(this->OpcUaClient->node(nodeIdStr));


  // Connect signal handlers for subscribed values
  // Coord from ASU coord X in TableTop frame
  if (!this->OpcUaScadaCoordAsuXNode)
  {
    return false;
  }
  QOpcUaNode* opcNode = this->OpcUaScadaCoordAsuXNode.get();
  QObject::connect(opcNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value)
      {
        bool ok = false;
        int32_t posX = value.toInt(&ok);
        if (ok && mrmlNode)
        {
          mrmlNode->SetCoordFromASU_X(posX);
          qDebug() << Q_FUNC_INFO << "Coord from ASU position X: " << posX;
        }
      }
    }
  );
  // Subscribe to data changes
  opcNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(100));

  // Coord from ASU coord Y in TableTop frame
  if (!this->OpcUaScadaCoordAsuYNode)
  {
    return false;
  }
  opcNode = this->OpcUaScadaCoordAsuYNode.get();
  QObject::connect(opcNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value)
      {
        bool ok = false;
        int32_t posY = value.toInt(&ok);
        if (ok && mrmlNode)
        {
          mrmlNode->SetCoordFromASU_Y(posY);
          qDebug() << Q_FUNC_INFO << "Coord from ASU position Y: " << posY;
        }
      }
    }
  );
  // Subscribe to data changes
  opcNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(100));

  // Coord from ASU coord Z in TableTop frame
  if (!this->OpcUaScadaCoordAsuZNode)
  {
    return false;
  }
  opcNode = this->OpcUaScadaCoordAsuZNode.get();
  QObject::connect(opcNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value)
      {
        bool ok = false;
        int32_t posZ = value.toInt(&ok);
        if (ok && mrmlNode)
        {
          mrmlNode->SetCoordFromASU_Z(posZ);
          qDebug() << Q_FUNC_INFO << "Coord from ASU position Z: " << posZ;
        }
      }
    }
  );
  // Subscribe to data changes
  opcNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(100));

  // Coord from ASU C-arm Z correction
  if (!this->OpcUaScadaCoordAsuXrayZNode)
  {
    return false;
  }
  opcNode = this->OpcUaScadaCoordAsuXrayZNode.get();
  QObject::connect(opcNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value)
      {
        bool ok = false;
        int32_t carmZ = value.toInt(&ok);
        if (ok && mrmlNode)
        {
          mrmlNode->SetCoordFromASU_XRAY_Z_correction(carmZ);
          qDebug() << Q_FUNC_INFO << "Coord from ASU position C-arm Z correction: " << carmZ;
        }
      }
    }
  );
  // Subscribe to data changes
  opcNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(100));

  // Coord from ASU TableTop (R1) angle-A
  if (!this->OpcUaScadaCoordAsuAngleAR1Node)
  {
    return false;
  }
  opcNode = this->OpcUaScadaCoordAsuAngleAR1Node.get();
  QObject::connect(opcNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value)
      {
        bool ok = false;
        int32_t angleA = value.toInt(&ok);
        if (ok && mrmlNode)
        {
          mrmlNode->SetCoordFromASU_AngleA_R1(angleA);
          qDebug() << Q_FUNC_INFO << "Coord from ASU TablTop robot (R1) angle-A: " << angleA;
        }
      }
    }
  );
  // Subscribe to data changes
  opcNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(100));

  // Coord from ASU TableTop (R1) angle-B
  if (!this->OpcUaScadaCoordAsuAngleBR1Node)
  {
    return false;
  }
  opcNode = this->OpcUaScadaCoordAsuAngleBR1Node.get();
  QObject::connect(opcNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value)
      {
        bool ok = false;
        int32_t angleB = value.toInt(&ok);
        if (ok && mrmlNode)
        {
          mrmlNode->SetCoordFromASU_AngleB_R1(angleB);
          qDebug() << Q_FUNC_INFO << "Coord from ASU TablTop robot (R1) angle-B: " << angleB;
        }
      }
    }
  );
  // Subscribe to data changes
  opcNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(100));

  // Coord from ASU TableTop (R1) angle-C
  if (!this->OpcUaScadaCoordAsuAngleCR1Node)
  {
    return false;
  }
  opcNode = this->OpcUaScadaCoordAsuAngleCR1Node.get();
  QObject::connect(opcNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value)
      {
        bool ok = false;
        int32_t angleC = value.toInt(&ok);
        if (ok && mrmlNode)
        {
          mrmlNode->SetCoordFromASU_AngleC_R1(angleC);
          qDebug() << Q_FUNC_INFO << "Coord from ASU TablTop robot (R1) angle-C: " << angleC;
        }
      }
    }
  );
  // Subscribe to data changes
  opcNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(100));

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerScadaOpcUaLogicPrivate::connectAutomaticMovementNodes()
{
  if (!this->OpcUaClient || !this->ParameterNode)
  {
    return false;
  }
  vtkMRMLScadaOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();
  // write values
  // Automatic movement R1_LoadToIso is enabled (read-only)
  QString nodeIdStr = QString::fromStdString(this->SCADA_TOASU_BUTTONS_AM_R1_LOADTOISO_ISENABLE_NODE_ID);
  this->OpcUaScadaAmR1LoadToIsoIsEnabledNode.reset(this->OpcUaClient->node(nodeIdStr));

  // Automatic movement R1_LoadToIso is pressed (read-write)
  nodeIdStr = QString::fromStdString(this->SCADA_TOASU_BUTTONS_AM_R1_LOADTOISO_ISPRESSED_NODE_ID);
  this->OpcUaScadaAmR1LoadToIsoIsPressedNode.reset(this->OpcUaClient->node(nodeIdStr));

  // Connect signal handlers for subscribed values
  // Automatic movement R1_LoadToIso is enabled (read-only)
  if (!this->OpcUaScadaAmR1LoadToIsoIsEnabledNode)
  {
    return false;
  }
  QOpcUaNode* opcNode = this->OpcUaScadaAmR1LoadToIsoIsEnabledNode.get();
  QObject::connect(opcNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value)
      {
        bool isEnabled = value.toBool();
        if (mrmlNode)
        {
          bool state[2] = {false, false};
          
          mrmlNode->GetAM_Buttons_R1_LoadToIso(state);
          state[0] = isEnabled;
          mrmlNode->SetAM_Buttons_R1_LoadToIso(state);
          qDebug() << Q_FUNC_INFO << "Automatic movement R1_LoadToIso is enabled: " << isEnabled;
        }
      }
    }
  );
  // Subscribe to data changes
  opcNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(100));

  // Automatic movement R1_LoadToIso is pressed (read-write)
  if (!this->OpcUaScadaAmR1LoadToIsoIsPressedNode)
  {
    return false;
  }
  opcNode = this->OpcUaScadaAmR1LoadToIsoIsPressedNode.get();
  QObject::connect(opcNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value)
      {
        bool isPressed = value.toBool();
        if (mrmlNode)
        {
          bool state[2] = {false, false};
          
          mrmlNode->GetAM_Buttons_R1_LoadToIso(state);
          state[1] = isPressed;
          mrmlNode->SetAM_Buttons_R1_LoadToIso(state);
          qDebug() << Q_FUNC_INFO << "Automatic movement R1_LoadToIso is pressed: " << isPressed;
        }
      }
    }
  );
  // Subscribe to data changes
  opcNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(100));

  return true;
}

//-----------------------------------------------------------------------------
// qSlicerScadaOpcUaLogic methods

//----------------------------------------------------------------------------
qSlicerScadaOpcUaLogic::qSlicerScadaOpcUaLogic(QObject* parent)
  : Superclass(parent)
  , d_ptr( new qSlicerScadaOpcUaLogicPrivate(*this) )
{
}

//----------------------------------------------------------------------------
qSlicerScadaOpcUaLogic::~qSlicerScadaOpcUaLogic() = default;

//-----------------------------------------------------------------------------

void qSlicerScadaOpcUaLogic::setMRMLScene(vtkMRMLScene* scene)
{
  this->qSlicerObject::setMRMLScene(scene);

  // Connect scene node added event so that the new subject hierarchy nodes can be claimed by a plugin
  qvtkReconnect(scene, vtkMRMLScene::NodeAddedEvent, this, SLOT( onNodeAdded(vtkObject*,vtkObject*) ) );
  // Connect scene import ended event so that subject hierarchy nodes can be created for supported data nodes if missing (backwards compatibility)
  qvtkReconnect(scene, vtkMRMLScene::EndImportEvent, this, SLOT( onSceneImportEnded(vtkObject*) ) );
}

//-----------------------------------------------------------------------------

void qSlicerScadaOpcUaLogic::onNodeAdded(vtkObject* sceneObject, vtkObject* nodeObject)
{
  vtkMRMLScene* scene = vtkMRMLScene::SafeDownCast(sceneObject);
  if (!scene)
  {
    return;
  }

  if (nodeObject->IsA("vtkMRMLScadaOpcUaNode"))
  {
    qDebug() << Q_FUNC_INFO << "Parameter node added";
    // Observe MLC control node changes
//    vtkMRMLIhepMlcControlNode* mlcNode = vtkMRMLIhepMlcControlNode::SafeDownCast(nodeObject);
//    qvtkConnect( mlcNode, vtkMRMLIhepMlcControlNode::Modified, this, SLOT( applyDoseEngineInPlan(vtkObject*) ) );
  }
}

//-----------------------------------------------------------------------------
void qSlicerScadaOpcUaLogic::onSceneImportEnded(vtkObject* sceneObject)
{
  vtkMRMLScene* scene = vtkMRMLScene::SafeDownCast(sceneObject);
  if (!scene)
  {
    return;
  }

  // Traverse all plan nodes in the scene and observe dose engine changed event so that default
  // beam parameters can be added for the newly selected engine in the beams contained by the plan
//  std::vector<vtkMRMLNode*> planNodes;
//  scene->GetNodesByClass("vtkMRMLRTPlanNode", planNodes);
//  for (std::vector<vtkMRMLNode*>::iterator planNodeIt = planNodes.begin(); planNodeIt != planNodes.end(); ++planNodeIt)
//  {
//    vtkMRMLNode* planNode = (*planNodeIt);
//    qvtkConnect( planNode, vtkMRMLRTPlanNode::DoseEngineChanged, this, SLOT( applyDoseEngineInPlan(vtkObject*) ) );
//  }
}

//-----------------------------------------------------------------------------
void qSlicerScadaOpcUaLogic::setParameterNode(vtkMRMLNode* node)
{
  Q_D(qSlicerScadaOpcUaLogic);

  vtkMRMLScadaOpcUaNode* parameterNode = vtkMRMLScadaOpcUaNode::SafeDownCast(node);
  // Each time the node is modified, the UI widgets are updated
  qvtkReconnect(d->ParameterNode, parameterNode, vtkCommand::ModifiedEvent, 
    this, SLOT(updateLogicFromMRML()));

  d->ParameterNode = parameterNode;
  qDebug() << Q_FUNC_INFO << "Parameter node added";
  this->updateLogicFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerScadaOpcUaLogic::updateLogicFromMRML()
{
  Q_D(qSlicerScadaOpcUaLogic);
  qDebug() << Q_FUNC_INFO << "Update SCADA OPC UA logic from node";
}

//-----------------------------------------------------------------------------
bool qSlicerScadaOpcUaLogic::connectToServer(const QString& clientPlugin, const QString& urlServerEndpoint)
{
  Q_D(qSlicerScadaOpcUaLogic);
  if (d->ClientConnectedFlag)
  {
    d->OpcUaClient->disconnectFromEndpoint();
    d->ClientConnectedFlag = false;
    emit opcUaClientConnected(false);
    return false;
  }

  d->OpcUaClient.reset(d->OpcUaProvider->createClient(clientPlugin));
  if (!d->OpcUaClient)
  {
    d->ClientConnectedFlag = false;
    emit opcUaClientConnected(false);
    return false;
  }

  connect(d->OpcUaClient.data(), SIGNAL(connected()), this, SLOT(clientConnected()));
  connect(d->OpcUaClient.data(), SIGNAL(disconnected()), this, SLOT(clientDisconnected()));
  connect(d->OpcUaClient.data(), SIGNAL(errorChanged(QOpcUaClient::ClientError)),
    this, SLOT(clientError(QOpcUaClient::ClientError)));
  connect(d->OpcUaClient.data(), SIGNAL(stateChanged(QOpcUaClient::ClientState)),
    this, SLOT(clientState(QOpcUaClient::ClientState)));

  d->OpcUaClient->connectToEndpoint(urlServerEndpoint);
  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerScadaOpcUaLogic::disconnectFromServer()
{
  Q_D(qSlicerScadaOpcUaLogic);
  if (!d->ClientConnectedFlag)
  {
    d->ClientConnectedFlag = false;
    emit opcUaClientConnected(false);
    return false;
  }
  
  d->OpcUaClient->disconnectFromEndpoint();
  return true;
}

//-----------------------------------------------------------------------------
void qSlicerScadaOpcUaLogic::clientConnected()
{
  Q_D(qSlicerScadaOpcUaLogic);

  // write values
  QString nodeIdStr = QString::fromStdString(d->SCADA_TEST_READ_NODE_ID);
  d->OpcUaScadaLocalTimeNode.reset(d->OpcUaClient->node(nodeIdStr));

  nodeIdStr = QString::fromStdString(d->SCADA_TEST_WRITE_NODE_ID);
  d->OpcUaPatPosLocalTimeNode.reset(d->OpcUaClient->node(nodeIdStr));

  nodeIdStr = QString::fromStdString(d->SCADA_ERROR_WRITE_NODE_ID);
  d->OpcUaPatPosErrorMessageNode.reset(d->OpcUaClient->node(nodeIdStr));

  nodeIdStr = QString::fromStdString(d->SCADA_EVENT_WRITE_NODE_ID);
  d->OpcUaPatPosEventMessageNode.reset(d->OpcUaClient->node(nodeIdStr));

//  nodeIdStr = QString::fromStdString(d->SCADA_TOASU_MODE_NODE_ID);
//  d->OpcUaPatPosEventMessageNode.reset(d->OpcUaClient->node(nodeIdStr));

  // Connect signal handlers for subscribed values
///  QObject::connect(d->OpcUaScadaLocalTimeNode.data(), SIGNAL(dataChangeOccurred(QOpcUa::NodeAttribute, QVariant)),
///    this, SLOT(onScadaLocalTimeAttributeChanged(QOpcUa::NodeAttribute, QVariant)));
   QOpcUaNode* opcNode = d->OpcUaScadaLocalTimeNode.get();
   vtkMRMLScadaOpcUaNode* mrmlNode = d->ParameterNode.GetPointer();
   QObject::connect(opcNode,
     &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
     {
       if (attr == QOpcUa::NodeAttribute::Value)
       {
         bool ok = false;
         uint64_t scadatime = value.toULongLong(&ok);
         if (ok && mrmlNode)
         {
           mrmlNode->SetSysTime(scadatime);
           qDebug() << Q_FUNC_INFO << "Scada time value: " << quint64(scadatime);
         }
       } 
     }
   );

  // Subscribe to data changes
  d->OpcUaScadaLocalTimeNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(100));
/*
  // Connect the handler for async reading
///  QObject::connect(d->OpcUaScadaLocalTimeNode.data(), SIGNAL(attributeRead(QOpcUa::NodeAttributes)),
///   this, SLOT(onScadaLocalTimeAttributeRead(QOpcUa::NodeAttributes)));
  QObject::connect(d->OpcUaScadaLocalTimeNode.data(),
    &QOpcUaNode::attributeRead, [opcNode, mrmlNode](QOpcUa::NodeAttributes)
    {
      if (opcNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
      {
        QVariant value = opcNode->attribute(QOpcUa::NodeAttribute::Value);
        bool ok = false;
        uint64_t scadatime = value.toULongLong(&ok); // Get the attribute from the cache
        if (ok && mrmlNode)
        {
          mrmlNode->SetSysTime(scadatime);
          qDebug() << Q_FUNC_INFO << "Scada time: " << scadatime;
        }
      }
    }
  );

  // Request the value attribute of the current Scada system time
  d->OpcUaScadaLocalTimeNode->readAttributes(QOpcUa::NodeAttribute::Value);

  // Add handlers for enableMonitoring results
///  QObject::connect(d->OpcUaScadaLocalTimeNode.data(), SIGNAL(enableMonitoringFinished(QOpcUa::NodeAttribute, QOpcUa::UaStatusCode)),
///    this, SLOT(onScadaLocalTimeEnableMonitoringFinished(QOpcUa::NodeAttribute, QOpcUa::UaStatusCode)));
  QObject::connect(d->OpcUaScadaLocalTimeNode.data(),
    &QOpcUaNode::enableMonitoringFinished, [this](QOpcUa::NodeAttribute, QOpcUa::UaStatusCode status)
    {
      if (!this->sender())
      {
        return;
      }
      if (status == QOpcUa::UaStatusCode::Good)
      {
        qDebug() << Q_FUNC_INFO << "Monitoring successfully enabled for" << qobject_cast<QOpcUaNode *>(this->sender())->nodeId();
      }
      else
      {
        qDebug() << Q_FUNC_INFO << "Failed to enable monitoring for" << qobject_cast<QOpcUaNode *>(this->sender())->nodeId() << ":" << status;
      }
    }
  );
*/
  // Positioning module -- Current Work Mode
//  QObject::connect(d->OpcUaScadaModeNode.data(), SIGNAL(enableMonitoringFinished(QOpcUa::NodeAttribute, QOpcUa::UaStatusCode)),
//    this, SLOT(onScadaModeEnableMonitoringFinished(QOpcUa::NodeAttribute, QOpcUa::UaStatusCode)));

  bool resStatusModes = d->connectStatusNodes();
  if (!resStatusModes)
  {
    qWarning() << Q_FUNC_INFO << "Can't connect status nodes";
  }

  resStatusModes = d->connectCoordFromAsuNodes();
  if (!resStatusModes)
  {
    qWarning() << Q_FUNC_INFO << "Can't connect coord from ASU nodes";
  }

  resStatusModes = d->connectAutomaticMovementNodes();
  if (!resStatusModes)
  {
    qWarning() << Q_FUNC_INFO << "Can't connect automatic movement nodes";
  }

  d->ClientConnectedFlag = true;

  // emit signal before node connection
  emit opcUaClientConnected(true);
}

//-----------------------------------------------------------------------------
void qSlicerScadaOpcUaLogic::clientDisconnected()
{
  Q_D(qSlicerScadaOpcUaLogic);
/*
  // Disconnect signal handlers for subscribed values
  QObject::disconnect(d->OpcUaScadaLocalTimeNode.data(), SIGNAL(dataChangeOccurred(QOpcUa::NodeAttribute, QVariant)),
    this, SLOT(onScadaLocalTimeAttributeChanged(QOpcUa::NodeAttribute, QVariant)));

  // Disconnect the handler for async reading
  QObject::disconnect(d->OpcUaScadaLocalTimeNode.data(), SIGNAL(attributeRead(QOpcUa::NodeAttributes)),
    this, SLOT(onScadaLocalTimeAttributeRead(QOpcUa::NodeAttributes)));

  // Remove handlers for enableMonitoring results
  QObject::disconnect(d->OpcUaScadaLocalTimeNode.data(), SIGNAL(enableMonitoringFinished(QOpcUa::NodeAttribute, QOpcUa::UaStatusCode)),
    this, SLOT(onScadaLocalTimeEnableMonitoringFinished(QOpcUa::NodeAttribute, QOpcUa::UaStatusCode)));
*/
  d->ClientConnectedFlag = false;
  d->OpcUaClient->deleteLater();
  d->OpcUaClient.take();

  // emit signal before node connection
  emit opcUaClientConnected(false);
}

//-----------------------------------------------------------------------------
void qSlicerScadaOpcUaLogic::clientError(QOpcUaClient::ClientError)
{
}

//-----------------------------------------------------------------------------
void qSlicerScadaOpcUaLogic::clientState(QOpcUaClient::ClientState)
{
}
/*
//-----------------------------------------------------------------------------
void qSlicerScadaOpcUaLogic::onScadaLocalTimeAttributeRead(QOpcUa::NodeAttributes)
{
  Q_D(qSlicerScadaOpcUaLogic);

  if (!d->OpcUaScadaLocalTimeNode)
  {
    return;
  }

  if (d->OpcUaScadaLocalTimeNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
  {
    QVariant value = d->OpcUaScadaLocalTimeNode->attribute(QOpcUa::NodeAttribute::Value);
    bool ok = false;
    uint64_t scadatime = value.toULongLong(&ok); // Get the attribute from the cache
    if (ok && d->ParameterNode)
    {
      d->ParameterNode->SetSysTime(scadatime);
      qDebug() << Q_FUNC_INFO << "Scada time: " << scadatime;
    }
    // Check if it is a structured type
//    if (value.canConvert<QOpcUaExtensionObject>())
//    {
//      QOpcUaExtensionObject extObj = value.value<QOpcUaExtensionObject>();
//      bool res = d->parseExtendedObject(extObj);
//    }
  }
}

void qSlicerScadaOpcUaLogic::onScadaLocalTimeAttributeChanged(
  QOpcUa::NodeAttribute attr, const QVariant &value)
{
  Q_D(qSlicerScadaOpcUaLogic);
 
  bool valueChanged = (attr == QOpcUa::NodeAttribute::Value);
  if (valueChanged)
  {
    bool ok = false;
    uint64_t scadatime = value.toULongLong(&ok);
    if (ok && d->ParameterNode)
    {
      d->ParameterNode->SetSysTime(scadatime);
      qDebug() << Q_FUNC_INFO << "Scada time value: " << quint64(scadatime);
    }
  }
}

void qSlicerScadaOpcUaLogic::onScadaLocalTimeEnableMonitoringFinished(QOpcUa::NodeAttribute attr, QOpcUa::UaStatusCode status)
{
  Q_UNUSED(attr);
  if (!sender())
  {
    return;
  }
  if (status == QOpcUa::UaStatusCode::Good)
  {
    qDebug() << Q_FUNC_INFO << "Monitoring successfully enabled for" << qobject_cast<QOpcUaNode *>(sender())->nodeId();
  }
  else
  {
    qDebug() << Q_FUNC_INFO << "Failed to enable monitoring for" << qobject_cast<QOpcUaNode *>(sender())->nodeId() << ":" << status;
  }
}
*/
//-----------------------------------------------------------------------------
bool qSlicerScadaOpcUaLogic::getClientConnectedFlag() const
{
  Q_D(const qSlicerScadaOpcUaLogic);
  return d->ClientConnectedFlag;
}

//-----------------------------------------------------------------------------
QOpcUaClient* qSlicerScadaOpcUaLogic::getClientConnected() const
{
  Q_D(const qSlicerScadaOpcUaLogic);
  return d->OpcUaClient.get();
}


bool qSlicerScadaOpcUaLogic::setPatientPositioningLocalTime(const QDateTime& dt)
{
  Q_D(qSlicerScadaOpcUaLogic);
  qint64 localtime = dt.toMSecsSinceEpoch();
  if (d->OpcUaPatPosLocalTimeNode)
  {
    return d->OpcUaPatPosLocalTimeNode->writeAttribute(QOpcUa::NodeAttribute::Value, localtime, QOpcUa::Types::UInt64);
  }
  return false;
}

bool qSlicerScadaOpcUaLogic::setPatientPositioningErrorMessage(const QString& errorMsg)
{
  Q_D(qSlicerScadaOpcUaLogic);
  QDateTime currentTime = QDateTime::currentDateTime();
  if (d->OpcUaPatPosErrorMessageNode)
  {
    QString val = currentTime.toString("dd.MM.yyyy hh:mm:ss") + QString(": ") + errorMsg;
    return d->OpcUaPatPosErrorMessageNode->writeAttribute(QOpcUa::NodeAttribute::Value, val, QOpcUa::Types::String);
  }
  return false;
}

bool qSlicerScadaOpcUaLogic::setPatientPositioningEventMessage(const QString& eventMsg)
{
  Q_D(qSlicerScadaOpcUaLogic);
  QDateTime currentTime = QDateTime::currentDateTime();
  if (d->OpcUaPatPosEventMessageNode)
  {
    QString val = currentTime.toString("dd.MM.yyyy hh:mm:ss") + QString(": ") + eventMsg;
    return d->OpcUaPatPosEventMessageNode->writeAttribute(QOpcUa::NodeAttribute::Value, val, QOpcUa::Types::String);
  }
  return false;
}

bool qSlicerScadaOpcUaLogic::setButtonAmR1LoadToIsoPressed(bool state)
{
  Q_D(qSlicerScadaOpcUaLogic);
  if (d->OpcUaScadaAmR1LoadToIsoIsPressedNode)
  {
    QVariant pressed(state);
    return d->OpcUaScadaAmR1LoadToIsoIsPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, pressed, QOpcUa::Types::Boolean);
  }
  return false;
}

bool qSlicerScadaOpcUaLogic::checkButtonAmR1LoadToIsoIsEnabled(bool& state)
{
  Q_D(qSlicerScadaOpcUaLogic);
  if (!d->OpcUaScadaLocalTimeNode)
  {
    state = false;
    return false;
  }
  bool res = false;
  QOpcUaNode* opcNode = d->OpcUaScadaLocalTimeNode.get();
  if (opcNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
  {
    QVariant value = opcNode->attribute(QOpcUa::NodeAttribute::Value);
    state = true;
    res = value.toBool(); // Get the attribute from the cache
  }
  return res;
}

bool qSlicerScadaOpcUaLogic::setButtonAmR1ToNewCoordsPressed(bool state)
{
  Q_D(qSlicerScadaOpcUaLogic);
  if (d->OpcUaScadaAmR1ToNewCoordsIsPressedNode)
  {
    QVariant pressed(state);
    return d->OpcUaScadaAmR1ToNewCoordsIsPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, pressed, QOpcUa::Types::Boolean);
  }
  return false;
}

bool qSlicerScadaOpcUaLogic::setButtonAmR1ToHomePressed(bool state)
{
  Q_D(qSlicerScadaOpcUaLogic);
  if (d->OpcUaScadaAmR1ToHomeIsPressedNode)
  {
    QVariant pressed(state);
    return d->OpcUaScadaAmR1ToHomeIsPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, pressed, QOpcUa::Types::Boolean);
  }
  return false;
}

bool qSlicerScadaOpcUaLogic::setButtonAmR1ToLoadPressed(bool state)
{
  Q_D(qSlicerScadaOpcUaLogic);
  if (d->OpcUaScadaAmR1ToLoadIsPressedNode)
  {
    QVariant pressed(state);
    return d->OpcUaScadaAmR1ToLoadIsPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, pressed, QOpcUa::Types::Boolean);
  }
  return false;
}

bool qSlicerScadaOpcUaLogic::setButtonAmR2ToIsoPressed(bool state)
{
  Q_D(qSlicerScadaOpcUaLogic);
  if (d->OpcUaScadaAmR2ToIsoIsPressedNode)
  {
    QVariant pressed(state);
    return d->OpcUaScadaAmR2ToIsoIsPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, pressed, QOpcUa::Types::Boolean);
  }
  return false;
}

bool qSlicerScadaOpcUaLogic::setButtonAmR2ToSecondPlanePressed(bool state)
{
  Q_D(qSlicerScadaOpcUaLogic);
  if (d->OpcUaScadaAmR2ToSecondPlaneIsPressedNode)
  {
    QVariant pressed(state);
    return d->OpcUaScadaAmR2ToSecondPlaneIsPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, pressed, QOpcUa::Types::Boolean);
  }
  return false;
}

bool qSlicerScadaOpcUaLogic::setButtonAmR2MakeXrayPressed(bool state)
{
  Q_D(qSlicerScadaOpcUaLogic);
  if (d->OpcUaScadaAmR2MakeXrayIsPressedNode)
  {
    QVariant pressed(state);
    return d->OpcUaScadaAmR2MakeXrayIsPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, pressed, QOpcUa::Types::Boolean);
  }
  return false;
}

bool qSlicerScadaOpcUaLogic::setButtonAmR2ToHomePressed(bool state)
{
  Q_D(qSlicerScadaOpcUaLogic);
  if (d->OpcUaScadaAmR2ToHomeIsPressedNode)
  {
    QVariant pressed(state);
    return d->OpcUaScadaAmR2ToHomeIsPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, pressed, QOpcUa::Types::Boolean);
  }
  return false;
}

bool qSlicerScadaOpcUaLogic::setButtonAmR2SetNewZPressed(bool state)
{
  Q_D(qSlicerScadaOpcUaLogic);
  if (d->OpcUaScadaAmR2SetNewZIsPressedNode)
  {
    QVariant pressed(state);
    return d->OpcUaScadaAmR2SetNewZIsPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, pressed, QOpcUa::Types::Boolean);
  }
  return false;
}
