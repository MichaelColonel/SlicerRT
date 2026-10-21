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
#include <QOpcUaClient>
#include <QWeakPointer>
#include <QSharedPointer>
#include <QScopedPointer>
#include <QOpcUaNode>
#include <QProgressDialog>
#include <QTimer>

// Slicer includes
#include <qSlicerSingletonViewFactory.h>
#include <qSlicerLayoutManager.h>
#include <qSlicerApplication.h>

// SiemensPlcOpcUa Widgets includes
#include "qSlicerSiemensPlcOpcUaWidget.h"
#include "ui_qSlicerSiemensPlcOpcUaWidget.h"

#include "OpcUaModel.h"
#include "OpcUaTreeItem.h"
#include "qSlicerOpcUaRobotsMessagesListModel.h"

#include <vtkMRMLSiemensPlcOpcUaNode.h>

#include <bitset>

namespace {

constexpr const char* const PARENT_NODE_DISPLAYED_NAME = "ServerInterfaces";
//constexpr const char* SIEMENS_PLC_CURRENT_TIME_NODE_ID = "ns=0;i=2258";

constexpr const char* const KUKAX = "KUKA_X";
constexpr const char* const KUKAY = "KUKA_Y";
constexpr const char* const KUKAZ = "KUKA_Z";
constexpr const char* const KUKAA = "KUKA_A";
constexpr const char* const KUKAB = "KUKA_B";
constexpr const char* const KUKAC = "KUKA_C";

constexpr const char* const AXISCOORDA1 = "AXIS_cord_A1";
constexpr const char* const AXISCOORDA2 = "AXIS_cord_A2";
constexpr const char* const AXISCOORDA3 = "AXIS_cord_A3";
constexpr const char* const AXISCOORDA4 = "AXIS_cord_A4";
constexpr const char* const AXISCOORDA5 = "AXIS_cord_A5";
constexpr const char* const AXISCOORDA6 = "AXIS_cord_A6";

constexpr const char* const ASU = "ASU";
constexpr const char* const RTK_PLC = "RTK_PLC";

const QString DOT = QString(QChar('.'));
const QString SIEMENS_PLC_SERVER_INTERFACES_NODE_ID = QLatin1String("ns=3;s=") + PARENT_NODE_DISPLAYED_NAME;
const QString RTK_PLC_NODE_NAME = QLatin1String(PARENT_NODE_DISPLAYED_NAME) + DOT + QLatin1String(RTK_PLC);
const QString ASU_NODE_NAME = RTK_PLC_NODE_NAME + DOT + QLatin1String(ASU);

const QString ASU_ERROR_MESSAGES_NODE_NAME = ASU_NODE_NAME + DOT + QLatin1String(OpcUaTreeItem::ERROR_MESSAGES_NODE);
const QString ASU_SERVICE_MESSAGES_NODE_NAME = ASU_NODE_NAME + DOT + QLatin1String(OpcUaTreeItem::SERVICE_MESSAGES_NODE);
const QString ASU_MESSAGES_NODE_NAME = ASU_NODE_NAME + DOT + QLatin1String(OpcUaTreeItem::MISC_MESSAGES_NODE);

constexpr const char* const MODE = "Mode";
constexpr const char* const STATUS_RTK = "Status_RTK";
constexpr const char* const STATUS_R1 = "Status_R1_deka";
constexpr const char* const STATUS_R2 = "Status_R2_C_Duga";
constexpr const char* const RTK_HOMERETURN_HOLDPRESSED = "RTK_ComingHome_PressBtn";

const QString ASU_MODE_NODE_NAME = ASU_NODE_NAME + DOT + QLatin1String(MODE);
const QString ASU_STATUS_RTK_NODE_NAME = ASU_NODE_NAME + DOT + QLatin1String(STATUS_RTK);
const QString ASU_STATUS_R1_TABLE_NODE_NAME = ASU_NODE_NAME + DOT + QLatin1String(STATUS_R1);
const QString ASU_STATUS_R2_CARM_NODE_NAME = ASU_NODE_NAME + DOT + QLatin1String(STATUS_R2);
const QString ASU_RTK_COMINGHOME_NODE_NAME = ASU_NODE_NAME + DOT + QLatin1String(RTK_HOMERETURN_HOLDPRESSED);

constexpr const char* const SETCOORDS = "SetCoords";
const QString ASU_COORDS_NODE_NAME = ASU_NODE_NAME + DOT + QLatin1String(SETCOORDS);
const QString ASU_COORDS_X_NODE_NAME = ASU_COORDS_NODE_NAME + DOT + QLatin1String(KUKAX);
const QString ASU_COORDS_Y_NODE_NAME = ASU_COORDS_NODE_NAME + DOT + QLatin1String(KUKAY);
const QString ASU_COORDS_Z_NODE_NAME = ASU_COORDS_NODE_NAME + DOT + QLatin1String(KUKAZ);
const QString ASU_COORDS_A_NODE_NAME = ASU_COORDS_NODE_NAME + DOT + QLatin1String(KUKAA);
const QString ASU_COORDS_B_NODE_NAME = ASU_COORDS_NODE_NAME + DOT + QLatin1String(KUKAB);
const QString ASU_COORDS_C_NODE_NAME = ASU_COORDS_NODE_NAME + DOT + QLatin1String(KUKAC);

constexpr const char* const READYXRAY1 = "RobotsReadyForXray1";
constexpr const char* const READYXRAY2 = "RobotsReadyForXray2";
constexpr const char* const READYBEAM = "RobotsReadyForBeam";
constexpr const char* const TABLEPATIENT = "HUMAN_ON_DEKA";
constexpr const char* const TABLEPOSITION = "DekaPosition";
constexpr const char* const EMEVACFLAG = "EmEvacuationSignal";

const QString ASU_ROBOTS_READY_XRAY1_NODE_NAME = ASU_NODE_NAME + DOT + QLatin1String(READYXRAY1);
const QString ASU_ROBOTS_READY_XRAY2_NODE_NAME = ASU_NODE_NAME + DOT + QLatin1String(READYXRAY2);
const QString ASU_ROBOTS_READY_BEAM_NODE_NAME = ASU_NODE_NAME + DOT + QLatin1String(READYBEAM);
const QString ASU_PATIENT_ON_TABLE_NODE_NAME = ASU_NODE_NAME + DOT + QLatin1String(TABLEPATIENT);
const QString ASU_TABLE_POSITION_NODE_NAME = ASU_NODE_NAME + DOT + QLatin1String(TABLEPOSITION);
const QString ASU_EMEVAC_SIGNAL_NODE_NAME = ASU_NODE_NAME + DOT + QLatin1String(EMEVACFLAG);

constexpr const char* const PRESSED = "isPressed";
constexpr const char* const ENABLED = "isEnable";

constexpr const char* const BUTTONS = "Buttons";
constexpr const char* const AM_MM = "AM_MM";

const QString IS_PRESSED = ".isPressed";
const QString IS_ENABLED = ".isEnable";

const QString BUTTONS_NODE_NAME = ASU_NODE_NAME + DOT + QLatin1String(BUTTONS);
const QString BUTTONS_AM_NODE_NAME = BUTTONS_NODE_NAME + DOT + QLatin1String(AM_MM);

const QString BUTTONS_AM_R1_LOADTOISO_NODE_NAME = BUTTONS_AM_NODE_NAME + ".R1_LoadToIso";
const QString BUTTONS_AM_R1_LOADTOISO_PRESSED_NODE_NAME = BUTTONS_AM_R1_LOADTOISO_NODE_NAME + IS_PRESSED;
const QString BUTTONS_AM_R1_LOADTOISO_ENABLED_NODE_NAME = BUTTONS_AM_R1_LOADTOISO_NODE_NAME + IS_ENABLED;

const QString BUTTONS_AM_R1_NEWCOORDS_NODE_NAME = BUTTONS_AM_NODE_NAME + ".R1_ToNewCoords";
const QString BUTTONS_AM_R1_NEWCOORDS_PRESSED_NODE_NAME = BUTTONS_AM_R1_NEWCOORDS_NODE_NAME + IS_PRESSED;
const QString BUTTONS_AM_R1_NEWCOORDS_ENABLED_NODE_NAME = BUTTONS_AM_R1_NEWCOORDS_NODE_NAME + IS_ENABLED;

const QString BUTTONS_AM_R1_TOLOAD_NODE_NAME = BUTTONS_AM_NODE_NAME + ".R1_ToLoad";
const QString BUTTONS_AM_R1_TOLOAD_PRESSED_NODE_NAME = BUTTONS_AM_R1_TOLOAD_NODE_NAME + IS_PRESSED;
const QString BUTTONS_AM_R1_TOLOAD_ENABLED_NODE_NAME = BUTTONS_AM_R1_TOLOAD_NODE_NAME + IS_ENABLED;

const QString BUTTONS_AM_R2_TOPLANE1_NODE_NAME = BUTTONS_AM_NODE_NAME + ".R2_ToPL1";
const QString BUTTONS_AM_R2_TOPLANE1_PRESSED_NODE_NAME = BUTTONS_AM_R2_TOPLANE1_NODE_NAME + IS_PRESSED;
const QString BUTTONS_AM_R2_TOPLANE1_ENABLED_NODE_NAME = BUTTONS_AM_R2_TOPLANE1_NODE_NAME + IS_ENABLED;

const QString BUTTONS_AM_R2_TOPLANE2_NODE_NAME = BUTTONS_AM_NODE_NAME + ".R2_ToPL2";
const QString BUTTONS_AM_R2_TOPLANE2_PRESSED_NODE_NAME = BUTTONS_AM_R2_TOPLANE2_NODE_NAME + IS_PRESSED;
const QString BUTTONS_AM_R2_TOPLANE2_ENABLED_NODE_NAME = BUTTONS_AM_R2_TOPLANE2_NODE_NAME + IS_ENABLED;

const QString BUTTONS_AM_R2_TOHOME_NODE_NAME = BUTTONS_AM_NODE_NAME + ".R2_ToHome";
const QString BUTTONS_AM_R2_TOHOME_PRESSED_NODE_NAME = BUTTONS_AM_R2_TOHOME_NODE_NAME + IS_PRESSED;
const QString BUTTONS_AM_R2_TOHOME_ENABLED_NODE_NAME = BUTTONS_AM_R2_TOHOME_NODE_NAME + IS_ENABLED;

const QString BUTTONS_AM_R1R2_EMEVAC_NODE_NAME = BUTTONS_AM_NODE_NAME + ".R1R2_EmEvac";
const QString BUTTONS_AM_R1R2_EMEVAC_PRESSED_NODE_NAME = BUTTONS_AM_R1R2_EMEVAC_NODE_NAME + IS_PRESSED;
const QString BUTTONS_AM_R1R2_EMEVAC_ENABLED_NODE_NAME = BUTTONS_AM_R1R2_EMEVAC_NODE_NAME + IS_ENABLED;

const QString BUTTONS_AM_APPLYTABLEPOSITION_NODE_NAME = BUTTONS_AM_NODE_NAME + ".ApplyDekaPosition";
const QString BUTTONS_AM_APPLYTABLEPOSITION_PRESSED_NODE_NAME = BUTTONS_AM_APPLYTABLEPOSITION_NODE_NAME + IS_PRESSED;
const QString BUTTONS_AM_APPLYTABLEPOSITION_ENABLED_NODE_NAME = BUTTONS_AM_APPLYTABLEPOSITION_NODE_NAME + IS_ENABLED;

const QString BUTTONS_AM_APPLYNEWCOORDS_NODE_NAME = BUTTONS_AM_NODE_NAME + ".ApplyNewCoords";
const QString BUTTONS_AM_APPLYNEWCOORDS_PRESSED_NODE_NAME = BUTTONS_AM_APPLYNEWCOORDS_NODE_NAME + IS_PRESSED;
const QString BUTTONS_AM_APPLYNEWCOORDS_ENABLED_NODE_NAME = BUTTONS_AM_APPLYNEWCOORDS_NODE_NAME + IS_ENABLED;

constexpr const char* const SM = "SM";
const QString BUTTONS_SM_NODE_NAME = BUTTONS_NODE_NAME + DOT + QLatin1String(SM);

const QString BUTTONS_SM_R1_BREAKTEST_NODE_NAME = BUTTONS_SM_NODE_NAME + ".BrakeTestR1";
const QString BUTTONS_SM_R1_BREAKTEST_PRESSED_NODE_NAME = BUTTONS_SM_R1_BREAKTEST_NODE_NAME + IS_PRESSED;
const QString BUTTONS_SM_R1_BREAKTEST_ENABLED_NODE_NAME = BUTTONS_SM_R1_BREAKTEST_NODE_NAME + IS_ENABLED;

const QString BUTTONS_SM_R1_MASREFTEST_NODE_NAME = BUTTONS_SM_NODE_NAME + ".MasRefTestR1";
const QString BUTTONS_SM_R1_MASREFTEST_PRESSED_NODE_NAME = BUTTONS_SM_R1_MASREFTEST_NODE_NAME + IS_PRESSED;
const QString BUTTONS_SM_R1_MASREFTEST_ENABLED_NODE_NAME = BUTTONS_SM_R1_MASREFTEST_NODE_NAME + IS_ENABLED;

const QString BUTTONS_SM_R1_LOAD_NODE_NAME = BUTTONS_SM_NODE_NAME + ".LoadR1";
const QString BUTTONS_SM_R1_LOAD_PRESSED_NODE_NAME = BUTTONS_SM_R1_LOAD_NODE_NAME + IS_PRESSED;
const QString BUTTONS_SM_R1_LOAD_ENABLED_NODE_NAME = BUTTONS_SM_R1_LOAD_NODE_NAME + IS_ENABLED;

const QString BUTTONS_SM_R1_SERVICEPOS1_NODE_NAME = BUTTONS_SM_NODE_NAME + ".ServicePos1_R1";
const QString BUTTONS_SM_R1_SERVICEPOS1_PRESSED_NODE_NAME = BUTTONS_SM_R1_SERVICEPOS1_NODE_NAME + IS_PRESSED;
const QString BUTTONS_SM_R1_SERVICEPOS1_ENABLED_NODE_NAME = BUTTONS_SM_R1_SERVICEPOS1_NODE_NAME + IS_ENABLED;

const QString BUTTONS_SM_R1_SERVICEPOS2_NODE_NAME = BUTTONS_SM_NODE_NAME + ".ServicePos2_R1";
const QString BUTTONS_SM_R1_SERVICEPOS2_PRESSED_NODE_NAME = BUTTONS_SM_R1_SERVICEPOS2_NODE_NAME + IS_PRESSED;
const QString BUTTONS_SM_R1_SERVICEPOS2_ENABLED_NODE_NAME = BUTTONS_SM_R1_SERVICEPOS2_NODE_NAME + IS_ENABLED;

const QString BUTTONS_SM_R1_SERVICEPOS3_NODE_NAME = BUTTONS_SM_NODE_NAME + ".ServicePos3_R1";
const QString BUTTONS_SM_R1_SERVICEPOS3_PRESSED_NODE_NAME = BUTTONS_SM_R1_SERVICEPOS3_NODE_NAME + IS_PRESSED;
const QString BUTTONS_SM_R1_SERVICEPOS3_ENABLED_NODE_NAME = BUTTONS_SM_R1_SERVICEPOS3_NODE_NAME + IS_ENABLED;

const QString BUTTONS_SM_R2_BREAKTEST_NODE_NAME = BUTTONS_SM_NODE_NAME + ".BrakeTestR2";
const QString BUTTONS_SM_R2_BREAKTEST_PRESSED_NODE_NAME = BUTTONS_SM_R2_BREAKTEST_NODE_NAME + IS_PRESSED;
const QString BUTTONS_SM_R2_BREAKTEST_ENABLED_NODE_NAME = BUTTONS_SM_R2_BREAKTEST_NODE_NAME + IS_ENABLED;

const QString BUTTONS_SM_R2_MASREFTEST_NODE_NAME = BUTTONS_SM_NODE_NAME + ".MasRefTestR2";
const QString BUTTONS_SM_R2_MASREFTEST_PRESSED_NODE_NAME = BUTTONS_SM_R2_MASREFTEST_NODE_NAME + IS_PRESSED;
const QString BUTTONS_SM_R2_MASREFTEST_ENABLED_NODE_NAME = BUTTONS_SM_R2_MASREFTEST_NODE_NAME + IS_ENABLED;

const QString BUTTONS_SM_R2_HOME_NODE_NAME = BUTTONS_SM_NODE_NAME + ".HomeR2";
const QString BUTTONS_SM_R2_HOME_PRESSED_NODE_NAME = BUTTONS_SM_R2_HOME_NODE_NAME + IS_PRESSED;
const QString BUTTONS_SM_R2_HOME_ENABLED_NODE_NAME = BUTTONS_SM_R2_HOME_NODE_NAME + IS_ENABLED;

const QString BUTTONS_SM_R2_SERVICEPOS1_NODE_NAME = BUTTONS_SM_NODE_NAME + ".ServicePos1_R2";
const QString BUTTONS_SM_R2_SERVICEPOS1_PRESSED_NODE_NAME = BUTTONS_SM_R2_SERVICEPOS1_NODE_NAME + IS_PRESSED;
const QString BUTTONS_SM_R2_SERVICEPOS1_ENABLED_NODE_NAME = BUTTONS_SM_R2_SERVICEPOS1_NODE_NAME + IS_ENABLED;

const QString BUTTONS_SM_R2_SERVICEPOS2_NODE_NAME = BUTTONS_SM_NODE_NAME + ".ServicePos2_R2";
const QString BUTTONS_SM_R2_SERVICEPOS2_PRESSED_NODE_NAME = BUTTONS_SM_R2_SERVICEPOS2_NODE_NAME + IS_PRESSED;
const QString BUTTONS_SM_R2_SERVICEPOS2_ENABLED_NODE_NAME = BUTTONS_SM_R2_SERVICEPOS2_NODE_NAME + IS_ENABLED;

const QString BUTTONS_SM_R2_SERVICEPOS3_NODE_NAME = BUTTONS_SM_NODE_NAME + ".ServicePos3_R2";
const QString BUTTONS_SM_R2_SERVICEPOS3_PRESSED_NODE_NAME = BUTTONS_SM_R2_SERVICEPOS3_NODE_NAME + IS_PRESSED;
const QString BUTTONS_SM_R2_SERVICEPOS3_ENABLED_NODE_NAME = BUTTONS_SM_R2_SERVICEPOS3_NODE_NAME + IS_ENABLED;

const QString BUTTONS_SM_RESTARTSM_NODE_NAME = BUTTONS_SM_NODE_NAME + ".RestartSM";
const QString BUTTONS_SM_RESTARTSM_PRESSED_NODE_NAME = BUTTONS_SM_RESTARTSM_NODE_NAME + IS_PRESSED;
const QString BUTTONS_SM_RESTARTSM_ENABLED_NODE_NAME = BUTTONS_SM_RESTARTSM_NODE_NAME + IS_ENABLED;

constexpr const char* const KCM = "KCM";
const QString BUTTONS_KCM_NODE_NAME = BUTTONS_NODE_NAME + DOT + QLatin1String(KCM);

const QString BUTTONS_KCM_ALLOWT1_NODE_NAME = BUTTONS_KCM_NODE_NAME + ".AllowT1";
const QString BUTTONS_KCM_ALLOWT1_PRESSED_NODE_NAME = BUTTONS_KCM_ALLOWT1_NODE_NAME + IS_PRESSED;
const QString BUTTONS_KCM_ALLOWT1_ENABLED_NODE_NAME = BUTTONS_KCM_ALLOWT1_NODE_NAME + IS_ENABLED;

const QString BUTTONS_KCM_ALLOWXRAY_NODE_NAME = BUTTONS_KCM_NODE_NAME + ".AllowXRay";
const QString BUTTONS_KCM_ALLOWXRAY_PRESSED_NODE_NAME = BUTTONS_KCM_ALLOWXRAY_NODE_NAME + IS_PRESSED;
const QString BUTTONS_KCM_ALLOWXRAY_ENABLED_NODE_NAME = BUTTONS_KCM_ALLOWXRAY_NODE_NAME + IS_ENABLED;

const QString BUTTONS_KCM_ALLOWBEAM_NODE_NAME = BUTTONS_KCM_NODE_NAME + ".AllowBeam";
const QString BUTTONS_KCM_ALLOWBEAM_PRESSED_NODE_NAME = BUTTONS_KCM_ALLOWBEAM_NODE_NAME + IS_PRESSED;
const QString BUTTONS_KCM_ALLOWBEAM_ENABLED_NODE_NAME = BUTTONS_KCM_ALLOWBEAM_NODE_NAME + IS_ENABLED;

const QString BUTTONS_RESETERRORS_NODE_NAME = BUTTONS_NODE_NAME + ".ResetErrors";
const QString BUTTONS_RESETERRORS_PRESSED_NODE_NAME = BUTTONS_RESETERRORS_NODE_NAME + IS_PRESSED;
const QString BUTTONS_RESETERRORS_ENABLED_NODE_NAME = BUTTONS_RESETERRORS_NODE_NAME + IS_ENABLED;

const QString BUTTONS_MAKEXRAY_NODE_NAME = BUTTONS_NODE_NAME + ".MakeXRay";
const QString BUTTONS_MAKEXRAY_PRESSED_NODE_NAME = BUTTONS_MAKEXRAY_NODE_NAME + IS_PRESSED;
const QString BUTTONS_MAKEXRAY_ENABLED_NODE_NAME = BUTTONS_MAKEXRAY_NODE_NAME + IS_ENABLED;

// Reflection target coordinates R1 (confirmed or accepted coordinates by the R1 Tabletop robot)
const QString KUKA_REFLTARGETCOORDS_R1_NODE_NAME = RTK_PLC_NODE_NAME + ".ReflTargetCoords_R1";
const QString KUKA_REFLTARGETCOORDS_R1_X_NODE_NAME = KUKA_REFLTARGETCOORDS_R1_NODE_NAME + DOT + QLatin1String(KUKAX);
const QString KUKA_REFLTARGETCOORDS_R1_Y_NODE_NAME = KUKA_REFLTARGETCOORDS_R1_NODE_NAME + DOT + QLatin1String(KUKAY);
const QString KUKA_REFLTARGETCOORDS_R1_Z_NODE_NAME = KUKA_REFLTARGETCOORDS_R1_NODE_NAME + DOT + QLatin1String(KUKAZ);
const QString KUKA_REFLTARGETCOORDS_R1_A_NODE_NAME = KUKA_REFLTARGETCOORDS_R1_NODE_NAME + DOT + QLatin1String(KUKAA);
const QString KUKA_REFLTARGETCOORDS_R1_B_NODE_NAME = KUKA_REFLTARGETCOORDS_R1_NODE_NAME + DOT + QLatin1String(KUKAB);
const QString KUKA_REFLTARGETCOORDS_R1_C_NODE_NAME = KUKA_REFLTARGETCOORDS_R1_NODE_NAME + DOT + QLatin1String(KUKAC);

// Reflection target coordinates R2 (confirmed or accepted coordinates by the R2 C-Arm robot)
const QString KUKA_REFLTARGETCOORDS_R2_NODE_NAME = RTK_PLC_NODE_NAME + ".ReflTargetCoords_R2";
const QString KUKA_REFLTARGETCOORDS_R2_X_NODE_NAME = KUKA_REFLTARGETCOORDS_R2_NODE_NAME + DOT + QLatin1String(KUKAX);
const QString KUKA_REFLTARGETCOORDS_R2_Y_NODE_NAME = KUKA_REFLTARGETCOORDS_R2_NODE_NAME + DOT + QLatin1String(KUKAY);
const QString KUKA_REFLTARGETCOORDS_R2_Z_NODE_NAME = KUKA_REFLTARGETCOORDS_R2_NODE_NAME + DOT + QLatin1String(KUKAZ);
const QString KUKA_REFLTARGETCOORDS_R2_A_NODE_NAME = KUKA_REFLTARGETCOORDS_R2_NODE_NAME + DOT + QLatin1String(KUKAA);
const QString KUKA_REFLTARGETCOORDS_R2_B_NODE_NAME = KUKA_REFLTARGETCOORDS_R2_NODE_NAME + DOT + QLatin1String(KUKAB);
const QString KUKA_REFLTARGETCOORDS_R2_C_NODE_NAME = KUKA_REFLTARGETCOORDS_R2_NODE_NAME + DOT + QLatin1String(KUKAC);

const QString KUKA_CURRENTCOORDS_R1_NODE_NAME = RTK_PLC_NODE_NAME + ".CurrentCoords_R1";
const QString KUKA_CURRENTCOORDS_R1_X_NODE_NAME = KUKA_CURRENTCOORDS_R1_NODE_NAME + DOT + QLatin1String(KUKAX);
const QString KUKA_CURRENTCOORDS_R1_Y_NODE_NAME = KUKA_CURRENTCOORDS_R1_NODE_NAME + DOT + QLatin1String(KUKAY);
const QString KUKA_CURRENTCOORDS_R1_Z_NODE_NAME = KUKA_CURRENTCOORDS_R1_NODE_NAME + DOT + QLatin1String(KUKAZ);
const QString KUKA_CURRENTCOORDS_R1_A_NODE_NAME = KUKA_CURRENTCOORDS_R1_NODE_NAME + DOT + QLatin1String(KUKAA);
const QString KUKA_CURRENTCOORDS_R1_B_NODE_NAME = KUKA_CURRENTCOORDS_R1_NODE_NAME + DOT + QLatin1String(KUKAB);
const QString KUKA_CURRENTCOORDS_R1_C_NODE_NAME = KUKA_CURRENTCOORDS_R1_NODE_NAME + DOT + QLatin1String(KUKAC);
const QString KUKA_CURRENTCOORDS_R1_A1_NODE_NAME = KUKA_CURRENTCOORDS_R1_NODE_NAME + DOT + QLatin1String(AXISCOORDA1);
const QString KUKA_CURRENTCOORDS_R1_A2_NODE_NAME = KUKA_CURRENTCOORDS_R1_NODE_NAME + DOT + QLatin1String(AXISCOORDA2);
const QString KUKA_CURRENTCOORDS_R1_A3_NODE_NAME = KUKA_CURRENTCOORDS_R1_NODE_NAME + DOT + QLatin1String(AXISCOORDA3);
const QString KUKA_CURRENTCOORDS_R1_A4_NODE_NAME = KUKA_CURRENTCOORDS_R1_NODE_NAME + DOT + QLatin1String(AXISCOORDA4);
const QString KUKA_CURRENTCOORDS_R1_A5_NODE_NAME = KUKA_CURRENTCOORDS_R1_NODE_NAME + DOT + QLatin1String(AXISCOORDA5);
const QString KUKA_CURRENTCOORDS_R1_A6_NODE_NAME = KUKA_CURRENTCOORDS_R1_NODE_NAME + DOT + QLatin1String(AXISCOORDA6);

const QString KUKA_CURRENTCOORDS_R2_NODE_NAME = RTK_PLC_NODE_NAME + ".CurrentCoords_R2";
const QString KUKA_CURRENTCOORDS_R2_X_NODE_NAME = KUKA_CURRENTCOORDS_R2_NODE_NAME + DOT + QLatin1String(KUKAX);
const QString KUKA_CURRENTCOORDS_R2_Y_NODE_NAME = KUKA_CURRENTCOORDS_R2_NODE_NAME + DOT + QLatin1String(KUKAY);
const QString KUKA_CURRENTCOORDS_R2_Z_NODE_NAME = KUKA_CURRENTCOORDS_R2_NODE_NAME + DOT + QLatin1String(KUKAZ);
const QString KUKA_CURRENTCOORDS_R2_A_NODE_NAME = KUKA_CURRENTCOORDS_R2_NODE_NAME + DOT + QLatin1String(KUKAA);
const QString KUKA_CURRENTCOORDS_R2_B_NODE_NAME = KUKA_CURRENTCOORDS_R2_NODE_NAME + DOT + QLatin1String(KUKAB);
const QString KUKA_CURRENTCOORDS_R2_C_NODE_NAME = KUKA_CURRENTCOORDS_R2_NODE_NAME + DOT + QLatin1String(KUKAC);
const QString KUKA_CURRENTCOORDS_R2_A1_NODE_NAME = KUKA_CURRENTCOORDS_R2_NODE_NAME + DOT + QLatin1String(AXISCOORDA1);
const QString KUKA_CURRENTCOORDS_R2_A2_NODE_NAME = KUKA_CURRENTCOORDS_R2_NODE_NAME + DOT + QLatin1String(AXISCOORDA2);
const QString KUKA_CURRENTCOORDS_R2_A3_NODE_NAME = KUKA_CURRENTCOORDS_R2_NODE_NAME + DOT + QLatin1String(AXISCOORDA3);
const QString KUKA_CURRENTCOORDS_R2_A4_NODE_NAME = KUKA_CURRENTCOORDS_R2_NODE_NAME + DOT + QLatin1String(AXISCOORDA4);
const QString KUKA_CURRENTCOORDS_R2_A5_NODE_NAME = KUKA_CURRENTCOORDS_R2_NODE_NAME + DOT + QLatin1String(AXISCOORDA5);
const QString KUKA_CURRENTCOORDS_R2_A6_NODE_NAME = KUKA_CURRENTCOORDS_R2_NODE_NAME + DOT + QLatin1String(AXISCOORDA6);

const QOpcUaMonitoringParameters OpcUaMonitorParameters(100);

const QStringList ReadValueFullNodeNames = {
    ASU_ERROR_MESSAGES_NODE_NAME,
    ASU_SERVICE_MESSAGES_NODE_NAME,
    ASU_MESSAGES_NODE_NAME,
    ASU_MODE_NODE_NAME,
    ASU_STATUS_RTK_NODE_NAME,
    ASU_STATUS_R1_TABLE_NODE_NAME,
    ASU_STATUS_R2_CARM_NODE_NAME,
    BUTTONS_AM_R1_LOADTOISO_ENABLED_NODE_NAME,
    BUTTONS_AM_R1_TOLOAD_ENABLED_NODE_NAME,
    BUTTONS_AM_R1_NEWCOORDS_ENABLED_NODE_NAME,
    BUTTONS_AM_R2_TOHOME_ENABLED_NODE_NAME,
    BUTTONS_AM_R2_TOPLANE1_ENABLED_NODE_NAME,
    BUTTONS_AM_R2_TOPLANE2_ENABLED_NODE_NAME,
    BUTTONS_AM_R1R2_EMEVAC_ENABLED_NODE_NAME,
    BUTTONS_AM_APPLYTABLEPOSITION_ENABLED_NODE_NAME,
    BUTTONS_SM_R1_BREAKTEST_ENABLED_NODE_NAME,
    BUTTONS_SM_R1_MASREFTEST_ENABLED_NODE_NAME,
    BUTTONS_SM_R1_LOAD_ENABLED_NODE_NAME,
    BUTTONS_SM_R1_SERVICEPOS1_ENABLED_NODE_NAME,
    BUTTONS_SM_R1_SERVICEPOS2_ENABLED_NODE_NAME,
    BUTTONS_SM_R1_SERVICEPOS3_ENABLED_NODE_NAME,
    BUTTONS_SM_R2_BREAKTEST_ENABLED_NODE_NAME,
    BUTTONS_SM_R2_MASREFTEST_ENABLED_NODE_NAME,
    BUTTONS_SM_R2_HOME_ENABLED_NODE_NAME,
    BUTTONS_SM_R2_SERVICEPOS1_ENABLED_NODE_NAME,
    BUTTONS_SM_R2_SERVICEPOS2_ENABLED_NODE_NAME,
    BUTTONS_SM_R2_SERVICEPOS3_ENABLED_NODE_NAME,
    BUTTONS_SM_RESTARTSM_ENABLED_NODE_NAME,
    BUTTONS_KCM_ALLOWT1_ENABLED_NODE_NAME,
    BUTTONS_KCM_ALLOWXRAY_ENABLED_NODE_NAME,
    BUTTONS_KCM_ALLOWBEAM_ENABLED_NODE_NAME,
    ASU_ROBOTS_READY_XRAY1_NODE_NAME,
    ASU_ROBOTS_READY_XRAY2_NODE_NAME,
    ASU_TABLE_POSITION_NODE_NAME,
    ASU_EMEVAC_SIGNAL_NODE_NAME,
    KUKA_CURRENTCOORDS_R1_A1_NODE_NAME,
    KUKA_CURRENTCOORDS_R1_A2_NODE_NAME,
    KUKA_CURRENTCOORDS_R1_A3_NODE_NAME,
    KUKA_CURRENTCOORDS_R1_A4_NODE_NAME,
    KUKA_CURRENTCOORDS_R1_A5_NODE_NAME,
    KUKA_CURRENTCOORDS_R1_A6_NODE_NAME,
    KUKA_CURRENTCOORDS_R2_A1_NODE_NAME,
    KUKA_CURRENTCOORDS_R2_A2_NODE_NAME,
    KUKA_CURRENTCOORDS_R2_A3_NODE_NAME,
    KUKA_CURRENTCOORDS_R2_A4_NODE_NAME,
    KUKA_CURRENTCOORDS_R2_A5_NODE_NAME,
    KUKA_CURRENTCOORDS_R2_A6_NODE_NAME,
    KUKA_CURRENTCOORDS_R1_A_NODE_NAME,
    KUKA_CURRENTCOORDS_R1_B_NODE_NAME,
    KUKA_CURRENTCOORDS_R1_C_NODE_NAME,
    KUKA_CURRENTCOORDS_R1_X_NODE_NAME,
    KUKA_CURRENTCOORDS_R1_Y_NODE_NAME,
    KUKA_CURRENTCOORDS_R1_Z_NODE_NAME,
    KUKA_CURRENTCOORDS_R2_A_NODE_NAME,
    KUKA_CURRENTCOORDS_R2_B_NODE_NAME,
    KUKA_CURRENTCOORDS_R2_C_NODE_NAME,
    KUKA_CURRENTCOORDS_R2_X_NODE_NAME,
    KUKA_CURRENTCOORDS_R2_Y_NODE_NAME,
    KUKA_CURRENTCOORDS_R2_Z_NODE_NAME
};

}

//-----------------------------------------------------------------------------
class NodeData {
public:
  NodeData() = default;
  NodeData(QOpcUaNode* node, const QString& id, const QString& name, const QString& desc)
    : nodePtr(node)
    , nodeId(id)
    , nodeName(name)
    , nodeDescription(desc)
  {
  }
  NodeData(const NodeData& obj)
    : nodePtr(obj.getNode())
    , nodeId(obj.getNodeId())
    , nodeName(obj.getNodeName())
    , nodeDescription(obj.getNodeDescription())
  {
  }
  NodeData& operator=(const NodeData& obj)
  {
    this->nodePtr = obj.getNodePtr();
    this->nodeId = obj.getNodeId();
    this->nodeName = obj.getNodeName();
    this->nodeDescription = obj.getNodeDescription();
    return *this;
  }
  virtual ~NodeData() {}
  QSharedPointer< QOpcUaNode > getNodePtr() const { return nodePtr; }
  void setNode(QOpcUaNode* node) { this->nodePtr.reset(node); }
  QOpcUaNode* getNode() { return nodePtr.data(); }
  QOpcUaNode* getNode() const { return nodePtr.data(); }
  QString getNodeId() const { return nodeId; }
  QString getNodeName() const { return nodeName; }
  QString getNodeDescription() const { return nodeDescription; }
protected:
  QSharedPointer< QOpcUaNode > nodePtr;
  QString nodeId;
  QString nodeName;
  QString nodeDescription;
};

//-----------------------------------------------------------------------------
class qSlicerSiemensPlcOpcUaWidgetPrivate : public Ui_qSlicerSiemensPlcOpcUaWidget
{
  Q_DECLARE_PUBLIC(qSlicerSiemensPlcOpcUaWidget)
protected:
  qSlicerSiemensPlcOpcUaWidget* const q_ptr;

public:
  qSlicerSiemensPlcOpcUaWidgetPrivate(qSlicerSiemensPlcOpcUaWidget& object);
  virtual void setupUi(qSlicerSiemensPlcOpcUaWidget*);

  bool ParseServerParentNode(const QString& nodeDisplayName);
  QOpcUaNode* FindNodeFromFullDisplayName(const QString& nodeDisplayName);
  void ExpandAllAsync(QTreeView *view, QAbstractItemModel* model,
    const QModelIndex &parentIndex = QModelIndex());

  bool ConnectMessagesNodes();
  bool ConnectModeAndStatusNodes();
  bool ConnectSetCoordsXyzNodes();
  bool ConnectSetCoordsAbcNodes();

  bool ConnectSetCoordsNodes();
  bool ConnectAutoManualMovementNodes();
  bool ConnectServiceMovementNodes();
  bool ConnectKukaMovementNodes();

  bool ConnectFlagNodes();
  bool ConnectMlcNodes();

  bool ConnectCurrentCoordsR1Nodes();
  bool ConnectCurrentCoordsR2Nodes();
  bool ConnectReflTargetCoordsR1Nodes();
  bool ConnectReflTargetCoordsR2Nodes();

  bool ConnectAutoManualR1ToLoadNodes();
  bool ConnectAutoManualR1LoadToIsoNodes();
  bool ConnectAutoManualR1ToNewCoordsNodes();
  bool ConnectAutoManualR2ToHomeNodes();
  bool ConnectAutoManualR2ToPlane1Nodes();
  bool ConnectAutoManualR2ToPlane2Nodes();
  bool ConnectAutoManualR1R2EmergencyEvacuationNodes();
  bool ConnectAutoManualApplyTablePositionNodes();
  bool ConnectAutoManualApplyNewCoordsNodes();

  bool ConnectServiceR1BreakTestNodes();
  bool ConnectServiceR1MasterRefTestNodes();
  bool ConnectServiceR1LoadNodes();
  bool ConnectServiceR1ServicePos1Nodes();
  bool ConnectServiceR1ServicePos2Nodes();
  bool ConnectServiceR1ServicePos3Nodes();

  bool ConnectServiceR2BreakTestNodes();
  bool ConnectServiceR2MasterRefTestNodes();
  bool ConnectServiceR2HomeNodes();
  bool ConnectServiceR2ServicePos1Nodes();
  bool ConnectServiceR2ServicePos2Nodes();
  bool ConnectServiceR2ServicePos3Nodes();
  bool ConnectServiceRestartNodes();

  bool ConnectKukaAllowT1Nodes();
  bool ConnectKukaAllowXrayNodes();
  bool ConnectKukaAllowBeamNodes();

  bool ConnectReflTargetCoordXR1Node();
  bool ConnectReflTargetCoordYR1Node();
  bool ConnectReflTargetCoordZR1Node();
  bool ConnectReflTargetCoordAR1Node();
  bool ConnectReflTargetCoordBR1Node();
  bool ConnectReflTargetCoordCR1Node();

  bool ConnectReflTargetCoordXR2Node();
  bool ConnectReflTargetCoordYR2Node();
  bool ConnectReflTargetCoordZR2Node();
  bool ConnectReflTargetCoordAR2Node();
  bool ConnectReflTargetCoordBR2Node();
  bool ConnectReflTargetCoordCR2Node();

  bool ConnectCurrentCoordXR1Node();
  bool ConnectCurrentCoordYR1Node();
  bool ConnectCurrentCoordZR1Node();
  bool ConnectCurrentCoordAR1Node();
  bool ConnectCurrentCoordBR1Node();
  bool ConnectCurrentCoordCR1Node();
  bool ConnectCurrentCoordA1R1Node();
  bool ConnectCurrentCoordA2R1Node();
  bool ConnectCurrentCoordA3R1Node();
  bool ConnectCurrentCoordA4R1Node();
  bool ConnectCurrentCoordA5R1Node();
  bool ConnectCurrentCoordA6R1Node();

  bool ConnectCurrentCoordXR2Node();
  bool ConnectCurrentCoordYR2Node();
  bool ConnectCurrentCoordZR2Node();
  bool ConnectCurrentCoordAR2Node();
  bool ConnectCurrentCoordBR2Node();
  bool ConnectCurrentCoordCR2Node();
  bool ConnectCurrentCoordA1R2Node();
  bool ConnectCurrentCoordA2R2Node();
  bool ConnectCurrentCoordA3R2Node();
  bool ConnectCurrentCoordA4R2Node();
  bool ConnectCurrentCoordA5R2Node();
  bool ConnectCurrentCoordA6R2Node();

  bool ConnectResetErrorsNodes();
  bool ConnectMakeXrayNodes();

  bool ConnectPatientOnTableTopNode();
  bool ConnectTablePositionNode();
  bool ConnectEmergencyEvacuationSignalNode();

  QScopedPointer< OpcUaModel > SiemensPlcOpcUaModel;

  QScopedPointer< qSlicerOpcUaRobotsMessagesListModel > SiemensPlcErrorListModel;
  QScopedPointer< qSlicerOpcUaRobotsMessagesListModel > SiemensPlcServiceListModel;
  QScopedPointer< qSlicerOpcUaRobotsMessagesListModel > SiemensPlcMessagesListModel;

  QWeakPointer< QOpcUaClient > OpcUaClient; // client
  vtkWeakPointer< vtkMRMLSiemensPlcOpcUaNode > ParameterNode; // MRML OPC UA parameters node

  // Parse parent node
  QScopedPointer< QOpcUaNode > ServerInterfacesNode;
  // Monitored node
  QScopedPointer< QOpcUaNode > CurrentTimeNode; // Siemens PLC monitored node to prevent session timeout ending
  QMap< QString, NodeData > NodeNameDataMap; // key - node unique full name, value - node data

  QScopedPointer< QProgressDialog > ProgressDialog;
  QScopedPointer< QTimer > ProgressTimer;

  bool AutoConnectFlag{ false };
  int NofNodesRead{ 0 };
};

// --------------------------------------------------------------------------
qSlicerSiemensPlcOpcUaWidgetPrivate::qSlicerSiemensPlcOpcUaWidgetPrivate(qSlicerSiemensPlcOpcUaWidget& object)
  : q_ptr(&object)
  , SiemensPlcOpcUaModel(new OpcUaModel(&object))
  , SiemensPlcErrorListModel(new qSlicerOpcUaRobotsMessagesListModel(&object))
  , SiemensPlcServiceListModel(new qSlicerOpcUaRobotsMessagesListModel(&object))
  , SiemensPlcMessagesListModel(new qSlicerOpcUaRobotsMessagesListModel(&object))
  , ProgressDialog(new QProgressDialog(&object))
  , ProgressTimer(new QTimer(&object))
{
}

// --------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidgetPrivate::setupUi(qSlicerSiemensPlcOpcUaWidget* widget)
{
  Q_Q(qSlicerSiemensPlcOpcUaWidget);

  this->Ui_qSlicerSiemensPlcOpcUaWidget::setupUi(widget);

  this->ProgressDialog->setWindowTitle(QObject::tr("Siemens PLC OPC UA"));
  this->ProgressDialog->setWindowModality(Qt::NonModal);
  this->ProgressDialog->setMinimumDuration(500);
  this->ProgressDialog->setAutoReset(false);
  this->ProgressDialog->setAutoClose(false);
  this->ProgressDialog->close();

  this->ProgressTimer->setSingleShot(true);
  this->ProgressTimer->setInterval(300);

  this->TreeView_OpcUaModel->setModel(SiemensPlcOpcUaModel.data());
  this->ListView_ErrorMessages->setModel(SiemensPlcErrorListModel.data());
  this->ListView_ServiceMessages->setModel(SiemensPlcServiceListModel.data());
  this->ListView_MiscMessages->setModel(SiemensPlcMessagesListModel.data());
  
  this->TreeView_OpcUaModel->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ParseServerParentNode(const QString& serverNodeDispName)
{
  if (!this->ServerInterfacesNode)
  {
    return false;
  }
  if (serverNodeDispName.isEmpty())
  {
    return false;
  }

  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();
  if (!opcUaClient)
  {
    return false;
  }
  OpcUaTreeItem* parentItem = nullptr;
  OpcUaTreeItem* rootItem = SiemensPlcOpcUaModel->getRootItem();
  if (rootItem)
  {
    parentItem = OpcUaTreeItem::findParentItemByName(rootItem, serverNodeDispName);
  }
  else
  {
    return false;
  }
  this->NodeNameDataMap.clear();
  if (parentItem)
  {
    QList< QPointer< OpcUaTreeItem > > items = OpcUaTreeItem::findLeafValueItems(parentItem);
    for (QPointer< OpcUaTreeItem > item : items)
    {
      QStringList parentNames = OpcUaTreeItem::findParentNamesForItem(item.data(), parentItem);

      if (parentNames.size())
      {
        QString hierNames;

        for (auto iter = parentNames.rbegin(); iter != parentNames.rend(); ++iter)
        {
          if (iter != parentNames.rend() - 1)
          {
            hierNames += *iter + QString(".");
          }
          else
          {
            hierNames += *iter;
          }
        }
        QOpcUaNode* itemNode = opcUaClient->node(item->getNodeId());

        NodeData data(itemNode, item->getNodeId(), hierNames, item->getNodeDescription());
        this->NodeNameDataMap[hierNames] = data;
      }
    }
  }
  return true;
}

//-----------------------------------------------------------------------------
QOpcUaNode* qSlicerSiemensPlcOpcUaWidgetPrivate::FindNodeFromFullDisplayName(const QString& nodeDisplayName)
{
  if (!this->NodeNameDataMap.size())
  {
    return nullptr;
  }
  for (auto& nodeData : this->NodeNameDataMap)
  {
    const QString& fullName = nodeData.getNodeName();
    if (fullName == nodeDisplayName)
    {
      return nodeData.getNode();
    }
  }
  return nullptr;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectMessagesNodes()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();
  // Error messages
  QOpcUaNode* errMessagesNode = this->FindNodeFromFullDisplayName(ASU_ERROR_MESSAGES_NODE_NAME);
  // Service messages
  QOpcUaNode* servMessagesNode = this->FindNodeFromFullDisplayName(ASU_SERVICE_MESSAGES_NODE_NAME);
  // Miscellaneous messages
  QOpcUaNode* miscMessagesNode = this->FindNodeFromFullDisplayName(ASU_MESSAGES_NODE_NAME);

  QProgressDialog* progressDialog = this->ProgressDialog.data();

  // Connect signal handlers for subscribed values
  // Error messages
  if (!errMessagesNode)
  {
    return false;
  }
  QObject::connect(errMessagesNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode, progressDialog](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert<QVariantList>())
      {
        std::bitset< vtkMRMLSiemensPlcOpcUaNode::MESSAGES_SIZE > errors;
        QVariantList errList = value.toList(); // Get the attribute from the cache
        if (errList.size() == vtkMRMLSiemensPlcOpcUaNode::MESSAGES_SIZE)
        {
          int i = 0;
          for (const QVariant& errFlag : errList)
          {
            bool value = errFlag.toBool();
            errors.set(i, value);
            ++i;
          }
          uint64_t errValue = errors.to_ullong();
          mrmlNode->SetErrorMessages(errValue);
          qDebug() << Q_FUNC_INFO << "Error messages value changed:" << errValue;
        }
      }
    }
  );
  QObject::connect(errMessagesNode,
    &QOpcUaNode::attributeRead, [errMessagesNode, mrmlNode, progressDialog](QOpcUa::NodeAttributes attr)
    {
      if (!errMessagesNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (errMessagesNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = errMessagesNode->attribute(QOpcUa::NodeAttribute::Value);
          if (value.canConvert<QVariantList>())
          {
            std::bitset< vtkMRMLSiemensPlcOpcUaNode::MESSAGES_SIZE > errors;
            QVariantList errList = value.toList(); // Get the attribute from the cache
            if (errList.size() == vtkMRMLSiemensPlcOpcUaNode::MESSAGES_SIZE)
            {
              int i = 0;
              for (const QVariant& errFlag : errList)
              {
                bool value = errFlag.toBool();
                errors.set(i, value);
                ++i;
              }
              uint64_t errValue = errors.to_ullong();
              mrmlNode->SetErrorMessages(errValue);
              qDebug() << Q_FUNC_INFO << "Error messages value read:" << errValue;
            }
          }
        }
      }
    }
  );
  // Subscribe to data changes
  errMessagesNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  // Service messages
  if (!servMessagesNode)
  {
    return false;
  }
  QObject::connect(servMessagesNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode, progressDialog](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert<QVariantList>())
      {
        std::bitset< vtkMRMLSiemensPlcOpcUaNode::MESSAGES_SIZE > servFlags;
        QVariantList servList = value.toList(); // Get the attribute from the cache
        if (servList.size() == vtkMRMLSiemensPlcOpcUaNode::MESSAGES_SIZE)
        {
          int i = 0;
          for (const QVariant& servFlag : servList)
          {
            bool value = servFlag.toBool();
            servFlags.set(i, value);
            ++i;
          }
          uint64_t servValue = servFlags.to_ullong();
          mrmlNode->SetServiceMessages(servValue);
          qDebug() << Q_FUNC_INFO << "Service messages value changed:" << servValue;
        }
      }
    }
  );
  QObject::connect(servMessagesNode,
    &QOpcUaNode::attributeRead, [servMessagesNode, mrmlNode, progressDialog](QOpcUa::NodeAttributes attr)
    {
      if (!servMessagesNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (servMessagesNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = servMessagesNode->attribute(QOpcUa::NodeAttribute::Value);
          if (value.canConvert<QVariantList>())
          {
            std::bitset< vtkMRMLSiemensPlcOpcUaNode::MESSAGES_SIZE > servFlags;
            QVariantList servList = value.toList(); // Get the attribute from the cache
            if (servList.size() == vtkMRMLSiemensPlcOpcUaNode::MESSAGES_SIZE)
            {
              int i = 0;
              for (const QVariant& servFlag : servList)
              {
                bool value = servFlag.toBool();
                servFlags.set(i, value);
                ++i;
              }
              uint64_t servValue = servFlags.to_ullong();
              mrmlNode->SetServiceMessages(servValue);
              qDebug() << Q_FUNC_INFO << "Service messages value read:" << servValue;
            }
          }
        }
      }
    }
  );
  // Subscribe to data changes
  servMessagesNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  // Misc messages
  if (!miscMessagesNode)
  {
    return false;
  }
  QObject::connect(miscMessagesNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode, progressDialog](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert<QVariantList>())
      {
        std::bitset< vtkMRMLSiemensPlcOpcUaNode::MESSAGES_SIZE > miscFlags;
        QVariantList miscList = value.toList(); // Get the attribute from the cache
        if (miscList.size() == vtkMRMLSiemensPlcOpcUaNode::MESSAGES_SIZE)
        {
          int i = 0;
          for (const QVariant& miscFlag : miscList)
          {
            bool value = miscFlag.toBool();
            miscFlags.set(i, value);
            ++i;
          }
          uint64_t miscValue = miscFlags.to_ullong();
          mrmlNode->SetMiscMessages(miscValue);
          qDebug() << Q_FUNC_INFO << "Miscellaneous messages value changed:" << miscValue;
        }
      }
    }
  );
  QObject::connect(miscMessagesNode,
    &QOpcUaNode::attributeRead, [miscMessagesNode, mrmlNode, progressDialog](QOpcUa::NodeAttributes attr)
    {
      if (!miscMessagesNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (miscMessagesNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = miscMessagesNode->attribute(QOpcUa::NodeAttribute::Value);
          if (value.canConvert<QVariantList>())
          {
            std::bitset< vtkMRMLSiemensPlcOpcUaNode::MESSAGES_SIZE > miscFlags;
            QVariantList miscList = value.toList(); // Get the attribute from the cache
            if (miscList.size() == vtkMRMLSiemensPlcOpcUaNode::MESSAGES_SIZE)
            {
              int i = 0;
              for (const QVariant& miscFlag : miscList)
              {
                bool value = miscFlag.toBool();
                miscFlags.set(i, value);
                ++i;
              }
              uint64_t miscValue = miscFlags.to_ullong();
              mrmlNode->SetMiscMessages(miscValue);
              qDebug() << Q_FUNC_INFO << "Miscellaneous messages value read:" << miscValue;
            }
          }
        }
      }
    }
  );
  // Subscribe to data changes
  miscMessagesNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectAutoManualR1ToLoadNodes()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();
  // AutoManual R1 ToLoad button enabled node
  QOpcUaNode* amR1ToLoadEnabledNode = this->FindNodeFromFullDisplayName(BUTTONS_AM_R1_TOLOAD_ENABLED_NODE_NAME);
  // AutoManual R1 ToLoad button pressed node
  QOpcUaNode* amR1ToLoadPressedNode = this->FindNodeFromFullDisplayName(BUTTONS_AM_R1_TOLOAD_PRESSED_NODE_NAME);

  // AutoManual R1 ToLoad button enabled node
  if (!amR1ToLoadEnabledNode)
  {
    return false;
  }
  QObject::connect(amR1ToLoadEnabledNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool enabledValue = value.toBool();
        bool amR1ToLoadCurrentState[2] = { false, false };
        mrmlNode->GetAutoManualR1ToLoad(amR1ToLoadCurrentState);
        amR1ToLoadCurrentState[0] = enabledValue;
        mrmlNode->SetAutoManualR1ToLoad(amR1ToLoadCurrentState);
        qDebug() << Q_FUNC_INFO << "AutoManual R1 ToLoad button enabled changed:" << enabledValue;
      }
    }
  );
  QObject::connect(amR1ToLoadEnabledNode,
    &QOpcUaNode::attributeRead, [amR1ToLoadEnabledNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!amR1ToLoadEnabledNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (amR1ToLoadEnabledNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool enabledValue = amR1ToLoadEnabledNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool amR1ToLoadCurrentState[2] = { false, false };
          mrmlNode->GetAutoManualR1ToLoad(amR1ToLoadCurrentState);
          amR1ToLoadCurrentState[0] = enabledValue;
          mrmlNode->SetAutoManualR1ToLoad(amR1ToLoadCurrentState);
          qDebug() << Q_FUNC_INFO << "AutoManual R1 ToLoad button enabled read:" << enabledValue;
        }
      }
    }
  );
  // Subscribe to data changes
  amR1ToLoadEnabledNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  // AutoManual R1 ToLoad button pressed node
  if (!amR1ToLoadPressedNode)
  {
    return false;
  }
  QObject::connect(amR1ToLoadPressedNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool pressedValue = value.toBool();
        bool amR1ToLoadCurrentState[2] = { false, false };
        mrmlNode->GetAutoManualR1ToLoad(amR1ToLoadCurrentState);
        amR1ToLoadCurrentState[1] = pressedValue;
        mrmlNode->SetAutoManualR1ToLoad(amR1ToLoadCurrentState);
        qDebug() << Q_FUNC_INFO << "AutoManual R1 ToLoad button pressed changed:" << pressedValue;
      }
    }
  );
  QObject::connect(amR1ToLoadPressedNode,
    &QOpcUaNode::attributeRead, [amR1ToLoadPressedNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!amR1ToLoadPressedNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (amR1ToLoadPressedNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool pressedValue = amR1ToLoadPressedNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool amR1ToLoadCurrentState[2] = { false, false };
          mrmlNode->GetAutoManualR1ToLoad(amR1ToLoadCurrentState);
          amR1ToLoadCurrentState[1] = pressedValue;
          mrmlNode->SetAutoManualR1ToLoad(amR1ToLoadCurrentState);
          qDebug() << Q_FUNC_INFO << "AutoManual R1 ToLoad button pressed read:" << pressedValue;
        }
      }
    }
  );
  // Subscribe to data changes
  amR1ToLoadPressedNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);
  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectAutoManualR1LoadToIsoNodes()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();
  // AutoManual R1 LoadToIso button enabled node
  QOpcUaNode* amR1LoadToIsoEnabledNode = this->FindNodeFromFullDisplayName(BUTTONS_AM_R1_LOADTOISO_ENABLED_NODE_NAME);
  // AutoManual R1 LoadToIso button pressed node
  QOpcUaNode* amR1LoadToIsoPressedNode = this->FindNodeFromFullDisplayName(BUTTONS_AM_R1_LOADTOISO_PRESSED_NODE_NAME);

  // AutoManual R1 LoadToIso button enabled node
  if (!amR1LoadToIsoEnabledNode)
  {
    return false;
  }
  QObject::connect(amR1LoadToIsoEnabledNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool enabledValue = value.toBool();
        bool amR1LoadToIsoCurrentState[2] = { false, false };
        mrmlNode->GetAutoManualR1LoadToIso(amR1LoadToIsoCurrentState);
        amR1LoadToIsoCurrentState[0] = enabledValue;
        mrmlNode->SetAutoManualR1LoadToIso(amR1LoadToIsoCurrentState);
        qDebug() << Q_FUNC_INFO << "AutoManual R1 LoadToIso button enabled changed:" << enabledValue;
      }
    }
  );
  QObject::connect(amR1LoadToIsoEnabledNode,
    &QOpcUaNode::attributeRead, [amR1LoadToIsoEnabledNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!amR1LoadToIsoEnabledNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (amR1LoadToIsoEnabledNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool enabledValue = amR1LoadToIsoEnabledNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool amR1LoadToIsoCurrentState[2] = { false, false };
          mrmlNode->GetAutoManualR1LoadToIso(amR1LoadToIsoCurrentState);
          amR1LoadToIsoCurrentState[0] = enabledValue;
          mrmlNode->SetAutoManualR1LoadToIso(amR1LoadToIsoCurrentState);
          qDebug() << Q_FUNC_INFO << "AutoManual R1 LoadToIso button enabled read:" << enabledValue;
        }
      }
    }
  );
  // Subscribe to data changes
  amR1LoadToIsoEnabledNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  // AutoManual R1 LoadToIso button pressed node
  if (!amR1LoadToIsoPressedNode)
  {
    return false;
  }
  QObject::connect(amR1LoadToIsoPressedNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool pressedValue = value.toBool();
        bool amR1LoadToIsoCurrentState[2] = { false, false };
        mrmlNode->GetAutoManualR1LoadToIso(amR1LoadToIsoCurrentState);
        amR1LoadToIsoCurrentState[1] = pressedValue;
        mrmlNode->SetAutoManualR1LoadToIso(amR1LoadToIsoCurrentState);
        qDebug() << Q_FUNC_INFO << "AutoManual R1 LoadToIso button pressed changed:" << pressedValue;
      }
    }
  );
  QObject::connect(amR1LoadToIsoPressedNode,
    &QOpcUaNode::attributeRead, [amR1LoadToIsoPressedNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!amR1LoadToIsoPressedNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (amR1LoadToIsoPressedNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool pressedValue = amR1LoadToIsoPressedNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool amR1LoadToIsoCurrentState[2] = { false, false };
          mrmlNode->GetAutoManualR1LoadToIso(amR1LoadToIsoCurrentState);
          amR1LoadToIsoCurrentState[1] = pressedValue;
          mrmlNode->SetAutoManualR1LoadToIso(amR1LoadToIsoCurrentState);
          qDebug() << Q_FUNC_INFO << "AutoManual R1 LoadToIso button pressed read:" << pressedValue;
        }
      }
    }
  );
  // Subscribe to data changes
  amR1LoadToIsoPressedNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);
  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectAutoManualR1R2EmergencyEvacuationNodes()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();
  // AutoManual R1R2 EmergencyEvacuation button enabled node
  QOpcUaNode* amR1R2EmEvacEnabledNode = this->FindNodeFromFullDisplayName(BUTTONS_AM_R1R2_EMEVAC_ENABLED_NODE_NAME);
  // AutoManual R1R2 EmergencyEvacuation button pressed node
  QOpcUaNode* amR1R2EmEvacPressedNode = this->FindNodeFromFullDisplayName(BUTTONS_AM_R1R2_EMEVAC_PRESSED_NODE_NAME);

  // AutoManual R1R2 EmergencyEvacuation button enabled node
  if (!amR1R2EmEvacEnabledNode)
  {
    return false;
  }
  QObject::connect(amR1R2EmEvacEnabledNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool enabledValue = value.toBool();
        bool amR1R2EmEvacCurrentState[2] = { false, false };
        mrmlNode->GetAutoManualEmergencyEvac(amR1R2EmEvacCurrentState);
        amR1R2EmEvacCurrentState[0] = enabledValue;
        mrmlNode->SetAutoManualEmergencyEvac(amR1R2EmEvacCurrentState);
        qDebug() << Q_FUNC_INFO << "AutoManual R1R2 EmergencyEvacuation button enabled changed:" << enabledValue;
      }
    }
  );
  QObject::connect(amR1R2EmEvacEnabledNode,
    &QOpcUaNode::attributeRead, [amR1R2EmEvacEnabledNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!amR1R2EmEvacEnabledNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (amR1R2EmEvacEnabledNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool enabledValue = amR1R2EmEvacEnabledNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool amR1R2EmEvacCurrentState[2] = { false, false };
          mrmlNode->GetAutoManualEmergencyEvac(amR1R2EmEvacCurrentState);
          amR1R2EmEvacCurrentState[0] = enabledValue;
          mrmlNode->SetAutoManualEmergencyEvac(amR1R2EmEvacCurrentState);
          qDebug() << Q_FUNC_INFO << "AutoManual R1R2 EmergencyEvacuation button enabled read:" << enabledValue;
        }
      }
    }
  );
  // Subscribe to data changes
  amR1R2EmEvacEnabledNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  // AutoManual R1R2 EmergencyEvacuation button pressed node
  if (!amR1R2EmEvacPressedNode)
  {
    return false;
  }
  QObject::connect(amR1R2EmEvacPressedNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool pressedValue = value.toBool();
        bool amR1R2EmEvacCurrentState[2] = { false, false };
        mrmlNode->GetAutoManualEmergencyEvac(amR1R2EmEvacCurrentState);
        amR1R2EmEvacCurrentState[1] = pressedValue;
        mrmlNode->SetAutoManualEmergencyEvac(amR1R2EmEvacCurrentState);
        qDebug() << Q_FUNC_INFO << "AutoManual R1R2 EmergencyEvacuation button pressed changed:" << pressedValue;
      }
    }
  );
  QObject::connect(amR1R2EmEvacPressedNode,
    &QOpcUaNode::attributeRead, [amR1R2EmEvacPressedNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!amR1R2EmEvacPressedNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (amR1R2EmEvacPressedNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool pressedValue = amR1R2EmEvacPressedNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool amR1R2EmEvacCurrentState[2] = { false, false };
          mrmlNode->GetAutoManualEmergencyEvac(amR1R2EmEvacCurrentState);
          amR1R2EmEvacCurrentState[1] = pressedValue;
          mrmlNode->SetAutoManualEmergencyEvac(amR1R2EmEvacCurrentState);
          qDebug() << Q_FUNC_INFO << "AutoManual R1R2 EmergencyEvacuation button pressed read:" << pressedValue;
        }
      }
    }
  );
  // Subscribe to data changes
  amR1R2EmEvacPressedNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectServiceR1BreakTestNodes()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();
  // Service R1 BreakTest button enabled node
  QOpcUaNode* smR1BreakTestEnabledNode = this->FindNodeFromFullDisplayName(BUTTONS_SM_R1_BREAKTEST_ENABLED_NODE_NAME);
  // Service R1 BreakTest button pressed node
  QOpcUaNode* smR1BreakTestPressedNode = this->FindNodeFromFullDisplayName(BUTTONS_SM_R1_BREAKTEST_PRESSED_NODE_NAME);

  // Service R1 BreakTest button enabled node
  if (!smR1BreakTestEnabledNode)
  {
    return false;
  }
  QObject::connect(smR1BreakTestEnabledNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool enabledValue = value.toBool();
        bool smR1BreakTestCurrentState[2] = { false, false };
        mrmlNode->GetServiceR1BreakTest(smR1BreakTestCurrentState);
        smR1BreakTestCurrentState[0] = enabledValue;
        mrmlNode->SetServiceR1BreakTest(smR1BreakTestCurrentState);
        qDebug() << Q_FUNC_INFO << "Service R1 BreakTest button enabled changed:" << enabledValue;
      }
    }
  );
  QObject::connect(smR1BreakTestEnabledNode,
    &QOpcUaNode::attributeRead, [smR1BreakTestEnabledNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!smR1BreakTestEnabledNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (smR1BreakTestEnabledNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool enabledValue = smR1BreakTestEnabledNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool smR1BreakTestCurrentState[2] = { false, false };
          mrmlNode->GetServiceR1BreakTest(smR1BreakTestCurrentState);
          smR1BreakTestCurrentState[0] = enabledValue;
          mrmlNode->SetServiceR1BreakTest(smR1BreakTestCurrentState);
          qDebug() << Q_FUNC_INFO << "Service R1 BreakTest button enabled read:" << enabledValue;
        }
      }
    }
  );
  // Subscribe to data changes
  smR1BreakTestEnabledNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  //  Service R1 BreakTest button pressed node
  if (!smR1BreakTestPressedNode)
  {
    return false;
  }
  QObject::connect(smR1BreakTestPressedNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool pressedValue = value.toBool();
        bool smR1BreakTestCurrentState[2] = { false, false };
        mrmlNode->GetServiceR1BreakTest(smR1BreakTestCurrentState);
        smR1BreakTestCurrentState[1] = pressedValue;
        mrmlNode->SetServiceR1BreakTest(smR1BreakTestCurrentState);
        qDebug() << Q_FUNC_INFO << "Service R1 BreakTest button pressed changed:" << pressedValue;
      }
    }
  );
  QObject::connect(smR1BreakTestPressedNode,
    &QOpcUaNode::attributeRead, [smR1BreakTestPressedNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!smR1BreakTestPressedNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (smR1BreakTestPressedNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool pressedValue = smR1BreakTestPressedNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool smR1BreakTestCurrentState[2] = { false, false };
          mrmlNode->GetServiceR1BreakTest(smR1BreakTestCurrentState);
          smR1BreakTestCurrentState[1] = pressedValue;
          mrmlNode->SetServiceR1BreakTest(smR1BreakTestCurrentState);
          qDebug() << Q_FUNC_INFO << "Service R1 BreakTest button pressed read:" << pressedValue;
        }
      }
    }
  );
  // Subscribe to data changes
  smR1BreakTestPressedNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);
  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectServiceR1MasterRefTestNodes()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();
  // Service R1 MasterReferenceTest button enabled node
  QOpcUaNode* smR1MasRefTestEnabledNode = this->FindNodeFromFullDisplayName(BUTTONS_SM_R1_MASREFTEST_ENABLED_NODE_NAME);
  // Service R1 MasterReferenceTest button pressed node
  QOpcUaNode* smR1MasRefTestPressedNode = this->FindNodeFromFullDisplayName(BUTTONS_SM_R1_MASREFTEST_PRESSED_NODE_NAME);

  // Service R1 MasterReferenceTest button enabled node
  if (!smR1MasRefTestEnabledNode)
  {
    return false;
  }
  QObject::connect(smR1MasRefTestEnabledNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool enabledValue = value.toBool();
        bool smR1MasterRefTestCurrentState[2] = { false, false };
        mrmlNode->GetServiceR1MasterReferenceTest(smR1MasterRefTestCurrentState);
        smR1MasterRefTestCurrentState[0] = enabledValue;
        mrmlNode->GetServiceR1MasterReferenceTest(smR1MasterRefTestCurrentState);
        qDebug() << Q_FUNC_INFO << "Service R1 MasterReferenceTest button enabled changed:" << enabledValue;
      }
    }
  );
  QObject::connect(smR1MasRefTestEnabledNode,
    &QOpcUaNode::attributeRead, [smR1MasRefTestEnabledNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!smR1MasRefTestEnabledNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (smR1MasRefTestEnabledNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool enabledValue = smR1MasRefTestEnabledNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool smR1MasterRefTestCurrentState[2] = { false, false };
          mrmlNode->GetServiceR1MasterReferenceTest(smR1MasterRefTestCurrentState);
          smR1MasterRefTestCurrentState[0] = enabledValue;
          mrmlNode->GetServiceR1MasterReferenceTest(smR1MasterRefTestCurrentState);
          qDebug() << Q_FUNC_INFO << "Service R1 MasterReferenceTest button enabled read:" << enabledValue;
        }
      }
    }
  );
  // Subscribe to data changes
  smR1MasRefTestEnabledNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  //  Service R1 MasterReferenceTest button pressed node
  if (!smR1MasRefTestPressedNode)
  {
    return false;
  }
  QObject::connect(smR1MasRefTestPressedNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool pressedValue = value.toBool();
        bool smR1MasterRefTestCurrentState[2] = { false, false };
        mrmlNode->GetServiceR1MasterReferenceTest(smR1MasterRefTestCurrentState);
        smR1MasterRefTestCurrentState[1] = pressedValue;
        mrmlNode->GetServiceR1MasterReferenceTest(smR1MasterRefTestCurrentState);
        qDebug() << Q_FUNC_INFO << "Service R1 MasterReferenceTest button pressed changed:" << pressedValue;
      }
    }
  );
  QObject::connect(smR1MasRefTestPressedNode,
    &QOpcUaNode::attributeRead, [smR1MasRefTestPressedNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!smR1MasRefTestPressedNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (smR1MasRefTestPressedNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool pressedValue = smR1MasRefTestPressedNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool smR1MasterRefTestCurrentState[2] = { false, false };
          mrmlNode->GetServiceR1MasterReferenceTest(smR1MasterRefTestCurrentState);
          smR1MasterRefTestCurrentState[1] = pressedValue;
          mrmlNode->GetServiceR1MasterReferenceTest(smR1MasterRefTestCurrentState);
          qDebug() << Q_FUNC_INFO << "Service R1 MasterReferenceTest button pressed read:" << pressedValue;
        }
      }
    }
  );
  // Subscribe to data changes
  smR1MasRefTestPressedNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);
  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectServiceR1LoadNodes()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();
  // Service R1 Load button enabled node
  QOpcUaNode* smR1LoadEnabledNode = this->FindNodeFromFullDisplayName(BUTTONS_SM_R1_LOAD_ENABLED_NODE_NAME);
  // Service R1 Load button pressed node
  QOpcUaNode* smR1LoadPressedNode = this->FindNodeFromFullDisplayName(BUTTONS_SM_R1_LOAD_PRESSED_NODE_NAME);

  // Service R1 Load button enabled node
  if (!smR1LoadEnabledNode)
  {
    return false;
  }
  QObject::connect(smR1LoadEnabledNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool enabledValue = value.toBool();
        bool smR1LoadCurrentState[2] = { false, false };
        mrmlNode->GetServiceR1Load(smR1LoadCurrentState);
        smR1LoadCurrentState[0] = enabledValue;
        mrmlNode->GetServiceR1Load(smR1LoadCurrentState);
        qDebug() << Q_FUNC_INFO << "Service R1 Load button enabled changed:" << enabledValue;
      }
    }
  );
  QObject::connect(smR1LoadEnabledNode,
    &QOpcUaNode::attributeRead, [smR1LoadEnabledNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!smR1LoadEnabledNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (smR1LoadEnabledNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool enabledValue = smR1LoadEnabledNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool smR1LoadCurrentState[2] = { false, false };
          mrmlNode->GetServiceR1Load(smR1LoadCurrentState);
          smR1LoadCurrentState[0] = enabledValue;
          mrmlNode->GetServiceR1Load(smR1LoadCurrentState);
          qDebug() << Q_FUNC_INFO << "Service R1 Load button enabled read:" << enabledValue;
        }
      }
    }
  );
  // Subscribe to data changes
  smR1LoadEnabledNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  //  Service R1 Load button pressed node
  if (!smR1LoadPressedNode)
  {
    return false;
  }
  QObject::connect(smR1LoadPressedNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool pressedValue = value.toBool();
        bool smR1LoadCurrentState[2] = { false, false };
        mrmlNode->GetServiceR1Load(smR1LoadCurrentState);
        smR1LoadCurrentState[1] = pressedValue;
        mrmlNode->GetServiceR1Load(smR1LoadCurrentState);
        qDebug() << Q_FUNC_INFO << "Service R1 Load button pressed changed:" << pressedValue;
      }
    }
  );
  QObject::connect(smR1LoadPressedNode,
    &QOpcUaNode::attributeRead, [smR1LoadPressedNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!smR1LoadPressedNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (smR1LoadPressedNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool pressedValue = smR1LoadPressedNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool smR1LoadCurrentState[2] = { false, false };
          mrmlNode->GetServiceR1Load(smR1LoadCurrentState);
          smR1LoadCurrentState[1] = pressedValue;
          mrmlNode->GetServiceR1Load(smR1LoadCurrentState);
          qDebug() << Q_FUNC_INFO << "Service R1 Load button pressed read:" << pressedValue;
        }
      }
    }
  );
  // Subscribe to data changes
  smR1LoadPressedNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);
  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectServiceR1ServicePos1Nodes()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();
  // Service R1 ServicePosition1 button enabled node
  QOpcUaNode* smR1ServPos1EnabledNode = this->FindNodeFromFullDisplayName(BUTTONS_SM_R1_SERVICEPOS1_ENABLED_NODE_NAME);
  // Service R1 ServicePosition1 button pressed node
  QOpcUaNode* smR1ServPos1PressedNode = this->FindNodeFromFullDisplayName(BUTTONS_SM_R1_SERVICEPOS1_PRESSED_NODE_NAME);

  // Service R1 ServicePosition1 button enabled node
  if (!smR1ServPos1EnabledNode)
  {
    return false;
  }
  QObject::connect(smR1ServPos1EnabledNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool enabledValue = value.toBool();
        bool smR1ServicePos1CurrentState[2] = { false, false };
        mrmlNode->GetServiceR1Position1(smR1ServicePos1CurrentState);
        smR1ServicePos1CurrentState[0] = enabledValue;
        mrmlNode->GetServiceR1Position1(smR1ServicePos1CurrentState);
        qDebug() << Q_FUNC_INFO << "Service R1 ServicePosition1 button enabled changed:" << enabledValue;
      }
    }
  );
  QObject::connect(smR1ServPos1EnabledNode,
    &QOpcUaNode::attributeRead, [smR1ServPos1EnabledNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!smR1ServPos1EnabledNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (smR1ServPos1EnabledNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool enabledValue = smR1ServPos1EnabledNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool smR1ServicePos1CurrentState[2] = { false, false };
          mrmlNode->GetServiceR1Position1(smR1ServicePos1CurrentState);
          smR1ServicePos1CurrentState[0] = enabledValue;
          mrmlNode->GetServiceR1Position1(smR1ServicePos1CurrentState);
          qDebug() << Q_FUNC_INFO << "Service R1 ServicePosition1 button enabled read:" << enabledValue;
        }
      }
    }
  );
  // Subscribe to data changes
  smR1ServPos1EnabledNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  //  Service R1 ServicePosition1 button pressed node
  if (!smR1ServPos1PressedNode)
  {
    return false;
  }
  QObject::connect(smR1ServPos1PressedNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool pressedValue = value.toBool();
        bool smR1ServicePos1CurrentState[2] = { false, false };
        mrmlNode->GetServiceR1Position1(smR1ServicePos1CurrentState);
        smR1ServicePos1CurrentState[1] = pressedValue;
        mrmlNode->GetServiceR1Position1(smR1ServicePos1CurrentState);
        qDebug() << Q_FUNC_INFO << "Service R1 ServicePosition1 button pressed changed:" << pressedValue;
      }
    }
  );
  QObject::connect(smR1ServPos1PressedNode,
    &QOpcUaNode::attributeRead, [smR1ServPos1PressedNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!smR1ServPos1PressedNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (smR1ServPos1PressedNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool pressedValue = smR1ServPos1PressedNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool smR1ServicePos1CurrentState[2] = { false, false };
          mrmlNode->GetServiceR1Position1(smR1ServicePos1CurrentState);
          smR1ServicePos1CurrentState[1] = pressedValue;
          mrmlNode->GetServiceR1Position1(smR1ServicePos1CurrentState);
          qDebug() << Q_FUNC_INFO << "Service R1 ServicePosition1 button pressed read:" << pressedValue;
        }
      }
    }
  );
  // Subscribe to data changes
  smR1ServPos1PressedNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);
  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectServiceR1ServicePos2Nodes()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();
  // Service R1 ServicePosition2 button enabled node
  QOpcUaNode* smR1ServPos2EnabledNode = this->FindNodeFromFullDisplayName(BUTTONS_SM_R1_SERVICEPOS2_ENABLED_NODE_NAME);
  // Service R1 ServicePosition2 button pressed node
  QOpcUaNode* smR1ServPos2PressedNode = this->FindNodeFromFullDisplayName(BUTTONS_SM_R1_SERVICEPOS2_PRESSED_NODE_NAME);

  // Service R1 ServicePosition2 button enabled node
  if (!smR1ServPos2EnabledNode)
  {
    return false;
  }
  QObject::connect(smR1ServPos2EnabledNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool enabledValue = value.toBool();
        bool smR1ServicePos2CurrentState[2] = { false, false };
        mrmlNode->GetServiceR1Position2(smR1ServicePos2CurrentState);
        smR1ServicePos2CurrentState[0] = enabledValue;
        mrmlNode->GetServiceR1Position2(smR1ServicePos2CurrentState);
        qDebug() << Q_FUNC_INFO << "Service R1 ServicePosition2 button enabled changed:" << enabledValue;
      }
    }
  );
  QObject::connect(smR1ServPos2EnabledNode,
    &QOpcUaNode::attributeRead, [smR1ServPos2EnabledNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!smR1ServPos2EnabledNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (smR1ServPos2EnabledNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool enabledValue = smR1ServPos2EnabledNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool smR1ServicePos2CurrentState[2] = { false, false };
          mrmlNode->GetServiceR1Position2(smR1ServicePos2CurrentState);
          smR1ServicePos2CurrentState[0] = enabledValue;
          mrmlNode->GetServiceR1Position2(smR1ServicePos2CurrentState);
          qDebug() << Q_FUNC_INFO << "Service R1 ServicePosition2 button enabled read:" << enabledValue;
        }
      }
    }
  );
  // Subscribe to data changes
  smR1ServPos2EnabledNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  //  Service R1 ServicePosition2 button pressed node
  if (!smR1ServPos2PressedNode)
  {
    return false;
  }
  QObject::connect(smR1ServPos2PressedNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool pressedValue = value.toBool();
        bool smR1ServicePos2CurrentState[2] = { false, false };
        mrmlNode->GetServiceR1Position2(smR1ServicePos2CurrentState);
        smR1ServicePos2CurrentState[1] = pressedValue;
        mrmlNode->GetServiceR1Position2(smR1ServicePos2CurrentState);
        qDebug() << Q_FUNC_INFO << "Service R1 ServicePosition2 button pressed changed:" << pressedValue;
      }
    }
  );
  QObject::connect(smR1ServPos2PressedNode,
    &QOpcUaNode::attributeRead, [smR1ServPos2PressedNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!smR1ServPos2PressedNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (smR1ServPos2PressedNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool pressedValue = smR1ServPos2PressedNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool smR1ServicePos2CurrentState[2] = { false, false };
          mrmlNode->GetServiceR1Position2(smR1ServicePos2CurrentState);
          smR1ServicePos2CurrentState[1] = pressedValue;
          mrmlNode->GetServiceR1Position2(smR1ServicePos2CurrentState);
          qDebug() << Q_FUNC_INFO << "Service R1 ServicePosition2 button pressed read:" << pressedValue;
        }
      }
    }
  );
  // Subscribe to data changes
  smR1ServPos2PressedNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);
  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectServiceR1ServicePos3Nodes()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();
  // Service R1 ServicePosition3 button enabled node
  QOpcUaNode* smR1ServPos3EnabledNode = this->FindNodeFromFullDisplayName(BUTTONS_SM_R1_SERVICEPOS3_ENABLED_NODE_NAME);
  // Service R1 ServicePosition3 button pressed node
  QOpcUaNode* smR1ServPos3PressedNode = this->FindNodeFromFullDisplayName(BUTTONS_SM_R1_SERVICEPOS3_PRESSED_NODE_NAME);

  // Service R1 ServicePosition3 button enabled node
  if (!smR1ServPos3EnabledNode)
  {
    return false;
  }
  QObject::connect(smR1ServPos3EnabledNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool enabledValue = value.toBool();
        bool smR1ServicePos3CurrentState[2] = { false, false };
        mrmlNode->GetServiceR1Position3(smR1ServicePos3CurrentState);
        smR1ServicePos3CurrentState[0] = enabledValue;
        mrmlNode->GetServiceR1Position3(smR1ServicePos3CurrentState);
        qDebug() << Q_FUNC_INFO << "Service R1 ServicePosition3 button enabled changed:" << enabledValue;
      }
    }
  );
  QObject::connect(smR1ServPos3EnabledNode,
    &QOpcUaNode::attributeRead, [smR1ServPos3EnabledNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!smR1ServPos3EnabledNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (smR1ServPos3EnabledNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool enabledValue = smR1ServPos3EnabledNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool smR1ServicePos3CurrentState[2] = { false, false };
          mrmlNode->GetServiceR1Position3(smR1ServicePos3CurrentState);
          smR1ServicePos3CurrentState[0] = enabledValue;
          mrmlNode->GetServiceR1Position3(smR1ServicePos3CurrentState);
          qDebug() << Q_FUNC_INFO << "Service R1 ServicePosition3 button enabled read:" << enabledValue;
        }
      }
    }
  );
  // Subscribe to data changes
  smR1ServPos3EnabledNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  //  Service R1 ServicePosition3 button pressed node
  if (!smR1ServPos3PressedNode)
  {
    return false;
  }
  QObject::connect(smR1ServPos3PressedNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool pressedValue = value.toBool();
        bool smR1ServicePos3CurrentState[2] = { false, false };
        mrmlNode->GetServiceR1Position3(smR1ServicePos3CurrentState);
        smR1ServicePos3CurrentState[1] = pressedValue;
        mrmlNode->GetServiceR1Position3(smR1ServicePos3CurrentState);
        qDebug() << Q_FUNC_INFO << "Service R1 ServicePosition3 button pressed changed:" << pressedValue;
      }
    }
  );
  QObject::connect(smR1ServPos3PressedNode,
    &QOpcUaNode::attributeRead, [smR1ServPos3PressedNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!smR1ServPos3PressedNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (smR1ServPos3PressedNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool pressedValue = smR1ServPos3PressedNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool smR1ServicePos3CurrentState[2] = { false, false };
          mrmlNode->GetServiceR1Position3(smR1ServicePos3CurrentState);
          smR1ServicePos3CurrentState[1] = pressedValue;
          mrmlNode->GetServiceR1Position3(smR1ServicePos3CurrentState);
          qDebug() << Q_FUNC_INFO << "Service R1 ServicePosition3 button pressed read:" << pressedValue;
        }
      }
    }
  );
  // Subscribe to data changes
  smR1ServPos3PressedNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);
  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectServiceR2BreakTestNodes()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();
  // Service R2 BreakTest button enabled node
  QOpcUaNode* smR2BreakTestEnabledNode = this->FindNodeFromFullDisplayName(BUTTONS_SM_R2_BREAKTEST_ENABLED_NODE_NAME);
  // Service R2 BreakTest button pressed node
  QOpcUaNode* smR2BreakTestPressedNode = this->FindNodeFromFullDisplayName(BUTTONS_SM_R2_BREAKTEST_PRESSED_NODE_NAME);

  // Service R2 BreakTest button enabled node
  if (!smR2BreakTestEnabledNode)
  {
    return false;
  }
  QObject::connect(smR2BreakTestEnabledNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool enabledValue = value.toBool();
        bool smR2BreakTestCurrentState[2] = { false, false };
        mrmlNode->GetServiceR2BreakTest(smR2BreakTestCurrentState);
        smR2BreakTestCurrentState[0] = enabledValue;
        mrmlNode->SetServiceR2BreakTest(smR2BreakTestCurrentState);
        qDebug() << Q_FUNC_INFO << "Service R2 BreakTest button enabled changed:" << enabledValue;
      }
    }
  );
  QObject::connect(smR2BreakTestEnabledNode,
    &QOpcUaNode::attributeRead, [smR2BreakTestEnabledNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!smR2BreakTestEnabledNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (smR2BreakTestEnabledNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool enabledValue = smR2BreakTestEnabledNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool smR2BreakTestCurrentState[2] = { false, false };
          mrmlNode->GetServiceR2BreakTest(smR2BreakTestCurrentState);
          smR2BreakTestCurrentState[0] = enabledValue;
          mrmlNode->SetServiceR2BreakTest(smR2BreakTestCurrentState);
          qDebug() << Q_FUNC_INFO << "Service R2 BreakTest button enabled read:" << enabledValue;
        }
      }
    }
  );
  // Subscribe to data changes
  smR2BreakTestEnabledNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  //  Service R2 BreakTest button pressed node
  if (!smR2BreakTestPressedNode)
  {
    return false;
  }
  QObject::connect(smR2BreakTestPressedNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool pressedValue = value.toBool();
        bool smR2BreakTestCurrentState[2] = { false, false };
        mrmlNode->GetServiceR2BreakTest(smR2BreakTestCurrentState);
        smR2BreakTestCurrentState[1] = pressedValue;
        mrmlNode->SetServiceR2BreakTest(smR2BreakTestCurrentState);
        qDebug() << Q_FUNC_INFO << "Service R2 BreakTest button pressed changed:" << pressedValue;
      }
    }
  );
  QObject::connect(smR2BreakTestPressedNode,
    &QOpcUaNode::attributeRead, [smR2BreakTestPressedNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!smR2BreakTestPressedNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (smR2BreakTestPressedNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool pressedValue = smR2BreakTestPressedNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool smR2BreakTestCurrentState[2] = { false, false };
          mrmlNode->GetServiceR2BreakTest(smR2BreakTestCurrentState);
          smR2BreakTestCurrentState[1] = pressedValue;
          mrmlNode->SetServiceR2BreakTest(smR2BreakTestCurrentState);
          qDebug() << Q_FUNC_INFO << "Service R2 BreakTest button pressed read:" << pressedValue;
        }
      }
    }
  );
  // Subscribe to data changes
  smR2BreakTestPressedNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);
  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectServiceR2MasterRefTestNodes()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();
  // Service R2 MasterReferenceTest button enabled node
  QOpcUaNode* smR2MasRefTestEnabledNode = this->FindNodeFromFullDisplayName(BUTTONS_SM_R2_MASREFTEST_ENABLED_NODE_NAME);
  // Service R2 MasterReferenceTest button pressed node
  QOpcUaNode* smR2MasRefTestPressedNode = this->FindNodeFromFullDisplayName(BUTTONS_SM_R2_MASREFTEST_PRESSED_NODE_NAME);

  // Service R2 MasterReferenceTest button enabled node
  if (!smR2MasRefTestEnabledNode)
  {
    return false;
  }
  QObject::connect(smR2MasRefTestEnabledNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool enabledValue = value.toBool();
        bool smR2MasterRefTestCurrentState[2] = { false, false };
        mrmlNode->GetServiceR2MasterReferenceTest(smR2MasterRefTestCurrentState);
        smR2MasterRefTestCurrentState[0] = enabledValue;
        mrmlNode->SetServiceR2MasterReferenceTest(smR2MasterRefTestCurrentState);
        qDebug() << Q_FUNC_INFO << "Service R2 MasterReferenceTest button enabled changed:" << enabledValue;
      }
    }
  );
  QObject::connect(smR2MasRefTestEnabledNode,
    &QOpcUaNode::attributeRead, [smR2MasRefTestEnabledNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!smR2MasRefTestEnabledNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (smR2MasRefTestEnabledNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool enabledValue = smR2MasRefTestEnabledNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool smR2MasterRefTestCurrentState[2] = { false, false };
          mrmlNode->GetServiceR2MasterReferenceTest(smR2MasterRefTestCurrentState);
          smR2MasterRefTestCurrentState[0] = enabledValue;
          mrmlNode->SetServiceR2MasterReferenceTest(smR2MasterRefTestCurrentState);
          qDebug() << Q_FUNC_INFO << "Service R2 MasterReferenceTest button read changed:" << enabledValue;
        }
      }
    }
  );
  // Subscribe to data changes
  smR2MasRefTestEnabledNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  //  Service R2 MasterReferenceTest button pressed node
  if (!smR2MasRefTestPressedNode)
  {
    return false;
  }
  QObject::connect(smR2MasRefTestPressedNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool pressedValue = value.toBool();
        bool smR2MasterRefTestCurrentState[2] = { false, false };
        mrmlNode->GetServiceR2MasterReferenceTest(smR2MasterRefTestCurrentState);
        smR2MasterRefTestCurrentState[1] = pressedValue;
        mrmlNode->SetServiceR2MasterReferenceTest(smR2MasterRefTestCurrentState);
        qDebug() << Q_FUNC_INFO << "Service R2 MasterReferenceTest button pressed changed:" << pressedValue;
      }
    }
  );
  QObject::connect(smR2MasRefTestPressedNode,
    &QOpcUaNode::attributeRead, [smR2MasRefTestPressedNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!smR2MasRefTestPressedNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (smR2MasRefTestPressedNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool pressedValue = smR2MasRefTestPressedNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool smR2MasterRefTestCurrentState[2] = { false, false };
          mrmlNode->GetServiceR2MasterReferenceTest(smR2MasterRefTestCurrentState);
          smR2MasterRefTestCurrentState[1] = pressedValue;
          mrmlNode->SetServiceR2MasterReferenceTest(smR2MasterRefTestCurrentState);
          qDebug() << Q_FUNC_INFO << "Service R2 MasterReferenceTest button pressed read:" << pressedValue;
        }
      }
    }
  );
  // Subscribe to data changes
  smR2MasRefTestPressedNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);
  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectServiceR2HomeNodes()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();
  // Service R2 Home button enabled node
  QOpcUaNode* smR2HomeEnabledNode = this->FindNodeFromFullDisplayName(BUTTONS_SM_R2_HOME_ENABLED_NODE_NAME);
  // Service R2 Home button pressed node
  QOpcUaNode* smR2HomePressedNode = this->FindNodeFromFullDisplayName(BUTTONS_SM_R2_HOME_PRESSED_NODE_NAME);

  // Service R1 Load button enabled node
  if (!smR2HomeEnabledNode)
  {
    return false;
  }
  QObject::connect(smR2HomeEnabledNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool enabledValue = value.toBool();
        bool smR2HomeCurrentState[2] = { false, false };
        mrmlNode->GetServiceR2Home(smR2HomeCurrentState);
        smR2HomeCurrentState[0] = enabledValue;
        mrmlNode->SetServiceR2Home(smR2HomeCurrentState);
        qDebug() << Q_FUNC_INFO << "Service R2 Home button enabled changed:" << enabledValue;
      }
    }
  );
  QObject::connect(smR2HomeEnabledNode,
    &QOpcUaNode::attributeRead, [smR2HomeEnabledNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!smR2HomeEnabledNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (smR2HomeEnabledNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool enabledValue = smR2HomeEnabledNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool smR2HomeCurrentState[2] = { false, false };
          mrmlNode->GetServiceR2Home(smR2HomeCurrentState);
          smR2HomeCurrentState[0] = enabledValue;
          mrmlNode->SetServiceR2Home(smR2HomeCurrentState);
          qDebug() << Q_FUNC_INFO << "Service R2 Home button enabled read:" << enabledValue;
        }
      }
    }
  );
  // Subscribe to data changes
  smR2HomeEnabledNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  //  Service R2 Home button pressed node
  if (!smR2HomePressedNode)
  {
    return false;
  }
  QObject::connect(smR2HomePressedNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool pressedValue = value.toBool();
        bool smR2HomeCurrentState[2] = { false, false };
        mrmlNode->GetServiceR2Home(smR2HomeCurrentState);
        smR2HomeCurrentState[1] = pressedValue;
        mrmlNode->SetServiceR2Home(smR2HomeCurrentState);
        qDebug() << Q_FUNC_INFO << "Service R2 Home button pressed changed:" << pressedValue;
      }
    }
  );
  QObject::connect(smR2HomePressedNode,
    &QOpcUaNode::attributeRead, [smR2HomePressedNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!smR2HomePressedNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (smR2HomePressedNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool pressedValue = smR2HomePressedNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool smR2HomeCurrentState[2] = { false, false };
          mrmlNode->GetServiceR2Home(smR2HomeCurrentState);
          smR2HomeCurrentState[1] = pressedValue;
          mrmlNode->SetServiceR2Home(smR2HomeCurrentState);
          qDebug() << Q_FUNC_INFO << "Service R2 Home button pressed read:" << pressedValue;
        }
      }
    }
  );
  // Subscribe to data changes
  smR2HomePressedNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);
  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectServiceR2ServicePos1Nodes()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();
  // Service R2 ServicePosition1 button enabled node
  QOpcUaNode* smR2ServPos1EnabledNode = this->FindNodeFromFullDisplayName(BUTTONS_SM_R2_SERVICEPOS1_ENABLED_NODE_NAME);
  // Service R2 ServicePosition1 button pressed node
  QOpcUaNode* smR2ServPos1PressedNode = this->FindNodeFromFullDisplayName(BUTTONS_SM_R2_SERVICEPOS1_PRESSED_NODE_NAME);

  // Service R2 ServicePosition1 button enabled node
  if (!smR2ServPos1EnabledNode)
  {
    return false;
  }
  QObject::connect(smR2ServPos1EnabledNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool enabledValue = value.toBool();
        bool smR2ServicePos1CurrentState[2] = { false, false };
        mrmlNode->GetServiceR2Position1(smR2ServicePos1CurrentState);
        smR2ServicePos1CurrentState[0] = enabledValue;
        mrmlNode->SetServiceR2Position1(smR2ServicePos1CurrentState);
        qDebug() << Q_FUNC_INFO << "Service R2 ServicePosition1 button enabled changed:" << enabledValue;
      }
    }
  );
  QObject::connect(smR2ServPos1EnabledNode,
    &QOpcUaNode::attributeRead, [smR2ServPos1EnabledNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!smR2ServPos1EnabledNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (smR2ServPos1EnabledNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool enabledValue = smR2ServPos1EnabledNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool smR2ServicePos1CurrentState[2] = { false, false };
          mrmlNode->GetServiceR2Position1(smR2ServicePos1CurrentState);
          smR2ServicePos1CurrentState[0] = enabledValue;
          mrmlNode->SetServiceR2Position1(smR2ServicePos1CurrentState);
          qDebug() << Q_FUNC_INFO << "Service R2 ServicePosition1 button enabled read:" << enabledValue;
        }
      }
    }
  );
  // Subscribe to data changes
  smR2ServPos1EnabledNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  //  Service R2 ServicePosition1 button pressed node
  if (!smR2ServPos1PressedNode)
  {
    return false;
  }
  QObject::connect(smR2ServPos1PressedNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool pressedValue = value.toBool();
        bool smR2ServicePos1CurrentState[2] = { false, false };
        mrmlNode->GetServiceR2Position1(smR2ServicePos1CurrentState);
        smR2ServicePos1CurrentState[1] = pressedValue;
        mrmlNode->SetServiceR2Position1(smR2ServicePos1CurrentState);
        qDebug() << Q_FUNC_INFO << "Service R2 ServicePosition1 button pressed changed:" << pressedValue;
      }
    }
  );
  QObject::connect(smR2ServPos1PressedNode,
    &QOpcUaNode::attributeRead, [smR2ServPos1PressedNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!smR2ServPos1PressedNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (smR2ServPos1PressedNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool pressedValue = smR2ServPos1PressedNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool smR2ServicePos1CurrentState[2] = { false, false };
          mrmlNode->GetServiceR2Position1(smR2ServicePos1CurrentState);
          smR2ServicePos1CurrentState[1] = pressedValue;
          mrmlNode->SetServiceR2Position1(smR2ServicePos1CurrentState);
          qDebug() << Q_FUNC_INFO << "Service R2 ServicePosition1 button pressed read:" << pressedValue;
        }
      }
    }
  );
  // Subscribe to data changes
  smR2ServPos1PressedNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);
  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectServiceR2ServicePos2Nodes()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();
  // Service R2 ServicePosition2 button enabled node
  QOpcUaNode* smR2ServPos2EnabledNode = this->FindNodeFromFullDisplayName(BUTTONS_SM_R2_SERVICEPOS2_ENABLED_NODE_NAME);
  // Service R2 ServicePosition2 button pressed node
  QOpcUaNode* smR2ServPos2PressedNode = this->FindNodeFromFullDisplayName(BUTTONS_SM_R2_SERVICEPOS2_PRESSED_NODE_NAME);

  // Service R2 ServicePosition2 button enabled node
  if (!smR2ServPos2EnabledNode)
  {
    return false;
  }
  QObject::connect(smR2ServPos2EnabledNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool enabledValue = value.toBool();
        bool smR2ServicePos2CurrentState[2] = { false, false };
        mrmlNode->GetServiceR2Position2(smR2ServicePos2CurrentState);
        smR2ServicePos2CurrentState[0] = enabledValue;
        mrmlNode->SetServiceR2Position2(smR2ServicePos2CurrentState);
        qDebug() << Q_FUNC_INFO << "Service R2 ServicePosition2 button enabled changed:" << enabledValue;
      }
    }
  );
  QObject::connect(smR2ServPos2EnabledNode,
    &QOpcUaNode::attributeRead, [smR2ServPos2EnabledNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!smR2ServPos2EnabledNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (smR2ServPos2EnabledNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool enabledValue = smR2ServPos2EnabledNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool smR2ServicePos2CurrentState[2] = { false, false };
          mrmlNode->GetServiceR2Position2(smR2ServicePos2CurrentState);
          smR2ServicePos2CurrentState[0] = enabledValue;
          mrmlNode->SetServiceR2Position2(smR2ServicePos2CurrentState);
          qDebug() << Q_FUNC_INFO << "Service R2 ServicePosition2 button enabled read:" << enabledValue;
        }
      }
    }
  );
  // Subscribe to data changes
  smR2ServPos2EnabledNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  //  Service R2 ServicePosition2 button pressed node
  if (!smR2ServPos2PressedNode)
  {
    return false;
  }
  QObject::connect(smR2ServPos2PressedNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool pressedValue = value.toBool();
        bool smR2ServicePos2CurrentState[2] = { false, false };
        mrmlNode->GetServiceR2Position2(smR2ServicePos2CurrentState);
        smR2ServicePos2CurrentState[1] = pressedValue;
        mrmlNode->SetServiceR2Position2(smR2ServicePos2CurrentState);
        qDebug() << Q_FUNC_INFO << "Service R2 ServicePosition2 button pressed changed:" << pressedValue;
      }
    }
  );
  QObject::connect(smR2ServPos2PressedNode,
    &QOpcUaNode::attributeRead, [smR2ServPos2PressedNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!smR2ServPos2PressedNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (smR2ServPos2PressedNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool pressedValue = smR2ServPos2PressedNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool smR2ServicePos2CurrentState[2] = { false, false };
          mrmlNode->GetServiceR2Position2(smR2ServicePos2CurrentState);
          smR2ServicePos2CurrentState[1] = pressedValue;
          mrmlNode->SetServiceR2Position2(smR2ServicePos2CurrentState);
          qDebug() << Q_FUNC_INFO << "Service R2 ServicePosition2 button pressed read:" << pressedValue;
        }
      }
    }
  );
  // Subscribe to data changes
  smR2ServPos2PressedNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);
  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectServiceR2ServicePos3Nodes()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();
  // Service R2 ServicePosition3 button enabled node
  QOpcUaNode* smR2ServPos3EnabledNode = this->FindNodeFromFullDisplayName(BUTTONS_SM_R2_SERVICEPOS3_ENABLED_NODE_NAME);
  // Service R2 ServicePosition3 button pressed node
  QOpcUaNode* smR2ServPos3PressedNode = this->FindNodeFromFullDisplayName(BUTTONS_SM_R2_SERVICEPOS3_PRESSED_NODE_NAME);

  // Service R2 ServicePosition3 button enabled node
  if (!smR2ServPos3EnabledNode)
  {
    return false;
  }
  QObject::connect(smR2ServPos3EnabledNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool enabledValue = value.toBool();
        bool smR2ServicePos3CurrentState[2] = { false, false };
        mrmlNode->GetServiceR2Position3(smR2ServicePos3CurrentState);
        smR2ServicePos3CurrentState[0] = enabledValue;
        mrmlNode->SetServiceR2Position3(smR2ServicePos3CurrentState);
        qDebug() << Q_FUNC_INFO << "Service R2 ServicePosition3 button enabled changed:" << enabledValue;
      }
    }
  );
  QObject::connect(smR2ServPos3EnabledNode,
    &QOpcUaNode::attributeRead, [smR2ServPos3EnabledNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!smR2ServPos3EnabledNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (smR2ServPos3EnabledNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool enabledValue = smR2ServPos3EnabledNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool smR2ServicePos3CurrentState[2] = { false, false };
          mrmlNode->GetServiceR2Position3(smR2ServicePos3CurrentState);
          smR2ServicePos3CurrentState[0] = enabledValue;
          mrmlNode->SetServiceR2Position3(smR2ServicePos3CurrentState);
          qDebug() << Q_FUNC_INFO << "Service R2 ServicePosition3 button enabled read:" << enabledValue;
        }
      }
    }
  );
  // Subscribe to data changes
  smR2ServPos3EnabledNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  //  Service R2 ServicePosition3 button pressed node
  if (!smR2ServPos3PressedNode)
  {
    return false;
  }
  QObject::connect(smR2ServPos3PressedNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool pressedValue = value.toBool();
        bool smR2ServicePos3CurrentState[2] = { false, false };
        mrmlNode->GetServiceR2Position3(smR2ServicePos3CurrentState);
        smR2ServicePos3CurrentState[1] = pressedValue;
        mrmlNode->SetServiceR2Position3(smR2ServicePos3CurrentState);
        qDebug() << Q_FUNC_INFO << "Service R2 ServicePosition3 button pressed changed:" << pressedValue;
      }
    }
  );
  QObject::connect(smR2ServPos3PressedNode,
    &QOpcUaNode::attributeRead, [smR2ServPos3PressedNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!smR2ServPos3PressedNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (smR2ServPos3PressedNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool pressedValue = smR2ServPos3PressedNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool smR2ServicePos3CurrentState[2] = { false, false };
          mrmlNode->GetServiceR2Position3(smR2ServicePos3CurrentState);
          smR2ServicePos3CurrentState[1] = pressedValue;
          mrmlNode->SetServiceR2Position3(smR2ServicePos3CurrentState);
          qDebug() << Q_FUNC_INFO << "Service R2 ServicePosition3 button pressed read:" << pressedValue;
        }
      }
    }
  );
  // Subscribe to data changes
  smR2ServPos3PressedNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);
  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectServiceRestartNodes()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();
  // Service RestartSM button enabled node
  QOpcUaNode* smRestartEnabledNode = this->FindNodeFromFullDisplayName(BUTTONS_SM_RESTARTSM_ENABLED_NODE_NAME);
  // Service RestartSM button pressed node
  QOpcUaNode* smRestartPressedNode = this->FindNodeFromFullDisplayName(BUTTONS_SM_RESTARTSM_PRESSED_NODE_NAME);

  // Service RestartSM button enabled node
  if (!smRestartEnabledNode)
  {
    return false;
  }
  QObject::connect(smRestartEnabledNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool enabledValue = value.toBool();
        bool smRestartCurrentState[2] = { false, false };
        mrmlNode->GetServiceRestart(smRestartCurrentState);
        smRestartCurrentState[0] = enabledValue;
        mrmlNode->SetServiceRestart(smRestartCurrentState);
        qDebug() << Q_FUNC_INFO << "Service RestartSM button enabled changed:" << enabledValue;
      }
    }
  );
  QObject::connect(smRestartEnabledNode,
    &QOpcUaNode::attributeRead, [smRestartEnabledNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!smRestartEnabledNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (smRestartEnabledNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool enabledValue = smRestartEnabledNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool smRestartCurrentState[2] = { false, false };
          mrmlNode->GetServiceRestart(smRestartCurrentState);
          smRestartCurrentState[0] = enabledValue;
          mrmlNode->SetServiceRestart(smRestartCurrentState);
          qDebug() << Q_FUNC_INFO << "Service RestartSM button enabled read:" << enabledValue;
        }
      }
    }
  );
  // Subscribe to data changes
  smRestartEnabledNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  //  Service RestartSM button pressed node
  if (!smRestartPressedNode)
  {
    return false;
  }
  QObject::connect(smRestartPressedNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool pressedValue = value.toBool();
        bool smRestartCurrentState[2] = { false, false };
        mrmlNode->GetServiceRestart(smRestartCurrentState);
        smRestartCurrentState[1] = pressedValue;
        mrmlNode->SetServiceRestart(smRestartCurrentState);
        qDebug() << Q_FUNC_INFO << "Service RestartSM button pressed changed:" << pressedValue;
      }
    }
  );
  QObject::connect(smRestartPressedNode,
    &QOpcUaNode::attributeRead, [smRestartPressedNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!smRestartPressedNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (smRestartPressedNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool pressedValue = smRestartPressedNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool smRestartCurrentState[2] = { false, false };
          mrmlNode->GetServiceRestart(smRestartCurrentState);
          smRestartCurrentState[1] = pressedValue;
          mrmlNode->SetServiceRestart(smRestartCurrentState);
          qDebug() << Q_FUNC_INFO << "Service RestartSM button pressed read:" << pressedValue;
        }
      }
    }
  );
  // Subscribe to data changes
  smRestartPressedNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);
  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectServiceMovementNodes()
{
  bool res = true;

  res &= this->ConnectServiceR1BreakTestNodes();
  res &= this->ConnectServiceR1MasterRefTestNodes();
  res &= this->ConnectServiceR1LoadNodes();
  res &= this->ConnectServiceR1ServicePos1Nodes();
  res &= this->ConnectServiceR1ServicePos2Nodes();
  res &= this->ConnectServiceR1ServicePos3Nodes();

  res &= this->ConnectServiceR2BreakTestNodes();
  res &= this->ConnectServiceR2MasterRefTestNodes();
  res &= this->ConnectServiceR2HomeNodes();
  res &= this->ConnectServiceR2ServicePos1Nodes();
  res &= this->ConnectServiceR2ServicePos2Nodes();
  res &= this->ConnectServiceR2ServicePos3Nodes();

  return res;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectResetErrorsNodes()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();
  // ResetErrors button enabled node
  QOpcUaNode* reEnabledNode = this->FindNodeFromFullDisplayName(BUTTONS_RESETERRORS_ENABLED_NODE_NAME);
  // ResetErrors button pressed node
  QOpcUaNode* rePressedNode = this->FindNodeFromFullDisplayName(BUTTONS_RESETERRORS_PRESSED_NODE_NAME);

  // ResetErrors button enabled node
  if (!reEnabledNode)
  {
    return false;
  }
  QObject::connect(reEnabledNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool enabledValue = value.toBool();
        bool resetErrorsCurrentState[2] = { false, false };
        mrmlNode->GetResetErrors(resetErrorsCurrentState);
        resetErrorsCurrentState[0] = enabledValue;
        mrmlNode->SetResetErrors(resetErrorsCurrentState);
        qDebug() << Q_FUNC_INFO << "ResetErrors button enabled changed:" << enabledValue;
      }
    }
  );
  QObject::connect(reEnabledNode,
    &QOpcUaNode::attributeRead, [reEnabledNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!reEnabledNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (reEnabledNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool enabledValue = reEnabledNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool resetErrorsCurrentState[2] = { false, false };
          mrmlNode->GetResetErrors(resetErrorsCurrentState);
          resetErrorsCurrentState[0] = enabledValue;
          mrmlNode->SetResetErrors(resetErrorsCurrentState);
          qDebug() << Q_FUNC_INFO << "ResetErrors button enabled read:" << enabledValue;
        }
      }
    }
  );
  // Subscribe to data changes
  reEnabledNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  // ResetErrors button pressed node
  if (!rePressedNode)
  {
    return false;
  }
  QObject::connect(rePressedNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool pressedValue = value.toBool();
        bool resetErrorsCurrentState[2] = { false, false };
        mrmlNode->GetResetErrors(resetErrorsCurrentState);
        resetErrorsCurrentState[1] = pressedValue;
        mrmlNode->SetResetErrors(resetErrorsCurrentState);
        qDebug() << Q_FUNC_INFO << "ResetErrors button pressed changed:" << pressedValue;
      }
    }
  );
  QObject::connect(rePressedNode,
    &QOpcUaNode::attributeRead, [rePressedNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!rePressedNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (rePressedNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool pressedValue = rePressedNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool resetErrorsCurrentState[2] = { false, false };
          mrmlNode->GetResetErrors(resetErrorsCurrentState);
          resetErrorsCurrentState[1] = pressedValue;
          mrmlNode->SetResetErrors(resetErrorsCurrentState);
          qDebug() << Q_FUNC_INFO << "ResetErrors button pressed read:" << pressedValue;
        }
      }
    }
  );
  // Subscribe to data changes
  rePressedNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);
  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectMakeXrayNodes()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();
  // MakeXRay button enabled node
  QOpcUaNode* mxEnabledNode = this->FindNodeFromFullDisplayName(BUTTONS_MAKEXRAY_ENABLED_NODE_NAME);
  // MakeXRay button pressed node
  QOpcUaNode* mxPressedNode = this->FindNodeFromFullDisplayName(BUTTONS_MAKEXRAY_PRESSED_NODE_NAME);

  // MakeXRay button enabled node
  if (!mxEnabledNode)
  {
    return false;
  }
  QObject::connect(mxEnabledNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool enabledValue = value.toBool();
        bool makeXrayCurrentState[2] = { false, false };
        mrmlNode->GetMakeXray(makeXrayCurrentState);
        makeXrayCurrentState[0] = enabledValue;
        mrmlNode->SetMakeXray(makeXrayCurrentState);
        qDebug() << Q_FUNC_INFO << "MakeXRay button enabled changed:" << enabledValue;
      }
    }
  );
  QObject::connect(mxEnabledNode,
    &QOpcUaNode::attributeRead, [mxEnabledNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!mxEnabledNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (mxEnabledNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool enabledValue = mxEnabledNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool makeXrayCurrentState[2] = { false, false };
          mrmlNode->GetMakeXray(makeXrayCurrentState);
          makeXrayCurrentState[0] = enabledValue;
          mrmlNode->SetMakeXray(makeXrayCurrentState);
          qDebug() << Q_FUNC_INFO << "MakeXRay button enabled read:" << enabledValue;
        }
      }
    }
  );
  // Subscribe to data changes
  mxEnabledNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  // MakeXRay button pressed node
  if (!mxPressedNode)
  {
    return false;
  }
  QObject::connect(mxPressedNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool pressedValue = value.toBool();
        bool makeXrayCurrentState[2] = { false, false };
        mrmlNode->GetMakeXray(makeXrayCurrentState);
        makeXrayCurrentState[1] = pressedValue;
        mrmlNode->SetMakeXray(makeXrayCurrentState);
        qDebug() << Q_FUNC_INFO << "MakeXRay button pressed changed:" << pressedValue;
      }
    }
  );
  QObject::connect(mxPressedNode,
    &QOpcUaNode::attributeRead, [mxPressedNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!mxPressedNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (mxPressedNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool pressedValue = mxPressedNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool makeXrayCurrentState[2] = { false, false };
          mrmlNode->GetMakeXray(makeXrayCurrentState);
          makeXrayCurrentState[1] = pressedValue;
          mrmlNode->SetMakeXray(makeXrayCurrentState);
          qDebug() << Q_FUNC_INFO << "MakeXRay button pressed read:" << pressedValue;
        }
      }
    }
  );
  // Subscribe to data changes
  mxPressedNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);
  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectAutoManualR1ToNewCoordsNodes()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();
  // AutoManual R1 ToNewCoords button enabled node
  QOpcUaNode* amR1ToNewCoordsEnabledNode = this->FindNodeFromFullDisplayName(BUTTONS_AM_R1_NEWCOORDS_ENABLED_NODE_NAME);
  // AutoManual R1 ToNewCoords button pressed node
  QOpcUaNode* amR1ToNewCoordsPressedNode = this->FindNodeFromFullDisplayName(BUTTONS_AM_R1_NEWCOORDS_PRESSED_NODE_NAME);

  // AutoManual R1 ToNewCoords button enabled node
  if (!amR1ToNewCoordsEnabledNode)
  {
    return false;
  }
  QObject::connect(amR1ToNewCoordsEnabledNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool enabledValue = value.toBool();
        bool amR1ToNewCoordsCurrentState[2] = { false, false };
        mrmlNode->GetAutoManualR1ToNewCoords(amR1ToNewCoordsCurrentState);
        amR1ToNewCoordsCurrentState[0] = enabledValue;
        mrmlNode->SetAutoManualR1ToNewCoords(amR1ToNewCoordsCurrentState);
        qDebug() << Q_FUNC_INFO << "AutoManual R1 ToNewCoords button enabled changed:" << enabledValue;
      }
    }
  );
  QObject::connect(amR1ToNewCoordsEnabledNode,
    &QOpcUaNode::attributeRead, [amR1ToNewCoordsEnabledNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!amR1ToNewCoordsEnabledNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (amR1ToNewCoordsEnabledNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool enabledValue = amR1ToNewCoordsEnabledNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool amR1ToNewCoordsCurrentState[2] = { false, false };
          mrmlNode->GetAutoManualR1ToNewCoords(amR1ToNewCoordsCurrentState);
          amR1ToNewCoordsCurrentState[0] = enabledValue;
          mrmlNode->SetAutoManualR1ToNewCoords(amR1ToNewCoordsCurrentState);
          qDebug() << Q_FUNC_INFO << "AutoManual R1 ToNewCoords button enabled read:" << enabledValue;
        }
      }
    }
  );
  // Subscribe to data changes
  amR1ToNewCoordsEnabledNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  // AutoManual R1 ToNewCoords button pressed node
  if (!amR1ToNewCoordsPressedNode)
  {
    return false;
  }
  QObject::connect(amR1ToNewCoordsPressedNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool pressedValue = value.toBool();
        bool amR1ToNewCoordsCurrentState[2] = { false, false };
        mrmlNode->GetAutoManualR1ToNewCoords(amR1ToNewCoordsCurrentState);
        amR1ToNewCoordsCurrentState[1] = pressedValue;
        mrmlNode->SetAutoManualR1ToNewCoords(amR1ToNewCoordsCurrentState);
        qDebug() << Q_FUNC_INFO << "AutoManual R1 ToNewCoords button pressed changed:" << pressedValue;
      }
    }
  );
  QObject::connect(amR1ToNewCoordsPressedNode,
    &QOpcUaNode::attributeRead, [amR1ToNewCoordsPressedNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!amR1ToNewCoordsPressedNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (amR1ToNewCoordsPressedNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool pressedValue = amR1ToNewCoordsPressedNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool amR1ToNewCoordsCurrentState[2] = { false, false };
          mrmlNode->GetAutoManualR1ToNewCoords(amR1ToNewCoordsCurrentState);
          amR1ToNewCoordsCurrentState[1] = pressedValue;
          mrmlNode->SetAutoManualR1ToNewCoords(amR1ToNewCoordsCurrentState);
          qDebug() << Q_FUNC_INFO << "AutoManual R1 ToNewCoords button pressed read:" << pressedValue;
        }
      }
    }
  );
  // Subscribe to data changes
  amR1ToNewCoordsPressedNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);
  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectAutoManualR2ToHomeNodes()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();
  // AutoManual R2 ToHome button enabled node
  QOpcUaNode* amR2ToHomeEnabledNode = this->FindNodeFromFullDisplayName(BUTTONS_AM_R2_TOHOME_ENABLED_NODE_NAME);
  // AutoManual R2 ToHome button pressed node
  QOpcUaNode* amR2ToHomePressedNode = this->FindNodeFromFullDisplayName(BUTTONS_AM_R2_TOHOME_PRESSED_NODE_NAME);

  // AutoManual R2 ToHome button enabled node
  if (!amR2ToHomeEnabledNode)
  {
    return false;
  }
  QObject::connect(amR2ToHomeEnabledNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool enabledValue = value.toBool();
        bool amR2ToHomeCurrentState[2] = { false, false };
        mrmlNode->GetAutoManualR2ToHome(amR2ToHomeCurrentState);
        amR2ToHomeCurrentState[0] = enabledValue;
        mrmlNode->SetAutoManualR2ToHome(amR2ToHomeCurrentState);
        qDebug() << Q_FUNC_INFO << "AutoManual R2 ToHome button enabled changed:" << enabledValue;
      }
    }
  );
  QObject::connect(amR2ToHomeEnabledNode,
    &QOpcUaNode::attributeRead, [amR2ToHomeEnabledNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!amR2ToHomeEnabledNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (amR2ToHomeEnabledNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool enabledValue = amR2ToHomeEnabledNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool amR2ToHomeCurrentState[2] = { false, false };
          mrmlNode->GetAutoManualR2ToHome(amR2ToHomeCurrentState);
          amR2ToHomeCurrentState[0] = enabledValue;
          mrmlNode->SetAutoManualR2ToHome(amR2ToHomeCurrentState);
          qDebug() << Q_FUNC_INFO << "AutoManual R2 ToHome button enabled read:" << enabledValue;
        }
      }
    }
  );
  // Subscribe to data changes
  amR2ToHomeEnabledNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  // AutoManual R2 ToHome button pressed node
  if (!amR2ToHomePressedNode)
  {
    return false;
  }
  QObject::connect(amR2ToHomePressedNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool pressedValue = value.toBool();
        bool amR2ToHomeCurrentState[2] = { false, false };
        mrmlNode->GetAutoManualR2ToHome(amR2ToHomeCurrentState);
        amR2ToHomeCurrentState[1] = pressedValue;
        mrmlNode->SetAutoManualR2ToHome(amR2ToHomeCurrentState);
        qDebug() << Q_FUNC_INFO << "AutoManual R2 ToHome button pressed changed:" << pressedValue;
      }
    }
  );
  QObject::connect(amR2ToHomePressedNode,
    &QOpcUaNode::attributeRead, [amR2ToHomePressedNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!amR2ToHomePressedNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (amR2ToHomePressedNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool pressedValue = amR2ToHomePressedNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool amR2ToHomeCurrentState[2] = { false, false };
          mrmlNode->GetAutoManualR2ToHome(amR2ToHomeCurrentState);
          amR2ToHomeCurrentState[1] = pressedValue;
          mrmlNode->SetAutoManualR2ToHome(amR2ToHomeCurrentState);
          qDebug() << Q_FUNC_INFO << "AutoManual R2 ToHome button pressed read:" << pressedValue;
        }
      }
    }
  );
  // Subscribe to data changes
  amR2ToHomePressedNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);
  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectAutoManualR2ToPlane1Nodes()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }
  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();
  // AutoManual R2 ToPlane1 button enabled node
  QOpcUaNode* amR2ToPlane1EnabledNode = this->FindNodeFromFullDisplayName(BUTTONS_AM_R2_TOPLANE1_ENABLED_NODE_NAME);
  // AutoManual R2 ToPlane1 button pressed node
  QOpcUaNode* amR2ToPlane1PressedNode = this->FindNodeFromFullDisplayName(BUTTONS_AM_R2_TOPLANE1_PRESSED_NODE_NAME);

  // AutoManual R2 ToPlane1 button enabled node
  if (!amR2ToPlane1EnabledNode)
  {
    return false;
  }
  QObject::connect(amR2ToPlane1EnabledNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool enabledValue = value.toBool();
        bool amR2ToPlane1CurrentState[2] = { false, false };
        mrmlNode->GetAutoManualR2ToPlane1(amR2ToPlane1CurrentState);
        amR2ToPlane1CurrentState[0] = enabledValue;
        mrmlNode->SetAutoManualR2ToPlane1(amR2ToPlane1CurrentState);
        qDebug() << Q_FUNC_INFO << "AutoManual R2 ToPlane1 button enabled changed:" << enabledValue;
      }
    }
  );
  QObject::connect(amR2ToPlane1EnabledNode,
    &QOpcUaNode::attributeRead, [amR2ToPlane1EnabledNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!amR2ToPlane1EnabledNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (amR2ToPlane1EnabledNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool enabledValue = amR2ToPlane1EnabledNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool amR2ToPlane1CurrentState[2] = { false, false };
          mrmlNode->GetAutoManualR2ToPlane1(amR2ToPlane1CurrentState);
          amR2ToPlane1CurrentState[0] = enabledValue;
          mrmlNode->SetAutoManualR2ToPlane1(amR2ToPlane1CurrentState);
          qDebug() << Q_FUNC_INFO << "AutoManual R2 ToPlane1 button enabled read:" << enabledValue;
        }
      }
    }
  );
  // Subscribe to data changes
  amR2ToPlane1EnabledNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  // AutoManual R2 ToPlane1 button pressed node
  if (!amR2ToPlane1PressedNode)
  {
    return false;
  }
  QObject::connect(amR2ToPlane1PressedNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool pressedValue = value.toBool();
        bool amR2ToPlane1CurrentState[2] = { false, false };
        mrmlNode->GetAutoManualR2ToPlane1(amR2ToPlane1CurrentState);
        amR2ToPlane1CurrentState[1] = pressedValue;
        mrmlNode->SetAutoManualR2ToPlane1(amR2ToPlane1CurrentState);
        qDebug() << Q_FUNC_INFO << "AutoManual R2 ToPlane1 button pressed changed:" << pressedValue;
      }
    }
  );
  QObject::connect(amR2ToPlane1PressedNode,
    &QOpcUaNode::attributeRead, [amR2ToPlane1PressedNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!amR2ToPlane1PressedNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (amR2ToPlane1PressedNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool pressedValue = amR2ToPlane1PressedNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool amR2ToPlane1CurrentState[2] = { false, false };
          mrmlNode->GetAutoManualR2ToPlane1(amR2ToPlane1CurrentState);
          amR2ToPlane1CurrentState[1] = pressedValue;
          mrmlNode->SetAutoManualR2ToPlane1(amR2ToPlane1CurrentState);
          qDebug() << Q_FUNC_INFO << "AutoManual R2 ToPlane1 button pressed read:" << pressedValue;
        }
      }
    }
  );
  // Subscribe to data changes
  amR2ToPlane1PressedNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);
  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectAutoManualR2ToPlane2Nodes()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();
  // AutoManual R2 ToPlane2 button enabled node
  QOpcUaNode* amR2ToPlane2EnabledNode = this->FindNodeFromFullDisplayName(BUTTONS_AM_R2_TOPLANE2_ENABLED_NODE_NAME);
  // AutoManual R2 ToPlane2 button pressed node
  QOpcUaNode* amR2ToPlane2PressedNode = this->FindNodeFromFullDisplayName(BUTTONS_AM_R2_TOPLANE2_PRESSED_NODE_NAME);

  // AutoManual R2 ToPlane2 button enabled node
  if (!amR2ToPlane2EnabledNode)
  {
    return false;
  }
  QObject::connect(amR2ToPlane2EnabledNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool enabledValue = value.toBool();
        bool amR2ToPlane2CurrentState[2] = { false, false };
        mrmlNode->GetAutoManualR2ToPlane2(amR2ToPlane2CurrentState);
        amR2ToPlane2CurrentState[0] = enabledValue;
        mrmlNode->SetAutoManualR2ToPlane2(amR2ToPlane2CurrentState);
        qDebug() << Q_FUNC_INFO << "AutoManual R2 ToPlane2 button enabled changed:" << enabledValue;
      }
    }
  );
  QObject::connect(amR2ToPlane2EnabledNode,
    &QOpcUaNode::attributeRead, [amR2ToPlane2EnabledNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!amR2ToPlane2EnabledNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (amR2ToPlane2EnabledNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool enabledValue = amR2ToPlane2EnabledNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool amR2ToPlane2CurrentState[2] = { false, false };
          mrmlNode->GetAutoManualR2ToPlane2(amR2ToPlane2CurrentState);
          amR2ToPlane2CurrentState[0] = enabledValue;
          mrmlNode->SetAutoManualR2ToPlane2(amR2ToPlane2CurrentState);
          qDebug() << Q_FUNC_INFO << "AutoManual R2 ToPlane2 button enabled read:" << enabledValue;
        }
      }
    }
  );
  // Subscribe to data changes
  amR2ToPlane2EnabledNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  // AutoManual R2 ToPlane2 button pressed node
  if (!amR2ToPlane2PressedNode)
  {
    return false;
  }
  QObject::connect(amR2ToPlane2PressedNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool pressedValue = value.toBool();
        bool amR2ToPlane2CurrentState[2] = { false, false };
        mrmlNode->GetAutoManualR2ToPlane2(amR2ToPlane2CurrentState);
        amR2ToPlane2CurrentState[1] = pressedValue;
        mrmlNode->SetAutoManualR2ToPlane2(amR2ToPlane2CurrentState);
        qDebug() << Q_FUNC_INFO << "AutoManual R2 ToPlane2 button pressed changed:" << pressedValue;
      }
    }
  );
  QObject::connect(amR2ToPlane2PressedNode,
    &QOpcUaNode::attributeRead, [amR2ToPlane2PressedNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!amR2ToPlane2PressedNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (amR2ToPlane2PressedNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool pressedValue = amR2ToPlane2PressedNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool amR2ToPlane2CurrentState[2] = { false, false };
          mrmlNode->GetAutoManualR2ToPlane2(amR2ToPlane2CurrentState);
          amR2ToPlane2CurrentState[1] = pressedValue;
          mrmlNode->SetAutoManualR2ToPlane2(amR2ToPlane2CurrentState);
          qDebug() << Q_FUNC_INFO << "AutoManual R2 ToPlane2 button pressed read:" << pressedValue;
        }
      }
    }
  );
  // Subscribe to data changes
  amR2ToPlane2PressedNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);
  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectAutoManualMovementNodes()
{
  bool res = this->ConnectAutoManualR1ToLoadNodes();

  res &= this->ConnectAutoManualR1LoadToIsoNodes();
  res &= this->ConnectAutoManualR1ToNewCoordsNodes();
  res &= this->ConnectAutoManualR2ToHomeNodes();

  res &= this->ConnectAutoManualR2ToPlane1Nodes();
  res &= this->ConnectAutoManualR2ToPlane2Nodes();
  res &= this->ConnectAutoManualR1R2EmergencyEvacuationNodes();
  res &= this->ConnectAutoManualApplyTablePositionNodes();
  res &= this->ConnectAutoManualApplyNewCoordsNodes();

  return res;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectAutoManualApplyTablePositionNodes()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();
  // AutoManual ApplyTablePosition button enabled node
  QOpcUaNode* amApplyTablePositionEnabledNode = this->FindNodeFromFullDisplayName(BUTTONS_AM_APPLYTABLEPOSITION_ENABLED_NODE_NAME);
  // AutoManual ApplyTablePosition button pressed node
  QOpcUaNode* amApplyTablePositionPressedNode = this->FindNodeFromFullDisplayName(BUTTONS_AM_APPLYTABLEPOSITION_PRESSED_NODE_NAME);

  // AutoManual ApplyTablePosition button enabled node
  if (!amApplyTablePositionEnabledNode)
  {
    return false;
  }
  QObject::connect(amApplyTablePositionEnabledNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool enabledValue = value.toBool();
        bool amApplyTablePositionCurrentState[2] = { false, false };
        mrmlNode->GetAutoManualApplyTabletopPosition(amApplyTablePositionCurrentState);
        amApplyTablePositionCurrentState[0] = enabledValue;
        mrmlNode->SetAutoManualApplyTabletopPosition(amApplyTablePositionCurrentState);
        qDebug() << Q_FUNC_INFO << "AutoManual ApplyTablePosition button enabled changed:" << enabledValue;
      }
    }
  );
  QObject::connect(amApplyTablePositionEnabledNode,
    &QOpcUaNode::attributeRead, [amApplyTablePositionEnabledNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!amApplyTablePositionEnabledNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (amApplyTablePositionEnabledNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool enabledValue = amApplyTablePositionEnabledNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool amApplyTablePositionCurrentState[2] = { false, false };
          mrmlNode->GetAutoManualApplyTabletopPosition(amApplyTablePositionCurrentState);
          amApplyTablePositionCurrentState[0] = enabledValue;
          mrmlNode->SetAutoManualApplyTabletopPosition(amApplyTablePositionCurrentState);
          qDebug() << Q_FUNC_INFO << "AutoManual ApplyTablePosition button enabled read:" << enabledValue;
        }
      }
    }
  );
  // Subscribe to data changes
  amApplyTablePositionEnabledNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  // AutoManual ApplyTablePosition button pressed node
  if (!amApplyTablePositionPressedNode)
  {
    return false;
  }
  QObject::connect(amApplyTablePositionPressedNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool pressedValue = value.toBool();
        bool amApplyTablePositionCurrentState[2] = { false, false };
        mrmlNode->GetAutoManualApplyTabletopPosition(amApplyTablePositionCurrentState);
        amApplyTablePositionCurrentState[1] = pressedValue;
        mrmlNode->SetAutoManualApplyTabletopPosition(amApplyTablePositionCurrentState);
        qDebug() << Q_FUNC_INFO << "AutoManual ApplyTablePosition button pressed changed:" << pressedValue;
      }
    }
  );
  QObject::connect(amApplyTablePositionPressedNode,
    &QOpcUaNode::attributeRead, [amApplyTablePositionPressedNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!amApplyTablePositionPressedNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (amApplyTablePositionPressedNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool pressedValue = amApplyTablePositionPressedNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool amApplyTablePositionCurrentState[2] = { false, false };
          mrmlNode->GetAutoManualApplyTabletopPosition(amApplyTablePositionCurrentState);
          amApplyTablePositionCurrentState[1] = pressedValue;
          mrmlNode->SetAutoManualApplyTabletopPosition(amApplyTablePositionCurrentState);
          qDebug() << Q_FUNC_INFO << "AutoManual ApplyTablePosition button pressed read:" << pressedValue;
        }
      }
    }
  );
  // Subscribe to data changes
  amApplyTablePositionPressedNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectAutoManualApplyNewCoordsNodes()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();
  // AutoManual ApplyNewCoords button enabled node
  QOpcUaNode* amApplyNewCoordsEnabledNode = this->FindNodeFromFullDisplayName(BUTTONS_AM_APPLYNEWCOORDS_ENABLED_NODE_NAME);
  // AutoManual ApplyNewCoords button pressed node
  QOpcUaNode* amAApplyNewCoordsPressedNode = this->FindNodeFromFullDisplayName(BUTTONS_AM_APPLYNEWCOORDS_PRESSED_NODE_NAME);

  // AutoManual ApplyNewCoords button enabled node
  if (!amApplyNewCoordsEnabledNode)
  {
    return false;
  }
  QObject::connect(amApplyNewCoordsEnabledNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool enabledValue = value.toBool();
        bool amApplyNewCoordsCurrentState[2] = { false, false };
        mrmlNode->GetAutoManualApplyNewCoords(amApplyNewCoordsCurrentState);
        amApplyNewCoordsCurrentState[0] = enabledValue;
        mrmlNode->GetAutoManualApplyNewCoords(amApplyNewCoordsCurrentState);
        qDebug() << Q_FUNC_INFO << "AutoManual ApplyNewCoords button enabled changed:" << enabledValue;
      }
    }
  );
  QObject::connect(amApplyNewCoordsEnabledNode,
    &QOpcUaNode::attributeRead, [amApplyNewCoordsEnabledNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!amApplyNewCoordsEnabledNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (amApplyNewCoordsEnabledNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool enabledValue = amApplyNewCoordsEnabledNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool amApplyNewCoordsCurrentState[2] = { false, false };
          mrmlNode->GetAutoManualApplyNewCoords(amApplyNewCoordsCurrentState);
          amApplyNewCoordsCurrentState[0] = enabledValue;
          mrmlNode->GetAutoManualApplyNewCoords(amApplyNewCoordsCurrentState);
          qDebug() << Q_FUNC_INFO << "AutoManual ApplyNewCoords button enabled read:" << enabledValue;
        }
      }
    }
  );
  // Subscribe to data changes
  amApplyNewCoordsEnabledNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  // AutoManual ApplyNewCoords button pressed node
  if (!amAApplyNewCoordsPressedNode)
  {
    return false;
  }
  QObject::connect(amAApplyNewCoordsPressedNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool pressedValue = value.toBool();
        bool amApplyNewCoordsCurrentState[2] = { false, false };
        mrmlNode->GetAutoManualApplyNewCoords(amApplyNewCoordsCurrentState);
        amApplyNewCoordsCurrentState[1] = pressedValue;
        mrmlNode->GetAutoManualApplyNewCoords(amApplyNewCoordsCurrentState);
        qDebug() << Q_FUNC_INFO << "AutoManual ApplyNewCoords button pressed changed:" << pressedValue;
      }
    }
  );
  QObject::connect(amAApplyNewCoordsPressedNode,
    &QOpcUaNode::attributeRead, [amAApplyNewCoordsPressedNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!amAApplyNewCoordsPressedNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (amAApplyNewCoordsPressedNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool pressedValue = amAApplyNewCoordsPressedNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool amApplyNewCoordsCurrentState[2] = { false, false };
          mrmlNode->GetAutoManualApplyNewCoords(amApplyNewCoordsCurrentState);
          amApplyNewCoordsCurrentState[1] = pressedValue;
          mrmlNode->GetAutoManualApplyNewCoords(amApplyNewCoordsCurrentState);
          qDebug() << Q_FUNC_INFO << "AutoManual ApplyNewCoords button pressed read:" << pressedValue;
        }
      }
    }
  );
  // Subscribe to data changes
  amAApplyNewCoordsPressedNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectKukaAllowT1Nodes()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();
  // KukaMovement AllowT1 button enabled node
  QOpcUaNode* kcmAllowT1EnabledNode = this->FindNodeFromFullDisplayName(BUTTONS_KCM_ALLOWT1_ENABLED_NODE_NAME);
  // KukaMovement AllowT1 button pressed node
  QOpcUaNode* kcmAllowT1PressedNode = this->FindNodeFromFullDisplayName(BUTTONS_KCM_ALLOWT1_PRESSED_NODE_NAME);

  // KukaMovement AllowT1 button enabled node
  if (!kcmAllowT1EnabledNode)
  {
    return false;
  }
  QObject::connect(kcmAllowT1EnabledNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool enabledValue = value.toBool();
        bool kcmAllowT1CurrentState[2] = { false, false };
        mrmlNode->GetKukaAllowT1(kcmAllowT1CurrentState);
        kcmAllowT1CurrentState[0] = enabledValue;
        mrmlNode->SetKukaAllowT1(kcmAllowT1CurrentState);
        qDebug() << Q_FUNC_INFO << "KukaMovement AllowT1 button enabled changed:" << enabledValue;
      }
    }
  );
  QObject::connect(kcmAllowT1EnabledNode,
    &QOpcUaNode::attributeRead, [kcmAllowT1EnabledNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!kcmAllowT1EnabledNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (kcmAllowT1EnabledNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool enabledValue = kcmAllowT1EnabledNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool kcmAllowT1CurrentState[2] = { false, false };
          mrmlNode->GetKukaAllowT1(kcmAllowT1CurrentState);
          kcmAllowT1CurrentState[0] = enabledValue;
          mrmlNode->SetKukaAllowT1(kcmAllowT1CurrentState);
          qDebug() << Q_FUNC_INFO << "KukaMovement AllowT1 button enabled read:" << enabledValue;
        }
      }
    }
  );
  // Subscribe to data changes
  kcmAllowT1EnabledNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  // KukaMovement AllowT1 button pressed node
  if (!kcmAllowT1PressedNode)
  {
    return false;
  }
  QObject::connect(kcmAllowT1PressedNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool pressedValue = value.toBool();
        bool kcmAllowT1CurrentState[2] = { false, false };
        mrmlNode->GetKukaAllowT1(kcmAllowT1CurrentState);
        kcmAllowT1CurrentState[1] = pressedValue;
        mrmlNode->SetKukaAllowT1(kcmAllowT1CurrentState);
        qDebug() << Q_FUNC_INFO << "KukaMovement AllowT1 button pressed changed:" << pressedValue;
      }
    }
  );
  QObject::connect(kcmAllowT1PressedNode,
    &QOpcUaNode::attributeRead, [kcmAllowT1PressedNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!kcmAllowT1PressedNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (kcmAllowT1PressedNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool pressedValue = kcmAllowT1PressedNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool kcmAllowT1CurrentState[2] = { false, false };
          mrmlNode->GetKukaAllowT1(kcmAllowT1CurrentState);
          kcmAllowT1CurrentState[1] = pressedValue;
          mrmlNode->SetKukaAllowT1(kcmAllowT1CurrentState);
          qDebug() << Q_FUNC_INFO << "KukaMovement AllowT1 button pressed read:" << pressedValue;
        }
      }
    }
  );
  // Subscribe to data changes
  kcmAllowT1PressedNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectKukaAllowXrayNodes()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();
  // KukaMovement AllowXRay button enabled node
  QOpcUaNode* kcmAllowXrayEnabledNode = this->FindNodeFromFullDisplayName(BUTTONS_KCM_ALLOWXRAY_ENABLED_NODE_NAME);
  // KukaMovement AllowXRay button pressed node
  QOpcUaNode* kcmAllowXrayPressedNode = this->FindNodeFromFullDisplayName(BUTTONS_KCM_ALLOWXRAY_PRESSED_NODE_NAME);

  // KukaMovement AllowXRay button enabled node
  if (!kcmAllowXrayEnabledNode)
  {
    return false;
  }
  QObject::connect(kcmAllowXrayEnabledNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool enabledValue = value.toBool();
        bool kcmAllowXrayCurrentState[2] = { false, false };
        mrmlNode->GetKukaAllowXray(kcmAllowXrayCurrentState);
        kcmAllowXrayCurrentState[0] = enabledValue;
        mrmlNode->SetKukaAllowXray(kcmAllowXrayCurrentState);
        qDebug() << Q_FUNC_INFO << "KukaMovement AllowXRay button enabled changed:" << enabledValue;
      }
    }
  );
  QObject::connect(kcmAllowXrayEnabledNode,
    &QOpcUaNode::attributeRead, [kcmAllowXrayEnabledNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!kcmAllowXrayEnabledNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (kcmAllowXrayEnabledNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool enabledValue = kcmAllowXrayEnabledNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool kcmAllowXrayCurrentState[2] = { false, false };
          mrmlNode->GetKukaAllowXray(kcmAllowXrayCurrentState);
          kcmAllowXrayCurrentState[0] = enabledValue;
          mrmlNode->SetKukaAllowXray(kcmAllowXrayCurrentState);
          qDebug() << Q_FUNC_INFO << "KukaMovement AllowXRay button enabled read:" << enabledValue;
        }
      }
    }
  );
  // Subscribe to data changes
  kcmAllowXrayEnabledNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  // KukaMovement AllowXRay button pressed node
  if (!kcmAllowXrayPressedNode)
  {
    return false;
  }
  QObject::connect(kcmAllowXrayPressedNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool pressedValue = value.toBool();
        bool kcmAllowXrayCurrentState[2] = { false, false };
        mrmlNode->GetKukaAllowXray(kcmAllowXrayCurrentState);
        kcmAllowXrayCurrentState[1] = pressedValue;
        mrmlNode->SetKukaAllowXray(kcmAllowXrayCurrentState);
        qDebug() << Q_FUNC_INFO << "KukaMovement AllowXRay button pressed changed:" << pressedValue;
      }
    }
  );
  QObject::connect(kcmAllowXrayPressedNode,
    &QOpcUaNode::attributeRead, [kcmAllowXrayPressedNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!kcmAllowXrayPressedNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (kcmAllowXrayPressedNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool pressedValue = kcmAllowXrayPressedNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool kcmAllowXrayCurrentState[2] = { false, false };
          mrmlNode->GetKukaAllowXray(kcmAllowXrayCurrentState);
          kcmAllowXrayCurrentState[1] = pressedValue;
          mrmlNode->SetKukaAllowXray(kcmAllowXrayCurrentState);
          qDebug() << Q_FUNC_INFO << "KukaMovement AllowXRay button pressed read:" << pressedValue;
        }
      }
    }
  );
  // Subscribe to data changes
  kcmAllowXrayPressedNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectKukaAllowBeamNodes()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();
  // KukaMovement AllowBeam button enabled node
  QOpcUaNode* kcmAllowBeamEnabledNode = this->FindNodeFromFullDisplayName(BUTTONS_KCM_ALLOWBEAM_ENABLED_NODE_NAME);
  // KukaMovement AllowBeam button pressed node
  QOpcUaNode* kcmAllowBeamPressedNode = this->FindNodeFromFullDisplayName(BUTTONS_KCM_ALLOWBEAM_PRESSED_NODE_NAME);

  // KukaMovement AllowBeam button enabled node
  if (!kcmAllowBeamEnabledNode)
  {
    return false;
  }
  QObject::connect(kcmAllowBeamEnabledNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool enabledValue = value.toBool();
        bool kcmAllowBeamCurrentState[2] = { false, false };
        mrmlNode->GetKukaAllowBeam(kcmAllowBeamCurrentState);
        kcmAllowBeamCurrentState[0] = enabledValue;
        mrmlNode->SetKukaAllowBeam(kcmAllowBeamCurrentState);
        qDebug() << Q_FUNC_INFO << "KukaMovement AllowBeam button enabled changed:" << enabledValue;
      }
    }
  );
  QObject::connect(kcmAllowBeamEnabledNode,
    &QOpcUaNode::attributeRead, [kcmAllowBeamEnabledNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!kcmAllowBeamEnabledNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (kcmAllowBeamEnabledNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool enabledValue = kcmAllowBeamEnabledNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool kcmAllowBeamCurrentState[2] = { false, false };
          mrmlNode->GetKukaAllowBeam(kcmAllowBeamCurrentState);
          kcmAllowBeamCurrentState[0] = enabledValue;
          mrmlNode->SetKukaAllowBeam(kcmAllowBeamCurrentState);
          qDebug() << Q_FUNC_INFO << "KukaMovement AllowBeam button enabled read:" << enabledValue;
        }
      }
    }
  );

  // Subscribe to data changes
  kcmAllowBeamEnabledNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  // KukaMovement AllowBeam button pressed node
  if (!kcmAllowBeamPressedNode)
  {
    return false;
  }
  QObject::connect(kcmAllowBeamPressedNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool pressedValue = value.toBool();
        bool kcmAllowBeamCurrentState[2] = { false, false };
        mrmlNode->GetKukaAllowBeam(kcmAllowBeamCurrentState);
        kcmAllowBeamCurrentState[1] = pressedValue;
        mrmlNode->SetKukaAllowBeam(kcmAllowBeamCurrentState);
        qDebug() << Q_FUNC_INFO << "KukaMovement AllowBeam button pressed changed:" << pressedValue;
      }
    }
  );

  QObject::connect(kcmAllowBeamPressedNode,
    &QOpcUaNode::attributeRead, [kcmAllowBeamPressedNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!kcmAllowBeamPressedNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (kcmAllowBeamPressedNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          bool pressedValue = kcmAllowBeamPressedNode->attribute(QOpcUa::NodeAttribute::Value).toBool();
          bool kcmAllowBeamCurrentState[2] = { false, false };
          mrmlNode->GetKukaAllowBeam(kcmAllowBeamCurrentState);
          kcmAllowBeamCurrentState[1] = pressedValue;
          mrmlNode->SetKukaAllowBeam(kcmAllowBeamCurrentState);
          qDebug() << Q_FUNC_INFO << "KukaMovement AllowBeam button pressed read:" << pressedValue;
        }
      }
    }
  );
  // Subscribe to data changes
  kcmAllowBeamPressedNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectKukaMovementNodes()
{
  bool res = this->ConnectKukaAllowT1Nodes();
  res &= this->ConnectKukaAllowXrayNodes();
  res &= this->ConnectKukaAllowBeamNodes();
  return res;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectModeAndStatusNodes()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();

  // Mode node
  QOpcUaNode* modeNode = this->FindNodeFromFullDisplayName(ASU_MODE_NODE_NAME);
  // Status RTK node
  QOpcUaNode* statusRtkNode = this->FindNodeFromFullDisplayName(ASU_STATUS_RTK_NODE_NAME);
  // Status R1 (TableTop) node
  QOpcUaNode* statusR1Node = this->FindNodeFromFullDisplayName(ASU_STATUS_R1_TABLE_NODE_NAME);
  // Status R2 (C-Arm) node
  QOpcUaNode* statusR2Node = this->FindNodeFromFullDisplayName(ASU_STATUS_R2_CARM_NODE_NAME);

  // Connect signal handlers for subscribed values
  // Mode node
  if (!modeNode)
  {
    return false;
  }
  QObject::connect(modeNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool ok = false;
        int modeValue = value.toInt(&ok);
        if (ok)
        {
          mrmlNode->SetMode(static_cast< vtkMRMLSiemensPlcOpcUaNode::ModeType >(modeValue));
          qDebug() << Q_FUNC_INFO << "Mode value changed:" << modeValue;
        }
      }
    }
  );
  QObject::connect(modeNode,
    &QOpcUaNode::attributeRead, [modeNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!modeNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (modeNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = modeNode->attribute(QOpcUa::NodeAttribute::Value);
          bool ok = false;
          int modeValue = value.toInt(&ok); // Get the attribute from the cache
          if (ok)
          {
            mrmlNode->SetMode(static_cast< vtkMRMLSiemensPlcOpcUaNode::ModeType >(modeValue));
            qDebug() << Q_FUNC_INFO << "Mode value read:" << modeValue;
          }
        }
      }
    }
  );
  // Subscribe to data changes
  modeNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  // Status RTK node
  if (!statusRtkNode)
  {
    return false;
  }
  QObject::connect(statusRtkNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool ok = false;
        uint16_t statusRtkValue = value.toInt(&ok);
        if (ok)
        {
          mrmlNode->SetStateRtk(statusRtkValue);
          qDebug() << Q_FUNC_INFO << "Status RTK value changed:" << statusRtkValue;
        }
      }
    }
  );
  QObject::connect(statusRtkNode,
    &QOpcUaNode::attributeRead, [statusRtkNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!statusRtkNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (statusRtkNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = statusRtkNode->attribute(QOpcUa::NodeAttribute::Value);
          bool ok = false;
          uint16_t statusRtkValue = value.toInt(&ok); // Get the attribute from the cache
          if (ok)
          {
            mrmlNode->SetStateRtk(statusRtkValue);
            qDebug() << Q_FUNC_INFO << "Status RTK value read:" << statusRtkValue;
          }
        }
      }
    }
  );
  // Subscribe to data changes
  statusRtkNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  // Status R1 (TableTop) node
  if (!statusR1Node)
  {
    return false;
  }
  QObject::connect(statusR1Node,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool ok = false;
        uint16_t statusR1Value = value.toInt(&ok);
        if (ok)
        {
          mrmlNode->SetStateR1(statusR1Value);
          qDebug() << Q_FUNC_INFO << "Status R1 value changed:" << statusR1Value;
        }
      }
    }
  );
  QObject::connect(statusR1Node,
    &QOpcUaNode::attributeRead, [statusR1Node, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!statusR1Node || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (statusR1Node->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = statusR1Node->attribute(QOpcUa::NodeAttribute::Value);
          bool ok = false;
          uint16_t statusR1Value = value.toInt(&ok); // Get the attribute from the cache
          if (ok)
          {
            mrmlNode->SetStateR1(statusR1Value);
            qDebug() << Q_FUNC_INFO << "Status R1 value read:" << statusR1Value;
          }
        }
      }
    }
  );
  // Subscribe to data changes
  statusR1Node->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  // Status R2 (C-Arm) node
  if (!statusR2Node)
  {
    return false;
  }
  QObject::connect(statusR2Node,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool ok = false;
        uint16_t statusR2Value = value.toInt(&ok);
        if (ok)
        {
          mrmlNode->SetStateR2(statusR2Value);
          qDebug() << Q_FUNC_INFO << "Status R2 value changed:" << statusR2Value;
        }
      }
    }
  );
  QObject::connect(statusR2Node,
    &QOpcUaNode::attributeRead, [statusR2Node, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!statusR2Node || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (statusR2Node->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = statusR2Node->attribute(QOpcUa::NodeAttribute::Value);
          bool ok = false;
          uint16_t statusR2Value = value.toInt(&ok); // Get the attribute from the cache
          if (ok)
          {
            mrmlNode->SetStateR2(statusR2Value);
            qDebug() << Q_FUNC_INFO << "Status R2 value read:" << statusR2Value;
          }
        }
      }
    }
  );
  // Subscribe to data changes
  statusR2Node->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectCurrentCoordA1R1Node()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();

  // KUKA R1 Axis A1 node
  QOpcUaNode* r1AxisA1Node = this->FindNodeFromFullDisplayName(KUKA_CURRENTCOORDS_R1_A1_NODE_NAME);

  // Connect signal handlers for subscribed values
  if (!r1AxisA1Node)
  {
    return false;
  }
  QObject::connect(r1AxisA1Node,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool ok = false;
        int angleValue = value.toInt(&ok);
        if (ok)
        {
          int32_t r1AxisCoords[6] = {};
          mrmlNode->GetCurrentAxisCoordsR1(r1AxisCoords);
          r1AxisCoords[0] = angleValue;
          {
            MRMLNodeModifyBlocker block(mrmlNode);
            mrmlNode->SetCurrentAxisCoordsR1(r1AxisCoords);
          }
          mrmlNode->InvokeCustomModifiedEvent(vtkMRMLSiemensPlcOpcUaNode::TableRobotAxisCoordsModified);
          qDebug() << Q_FUNC_INFO << "R1 Axis A1 angle changed:" << angleValue;
        }
      }
    }
  );
  QObject::connect(r1AxisA1Node,
    &QOpcUaNode::attributeRead, [r1AxisA1Node, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!r1AxisA1Node || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (r1AxisA1Node->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = r1AxisA1Node->attribute(QOpcUa::NodeAttribute::Value);
          bool ok = false;
          int angleValue = value.toInt(&ok); // Get the attribute from the cache
          if (ok)
          {
            int32_t r1AxisCoords[6] = {};
            mrmlNode->GetCurrentAxisCoordsR1(r1AxisCoords);
            r1AxisCoords[0] = angleValue;
            {
              MRMLNodeModifyBlocker block(mrmlNode);
              mrmlNode->SetCurrentAxisCoordsR1(r1AxisCoords);
            }
            mrmlNode->InvokeCustomModifiedEvent(vtkMRMLSiemensPlcOpcUaNode::TableRobotAxisCoordsModified);
            qDebug() << Q_FUNC_INFO << "R1 Axis A1 angle read:" << angleValue;
          }
        }
      }
    }
  );
  // Subscribe to data changes
  r1AxisA1Node->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectCurrentCoordA2R1Node()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();

  // KUKA R1 Axis A2 node
  QOpcUaNode* r1AxisA2Node = this->FindNodeFromFullDisplayName(KUKA_CURRENTCOORDS_R1_A2_NODE_NAME);

  // Connect signal handlers for subscribed values
  if (!r1AxisA2Node)
  {
    return false;
  }
  QObject::connect(r1AxisA2Node,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool ok = false;
        int angleValue = value.toInt(&ok);
        if (ok)
        {
          int32_t r1AxisCoords[6] = {};
          mrmlNode->GetCurrentAxisCoordsR1(r1AxisCoords);
          r1AxisCoords[1] = angleValue;
          {
            MRMLNodeModifyBlocker block(mrmlNode);
            mrmlNode->SetCurrentAxisCoordsR1(r1AxisCoords);
          }
          mrmlNode->InvokeCustomModifiedEvent(vtkMRMLSiemensPlcOpcUaNode::TableRobotAxisCoordsModified);
          qDebug() << Q_FUNC_INFO << "R1 Axis A2 angle changed:" << angleValue;
        }
      }
    }
  );
  QObject::connect(r1AxisA2Node,
    &QOpcUaNode::attributeRead, [r1AxisA2Node, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!r1AxisA2Node || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (r1AxisA2Node->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = r1AxisA2Node->attribute(QOpcUa::NodeAttribute::Value);
          bool ok = false;
          int angleValue = value.toInt(&ok); // Get the attribute from the cache
          if (ok)
          {
            int32_t r1AxisCoords[6] = {};
            mrmlNode->GetCurrentAxisCoordsR1(r1AxisCoords);
            r1AxisCoords[1] = angleValue;
            {
              MRMLNodeModifyBlocker block(mrmlNode);
              mrmlNode->SetCurrentAxisCoordsR1(r1AxisCoords);
            }
            mrmlNode->InvokeCustomModifiedEvent(vtkMRMLSiemensPlcOpcUaNode::TableRobotAxisCoordsModified);
            qDebug() << Q_FUNC_INFO << "R1 Axis A2 angle read:" << angleValue;
          }
        }
      }
    }
  );
  // Subscribe to data changes
  r1AxisA2Node->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectCurrentCoordA3R1Node()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();

  // KUKA R1 Axis A3 node
  QOpcUaNode* r1AxisA3Node = this->FindNodeFromFullDisplayName(KUKA_CURRENTCOORDS_R1_A3_NODE_NAME);

  // Connect signal handlers for subscribed values
  if (!r1AxisA3Node)
  {
    return false;
  }
  QObject::connect(r1AxisA3Node,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool ok = false;
        int angleValue = value.toInt(&ok);
        if (ok)
        {
          int32_t r1AxisCoords[6] = {};
          mrmlNode->GetCurrentAxisCoordsR1(r1AxisCoords);
          r1AxisCoords[2] = angleValue;
          {
            MRMLNodeModifyBlocker block(mrmlNode);
            mrmlNode->SetCurrentAxisCoordsR1(r1AxisCoords);
          }
          mrmlNode->InvokeCustomModifiedEvent(vtkMRMLSiemensPlcOpcUaNode::TableRobotAxisCoordsModified);
          qDebug() << Q_FUNC_INFO << "R1 Axis A3 angle changed:" << angleValue;
        }
      }
    }
  );
  QObject::connect(r1AxisA3Node,
    &QOpcUaNode::attributeRead, [r1AxisA3Node, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!r1AxisA3Node || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (r1AxisA3Node->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = r1AxisA3Node->attribute(QOpcUa::NodeAttribute::Value);
          bool ok = false;
          int angleValue = value.toInt(&ok); // Get the attribute from the cache
          if (ok)
          {
            int32_t r1AxisCoords[6] = {};
            mrmlNode->GetCurrentAxisCoordsR1(r1AxisCoords);
            r1AxisCoords[2] = angleValue;
            {
              MRMLNodeModifyBlocker block(mrmlNode);
              mrmlNode->SetCurrentAxisCoordsR1(r1AxisCoords);
            }
            mrmlNode->InvokeCustomModifiedEvent(vtkMRMLSiemensPlcOpcUaNode::TableRobotAxisCoordsModified);
            qDebug() << Q_FUNC_INFO << "R1 Axis A3 angle read:" << angleValue;
          }
        }
      }
    }
  );

  // Subscribe to data changes
  r1AxisA3Node->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectCurrentCoordA4R1Node()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();

  // KUKA R1 Axis A4 node
  QOpcUaNode* r1AxisA4Node = this->FindNodeFromFullDisplayName(KUKA_CURRENTCOORDS_R1_A4_NODE_NAME);

  // Connect signal handlers for subscribed values
  if (!r1AxisA4Node)
  {
    return false;
  }
  QObject::connect(r1AxisA4Node,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool ok = false;
        int angleValue = value.toInt(&ok);
        if (ok)
        {
          int32_t r1AxisCoords[6] = {};
          mrmlNode->GetCurrentAxisCoordsR1(r1AxisCoords);
          r1AxisCoords[3] = angleValue;
          {
            MRMLNodeModifyBlocker block(mrmlNode);
            mrmlNode->SetCurrentAxisCoordsR1(r1AxisCoords);
          }
          mrmlNode->InvokeCustomModifiedEvent(vtkMRMLSiemensPlcOpcUaNode::TableRobotAxisCoordsModified);
          qDebug() << Q_FUNC_INFO << "R1 Axis A4 angle changed:" << angleValue;
        }
      }
    }
  );
  QObject::connect(r1AxisA4Node,
    &QOpcUaNode::attributeRead, [r1AxisA4Node, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!r1AxisA4Node || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (r1AxisA4Node->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = r1AxisA4Node->attribute(QOpcUa::NodeAttribute::Value);
          bool ok = false;
          int angleValue = value.toInt(&ok); // Get the attribute from the cache
          if (ok)
          {
            int32_t r1AxisCoords[6] = {};
            mrmlNode->GetCurrentAxisCoordsR1(r1AxisCoords);
            r1AxisCoords[3] = angleValue;
            {
              MRMLNodeModifyBlocker block(mrmlNode);
              mrmlNode->SetCurrentAxisCoordsR1(r1AxisCoords);
            }
            mrmlNode->InvokeCustomModifiedEvent(vtkMRMLSiemensPlcOpcUaNode::TableRobotAxisCoordsModified);
            qDebug() << Q_FUNC_INFO << "R1 Axis A4 angle read:" << angleValue;
          }
        }
      }
    }
  );
  // Subscribe to data changes
  r1AxisA4Node->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectCurrentCoordA5R1Node()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();

  // KUKA R1 Axis A5 node
  QOpcUaNode* r1AxisA5Node = this->FindNodeFromFullDisplayName(KUKA_CURRENTCOORDS_R1_A5_NODE_NAME);

  // Connect signal handlers for subscribed values
  if (!r1AxisA5Node)
  {
    return false;
  }
  QObject::connect(r1AxisA5Node,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool ok = false;
        int angleValue = value.toInt(&ok);
        if (ok)
        {
          int32_t r1AxisCoords[6] = {};
          mrmlNode->GetCurrentAxisCoordsR1(r1AxisCoords);
          r1AxisCoords[4] = angleValue;
          {
            MRMLNodeModifyBlocker block(mrmlNode);
            mrmlNode->SetCurrentAxisCoordsR1(r1AxisCoords);
          }
          mrmlNode->InvokeCustomModifiedEvent(vtkMRMLSiemensPlcOpcUaNode::TableRobotAxisCoordsModified);
          qDebug() << Q_FUNC_INFO << "R1 Axis A5 angle changed:" << angleValue;
        }
      }
    }
  );
  QObject::connect(r1AxisA5Node,
    &QOpcUaNode::attributeRead, [r1AxisA5Node, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!r1AxisA5Node || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (r1AxisA5Node->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = r1AxisA5Node->attribute(QOpcUa::NodeAttribute::Value);
          bool ok = false;
          int angleValue = value.toInt(&ok); // Get the attribute from the cache
          if (ok)
          {
            int32_t r1AxisCoords[6] = {};
            mrmlNode->GetCurrentAxisCoordsR1(r1AxisCoords);
            r1AxisCoords[4] = angleValue;
            {
              MRMLNodeModifyBlocker block(mrmlNode);
              mrmlNode->SetCurrentAxisCoordsR1(r1AxisCoords);
            }
            mrmlNode->InvokeCustomModifiedEvent(vtkMRMLSiemensPlcOpcUaNode::TableRobotAxisCoordsModified);
            qDebug() << Q_FUNC_INFO << "R1 Axis A5 angle read:" << angleValue;
          }
        }
      }
    }
  );

  // Subscribe to data changes
  r1AxisA5Node->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectCurrentCoordA6R1Node()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();

  // KUKA R1 Axis A6 node
  QOpcUaNode* r1AxisA6Node = this->FindNodeFromFullDisplayName(KUKA_CURRENTCOORDS_R1_A6_NODE_NAME);

  // Connect signal handlers for subscribed values
  if (!r1AxisA6Node)
  {
    return false;
  }
  QObject::connect(r1AxisA6Node,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool ok = false;
        int angleValue = value.toInt(&ok);
        if (ok)
        {
          int32_t r1AxisCoords[6] = {};
          mrmlNode->GetCurrentAxisCoordsR1(r1AxisCoords);
          r1AxisCoords[5] = angleValue;
          {
            MRMLNodeModifyBlocker block(mrmlNode);
            mrmlNode->SetCurrentAxisCoordsR1(r1AxisCoords);
          }
          mrmlNode->InvokeCustomModifiedEvent(vtkMRMLSiemensPlcOpcUaNode::TableRobotAxisCoordsModified);
          qDebug() << Q_FUNC_INFO << "R1 Axis A6 angle changed:" << angleValue;
        }
      }
    }
  );
  QObject::connect(r1AxisA6Node,
    &QOpcUaNode::attributeRead, [r1AxisA6Node, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!r1AxisA6Node || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (r1AxisA6Node->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = r1AxisA6Node->attribute(QOpcUa::NodeAttribute::Value);
          bool ok = false;
          int angleValue = value.toInt(&ok); // Get the attribute from the cache
          if (ok)
          {
            int32_t r1AxisCoords[6] = {};
            mrmlNode->GetCurrentAxisCoordsR1(r1AxisCoords);
            r1AxisCoords[5] = angleValue;
            {
              MRMLNodeModifyBlocker block(mrmlNode);
              mrmlNode->SetCurrentAxisCoordsR1(r1AxisCoords);
            }
            mrmlNode->InvokeCustomModifiedEvent(vtkMRMLSiemensPlcOpcUaNode::TableRobotAxisCoordsModified);
            qDebug() << Q_FUNC_INFO << "R1 Axis A6 angle read:" << angleValue;
          }
        }
      }
    }
  );

  // Subscribe to data changes
  r1AxisA6Node->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectCurrentCoordAR1Node()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();

  // Current R1 KUKA_A node
  QOpcUaNode* r1CurrentKukaANode = this->FindNodeFromFullDisplayName(KUKA_CURRENTCOORDS_R1_A_NODE_NAME);

  // Connect signal handlers for subscribed values
  if (!r1CurrentKukaANode)
  {
    return false;
  }
  QObject::connect(r1CurrentKukaANode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool ok = false;
        int KUKA_A = value.toInt(&ok);
        if (ok)
        {
          int32_t r1Coords[6] = {};
          mrmlNode->GetCurrentCoordsR1(r1Coords);
          r1Coords[3] = KUKA_A;
          {
            MRMLNodeModifyBlocker block(mrmlNode);
            mrmlNode->SetCurrentCoordsR1(r1Coords);
          }
          qDebug() << Q_FUNC_INFO << "Current R1 KUKA_A changed:" << KUKA_A;
        }
      }
    }
  );
  QObject::connect(r1CurrentKukaANode,
    &QOpcUaNode::attributeRead, [r1CurrentKukaANode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!r1CurrentKukaANode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (r1CurrentKukaANode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = r1CurrentKukaANode->attribute(QOpcUa::NodeAttribute::Value);
          bool ok = false;
          int KUKA_A = value.toInt(&ok); // Get the attribute from the cache
          if (ok)
          {
            int32_t r1Coords[6] = {};
            mrmlNode->GetCurrentCoordsR1(r1Coords);
            r1Coords[3] = KUKA_A;
            {
              MRMLNodeModifyBlocker block(mrmlNode);
              mrmlNode->SetCurrentCoordsR1(r1Coords);
            }
            qDebug() << Q_FUNC_INFO << "Current R1 KUKA_A changed:" << KUKA_A;
          }
        }
      }
    }
  );

  // Subscribe to data changes
  r1CurrentKukaANode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectCurrentCoordBR1Node()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();

  // Current R1 KUKA_B node
  QOpcUaNode* r1CurrentKukaBNode = this->FindNodeFromFullDisplayName(KUKA_CURRENTCOORDS_R1_B_NODE_NAME);

  // Connect signal handlers for subscribed values
  if (!r1CurrentKukaBNode)
  {
    return false;
  }
  QObject::connect(r1CurrentKukaBNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool ok = false;
        int KUKA_B = value.toInt(&ok);
        if (ok)
        {
          int32_t r1Coords[6] = {};
          mrmlNode->GetCurrentCoordsR1(r1Coords);
          r1Coords[4] = KUKA_B;
          {
            MRMLNodeModifyBlocker block(mrmlNode);
            mrmlNode->SetCurrentCoordsR1(r1Coords);
          }
          qDebug() << Q_FUNC_INFO << "Current R1 KUKA_B changed:" << KUKA_B;
        }
      }
    }
  );
  QObject::connect(r1CurrentKukaBNode,
    &QOpcUaNode::attributeRead, [r1CurrentKukaBNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!r1CurrentKukaBNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (r1CurrentKukaBNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = r1CurrentKukaBNode->attribute(QOpcUa::NodeAttribute::Value);
          bool ok = false;
          int KUKA_B = value.toInt(&ok); // Get the attribute from the cache
          if (ok)
          {
            int32_t r1Coords[6] = {};
            mrmlNode->GetCurrentCoordsR1(r1Coords);
            r1Coords[4] = KUKA_B;
            {
              MRMLNodeModifyBlocker block(mrmlNode);
              mrmlNode->SetCurrentCoordsR1(r1Coords);
            }
            qDebug() << Q_FUNC_INFO << "Current R1 KUKA_B changed:" << KUKA_B;
          }
        }
      }
    }
  );

  // Subscribe to data changes
  r1CurrentKukaBNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectCurrentCoordCR1Node()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();

  // Current R1 KUKA_C node
  QOpcUaNode* r1CurrentKukaCNode = this->FindNodeFromFullDisplayName(KUKA_CURRENTCOORDS_R1_C_NODE_NAME);

  // Connect signal handlers for subscribed values
  if (!r1CurrentKukaCNode)
  {
    return false;
  }
  QObject::connect(r1CurrentKukaCNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool ok = false;
        int KUKA_C = value.toInt(&ok);
        if (ok)
        {
          int32_t r1Coords[6] = {};
          mrmlNode->GetCurrentCoordsR1(r1Coords);
          r1Coords[5] = KUKA_C;
          {
            MRMLNodeModifyBlocker block(mrmlNode);
            mrmlNode->SetCurrentCoordsR1(r1Coords);
          }
          qDebug() << Q_FUNC_INFO << "Current R1 KUKA_C changed:" << KUKA_C;
        }
      }
    }
  );
  QObject::connect(r1CurrentKukaCNode,
    &QOpcUaNode::attributeRead, [r1CurrentKukaCNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!r1CurrentKukaCNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (r1CurrentKukaCNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = r1CurrentKukaCNode->attribute(QOpcUa::NodeAttribute::Value);
          bool ok = false;
          int KUKA_C = value.toInt(&ok); // Get the attribute from the cache
          if (ok)
          {
            int32_t r1Coords[6] = {};
            mrmlNode->GetCurrentCoordsR1(r1Coords);
            r1Coords[5] = KUKA_C;
            {
              MRMLNodeModifyBlocker block(mrmlNode);
              mrmlNode->SetCurrentCoordsR1(r1Coords);
            }
            qDebug() << Q_FUNC_INFO << "Current R1 KUKA_C changed:" << KUKA_C;
          }
        }
      }
    }
  );

  // Subscribe to data changes
  r1CurrentKukaCNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectCurrentCoordXR1Node()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();

  // Current R1 KUKA_X node
  QOpcUaNode* r1CurrentKukaXNode = this->FindNodeFromFullDisplayName(KUKA_CURRENTCOORDS_R1_X_NODE_NAME);

  // Connect signal handlers for subscribed values
  if (!r1CurrentKukaXNode)
  {
    return false;
  }
  QObject::connect(r1CurrentKukaXNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool ok = false;
        int KUKA_X = value.toInt(&ok);
        if (ok)
        {
          int32_t r1Coords[6] = {};
          mrmlNode->GetCurrentCoordsR1(r1Coords);
          r1Coords[0] = KUKA_X;
          {
            MRMLNodeModifyBlocker block(mrmlNode);
            mrmlNode->SetCurrentCoordsR1(r1Coords);
          }
          qDebug() << Q_FUNC_INFO << "Current R1 KUKA_X changed:" << KUKA_X;
        }
      }
    }
  );
  QObject::connect(r1CurrentKukaXNode,
    &QOpcUaNode::attributeRead, [r1CurrentKukaXNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!r1CurrentKukaXNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (r1CurrentKukaXNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = r1CurrentKukaXNode->attribute(QOpcUa::NodeAttribute::Value);
          bool ok = false;
          int KUKA_X = value.toInt(&ok); // Get the attribute from the cache
          if (ok)
          {
            int32_t r1Coords[6] = {};
            mrmlNode->GetCurrentCoordsR1(r1Coords);
            r1Coords[0] = KUKA_X;
            {
              MRMLNodeModifyBlocker block(mrmlNode);
              mrmlNode->SetCurrentCoordsR1(r1Coords);
            }
            qDebug() << Q_FUNC_INFO << "Current R1 KUKA_X changed:" << KUKA_X;
          }
        }
      }
    }
  );

  // Subscribe to data changes
  r1CurrentKukaXNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectCurrentCoordYR1Node()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();

  // Current R1 KUKA_Y node
  QOpcUaNode* r1CurrentKukaYNode = this->FindNodeFromFullDisplayName(KUKA_CURRENTCOORDS_R1_Y_NODE_NAME);

  // Connect signal handlers for subscribed values
  if (!r1CurrentKukaYNode)
  {
    return false;
  }
  QObject::connect(r1CurrentKukaYNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool ok = false;
        int KUKA_Y = value.toInt(&ok);
        if (ok)
        {
          int32_t r1Coords[6] = {};
          mrmlNode->GetCurrentCoordsR1(r1Coords);
          r1Coords[1] = KUKA_Y;
          {
            MRMLNodeModifyBlocker block(mrmlNode);
            mrmlNode->SetCurrentCoordsR1(r1Coords);
          }
          qDebug() << Q_FUNC_INFO << "Current R1 KUKA_Y changed:" << KUKA_Y;
        }
      }
    }
  );
  QObject::connect(r1CurrentKukaYNode,
    &QOpcUaNode::attributeRead, [r1CurrentKukaYNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!r1CurrentKukaYNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (r1CurrentKukaYNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = r1CurrentKukaYNode->attribute(QOpcUa::NodeAttribute::Value);
          bool ok = false;
          int KUKA_Y = value.toInt(&ok); // Get the attribute from the cache
          if (ok)
          {
            int32_t r1Coords[6] = {};
            mrmlNode->GetCurrentCoordsR1(r1Coords);
            r1Coords[1] = KUKA_Y;
            {
              MRMLNodeModifyBlocker block(mrmlNode);
              mrmlNode->SetCurrentCoordsR1(r1Coords);
            }
            qDebug() << Q_FUNC_INFO << "Current R1 KUKA_Y changed:" << KUKA_Y;
          }
        }
      }
    }
  );

  // Subscribe to data changes
  r1CurrentKukaYNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectCurrentCoordZR1Node()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();

  // Current R1 KUKA_Z node
  QOpcUaNode* r1CurrentKukaZNode = this->FindNodeFromFullDisplayName(KUKA_CURRENTCOORDS_R1_Z_NODE_NAME);

  // Connect signal handlers for subscribed values
  if (!r1CurrentKukaZNode)
  {
    return false;
  }
  QObject::connect(r1CurrentKukaZNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool ok = false;
        int KUKA_Z = value.toInt(&ok);
        if (ok)
        {
          int32_t r1Coords[6] = {};
          mrmlNode->GetCurrentCoordsR1(r1Coords);
          r1Coords[2] = KUKA_Z;
          {
            MRMLNodeModifyBlocker block(mrmlNode);
            mrmlNode->SetCurrentCoordsR1(r1Coords);
          }
          qDebug() << Q_FUNC_INFO << "Current R1 KUKA_Z changed:" << KUKA_Z;
        }
      }
    }
  );
  QObject::connect(r1CurrentKukaZNode,
    &QOpcUaNode::attributeRead, [r1CurrentKukaZNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!r1CurrentKukaZNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (r1CurrentKukaZNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = r1CurrentKukaZNode->attribute(QOpcUa::NodeAttribute::Value);
          bool ok = false;
          int KUKA_Z = value.toInt(&ok); // Get the attribute from the cache
          if (ok)
          {
            int32_t r1Coords[6] = {};
            mrmlNode->GetCurrentCoordsR1(r1Coords);
            r1Coords[2] = KUKA_Z;
            {
              MRMLNodeModifyBlocker block(mrmlNode);
              mrmlNode->SetCurrentCoordsR1(r1Coords);
            }
            qDebug() << Q_FUNC_INFO << "Current R1 KUKA_Z changed:" << KUKA_Z;
          }
        }
      }
    }
  );

  // Subscribe to data changes
  r1CurrentKukaZNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectCurrentCoordAR2Node()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();

  // Current R2 KUKA_A node
  QOpcUaNode* r2CurrentKukaANode = this->FindNodeFromFullDisplayName(KUKA_CURRENTCOORDS_R2_A_NODE_NAME);

  // Connect signal handlers for subscribed values
  if (!r2CurrentKukaANode)
  {
    return false;
  }
  QObject::connect(r2CurrentKukaANode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool ok = false;
        int KUKA_A = value.toInt(&ok);
        if (ok)
        {
          int32_t r2Coords[6] = {};
          mrmlNode->GetCurrentCoordsR2(r2Coords);
          r2Coords[3] = KUKA_A;
          {
            MRMLNodeModifyBlocker block(mrmlNode);
            mrmlNode->SetCurrentCoordsR2(r2Coords);
          }
          qDebug() << Q_FUNC_INFO << "Current R2 KUKA_A changed:" << KUKA_A;
        }
      }
    }
  );
  QObject::connect(r2CurrentKukaANode,
    &QOpcUaNode::attributeRead, [r2CurrentKukaANode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!r2CurrentKukaANode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (r2CurrentKukaANode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = r2CurrentKukaANode->attribute(QOpcUa::NodeAttribute::Value);
          bool ok = false;
          int KUKA_A = value.toInt(&ok); // Get the attribute from the cache
          if (ok)
          {
            int32_t r2Coords[6] = {};
            mrmlNode->GetCurrentCoordsR2(r2Coords);
            r2Coords[3] = KUKA_A;
            {
              MRMLNodeModifyBlocker block(mrmlNode);
              mrmlNode->SetCurrentCoordsR2(r2Coords);
            }
            qDebug() << Q_FUNC_INFO << "Current R2 KUKA_A changed:" << KUKA_A;
          }
        }
      }
    }
  );

  // Subscribe to data changes
  r2CurrentKukaANode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectCurrentCoordBR2Node()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();

  // Current R2 KUKA_B node
  QOpcUaNode* r2CurrentKukaBNode = this->FindNodeFromFullDisplayName(KUKA_CURRENTCOORDS_R2_B_NODE_NAME);

  // Connect signal handlers for subscribed values
  if (!r2CurrentKukaBNode)
  {
    return false;
  }
  QObject::connect(r2CurrentKukaBNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool ok = false;
        int KUKA_B = value.toInt(&ok);
        if (ok)
        {
          int32_t r2Coords[6] = {};
          mrmlNode->GetCurrentCoordsR2(r2Coords);
          r2Coords[4] = KUKA_B;
          {
            MRMLNodeModifyBlocker block(mrmlNode);
            mrmlNode->SetCurrentCoordsR2(r2Coords);
          }
          qDebug() << Q_FUNC_INFO << "Current R2 KUKA_B changed:" << KUKA_B;
        }
      }
    }
  );
  QObject::connect(r2CurrentKukaBNode,
    &QOpcUaNode::attributeRead, [r2CurrentKukaBNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!r2CurrentKukaBNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (r2CurrentKukaBNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = r2CurrentKukaBNode->attribute(QOpcUa::NodeAttribute::Value);
          bool ok = false;
          int KUKA_B = value.toInt(&ok); // Get the attribute from the cache
          if (ok)
          {
            int32_t r2Coords[6] = {};
            mrmlNode->GetCurrentCoordsR2(r2Coords);
            r2Coords[4] = KUKA_B;
            {
              MRMLNodeModifyBlocker block(mrmlNode);
              mrmlNode->SetCurrentCoordsR2(r2Coords);
            }
            qDebug() << Q_FUNC_INFO << "Current R2 KUKA_B changed:" << KUKA_B;
          }
        }
      }
    }
  );

  // Subscribe to data changes
  r2CurrentKukaBNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectCurrentCoordCR2Node()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();

  // Current R2 KUKA_C node
  QOpcUaNode* r2CurrentKukaCNode = this->FindNodeFromFullDisplayName(KUKA_CURRENTCOORDS_R2_C_NODE_NAME);

  // Connect signal handlers for subscribed values
  if (!r2CurrentKukaCNode)
  {
    return false;
  }
  QObject::connect(r2CurrentKukaCNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool ok = false;
        int KUKA_C = value.toInt(&ok);
        if (ok)
        {
          int32_t r2Coords[6] = {};
          mrmlNode->GetCurrentCoordsR2(r2Coords);
          r2Coords[5] = KUKA_C;
          {
            MRMLNodeModifyBlocker block(mrmlNode);
            mrmlNode->SetCurrentCoordsR2(r2Coords);
          }
          qDebug() << Q_FUNC_INFO << "Current R2 KUKA_C changed:" << KUKA_C;
        }
      }
    }
  );
  QObject::connect(r2CurrentKukaCNode,
    &QOpcUaNode::attributeRead, [r2CurrentKukaCNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!r2CurrentKukaCNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (r2CurrentKukaCNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = r2CurrentKukaCNode->attribute(QOpcUa::NodeAttribute::Value);
          bool ok = false;
          int KUKA_C = value.toInt(&ok); // Get the attribute from the cache
          if (ok)
          {
            int32_t r2Coords[6] = {};
            mrmlNode->GetCurrentCoordsR2(r2Coords);
            r2Coords[5] = KUKA_C;
            {
              MRMLNodeModifyBlocker block(mrmlNode);
              mrmlNode->SetCurrentCoordsR2(r2Coords);
            }
            qDebug() << Q_FUNC_INFO << "Current R2 KUKA_C changed:" << KUKA_C;
          }
        }
      }
    }
  );

  // Subscribe to data changes
  r2CurrentKukaCNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectCurrentCoordXR2Node()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();

  // Current R2 KUKA_X node
  QOpcUaNode* r2CurrentKukaXNode = this->FindNodeFromFullDisplayName(KUKA_CURRENTCOORDS_R2_X_NODE_NAME);

  // Connect signal handlers for subscribed values
  if (!r2CurrentKukaXNode)
  {
    return false;
  }
  QObject::connect(r2CurrentKukaXNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool ok = false;
        int KUKA_X = value.toInt(&ok);
        if (ok)
        {
          int32_t r2Coords[6] = {};
          mrmlNode->GetCurrentCoordsR2(r2Coords);
          r2Coords[0] = KUKA_X;
          {
            MRMLNodeModifyBlocker block(mrmlNode);
            mrmlNode->SetCurrentCoordsR2(r2Coords);
          }
          qDebug() << Q_FUNC_INFO << "Current R2 KUKA_X changed:" << KUKA_X;
        }
      }
    }
  );
  QObject::connect(r2CurrentKukaXNode,
    &QOpcUaNode::attributeRead, [r2CurrentKukaXNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!r2CurrentKukaXNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (r2CurrentKukaXNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = r2CurrentKukaXNode->attribute(QOpcUa::NodeAttribute::Value);
          bool ok = false;
          int KUKA_X = value.toInt(&ok); // Get the attribute from the cache
          if (ok)
          {
            int32_t r2Coords[6] = {};
            mrmlNode->GetCurrentCoordsR2(r2Coords);
            r2Coords[0] = KUKA_X;
            {
              MRMLNodeModifyBlocker block(mrmlNode);
              mrmlNode->SetCurrentCoordsR2(r2Coords);
            }
            qDebug() << Q_FUNC_INFO << "Current R2 KUKA_X changed:" << KUKA_X;
          }
        }
      }
    }
  );

  // Subscribe to data changes
  r2CurrentKukaXNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectCurrentCoordYR2Node()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();

  // Current R2 KUKA_Y node
  QOpcUaNode* r2CurrentKukaYNode = this->FindNodeFromFullDisplayName(KUKA_CURRENTCOORDS_R2_Y_NODE_NAME);

  // Connect signal handlers for subscribed values
  if (!r2CurrentKukaYNode)
  {
    return false;
  }
  QObject::connect(r2CurrentKukaYNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool ok = false;
        int KUKA_Y = value.toInt(&ok);
        if (ok)
        {
          int32_t r2Coords[6] = {};
          mrmlNode->GetCurrentCoordsR2(r2Coords);
          r2Coords[1] = KUKA_Y;
          {
            MRMLNodeModifyBlocker block(mrmlNode);
            mrmlNode->SetCurrentCoordsR2(r2Coords);
          }
          qDebug() << Q_FUNC_INFO << "Current R2 KUKA_Y changed:" << KUKA_Y;
        }
      }
    }
  );
  QObject::connect(r2CurrentKukaYNode,
    &QOpcUaNode::attributeRead, [r2CurrentKukaYNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!r2CurrentKukaYNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (r2CurrentKukaYNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = r2CurrentKukaYNode->attribute(QOpcUa::NodeAttribute::Value);
          bool ok = false;
          int KUKA_Y = value.toInt(&ok); // Get the attribute from the cache
          if (ok)
          {
            int32_t r2Coords[6] = {};
            mrmlNode->GetCurrentCoordsR2(r2Coords);
            r2Coords[1] = KUKA_Y;
            {
              MRMLNodeModifyBlocker block(mrmlNode);
              mrmlNode->SetCurrentCoordsR2(r2Coords);
            }
            qDebug() << Q_FUNC_INFO << "Current R2 KUKA_Y changed:" << KUKA_Y;
          }
        }
      }
    }
  );

  // Subscribe to data changes
  r2CurrentKukaYNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectCurrentCoordZR2Node()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();

  // Current R2 KUKA_Z node
  QOpcUaNode* r2CurrentKukaZNode = this->FindNodeFromFullDisplayName(KUKA_CURRENTCOORDS_R2_Z_NODE_NAME);

  // Connect signal handlers for subscribed values
  if (!r2CurrentKukaZNode)
  {
    return false;
  }
  QObject::connect(r2CurrentKukaZNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool ok = false;
        int KUKA_Z = value.toInt(&ok);
        if (ok)
        {
          int32_t r2Coords[6] = {};
          mrmlNode->GetCurrentCoordsR2(r2Coords);
          r2Coords[2] = KUKA_Z;
          {
            MRMLNodeModifyBlocker block(mrmlNode);
            mrmlNode->SetCurrentCoordsR2(r2Coords);
          }
          qDebug() << Q_FUNC_INFO << "Current R2 KUKA_Z changed:" << KUKA_Z;
        }
      }
    }
  );
  QObject::connect(r2CurrentKukaZNode,
    &QOpcUaNode::attributeRead, [r2CurrentKukaZNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!r2CurrentKukaZNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (r2CurrentKukaZNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = r2CurrentKukaZNode->attribute(QOpcUa::NodeAttribute::Value);
          bool ok = false;
          int KUKA_Z = value.toInt(&ok); // Get the attribute from the cache
          if (ok)
          {
            int32_t r2Coords[6] = {};
            mrmlNode->GetCurrentCoordsR2(r2Coords);
            r2Coords[2] = KUKA_Z;
            {
              MRMLNodeModifyBlocker block(mrmlNode);
              mrmlNode->SetCurrentCoordsR2(r2Coords);
            }
            qDebug() << Q_FUNC_INFO << "Current R2 KUKA_Z changed:" << KUKA_Z;
          }
        }
      }
    }
  );

  // Subscribe to data changes
  r2CurrentKukaZNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectCurrentCoordsR1Nodes()
{
  bool res = this->ConnectCurrentCoordA1R1Node();
  res &= this->ConnectCurrentCoordA2R1Node();
  res &= this->ConnectCurrentCoordA3R1Node();
  res &= this->ConnectCurrentCoordA4R1Node();
  res &= this->ConnectCurrentCoordA5R1Node();
  res &= this->ConnectCurrentCoordA6R1Node();
  res &= this->ConnectCurrentCoordXR1Node();
  res &= this->ConnectCurrentCoordYR1Node();
  res &= this->ConnectCurrentCoordZR1Node();
  res &= this->ConnectCurrentCoordAR1Node();
  res &= this->ConnectCurrentCoordBR1Node();
  res &= this->ConnectCurrentCoordCR1Node();

  return res;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectReflTargetCoordsR1Nodes()
{
  bool res = this->ConnectReflTargetCoordAR1Node();
  res &= this->ConnectReflTargetCoordBR1Node();
  res &= this->ConnectReflTargetCoordCR1Node();
  res &= this->ConnectReflTargetCoordXR1Node();
  res &= this->ConnectReflTargetCoordYR1Node();
  res &= this->ConnectReflTargetCoordZR1Node();

  return res;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectReflTargetCoordAR1Node()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();

  // Reflective R1 KUKA_A node
  QOpcUaNode* kukaReflR1ANode = this->FindNodeFromFullDisplayName(KUKA_REFLTARGETCOORDS_R1_A_NODE_NAME);

  // Connect signal handlers for subscribed values
  if (!kukaReflR1ANode)
  {
    return false;
  }
  QObject::connect(kukaReflR1ANode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool ok = false;
        int KUKA_A = value.toInt(&ok);
        if (ok)
        {
          int32_t r1Coords[6] = {};
          mrmlNode->GetConfirmedCoordsR1(r1Coords);
          r1Coords[3] = KUKA_A;
          {
            MRMLNodeModifyBlocker block(mrmlNode);
            mrmlNode->SetConfirmedCoordsR1(r1Coords);
          }
          qDebug() << Q_FUNC_INFO << "Reflective R1 KUKA_A changed:" << KUKA_A;
        }
      }
    }
  );
  QObject::connect(kukaReflR1ANode,
    &QOpcUaNode::attributeRead, [kukaReflR1ANode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!kukaReflR1ANode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (kukaReflR1ANode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = kukaReflR1ANode->attribute(QOpcUa::NodeAttribute::Value);
          bool ok = false;
          int KUKA_A = value.toInt(&ok); // Get the attribute from the cache
          if (ok)
          {
            int32_t r1Coords[6] = {};
            mrmlNode->GetConfirmedCoordsR1(r1Coords);
            r1Coords[3] = KUKA_A;
            {
              MRMLNodeModifyBlocker block(mrmlNode);
              mrmlNode->SetConfirmedCoordsR1(r1Coords);
            }
            qDebug() << Q_FUNC_INFO << "Reflective R1 KUKA_A changed:" << KUKA_A;
          }
        }
      }
    }
  );

  // Subscribe to data changes
  kukaReflR1ANode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectReflTargetCoordBR1Node()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();

  // Reflective R1 KUKA_B node
  QOpcUaNode* kukaReflR1BNode = this->FindNodeFromFullDisplayName(KUKA_REFLTARGETCOORDS_R1_B_NODE_NAME);

  // Connect signal handlers for subscribed values
  if (!kukaReflR1BNode)
  {
    return false;
  }
  QObject::connect(kukaReflR1BNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool ok = false;
        int KUKA_B = value.toInt(&ok);
        if (ok)
        {
          int32_t r1Coords[6] = {};
          mrmlNode->GetConfirmedCoordsR1(r1Coords);
          r1Coords[4] = KUKA_B;
          {
            MRMLNodeModifyBlocker block(mrmlNode);
            mrmlNode->SetConfirmedCoordsR1(r1Coords);
          }
          qDebug() << Q_FUNC_INFO << "Reflective R1 KUKA_B changed:" << KUKA_B;
        }
      }
    }
  );
  QObject::connect(kukaReflR1BNode,
    &QOpcUaNode::attributeRead, [kukaReflR1BNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!kukaReflR1BNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (kukaReflR1BNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = kukaReflR1BNode->attribute(QOpcUa::NodeAttribute::Value);
          bool ok = false;
          int KUKA_B = value.toInt(&ok); // Get the attribute from the cache
          if (ok)
          {
            int32_t r1Coords[6] = {};
            mrmlNode->GetConfirmedCoordsR1(r1Coords);
            r1Coords[4] = KUKA_B;
            {
              MRMLNodeModifyBlocker block(mrmlNode);
              mrmlNode->SetConfirmedCoordsR1(r1Coords);
            }
            qDebug() << Q_FUNC_INFO << "Reflective R1 KUKA_B changed:" << KUKA_B;
          }
        }
      }
    }
  );

  // Subscribe to data changes
  kukaReflR1BNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectReflTargetCoordCR1Node()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();

  // Reflective R1 KUKA_C node
  QOpcUaNode* kukaReflR1CNode = this->FindNodeFromFullDisplayName(KUKA_REFLTARGETCOORDS_R1_C_NODE_NAME);

  // Connect signal handlers for subscribed values
  if (!kukaReflR1CNode)
  {
    return false;
  }
  QObject::connect(kukaReflR1CNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool ok = false;
        int KUKA_C = value.toInt(&ok);
        if (ok)
        {
          int32_t r1Coords[6] = {};
          mrmlNode->GetConfirmedCoordsR1(r1Coords);
          r1Coords[5] = KUKA_C;
          {
            MRMLNodeModifyBlocker block(mrmlNode);
            mrmlNode->SetConfirmedCoordsR1(r1Coords);
          }
          qDebug() << Q_FUNC_INFO << "Reflective R1 KUKA_C changed:" << KUKA_C;
        }
      }
    }
  );
  QObject::connect(kukaReflR1CNode,
    &QOpcUaNode::attributeRead, [kukaReflR1CNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!kukaReflR1CNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (kukaReflR1CNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = kukaReflR1CNode->attribute(QOpcUa::NodeAttribute::Value);
          bool ok = false;
          int KUKA_C = value.toInt(&ok); // Get the attribute from the cache
          if (ok)
          {
            int32_t r1Coords[6] = {};
            mrmlNode->GetConfirmedCoordsR1(r1Coords);
            r1Coords[5] = KUKA_C;
            {
              MRMLNodeModifyBlocker block(mrmlNode);
              mrmlNode->SetConfirmedCoordsR1(r1Coords);
            }
            qDebug() << Q_FUNC_INFO << "Reflective R1 KUKA_C changed:" << KUKA_C;
          }
        }
      }
    }
  );

  // Subscribe to data changes
  kukaReflR1CNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectReflTargetCoordXR1Node()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();

  // Reflective R1 KUKA_X node
  QOpcUaNode* kukaReflR1XNode = this->FindNodeFromFullDisplayName(KUKA_REFLTARGETCOORDS_R1_X_NODE_NAME);

  // Connect signal handlers for subscribed values
  if (!kukaReflR1XNode)
  {
    return false;
  }
  QObject::connect(kukaReflR1XNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool ok = false;
        int KUKA_X = value.toInt(&ok);
        if (ok)
        {
          int32_t r1Coords[6] = {};
          mrmlNode->GetConfirmedCoordsR1(r1Coords);
          r1Coords[0] = KUKA_X;
          {
            MRMLNodeModifyBlocker block(mrmlNode);
            mrmlNode->SetConfirmedCoordsR1(r1Coords);
          }
          qDebug() << Q_FUNC_INFO << "Reflective R1 KUKA_X changed:" << KUKA_X;
        }
      }
    }
  );
  QObject::connect(kukaReflR1XNode,
    &QOpcUaNode::attributeRead, [kukaReflR1XNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!kukaReflR1XNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (kukaReflR1XNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = kukaReflR1XNode->attribute(QOpcUa::NodeAttribute::Value);
          bool ok = false;
          int KUKA_X = value.toInt(&ok); // Get the attribute from the cache
          if (ok)
          {
            int32_t r1Coords[6] = {};
            mrmlNode->GetConfirmedCoordsR1(r1Coords);
            r1Coords[0] = KUKA_X;
            {
              MRMLNodeModifyBlocker block(mrmlNode);
              mrmlNode->SetConfirmedCoordsR1(r1Coords);
            }
            qDebug() << Q_FUNC_INFO << "Reflective R1 KUKA_X changed:" << KUKA_X;
          }
        }
      }
    }
  );

  // Subscribe to data changes
  kukaReflR1XNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectReflTargetCoordYR1Node()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();

  // Reflective R1 KUKA_Y node
  QOpcUaNode* kukaReflR1YNode = this->FindNodeFromFullDisplayName(KUKA_REFLTARGETCOORDS_R1_Y_NODE_NAME);

  // Connect signal handlers for subscribed values
  if (!kukaReflR1YNode)
  {
    return false;
  }
  QObject::connect(kukaReflR1YNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool ok = false;
        int KUKA_Y = value.toInt(&ok);
        if (ok)
        {
          int32_t r1Coords[6] = {};
          mrmlNode->GetConfirmedCoordsR1(r1Coords);
          r1Coords[1] = KUKA_Y;
          {
            MRMLNodeModifyBlocker block(mrmlNode);
            mrmlNode->SetConfirmedCoordsR1(r1Coords);
          }
          qDebug() << Q_FUNC_INFO << "Reflective R1 KUKA_Y changed:" << KUKA_Y;
        }
      }
    }
  );
  QObject::connect(kukaReflR1YNode,
    &QOpcUaNode::attributeRead, [kukaReflR1YNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!kukaReflR1YNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (kukaReflR1YNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = kukaReflR1YNode->attribute(QOpcUa::NodeAttribute::Value);
          bool ok = false;
          int KUKA_Y = value.toInt(&ok); // Get the attribute from the cache
          if (ok)
          {
            int32_t r1Coords[6] = {};
            mrmlNode->GetConfirmedCoordsR1(r1Coords);
            r1Coords[1] = KUKA_Y;
            {
              MRMLNodeModifyBlocker block(mrmlNode);
              mrmlNode->SetConfirmedCoordsR1(r1Coords);
            }
            qDebug() << Q_FUNC_INFO << "Reflective R1 KUKA_Y changed:" << KUKA_Y;
          }
        }
      }
    }
  );

  // Subscribe to data changes
  kukaReflR1YNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectReflTargetCoordZR1Node()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();

  // Reflective R1 KUKA_Z node
  QOpcUaNode* kukaReflR1ZNode = this->FindNodeFromFullDisplayName(KUKA_REFLTARGETCOORDS_R1_Z_NODE_NAME);

  // Connect signal handlers for subscribed values
  if (!kukaReflR1ZNode)
  {
    return false;
  }
  QObject::connect(kukaReflR1ZNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool ok = false;
        int KUKA_Z = value.toInt(&ok);
        if (ok)
        {
          int32_t r1Coords[6] = {};
          mrmlNode->GetConfirmedCoordsR1(r1Coords);
          r1Coords[2] = KUKA_Z;
          {
            MRMLNodeModifyBlocker block(mrmlNode);
            mrmlNode->SetConfirmedCoordsR1(r1Coords);
          }
          qDebug() << Q_FUNC_INFO << "Reflective R1 KUKA_Z changed:" << KUKA_Z;
        }
      }
    }
  );
  QObject::connect(kukaReflR1ZNode,
    &QOpcUaNode::attributeRead, [kukaReflR1ZNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!kukaReflR1ZNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (kukaReflR1ZNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = kukaReflR1ZNode->attribute(QOpcUa::NodeAttribute::Value);
          bool ok = false;
          int KUKA_Z = value.toInt(&ok); // Get the attribute from the cache
          if (ok)
          {
            int32_t r1Coords[6] = {};
            mrmlNode->GetConfirmedCoordsR1(r1Coords);
            r1Coords[2] = KUKA_Z;
            {
              MRMLNodeModifyBlocker block(mrmlNode);
              mrmlNode->SetConfirmedCoordsR1(r1Coords);
            }
            qDebug() << Q_FUNC_INFO << "Reflective R1 KUKA_Z changed:" << KUKA_Z;
          }
        }
      }
    }
  );

  // Subscribe to data changes
  kukaReflR1ZNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectReflTargetCoordXR2Node()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();

  // Reflective R2 KUKA_X node
  QOpcUaNode* kukaReflR2XNode = this->FindNodeFromFullDisplayName(KUKA_REFLTARGETCOORDS_R2_X_NODE_NAME);

  // Connect signal handlers for subscribed values
  if (!kukaReflR2XNode)
  {
    return false;
  }
  QObject::connect(kukaReflR2XNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool ok = false;
        int KUKA_X = value.toInt(&ok);
        if (ok)
        {
          int32_t r2Coords[6] = {};
          mrmlNode->GetConfirmedCoordsR2(r2Coords);
          r2Coords[0] = KUKA_X;
          {
            MRMLNodeModifyBlocker block(mrmlNode);
            mrmlNode->SetConfirmedCoordsR2(r2Coords);
          }
          qDebug() << Q_FUNC_INFO << "Reflective R2 KUKA_X changed:" << KUKA_X;
        }
      }
    }
  );
  QObject::connect(kukaReflR2XNode,
    &QOpcUaNode::attributeRead, [kukaReflR2XNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!kukaReflR2XNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (kukaReflR2XNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = kukaReflR2XNode->attribute(QOpcUa::NodeAttribute::Value);
          bool ok = false;
          int KUKA_X = value.toInt(&ok); // Get the attribute from the cache
          if (ok)
          {
            int32_t r2Coords[6] = {};
            mrmlNode->GetConfirmedCoordsR2(r2Coords);
            r2Coords[0] = KUKA_X;
            {
              MRMLNodeModifyBlocker block(mrmlNode);
              mrmlNode->SetConfirmedCoordsR2(r2Coords);
            }
            qDebug() << Q_FUNC_INFO << "Reflective R2 KUKA_X changed:" << KUKA_X;
          }
        }
      }
    }
  );

  // Subscribe to data changes
  kukaReflR2XNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectReflTargetCoordYR2Node()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();

  // Reflective R2 KUKA_Y node
  QOpcUaNode* kukaReflR2YNode = this->FindNodeFromFullDisplayName(KUKA_REFLTARGETCOORDS_R2_Y_NODE_NAME);

  // Connect signal handlers for subscribed values
  if (!kukaReflR2YNode)
  {
    return false;
  }
  QObject::connect(kukaReflR2YNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool ok = false;
        int KUKA_Y = value.toInt(&ok);
        if (ok)
        {
          int32_t r2Coords[6] = {};
          mrmlNode->GetConfirmedCoordsR2(r2Coords);
          r2Coords[1] = KUKA_Y;
          {
            MRMLNodeModifyBlocker block(mrmlNode);
            mrmlNode->SetConfirmedCoordsR2(r2Coords);
          }
          qDebug() << Q_FUNC_INFO << "Reflective R2 KUKA_Y changed:" << KUKA_Y;
        }
      }
    }
  );
  QObject::connect(kukaReflR2YNode,
    &QOpcUaNode::attributeRead, [kukaReflR2YNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!kukaReflR2YNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (kukaReflR2YNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = kukaReflR2YNode->attribute(QOpcUa::NodeAttribute::Value);
          bool ok = false;
          int KUKA_Y = value.toInt(&ok); // Get the attribute from the cache
          if (ok)
          {
            int32_t r2Coords[6] = {};
            mrmlNode->GetConfirmedCoordsR2(r2Coords);
            r2Coords[1] = KUKA_Y;
            {
              MRMLNodeModifyBlocker block(mrmlNode);
              mrmlNode->SetConfirmedCoordsR2(r2Coords);
            }
            qDebug() << Q_FUNC_INFO << "Reflective R2 KUKA_Y changed:" << KUKA_Y;
          }
        }
      }
    }
  );

  // Subscribe to data changes
  kukaReflR2YNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectReflTargetCoordZR2Node()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();

  // Reflective R2 KUKA_Z node
  QOpcUaNode* kukaReflR2ZNode = this->FindNodeFromFullDisplayName(KUKA_REFLTARGETCOORDS_R2_Z_NODE_NAME);

  // Connect signal handlers for subscribed values
  if (!kukaReflR2ZNode)
  {
    return false;
  }
  QObject::connect(kukaReflR2ZNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool ok = false;
        int KUKA_Z = value.toInt(&ok);
        if (ok)
        {
          int32_t r2Coords[6] = {};
          mrmlNode->GetConfirmedCoordsR2(r2Coords);
          r2Coords[2] = KUKA_Z;
          {
            MRMLNodeModifyBlocker block(mrmlNode);
            mrmlNode->SetConfirmedCoordsR2(r2Coords);
          }
          qDebug() << Q_FUNC_INFO << "Reflective R2 KUKA_Z changed:" << KUKA_Z;
        }
      }
    }
  );
  QObject::connect(kukaReflR2ZNode,
    &QOpcUaNode::attributeRead, [kukaReflR2ZNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!kukaReflR2ZNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (kukaReflR2ZNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = kukaReflR2ZNode->attribute(QOpcUa::NodeAttribute::Value);
          bool ok = false;
          int KUKA_Z = value.toInt(&ok); // Get the attribute from the cache
          if (ok)
          {
            int32_t r2Coords[6] = {};
            mrmlNode->GetConfirmedCoordsR2(r2Coords);
            r2Coords[2] = KUKA_Z;
            {
              MRMLNodeModifyBlocker block(mrmlNode);
              mrmlNode->SetConfirmedCoordsR2(r2Coords);
            }
            qDebug() << Q_FUNC_INFO << "Reflective R2 KUKA_Z changed:" << KUKA_Z;
          }
        }
      }
    }
  );

  // Subscribe to data changes
  kukaReflR2ZNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  return true;
}


//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectReflTargetCoordAR2Node()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();

  // Reflective R2 KUKA_A node
  QOpcUaNode* kukaReflR2ANode = this->FindNodeFromFullDisplayName(KUKA_REFLTARGETCOORDS_R2_A_NODE_NAME);

  // Connect signal handlers for subscribed values
  if (!kukaReflR2ANode)
  {
    return false;
  }
  QObject::connect(kukaReflR2ANode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool ok = false;
        int KUKA_A = value.toInt(&ok);
        if (ok)
        {
          int32_t r2Coords[6] = {};
          mrmlNode->GetConfirmedCoordsR2(r2Coords);
          r2Coords[3] = KUKA_A;
          {
            MRMLNodeModifyBlocker block(mrmlNode);
            mrmlNode->SetConfirmedCoordsR2(r2Coords);
          }
          qDebug() << Q_FUNC_INFO << "Reflective R2 KUKA_A changed:" << KUKA_A;
        }
      }
    }
  );
  QObject::connect(kukaReflR2ANode,
    &QOpcUaNode::attributeRead, [kukaReflR2ANode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!kukaReflR2ANode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (kukaReflR2ANode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = kukaReflR2ANode->attribute(QOpcUa::NodeAttribute::Value);
          bool ok = false;
          int KUKA_A = value.toInt(&ok); // Get the attribute from the cache
          if (ok)
          {
            int32_t r2Coords[6] = {};
            mrmlNode->GetConfirmedCoordsR2(r2Coords);
            r2Coords[3] = KUKA_A;
            {
              MRMLNodeModifyBlocker block(mrmlNode);
              mrmlNode->SetConfirmedCoordsR2(r2Coords);
            }
            qDebug() << Q_FUNC_INFO << "Reflective R2 KUKA_A read:" << KUKA_A;
          }
        }
      }
    }
  );

  // Subscribe to data changes
  kukaReflR2ANode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectReflTargetCoordBR2Node()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();

  // Reflective R2 KUKA_B node
  QOpcUaNode* kukaReflR2BNode = this->FindNodeFromFullDisplayName(KUKA_REFLTARGETCOORDS_R2_B_NODE_NAME);

  // Connect signal handlers for subscribed values
  if (!kukaReflR2BNode)
  {
    return false;
  }
  QObject::connect(kukaReflR2BNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool ok = false;
        int KUKA_B = value.toInt(&ok);
        if (ok)
        {
          int32_t r2Coords[6] = {};
          mrmlNode->GetConfirmedCoordsR2(r2Coords);
          r2Coords[4] = KUKA_B;
          {
            MRMLNodeModifyBlocker block(mrmlNode);
            mrmlNode->SetConfirmedCoordsR2(r2Coords);
          }
          qDebug() << Q_FUNC_INFO << "Reflective R2 KUKA_B changed:" << KUKA_B;
        }
      }
    }
  );
  QObject::connect(kukaReflR2BNode,
    &QOpcUaNode::attributeRead, [kukaReflR2BNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!kukaReflR2BNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (kukaReflR2BNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = kukaReflR2BNode->attribute(QOpcUa::NodeAttribute::Value);
          bool ok = false;
          int KUKA_B = value.toInt(&ok); // Get the attribute from the cache
          if (ok)
          {
            int32_t r2Coords[6] = {};
            mrmlNode->GetConfirmedCoordsR2(r2Coords);
            r2Coords[4] = KUKA_B;
            {
              MRMLNodeModifyBlocker block(mrmlNode);
              mrmlNode->SetConfirmedCoordsR2(r2Coords);
            }
            qDebug() << Q_FUNC_INFO << "Reflective R2 KUKA_B changed:" << KUKA_B;
          }
        }
      }
    }
  );

  // Subscribe to data changes
  kukaReflR2BNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectReflTargetCoordCR2Node()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();

  // Reflective R2 KUKA_C node
  QOpcUaNode* kukaReflR2CNode = this->FindNodeFromFullDisplayName(KUKA_REFLTARGETCOORDS_R2_C_NODE_NAME);

  // Connect signal handlers for subscribed values
  if (!kukaReflR2CNode)
  {
    return false;
  }
  QObject::connect(kukaReflR2CNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool ok = false;
        int KUKA_C = value.toInt(&ok);
        if (ok)
        {
          int32_t r2Coords[6] = {};
          mrmlNode->GetConfirmedCoordsR2(r2Coords);
          r2Coords[5] = KUKA_C;
          {
            MRMLNodeModifyBlocker block(mrmlNode);
            mrmlNode->SetConfirmedCoordsR2(r2Coords);
          }
          qDebug() << Q_FUNC_INFO << "Reflective R2 KUKA_C changed:" << KUKA_C;
        }
      }
    }
  );
  QObject::connect(kukaReflR2CNode,
    &QOpcUaNode::attributeRead, [kukaReflR2CNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!kukaReflR2CNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (kukaReflR2CNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = kukaReflR2CNode->attribute(QOpcUa::NodeAttribute::Value);
          bool ok = false;
          int KUKA_C = value.toInt(&ok); // Get the attribute from the cache
          if (ok)
          {
            int32_t r2Coords[6] = {};
            mrmlNode->GetConfirmedCoordsR2(r2Coords);
            r2Coords[5] = KUKA_C;
            {
              MRMLNodeModifyBlocker block(mrmlNode);
              mrmlNode->SetConfirmedCoordsR2(r2Coords);
            }
            qDebug() << Q_FUNC_INFO << "Reflective R2 KUKA_C changed:" << KUKA_C;
          }
        }
      }
    }
  );

  // Subscribe to data changes
  kukaReflR2CNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectCurrentCoordA1R2Node()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();

  // KUKA R2 Axis A1 node
  QOpcUaNode* r2AxisA1Node = this->FindNodeFromFullDisplayName(KUKA_CURRENTCOORDS_R2_A1_NODE_NAME);

  // Connect signal handlers for subscribed values
  if (!r2AxisA1Node)
  {
    return false;
  }
  QObject::connect(r2AxisA1Node,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool ok = false;
        int angleValue = value.toInt(&ok);
        if (ok)
        {
          int32_t r2AxisCoords[6] = {};
          mrmlNode->GetCurrentAxisCoordsR2(r2AxisCoords);
          r2AxisCoords[0] = angleValue;
          {
            MRMLNodeModifyBlocker block(mrmlNode);
            mrmlNode->SetCurrentAxisCoordsR2(r2AxisCoords);
          }
          mrmlNode->InvokeCustomModifiedEvent(vtkMRMLSiemensPlcOpcUaNode::CarmRobotAxisCoordsModified);
          qDebug() << Q_FUNC_INFO << "R2 Axis A1 angle changed:" << angleValue;
        }
      }
    }
  );
  QObject::connect(r2AxisA1Node,
    &QOpcUaNode::attributeRead, [r2AxisA1Node, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!r2AxisA1Node || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (r2AxisA1Node->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = r2AxisA1Node->attribute(QOpcUa::NodeAttribute::Value);
          bool ok = false;
          int angleValue = value.toInt(&ok); // Get the attribute from the cache
          if (ok)
          {
            int32_t r2AxisCoords[6] = {};
            mrmlNode->GetCurrentAxisCoordsR2(r2AxisCoords);
            r2AxisCoords[0] = angleValue;
            {
              MRMLNodeModifyBlocker block(mrmlNode);
              mrmlNode->SetCurrentAxisCoordsR2(r2AxisCoords);
            }
            mrmlNode->InvokeCustomModifiedEvent(vtkMRMLSiemensPlcOpcUaNode::CarmRobotAxisCoordsModified);
            qDebug() << Q_FUNC_INFO << "R2 Axis A1 angle read:" << angleValue;
          }
        }
      }
    }
  );
  // Subscribe to data changes
  r2AxisA1Node->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectCurrentCoordA2R2Node()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();

  // KUKA R2 Axis A2 node
  QOpcUaNode* r2AxisA2Node = this->FindNodeFromFullDisplayName(KUKA_CURRENTCOORDS_R2_A2_NODE_NAME);

  // Connect signal handlers for subscribed values
  if (!r2AxisA2Node)
  {
    return false;
  }
  QObject::connect(r2AxisA2Node,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool ok = false;
        int angleValue = value.toInt(&ok);
        if (ok)
        {
          int32_t r2AxisCoords[6] = {};
          mrmlNode->GetCurrentAxisCoordsR2(r2AxisCoords);
          r2AxisCoords[1] = angleValue;
          {
            MRMLNodeModifyBlocker block(mrmlNode);
            mrmlNode->SetCurrentAxisCoordsR2(r2AxisCoords);
          }
          mrmlNode->InvokeCustomModifiedEvent(vtkMRMLSiemensPlcOpcUaNode::CarmRobotAxisCoordsModified);
          qDebug() << Q_FUNC_INFO << "R2 Axis A2 angle changed:" << angleValue;
        }
      }
    }
  );
  QObject::connect(r2AxisA2Node,
    &QOpcUaNode::attributeRead, [r2AxisA2Node, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!r2AxisA2Node || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (r2AxisA2Node->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = r2AxisA2Node->attribute(QOpcUa::NodeAttribute::Value);
          bool ok = false;
          int angleValue = value.toInt(&ok); // Get the attribute from the cache
          if (ok)
          {
            int32_t r2AxisCoords[6] = {};
            mrmlNode->GetCurrentAxisCoordsR2(r2AxisCoords);
            r2AxisCoords[1] = angleValue;
            {
              MRMLNodeModifyBlocker block(mrmlNode);
              mrmlNode->SetCurrentAxisCoordsR2(r2AxisCoords);
            }
            mrmlNode->InvokeCustomModifiedEvent(vtkMRMLSiemensPlcOpcUaNode::CarmRobotAxisCoordsModified);
            qDebug() << Q_FUNC_INFO << "R2 Axis A2 angle read:" << angleValue;
          }
        }
      }
    }
  );
  // Subscribe to data changes
  r2AxisA2Node->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectCurrentCoordA3R2Node()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();

  // KUKA R2 Axis A3 node
  QOpcUaNode* r2AxisA3Node = this->FindNodeFromFullDisplayName(KUKA_CURRENTCOORDS_R2_A3_NODE_NAME);

  // Connect signal handlers for subscribed values
  if (!r2AxisA3Node)
  {
    return false;
  }
  QObject::connect(r2AxisA3Node,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool ok = false;
        int angleValue = value.toInt(&ok);
        if (ok)
        {
          int32_t r2AxisCoords[6] = {};
          mrmlNode->GetCurrentAxisCoordsR2(r2AxisCoords);
          r2AxisCoords[2] = angleValue;
          {
            MRMLNodeModifyBlocker block(mrmlNode);
            mrmlNode->SetCurrentAxisCoordsR2(r2AxisCoords);
          }
          mrmlNode->InvokeCustomModifiedEvent(vtkMRMLSiemensPlcOpcUaNode::CarmRobotAxisCoordsModified);
          qDebug() << Q_FUNC_INFO << "R2 Axis A3 angle changed:" << angleValue;
        }
      }
    }
  );
  QObject::connect(r2AxisA3Node,
    &QOpcUaNode::attributeRead, [r2AxisA3Node, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!r2AxisA3Node || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (r2AxisA3Node->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = r2AxisA3Node->attribute(QOpcUa::NodeAttribute::Value);
          bool ok = false;
          int angleValue = value.toInt(&ok); // Get the attribute from the cache
          if (ok)
          {
            int32_t r2AxisCoords[6] = {};
            mrmlNode->GetCurrentAxisCoordsR2(r2AxisCoords);
            r2AxisCoords[2] = angleValue;
            {
              MRMLNodeModifyBlocker block(mrmlNode);
              mrmlNode->SetCurrentAxisCoordsR2(r2AxisCoords);
            }
            mrmlNode->InvokeCustomModifiedEvent(vtkMRMLSiemensPlcOpcUaNode::CarmRobotAxisCoordsModified);
            qDebug() << Q_FUNC_INFO << "R2 Axis A3 angle read:" << angleValue;
          }
        }
      }
    }
  );
  // Subscribe to data changes
  r2AxisA3Node->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectCurrentCoordA4R2Node()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();

  // KUKA R2 Axis A4 node
  QOpcUaNode* r2AxisA4Node = this->FindNodeFromFullDisplayName(KUKA_CURRENTCOORDS_R2_A4_NODE_NAME);

  // Connect signal handlers for subscribed values
  if (!r2AxisA4Node)
  {
    return false;
  }
  QObject::connect(r2AxisA4Node,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool ok = false;
        int angleValue = value.toInt(&ok);
        if (ok)
        {
          int32_t r2AxisCoords[6] = {};
          mrmlNode->GetCurrentAxisCoordsR2(r2AxisCoords);
          r2AxisCoords[3] = angleValue;
          {
            MRMLNodeModifyBlocker block(mrmlNode);
            mrmlNode->SetCurrentAxisCoordsR2(r2AxisCoords);
          }
          mrmlNode->InvokeCustomModifiedEvent(vtkMRMLSiemensPlcOpcUaNode::CarmRobotAxisCoordsModified);
          qDebug() << Q_FUNC_INFO << "R2 Axis A4 angle changed:" << angleValue;
        }
      }
    }
  );
  QObject::connect(r2AxisA4Node,
    &QOpcUaNode::attributeRead, [r2AxisA4Node, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!r2AxisA4Node || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (r2AxisA4Node->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = r2AxisA4Node->attribute(QOpcUa::NodeAttribute::Value);
          bool ok = false;
          int angleValue = value.toInt(&ok); // Get the attribute from the cache
          if (ok)
          {
            int32_t r2AxisCoords[6] = {};
            mrmlNode->GetCurrentAxisCoordsR2(r2AxisCoords);
            r2AxisCoords[3] = angleValue;
            {
              MRMLNodeModifyBlocker block(mrmlNode);
              mrmlNode->SetCurrentAxisCoordsR2(r2AxisCoords);
            }
            mrmlNode->InvokeCustomModifiedEvent(vtkMRMLSiemensPlcOpcUaNode::CarmRobotAxisCoordsModified);
            qDebug() << Q_FUNC_INFO << "R2 Axis A4 angle read:" << angleValue;
          }
        }
      }
    }
  );
  // Subscribe to data changes
  r2AxisA4Node->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectCurrentCoordA5R2Node()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();

  // KUKA R2 Axis A5 node
  QOpcUaNode* r2AxisA5Node = this->FindNodeFromFullDisplayName(KUKA_CURRENTCOORDS_R2_A5_NODE_NAME);

  // Connect signal handlers for subscribed values
  if (!r2AxisA5Node)
  {
    return false;
  }
  QObject::connect(r2AxisA5Node,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool ok = false;
        int angleValue = value.toInt(&ok);
        if (ok)
        {
          int32_t r2AxisCoords[6] = {};
          mrmlNode->GetCurrentAxisCoordsR2(r2AxisCoords);
          r2AxisCoords[4] = angleValue;
          {
            MRMLNodeModifyBlocker block(mrmlNode);
            mrmlNode->SetCurrentAxisCoordsR2(r2AxisCoords);
          }
          mrmlNode->InvokeCustomModifiedEvent(vtkMRMLSiemensPlcOpcUaNode::CarmRobotAxisCoordsModified);
          qDebug() << Q_FUNC_INFO << "R2 Axis A5 angle changed:" << angleValue;
        }
      }
    }
  );
  QObject::connect(r2AxisA5Node,
    &QOpcUaNode::attributeRead, [r2AxisA5Node, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!r2AxisA5Node || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (r2AxisA5Node->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = r2AxisA5Node->attribute(QOpcUa::NodeAttribute::Value);
          bool ok = false;
          int angleValue = value.toInt(&ok); // Get the attribute from the cache
          if (ok)
          {
            int32_t r2AxisCoords[6] = {};
            mrmlNode->GetCurrentAxisCoordsR2(r2AxisCoords);
            r2AxisCoords[4] = angleValue;
            {
              MRMLNodeModifyBlocker block(mrmlNode);
              mrmlNode->SetCurrentAxisCoordsR2(r2AxisCoords);
            }
            mrmlNode->InvokeCustomModifiedEvent(vtkMRMLSiemensPlcOpcUaNode::CarmRobotAxisCoordsModified);
            qDebug() << Q_FUNC_INFO << "R2 Axis A5 angle read:" << angleValue;
          }
        }
      }
    }
  );
  // Subscribe to data changes
  r2AxisA5Node->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectCurrentCoordA6R2Node()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();

  // KUKA R2 Axis A6 node
  QOpcUaNode* r2AxisA6Node = this->FindNodeFromFullDisplayName(KUKA_CURRENTCOORDS_R2_A6_NODE_NAME);

  // Connect signal handlers for subscribed values
  if (!r2AxisA6Node)
  {
    return false;
  }
  QObject::connect(r2AxisA6Node,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool ok = false;
        int angleValue = value.toInt(&ok);
        if (ok)
        {
          int32_t r2AxisCoords[6] = {};
          mrmlNode->GetCurrentAxisCoordsR2(r2AxisCoords);
          r2AxisCoords[5] = angleValue;
          {
            MRMLNodeModifyBlocker block(mrmlNode);
            mrmlNode->SetCurrentAxisCoordsR2(r2AxisCoords);
          }
          mrmlNode->InvokeCustomModifiedEvent(vtkMRMLSiemensPlcOpcUaNode::CarmRobotAxisCoordsModified);
          qDebug() << Q_FUNC_INFO << "R2 Axis A6 angle changed:" << angleValue;
        }
      }
    }
  );
  QObject::connect(r2AxisA6Node,
    &QOpcUaNode::attributeRead, [r2AxisA6Node, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!r2AxisA6Node || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (r2AxisA6Node->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = r2AxisA6Node->attribute(QOpcUa::NodeAttribute::Value);
          bool ok = false;
          int angleValue = value.toInt(&ok); // Get the attribute from the cache
          if (ok)
          {
            int32_t r2AxisCoords[6] = {};
            mrmlNode->GetCurrentAxisCoordsR2(r2AxisCoords);
            r2AxisCoords[5] = angleValue;
            {
              MRMLNodeModifyBlocker block(mrmlNode);
              mrmlNode->SetCurrentAxisCoordsR2(r2AxisCoords);
            }
            mrmlNode->InvokeCustomModifiedEvent(vtkMRMLSiemensPlcOpcUaNode::CarmRobotAxisCoordsModified);
            qDebug() << Q_FUNC_INFO << "R1 Axis A6 angle read:" << angleValue;
          }
        }
      }
    }
  );
  // Subscribe to data changes
  r2AxisA6Node->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectCurrentCoordsR2Nodes()
{
  bool res = this->ConnectCurrentCoordA1R2Node();
  res &= this->ConnectCurrentCoordA2R2Node();
  res &= this->ConnectCurrentCoordA3R2Node();
  res &= this->ConnectCurrentCoordA4R2Node();
  res &= this->ConnectCurrentCoordA5R2Node();
  res &= this->ConnectCurrentCoordA6R2Node();
  res &= this->ConnectCurrentCoordXR2Node();
  res &= this->ConnectCurrentCoordYR2Node();
  res &= this->ConnectCurrentCoordZR2Node();
  res &= this->ConnectCurrentCoordAR2Node();
  res &= this->ConnectCurrentCoordBR2Node();
  res &= this->ConnectCurrentCoordCR2Node();
  return res;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectReflTargetCoordsR2Nodes()
{
  bool res = this->ConnectReflTargetCoordAR2Node();
  res &= this->ConnectReflTargetCoordBR2Node();
  res &= this->ConnectReflTargetCoordCR2Node();
  res &= this->ConnectReflTargetCoordXR2Node();
  res &= this->ConnectReflTargetCoordYR2Node();
  res &= this->ConnectReflTargetCoordZR2Node();
  
  return res;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectPatientOnTableTopNode()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();

  // Patient on the TableTop flag node
  QOpcUaNode* patTableTopNode = this->FindNodeFromFullDisplayName(ASU_PATIENT_ON_TABLE_NODE_NAME);

  // Connect signal handlers for subscribed values
  if (!patTableTopNode)
  {
    return false;
  }
  QObject::connect(patTableTopNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool flagValue = value.toBool();
        mrmlNode->SetPatientOnTableTop(flagValue);
        qDebug() << Q_FUNC_INFO << "Patient on the TableTop flag changed:" << flagValue;
      }
    }
  );
  QObject::connect(patTableTopNode,
    &QOpcUaNode::attributeRead, [patTableTopNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!patTableTopNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (patTableTopNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = patTableTopNode->attribute(QOpcUa::NodeAttribute::Value);
          bool flagValue = value.toBool(); // Get the attribute from the cache
          mrmlNode->SetPatientOnTableTop(flagValue);
          qDebug() << Q_FUNC_INFO << "Patient on the TableTop flag read:" << flagValue;
        }
      }
    }
  );
  // Subscribe to data changes
  patTableTopNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectTablePositionNode()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();

  // TableTop position (orientation) node
  QOpcUaNode* tablePosNode = this->FindNodeFromFullDisplayName(ASU_TABLE_POSITION_NODE_NAME);

  // Connect signal handlers for subscribed values
  if (!tablePosNode)
  {
    return false;
  }
  QObject::connect(tablePosNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool ok = false;
        int tablePos = value.toInt(&ok); // Get the attribute from the cache
        if (ok)
        {
          mrmlNode->SetTableTopPosition(tablePos);
          qDebug() << Q_FUNC_INFO << "TableTop position value changed:" << tablePos;
        }
      }
    }
  );
  QObject::connect(tablePosNode,
    &QOpcUaNode::attributeRead, [tablePosNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!tablePosNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (tablePosNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = tablePosNode->attribute(QOpcUa::NodeAttribute::Value);
          bool ok = false;
          int tablePos = value.toInt(&ok); // Get the attribute from the cache
          if (ok)
          {
            mrmlNode->SetTableTopPosition(tablePos);
            qDebug() << Q_FUNC_INFO << "TableTop position value read:" << tablePos;
          }
        }
      }
    }
  );
  // Subscribe to data changes
  tablePosNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectEmergencyEvacuationSignalNode()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();

  // Emergency Evacuation signal flag node
  QOpcUaNode* emEvacSignalNode = this->FindNodeFromFullDisplayName(ASU_EMEVAC_SIGNAL_NODE_NAME);

  // Connect signal handlers for subscribed values
  if (!emEvacSignalNode)
  {
    return false;
  }
  QObject::connect(emEvacSignalNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< bool >())
      {
        bool flagValue = value.toBool();
        mrmlNode->SetEmergencyEvacSignal(flagValue);
        qDebug() << Q_FUNC_INFO << "Emergency Evacuation signal flag changed:" << flagValue;
      }
    }
  );
  QObject::connect(emEvacSignalNode,
    &QOpcUaNode::attributeRead, [emEvacSignalNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!emEvacSignalNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (emEvacSignalNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = emEvacSignalNode->attribute(QOpcUa::NodeAttribute::Value);
          bool flagValue = value.toBool(); // Get the attribute from the cache
          mrmlNode->SetEmergencyEvacSignal(flagValue);
          qDebug() << Q_FUNC_INFO << "Emergency Evacuation signal flag read:" << flagValue;
        }
      }
    }
  );
  // Subscribe to data changes
  emEvacSignalNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  return true;
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidgetPrivate::ExpandAllAsync(QTreeView *view, QAbstractItemModel* model, const QModelIndex &parentIndex)
{
  if (parentIndex.isValid())
  {
    view->expand(parentIndex);
  }
  int rows = model->rowCount(parentIndex);
  for (int row = 0; row < rows; ++row)
  {
    this->ExpandAllAsync(view, model, model->index(row, 0, parentIndex));
  }
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectFlagNodes()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();

  // Robots are ready for x-ray #1 node
  QOpcUaNode* robReadyXray1Node = this->FindNodeFromFullDisplayName(ASU_ROBOTS_READY_XRAY1_NODE_NAME);
  // Robots areready for x-ray #2 node
  QOpcUaNode* robReadyXray2Node = this->FindNodeFromFullDisplayName(ASU_ROBOTS_READY_XRAY2_NODE_NAME);
  // Robots are ready for beam node
  QOpcUaNode* robReadyBeamNode = this->FindNodeFromFullDisplayName(ASU_ROBOTS_READY_BEAM_NODE_NAME);

  // Connect signal handlers for subscribed values
  // Robots ready for x-ray #1 node
  if (!robReadyXray1Node)
  {
    return false;
  }
  QObject::connect(robReadyXray1Node,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool flagValue = value.toBool();
        mrmlNode->SetRobotsReadyForXray1(flagValue);
        qDebug() << Q_FUNC_INFO << "Robots ready for x-ray #1 value changed:" << flagValue;
      }
    }
  );
  QObject::connect(robReadyXray1Node,
    &QOpcUaNode::attributeRead, [robReadyXray1Node, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!robReadyXray1Node || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (robReadyXray1Node->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = robReadyXray1Node->attribute(QOpcUa::NodeAttribute::Value);
          bool flagValue = value.toBool();
          mrmlNode->SetRobotsReadyForXray1(flagValue);
          qDebug() << Q_FUNC_INFO << "Robots ready for x-ray #1 value read:" << flagValue;
        }
      }
    }
  );
  // Subscribe to data changes
  robReadyXray1Node->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  // Connect signal handlers for subscribed values
  // Robots are ready for x-ray #2 node
  if (!robReadyXray2Node)
  {
    return false;
  }
  QObject::connect(robReadyXray2Node,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool flagValue = value.toBool();
        mrmlNode->SetRobotsReadyForXray2(flagValue);
        qDebug() << Q_FUNC_INFO << "Robots ready for x-ray #2 value changed:" << flagValue;
      }
    }
  );
  QObject::connect(robReadyXray2Node,
    &QOpcUaNode::attributeRead, [robReadyXray2Node, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!robReadyXray2Node || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (robReadyXray2Node->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = robReadyXray2Node->attribute(QOpcUa::NodeAttribute::Value);
          bool flagValue = value.toBool();
          mrmlNode->SetRobotsReadyForXray2(flagValue);
          qDebug() << Q_FUNC_INFO << "Robots ready for x-ray #2 value read:" << flagValue;
        }
      }
    }
  );
  // Subscribe to data changes
  robReadyXray2Node->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  // Robots are ready for beam node
  if (!robReadyBeamNode)
  {
    return false;
  }
  QObject::connect(robReadyBeamNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool flagValue = value.toBool();
        mrmlNode->SetRobotsReadyForBeam(flagValue);
        qDebug() << Q_FUNC_INFO << "Robots ready for beam value changed:" << flagValue;
      }
    }
  );
  QObject::connect(robReadyBeamNode,
    &QOpcUaNode::attributeRead, [robReadyBeamNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!robReadyBeamNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (robReadyBeamNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = robReadyBeamNode->attribute(QOpcUa::NodeAttribute::Value);
          bool flagValue = value.toBool();
          mrmlNode->SetRobotsReadyForBeam(flagValue);
          qDebug() << Q_FUNC_INFO << "Robots ready for beam value read:" << flagValue;
        }
      }
    }
  );
  // Subscribe to data changes
  robReadyBeamNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectSetCoordsXyzNodes()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();

  // Coord X node
  QOpcUaNode* coordXNode = this->FindNodeFromFullDisplayName(ASU_COORDS_X_NODE_NAME);
  // Coord Y node
  QOpcUaNode* coordYNode = this->FindNodeFromFullDisplayName(ASU_COORDS_Y_NODE_NAME);
  // Coord Z node
  QOpcUaNode* coordZNode = this->FindNodeFromFullDisplayName(ASU_COORDS_Z_NODE_NAME);

  // Connect signal handlers for subscribed values
  // Coord X node
  if (!coordXNode)
  {
    return false;
  }
  QObject::connect(coordXNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool ok = false;
        int32_t coordValue = value.toInt(&ok);
        if (ok)
        {
          mrmlNode->SetCoordsX(coordValue);
          qDebug() << Q_FUNC_INFO << "SetCoords X node value changed:" << coordValue;
        }
      }
    }
  );
  QObject::connect(coordXNode,
    &QOpcUaNode::attributeRead, [coordXNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!coordXNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (coordXNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = coordXNode->attribute(QOpcUa::NodeAttribute::Value);
          bool ok = false;
          int32_t coordValue = value.toInt(&ok); // Get the attribute from the cache
          if (ok)
          {
            mrmlNode->SetCoordsX(coordValue);
            qDebug() << Q_FUNC_INFO << "SetCoords X node value read:" << coordValue;
          }
        }
      }
    }
  );
  // Subscribe to data changes
  coordXNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  // Connect signal handlers for subscribed values
  // Coord Y node
  if (!coordYNode)
  {
    return false;
  }
  QObject::connect(coordYNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool ok = false;
        int32_t coordValue = value.toInt(&ok);
        if (ok)
        {
          mrmlNode->SetCoordsY(coordValue);
          qDebug() << Q_FUNC_INFO << "SetCoords Y node value changed:" << coordValue;
        }
      }
    }
  );
  QObject::connect(coordYNode,
    &QOpcUaNode::attributeRead, [coordYNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!coordYNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (coordYNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = coordYNode->attribute(QOpcUa::NodeAttribute::Value);
          bool ok = false;
          int32_t coordValue = value.toInt(&ok); // Get the attribute from the cache
          if (ok)
          {
            mrmlNode->SetCoordsY(coordValue);
            qDebug() << Q_FUNC_INFO << "SetCoords Y node value read:" << coordValue;
          }
        }
      }
    }
  );
  // Subscribe to data changes
  coordYNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  // Connect signal handlers for subscribed values
  // Coord Z node
  if (!coordZNode)
  {
    return false;
  }
  QObject::connect(coordZNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool ok = false;
        int32_t coordValue = value.toInt(&ok);
        if (ok)
        {
          mrmlNode->SetCoordsZ(coordValue);
          qDebug() << Q_FUNC_INFO << "SetCoords Z node value changed:" << coordValue;
        }
      }
    }
  );
  QObject::connect(coordZNode,
    &QOpcUaNode::attributeRead, [coordZNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!coordZNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (coordZNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = coordZNode->attribute(QOpcUa::NodeAttribute::Value);
          bool ok = false;
          int32_t coordValue = value.toInt(&ok); // Get the attribute from the cache
          if (ok)
          {
            mrmlNode->SetCoordsZ(coordValue);
            qDebug() << Q_FUNC_INFO << "SetCoords Z node value read:" << coordValue;
          }
        }
      }
    }
  );
  // Subscribe to data changes
  coordZNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectSetCoordsAbcNodes()
{
  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();

  if (!opcUaClient || !this->ParameterNode)
  {
    return false;
  }

  vtkMRMLSiemensPlcOpcUaNode* mrmlNode = this->ParameterNode.GetPointer();

  // Angle A node
  QOpcUaNode* r1AngleANode = this->FindNodeFromFullDisplayName(ASU_COORDS_A_NODE_NAME);
  // Angle B node
  QOpcUaNode* r1AngleBNode = this->FindNodeFromFullDisplayName(ASU_COORDS_B_NODE_NAME);
  // Angle C node
  QOpcUaNode* r1AngleCNode = this->FindNodeFromFullDisplayName(ASU_COORDS_C_NODE_NAME);

  // Connect signal handlers for subscribed values
  // Angle A node
  if (!r1AngleANode)
  {
    return false;
  }
  QObject::connect(r1AngleANode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool ok = false;
        int32_t angleValue = value.toInt(&ok);
        if (ok)
        {
          mrmlNode->SetCoordsA(angleValue);
          qDebug() << Q_FUNC_INFO << "SetCoords A node value changed:" << angleValue;
        }
      }
    }
  );
  QObject::connect(r1AngleANode,
    &QOpcUaNode::attributeRead, [r1AngleANode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!r1AngleANode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (r1AngleANode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = r1AngleANode->attribute(QOpcUa::NodeAttribute::Value);
          bool ok = false;
          int32_t angleValue = value.toInt(&ok);
          if (ok)
          {
            mrmlNode->SetCoordsA(angleValue);
            qDebug() << Q_FUNC_INFO << "SetCoords A node value read:" << angleValue;
          }
        }
      }
    }
  );
  // Subscribe to data changes
  r1AngleANode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  // Connect signal handlers for subscribed values
  // Angle B node
  if (!r1AngleBNode)
  {
    return false;
  }
  QObject::connect(r1AngleBNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool ok = false;
        int32_t angleValue = value.toInt(&ok);
        if (ok)
        {
          mrmlNode->SetCoordsB(angleValue);
          qDebug() << Q_FUNC_INFO << "SetCoords B node value changed:" << angleValue;
        }
      }
    }
  );
  QObject::connect(r1AngleBNode,
    &QOpcUaNode::attributeRead, [r1AngleBNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!r1AngleBNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (r1AngleBNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = r1AngleBNode->attribute(QOpcUa::NodeAttribute::Value);
          bool ok = false;
          int32_t angleValue = value.toInt(&ok);
          if (ok)
          {
            mrmlNode->SetCoordsB(angleValue);
            qDebug() << Q_FUNC_INFO << "SetCoords B node value read:" << angleValue;
          }
        }
      }
    }
  );
  // Subscribe to data changes
  r1AngleBNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  // Connect signal handlers for subscribed values
  // Angle C node
  if (!r1AngleCNode)
  {
    return false;
  }
  QObject::connect(r1AngleCNode,
    &QOpcUaNode::dataChangeOccurred, [mrmlNode](QOpcUa::NodeAttribute attr, QVariant value)
    {
      if (attr == QOpcUa::NodeAttribute::Value && value.canConvert< int >())
      {
        bool ok = false;
        int32_t angleValue = value.toInt(&ok);
        if (ok)
        {
          mrmlNode->SetCoordsC(angleValue);
          qDebug() << Q_FUNC_INFO << "SetCoords C node value changed:" << angleValue;
        }
      }
    }
  );
  QObject::connect(r1AngleCNode,
    &QOpcUaNode::attributeRead, [r1AngleCNode, mrmlNode](QOpcUa::NodeAttributes attr)
    {
      if (!r1AngleCNode || !mrmlNode)
      {
        return;
      }
      if (attr & QOpcUa::NodeAttribute::Value)
      {
        if (r1AngleCNode->attributeError(QOpcUa::NodeAttribute::Value) == QOpcUa::UaStatusCode::Good)
        {
          QVariant value = r1AngleCNode->attribute(QOpcUa::NodeAttribute::Value);
          bool ok = false;
          int32_t angleValue = value.toInt(&ok);
          if (ok)
          {
            mrmlNode->SetCoordsC(angleValue);
            qDebug() << Q_FUNC_INFO << "SetCoords C node value read:" << angleValue;
          }
        }
      }
    }
  );
  // Subscribe to data changes
  r1AngleCNode->enableMonitoring(QOpcUa::NodeAttribute::Value, OpcUaMonitorParameters);

  return true;
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ConnectSetCoordsNodes()
{
  bool res = this->ConnectSetCoordsXyzNodes();
  res &= this->ConnectSetCoordsAbcNodes();
  return res;
}

//-----------------------------------------------------------------------------
// qSlicerSiemensPlcOpcUaWidget methods

//-----------------------------------------------------------------------------
qSlicerSiemensPlcOpcUaWidget::qSlicerSiemensPlcOpcUaWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerSiemensPlcOpcUaWidgetPrivate(*this) )
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  d->setupUi(this);

  OpcUaModel* opcUaModel = d->SiemensPlcOpcUaModel.data();
  QTreeView* opcUaTreeView = d->TreeView_OpcUaModel;

  QObject::connect(d->ProgressTimer.data(), SIGNAL(timeout()), d->ProgressDialog.data(), SLOT(reset()));

  QObject::connect(opcUaModel, &QAbstractItemModel::rowsInserted, this,
    [this, opcUaModel, opcUaTreeView](const QModelIndex& parent, int first, int last)
    {
      opcUaTreeView->expand(parent);
      for (int row = first; row <= last; ++row)
      {
        QModelIndex childIndex = opcUaModel->index(row, 0, parent);
        opcUaTreeView->expand(childIndex);
        opcUaModel->rowCount(childIndex);
      }
    }
  );

  QProgressDialog* progressDialog = d->ProgressDialog.data();
  QTimer* progressTimer = d->ProgressTimer.data();
  QObject::connect(opcUaModel, &OpcUaModel::browseProgressChanged, this,
    [this, progressDialog, progressTimer](int finished, int started)
    {
      progressDialog->setLabelText(tr("Loading tree of nodes. Please, wait!"));
      progressDialog->setMaximum(started);
      progressDialog->setValue(finished);
      if (finished >= started)
      {
        progressTimer->start();
        progressDialog->close();
      }
      else
      {
        progressTimer->stop();
      }
    }
  );

  // Set coords from ASU buttons
  QObject::connect(d->PushButton_SetCoordX, SIGNAL(clicked()),
    this, SLOT(onSetCoordXClicked()));
  QObject::connect(d->PushButton_SetCoordY, SIGNAL(clicked()),
    this, SLOT(onSetCoordYClicked()));
  QObject::connect(d->PushButton_SetCoordZ, SIGNAL(clicked()),
    this, SLOT(onSetCoordZClicked()));
  QObject::connect(d->PushButton_SetAngleA, SIGNAL(clicked()),
    this, SLOT(onSetCoordAClicked())); 
  QObject::connect(d->PushButton_SetAngleB, SIGNAL(clicked()),
    this, SLOT(onSetCoordBClicked()));
  QObject::connect(d->PushButton_SetAngleC, SIGNAL(clicked()),
    this, SLOT(onSetCoordCClicked()));
  QObject::connect(d->PushButton_SetCoords, SIGNAL(clicked()),
    this, SLOT(onSetCoordsClicked()));

  // AutoManual R1 ToLoad button
  QObject::connect(d->PushButton_AMR1ToLoad, SIGNAL(pressed()),
    this, SLOT(onAutoManualR1ToLoadPressed()));
  QObject::connect(d->PushButton_AMR1ToLoad, SIGNAL(released()),
    this, SLOT(onAutoManualR1ToLoadReleased()));
  // AutoManual R1 LoadToIso button
  QObject::connect(d->PushButton_AMR1LoadToIso, SIGNAL(pressed()),
    this, SLOT(onAutoManualR1LoadToIsoPressed()));
  QObject::connect(d->PushButton_AMR1LoadToIso, SIGNAL(released()),
    this, SLOT(onAutoManualR1LoadToIsoReleased()));
  // AutoManual R1 ToNewCoords button
  QObject::connect(d->PushButton_AMR1ToNewCoords, SIGNAL(pressed()),
    this, SLOT(onAutoManualR1ToNewCoordsPressed()));
  QObject::connect(d->PushButton_AMR1ToNewCoords, SIGNAL(released()),
    this, SLOT(onAutoManualR1ToNewCoordsReleased()));

  // AutoManual R2 ToHome button
  QObject::connect(d->PushButton_AMR2ToHome, SIGNAL(pressed()),
    this, SLOT(onAutoManualR2ToHomePressed()));
  QObject::connect(d->PushButton_AMR2ToHome, SIGNAL(released()),
    this, SLOT(onAutoManualR2ToHomeReleased()));
  // AutoManual R2 ToPlane1 button
  QObject::connect(d->PushButton_AMR2ToPlane1, SIGNAL(pressed()),
    this, SLOT(onAutoManualR2ToPlane1Pressed()));
  QObject::connect(d->PushButton_AMR2ToPlane1, SIGNAL(released()),
    this, SLOT(onAutoManualR2ToPlane1Released()));
  // AutoManual R2 ToPlane2 button
  QObject::connect(d->PushButton_AMR2ToPlane2, SIGNAL(pressed()),
    this, SLOT(onAutoManualR2ToPlane2Pressed()));
  QObject::connect(d->PushButton_AMR2ToPlane2, SIGNAL(released()),
    this, SLOT(onAutoManualR2ToPlane2Released()));
  // AutoManual R1R2 EmerEvac button
  QObject::connect(d->PushButton_AMR1R2EmerEvac, SIGNAL(pressed()),
    this, SLOT(onAutoManualR1R2EmerEvacPressed()));
  QObject::connect(d->PushButton_AMR1R2EmerEvac, SIGNAL(released()),
    this, SLOT(onAutoManualR1R2EmerEvacReleased()));
  // AutoManual ApplyTableTopPosition
  QObject::connect(d->PushButton_AMApplyTableTopPosition, SIGNAL(pressed()),
    this, SLOT(onAutoManualApplyTablePositionPressed()));
  QObject::connect(d->PushButton_AMApplyTableTopPosition, SIGNAL(released()),
    this, SLOT(onAutoManualApplyTablePositionReleased()));
  QObject::connect(d->PushButton_AMApplyNewCoords, SIGNAL(pressed()),
    this, SLOT(onAutoManualApplyNewCoordsPressed()));
  QObject::connect(d->PushButton_AMApplyNewCoords, SIGNAL(released()),
    this, SLOT(onAutoManualApplyNewCoordsReleased()));

  // Service R1 BreakTest button
  QObject::connect(d->PushButton_SR1BrakeTest, SIGNAL(pressed()),
    this, SLOT(onServiceR1BreakTestPressed()));
  QObject::connect(d->PushButton_SR1BrakeTest, SIGNAL(released()),
    this, SLOT(onServiceR1BreakTestReleased()));
  // Service R1 MasterReferenceTest button
  QObject::connect(d->PushButton_SR1MasterRefTest, SIGNAL(pressed()),
    this, SLOT(onServiceR1MasterReferenceTestPressed()));
  QObject::connect(d->PushButton_SR1MasterRefTest, SIGNAL(released()),
    this, SLOT(onServiceR1MasterReferenceTestReleased()));
  // Service R1 ToLoad button
  QObject::connect(d->PushButton_SR1ToLoad, SIGNAL(pressed()),
    this, SLOT(onServiceR1LoadPressed()));
  QObject::connect(d->PushButton_SR1ToLoad, SIGNAL(released()),
    this, SLOT(onServiceR1LoadReleased()));
  // Service R1 ToServicePos1 button
  QObject::connect(d->PushButton_SMR1ServicePos1, SIGNAL(pressed()),
    this, SLOT(onServiceR1ServicePos1Pressed()));
  QObject::connect(d->PushButton_SMR1ServicePos1, SIGNAL(released()),
    this, SLOT(onServiceR1ServicePos1Released()));
  // Service R1 ToServicePos2 button
  QObject::connect(d->PushButton_SMR1ServicePos2, SIGNAL(pressed()),
    this, SLOT(onServiceR1ServicePos2Pressed()));
  QObject::connect(d->PushButton_SMR1ServicePos2, SIGNAL(released()),
    this, SLOT(onServiceR1ServicePos2Released()));
  // Service R1 ToServicePos3 button
  QObject::connect(d->PushButton_SMR1ServicePos3, SIGNAL(pressed()),
    this, SLOT(onServiceR1ServicePos3Pressed()));
  QObject::connect(d->PushButton_SMR1ServicePos3, SIGNAL(released()),
    this, SLOT(onServiceR1ServicePos3Released()));

  // Service R2 BreakTest button
  QObject::connect(d->PushButton_SR2BrakeTest, SIGNAL(pressed()),
    this, SLOT(onServiceR2BreakTestPressed()));
  QObject::connect(d->PushButton_SR2BrakeTest, SIGNAL(released()),
    this, SLOT(onServiceR2BreakTestReleased()));
  // Service R2 MasterReferenceTest button
  QObject::connect(d->PushButton_SR2MasterRefTest, SIGNAL(pressed()),
    this, SLOT(onServiceR2MasterReferenceTestPressed()));
  QObject::connect(d->PushButton_SR2MasterRefTest, SIGNAL(released()),
    this, SLOT(onServiceR2MasterReferenceTestReleased()));
  // Service R2 ToHome button
  QObject::connect(d->PushButton_SR2ToHome, SIGNAL(pressed()),
    this, SLOT(onServiceR2HomePressed()));
  QObject::connect(d->PushButton_SR2ToHome, SIGNAL(released()),
    this, SLOT(onServiceR2HomeReleased()));
  // Service R2 ToServicePos1 button
  QObject::connect(d->PushButton_SMR2ServicePos1, SIGNAL(pressed()),
    this, SLOT(onServiceR2ServicePos1Pressed()));
  QObject::connect(d->PushButton_SMR2ServicePos1, SIGNAL(released()),
    this, SLOT(onServiceR2ServicePos1Released()));
  // Service R2 ToServicePos2 button
  QObject::connect(d->PushButton_SMR2ServicePos2, SIGNAL(pressed()),
    this, SLOT(onServiceR2ServicePos2Pressed()));
  QObject::connect(d->PushButton_SMR2ServicePos2, SIGNAL(released()),
    this, SLOT(onServiceR2ServicePos2Released()));
  // Service R2 ToServicePos3 button
  QObject::connect(d->PushButton_SMR2ServicePos3, SIGNAL(pressed()),
    this, SLOT(onServiceR2ServicePos3Pressed()));
  QObject::connect(d->PushButton_SMR2ServicePos3, SIGNAL(released()),
    this, SLOT(onServiceR2ServicePos3Released()));
  // Service RestartSM button
  QObject::connect(d->PushButton_SMRestart, SIGNAL(pressed()),
    this, SLOT(onServiceRestartSmPressed()));
  QObject::connect(d->PushButton_SMRestart, SIGNAL(released()),
    this, SLOT(onServiceRestartSmReleased()));

  // KUKA controllers movement AllowT1 button
  QObject::connect(d->PushButton_KCMAllowT1, SIGNAL(pressed()),
    this, SLOT(onKukaAllowT1Pressed()));
  QObject::connect(d->PushButton_KCMAllowT1, SIGNAL(released()),
    this, SLOT(onKukaAllowT1Released()));
  // KUKA controllers movement AllowXRay button
  QObject::connect(d->PushButton_KCMAllowXRay, SIGNAL(pressed()),
    this, SLOT(onKukaAllowXrayPressed()));
  QObject::connect(d->PushButton_KCMAllowXRay, SIGNAL(released()),
    this, SLOT(onKukaAllowXrayReleased()));
  // KUKA controllers movement AllowBeam button
  QObject::connect(d->PushButton_KCMAllowBeam, SIGNAL(pressed()),
    this, SLOT(onKukaAllowBeamPressed()));
  QObject::connect(d->PushButton_KCMAllowBeam, SIGNAL(released()),
    this, SLOT(onKukaAllowBeamReleased()));

  // Reset Errors button
  QObject::connect(d->PushButton_ResetErrors, SIGNAL(pressed()),
    this, SLOT(onResetErrorsPressed()));
  QObject::connect(d->PushButton_ResetErrors, SIGNAL(released()),
    this, SLOT(onResetErrorsReleased()));

  // MakeXRay button
  QObject::connect(d->PushButton_MakeXRay, SIGNAL(pressed()),
    this, SLOT(onMakeXrayPressed()));
  QObject::connect(d->PushButton_MakeXRay, SIGNAL(released()),
    this, SLOT(onMakeXrayReleased()));

  QObject::connect(d->ButtonGroup_TableTopPosition, SIGNAL(buttonClicked(QAbstractButton*)),
    this, SLOT(onTablePositionChanged(QAbstractButton*)));

  // Read nodes manually 
  QObject::connect(d->PushButton_ReadStatusAndMessages, SIGNAL(clicked()),
    this, SLOT(onReadStatusAndMessagesClicked()));
}

//-----------------------------------------------------------------------------
qSlicerSiemensPlcOpcUaWidget::~qSlicerSiemensPlcOpcUaWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::setParameterNode(vtkMRMLNode* node)
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);

  vtkMRMLSiemensPlcOpcUaNode* parameterNode = vtkMRMLSiemensPlcOpcUaNode::SafeDownCast(node);
  // Each time the node is modified, the UI widgets are updated
  qvtkReconnect(d->ParameterNode, parameterNode, vtkCommand::ModifiedEvent, 
    this, SLOT(updateWidgetFromMRML()));

  d->ParameterNode = parameterNode;

  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::setSiemensPlcOpcUaClient(const QSharedPointer< QOpcUaClient >& sharedClient)
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);

  d->OpcUaClient = sharedClient;
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  if (!d->ParameterNode)
  {
    return;
  }
  d->CollapsibleButton_AutomaticManualMovement->setEnabled(true);
  d->CollapsibleButton_ServiceMovement->setEnabled(true);
  d->CollapsibleButton_KukaControllersMovement->setEnabled(true);

  uint64_t errBits = d->ParameterNode->GetErrorMessages();
  uint64_t servBits = d->ParameterNode->GetServiceMessages();
  uint64_t miscBits = d->ParameterNode->GetMiscMessages();
  d->SiemensPlcErrorListModel->updateMessagesBits(errBits, 0, 0);
  d->SiemensPlcServiceListModel->updateMessagesBits(0, servBits, 0);
  d->SiemensPlcMessagesListModel->updateMessagesBits(0, 0, miscBits);

  bool buttonState[2] = { false, false };
  // AM
  d->ParameterNode->GetAutoManualR1ToLoad(buttonState);
  d->PushButton_AMR1ToLoad->setEnabled(buttonState[0]);

  d->ParameterNode->GetAutoManualR1LoadToIso(buttonState);
  d->PushButton_AMR1LoadToIso->setEnabled(buttonState[0]);

  d->ParameterNode->GetAutoManualR1ToNewCoords(buttonState);
  d->PushButton_AMR1ToNewCoords->setEnabled(buttonState[0]);

  d->ParameterNode->GetAutoManualR2ToHome(buttonState);
  d->PushButton_AMR2ToHome->setEnabled(buttonState[0]);

  d->ParameterNode->GetAutoManualR2ToPlane1(buttonState);
  d->PushButton_AMR2ToPlane1->setEnabled(buttonState[0]);

  d->ParameterNode->GetAutoManualR2ToPlane2(buttonState);
  d->PushButton_AMR2ToPlane2->setEnabled(buttonState[0]);

  d->ParameterNode->GetAutoManualEmergencyEvac(buttonState);
  d->PushButton_AMR1R2EmerEvac->setEnabled(buttonState[0]);

  d->ParameterNode->GetAutoManualApplyTabletopPosition(buttonState);
  d->PushButton_AMApplyTableTopPosition->setEnabled(buttonState[0]);

  d->ParameterNode->GetAutoManualApplyNewCoords(buttonState);
  d->PushButton_AMApplyNewCoords->setEnabled(buttonState[0]);

  // SM
  d->ParameterNode->GetServiceR1BreakTest(buttonState);
  d->PushButton_SR1BrakeTest->setEnabled(buttonState[0]);

  d->ParameterNode->GetServiceR1MasterReferenceTest(buttonState);
  d->PushButton_SR1MasterRefTest->setEnabled(buttonState[0]);

  d->ParameterNode->GetServiceR1Load(buttonState);
  d->PushButton_SR1ToLoad->setEnabled(buttonState[0]);

  d->ParameterNode->GetServiceR1Position1(buttonState);
  d->PushButton_SMR1ServicePos1->setEnabled(buttonState[0]);

  d->ParameterNode->GetServiceR1Position2(buttonState);
  d->PushButton_SMR1ServicePos2->setEnabled(buttonState[0]);

  d->ParameterNode->GetServiceR1Position3(buttonState);
  d->PushButton_SMR1ServicePos3->setEnabled(buttonState[0]);

  d->ParameterNode->GetServiceR2BreakTest(buttonState);
  d->PushButton_SR2BrakeTest->setEnabled(buttonState[0]);

  d->ParameterNode->GetServiceR2MasterReferenceTest(buttonState);
  d->PushButton_SR2MasterRefTest->setEnabled(buttonState[0]);

  d->ParameterNode->GetServiceR2Home(buttonState);
  d->PushButton_SR2ToHome->setEnabled(buttonState[0]);

  d->ParameterNode->GetServiceR2Position1(buttonState);
  d->PushButton_SMR2ServicePos1->setEnabled(buttonState[0]);

  d->ParameterNode->GetServiceR2Position2(buttonState);
  d->PushButton_SMR2ServicePos2->setEnabled(buttonState[0]);

  d->ParameterNode->GetServiceR2Position3(buttonState);
  d->PushButton_SMR2ServicePos3->setEnabled(buttonState[0]);

  d->ParameterNode->GetServiceRestart(buttonState);
  d->PushButton_SMRestart->setEnabled(buttonState[0]);

  // KCM
  d->ParameterNode->GetKukaAllowT1(buttonState);
  d->PushButton_KCMAllowT1->setEnabled(buttonState[0]);

  d->ParameterNode->GetKukaAllowXray(buttonState);
  d->PushButton_KCMAllowXRay->setEnabled(buttonState[0]);

  d->ParameterNode->GetKukaAllowBeam(buttonState);
  d->PushButton_KCMAllowBeam->setEnabled(buttonState[0]);

  // Misc.
  d->ParameterNode->GetResetErrors(buttonState);
  d->PushButton_ResetErrors->setEnabled(buttonState[0]);

  d->ParameterNode->GetMakeXray(buttonState);
  d->PushButton_MakeXRay->setEnabled(buttonState[0]);

  {
    QSignalBlocker block(d->CheckBox_HUMAN_ON_DEKA);
    bool flag = d->ParameterNode->GetPatientOnTableTop();
    d->CheckBox_HUMAN_ON_DEKA->setChecked(flag);
  }

  {
    QSignalBlocker block(d->CheckBox_HUMAN_ON_DEKA);
    bool flag = d->ParameterNode->GetPatientOnTableTop();
    d->CheckBox_HUMAN_ON_DEKA->setChecked(flag);
  }

  {
    QSignalBlocker block0(d->CheckBox_RobotsReadyForXray1);
    bool flag = d->ParameterNode->GetRobotsReadyForXray1();
    d->CheckBox_RobotsReadyForXray1->setChecked(flag);

    QSignalBlocker block1(d->CheckBox_RobotsReadyForXray2);
    flag = d->ParameterNode->GetRobotsReadyForXray2();
    d->CheckBox_RobotsReadyForXray2->setChecked(flag);

    QSignalBlocker block2(d->CheckBox_RobotsReadyForBeam);
    flag = d->ParameterNode->GetRobotsReadyForBeam();
    d->CheckBox_RobotsReadyForBeam->setChecked(flag);
  }

  {
    QSignalBlocker block0(d->CheckBox_DoorLockSensor1);
    bool flag = d->ParameterNode->GetDoorLockSensor1();
    d->CheckBox_DoorLockSensor1->setChecked(flag);

    QSignalBlocker block1(d->CheckBox_DoorLockSensor2);
    flag = d->ParameterNode->GetDoorLockSensor2();
    d->CheckBox_DoorLockSensor2->setChecked(flag);

    QSignalBlocker block2(d->CheckBox_EmergencyEvacuationFlag);
    flag = d->ParameterNode->GetEmergencyEvacSignal();
    d->CheckBox_EmergencyEvacuationFlag->setChecked(flag);
  }

  {
    QSignalBlocker block0(d->CheckBox_RobotsReadyForXray1);
    bool flag = d->ParameterNode->GetRobotsReadyForXray1();
    d->CheckBox_RobotsReadyForXray1->setChecked(flag);

    QSignalBlocker block1(d->CheckBox_RobotsReadyForXray2);
    flag = d->ParameterNode->GetRobotsReadyForXray2();
    d->CheckBox_RobotsReadyForXray2->setChecked(flag);

    QSignalBlocker block2(d->CheckBox_RobotsReadyForBeam);
    flag = d->ParameterNode->GetRobotsReadyForBeam();
    d->CheckBox_RobotsReadyForBeam->setChecked(flag);
  }

  double r1Angles[6] = {};
  double r2Angles[6] = {};
  d->ParameterNode->GetCurrentAxisCoordsR1(r1Angles);
  d->ParameterNode->GetCurrentAxisCoordsR2(r2Angles);
  d->SliderWidget_R1A1->setValue(r1Angles[0]);
  d->SliderWidget_R1A2->setValue(r1Angles[1]);
  d->SliderWidget_R1A3->setValue(r1Angles[2]);
  d->SliderWidget_R1A4->setValue(r1Angles[3]);
  d->SliderWidget_R1A5->setValue(r1Angles[4]);
  d->SliderWidget_R1A6->setValue(r1Angles[5]);
  d->SliderWidget_R2A1->setValue(r2Angles[0]);
  d->SliderWidget_R2A2->setValue(r2Angles[1]);
  d->SliderWidget_R2A3->setValue(r2Angles[2]);
  d->SliderWidget_R2A4->setValue(r2Angles[3]);
  d->SliderWidget_R2A5->setValue(r2Angles[4]);
  d->SliderWidget_R2A6->setValue(r2Angles[5]);
  {
    QSignalBlocker block0(d->RadioButton_PositionNotDefined);
    QSignalBlocker block1(d->RadioButton_Position1);
    QSignalBlocker block2(d->RadioButton_Position2);
    QSignalBlocker block3(d->RadioButton_Position3);
    uint8_t position = d->ParameterNode->GetTableTopPosition();
    switch (position)
    {
    case 0:
      d->RadioButton_PositionNotDefined->setChecked(true);
      break;
    case 1:
      d->RadioButton_Position1->setChecked(true);
      break;
    case 2:
      d->RadioButton_Position2->setChecked(true);
      break;
    case 3:
      d->RadioButton_Position3->setChecked(true);
      break;
    default:
      break;
    }
  }

  double r1KukaCoords[6] = {};
  d->ParameterNode->GetCurrentCoordsR1(r1KukaCoords);
  double* r1KukaAngles = r1KukaCoords + 3;
  d->CoordinatesWidget_CurrCoordsR1->setCoordinates(r1KukaCoords);
  d->CoordinatesWidget_CurrAnglesR1->setCoordinates(r1KukaAngles);

  double r2KukaCoords[6] = {};
  d->ParameterNode->GetCurrentCoordsR2(r2KukaCoords);
  double* r2KukaAngles = r2KukaCoords + 3;
  d->CoordinatesWidget_CurrCoordsR2->setCoordinates(r2KukaCoords);
  d->CoordinatesWidget_CurrAnglesR2->setCoordinates(r2KukaAngles);

  d->ParameterNode->GetConfirmedTargetCoordsR1(r1KukaCoords);
  d->CoordinatesWidget_ConfirmedTargetCoordsR1->setCoordinates(r1KukaCoords);
  d->CoordinatesWidget_ConfirmedTargetAnglesR1->setCoordinates(r1KukaAngles);

  d->ParameterNode->GetConfirmedTargetCoordsR2(r2KukaCoords);
  d->CoordinatesWidget_ConfirmedTargetCoordsR2->setCoordinates(r2KukaCoords);
  d->CoordinatesWidget_ConfirmedTargetAnglesR2->setCoordinates(r2KukaAngles);
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onOpcUaClientConnected()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  
  QSharedPointer< QOpcUaClient > sharedClient = d->OpcUaClient.toStrongRef();
  if (!sharedClient)
  {
    return;
  }

  d->SiemensPlcOpcUaModel->setOpcUaClient(sharedClient.data(), OpcUaTreeItem::SIEMENS_PLC_SERVER_INTERFACES_NODE_ID);
//  d->SiemensPlcOpcUaModel->setOpcUaClient(sharedClient.data(), QOpcUa::namespace0Id(QOpcUa::NodeIds::Namespace0::RootFolder));
//  d->TreeView_OpcUaModel->header()->setSectionResizeMode(0 /* BrowseName column*/, QHeaderView::Interactive);
  d->TreeView_OpcUaModel->header()->setSectionResizeMode(1 /* Value column*/, QHeaderView::Interactive);
//  d->TreeView_OpcUaModel->header()->setSectionResizeMode(2 /* NodeClass column*/, QHeaderView::Interactive);
//  d->TreeView_OpcUaModel->header()->setSectionResizeMode(3 /* DataType column*/, QHeaderView::Interactive);
//  d->TreeView_OpcUaModel->header()->setSectionResizeMode(4 /* NodeID column*/, QHeaderView::Interactive);
//  d->TreeView_OpcUaModel->header()->setSectionResizeMode(5 /* DisplayName column*/, QHeaderView::Interactive);
//  d->TreeView_OpcUaModel->header()->setSectionResizeMode(6 /* Description column*/, QHeaderView::Interactive);

  d->CurrentTimeNode.reset(sharedClient->node(QOpcUa::namespace0Id(QOpcUa::NodeIds::Namespace0::Server_ServerStatus_CurrentTime)));
  if (d->CurrentTimeNode)
  {
    // Subscribe to data changes
    d->CurrentTimeNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(1000));
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onParseServerInterfacesClicked()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  
  QSharedPointer< QOpcUaClient > sharedClient = d->OpcUaClient.toStrongRef();
  if (!sharedClient)
  {
    return;
  }

  d->ServerInterfacesNode.reset(sharedClient->node(OpcUaTreeItem::SIEMENS_PLC_SERVER_INTERFACES_NODE_ID));
  if (!d->ServerInterfacesNode)
  {
    return;
  }

  QObject::connect(d->ServerInterfacesNode.data(), &QOpcUaNode::attributeRead,
    this, &qSlicerSiemensPlcOpcUaWidget::onServerInterfacesRead);

  d->ServerInterfacesNode->readAttributes(QOpcUa::NodeAttribute::DisplayName);
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onOpcUaClientDisconnected()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);

  d->PushButton_ReadStatusAndMessages->setEnabled(false);
  if (d->CurrentTimeNode)
  {
    d->CurrentTimeNode->disableMonitoring(QOpcUa::NodeAttribute::Value);
  }
  d->SiemensPlcOpcUaModel->clear();
  d->SiemensPlcErrorListModel->clear();
  d->SiemensPlcServiceListModel->clear();
  d->SiemensPlcMessagesListModel->clear();
  
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onOpcUaModeChanged(vtkMRMLSiemensPlcOpcUaNode::ModeType mode)
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Write Mode value
  QOpcUaNode* modeNode = d->FindNodeFromFullDisplayName(ASU_MODE_NODE_NAME);
  if (modeNode)
  {
    qDebug() << Q_FUNC_INFO << ": Set new mode:" << mode;
    modeNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(mode), QOpcUa::Types::Byte);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualR1LoadToIsoPressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutoManual R1 LoadToIso button pressed node
  QOpcUaNode* amR1LoadToIsoPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_AM_R1_LOADTOISO_PRESSED_NODE_NAME);
  if (amR1LoadToIsoPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R1 LoadToIso pressed";
    amR1LoadToIsoPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualR1LoadToIsoReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutoManual R1 LoadToIso button pressed node
  QOpcUaNode* amR1LoadToIsoPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_AM_R1_LOADTOISO_PRESSED_NODE_NAME);
  if (amR1LoadToIsoPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R1 LoadToIso released";
    amR1LoadToIsoPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualR1ToLoadPressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutoManual R1 ToLoad button pressed node
  QOpcUaNode* amR1ToLoadPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_AM_R1_TOLOAD_PRESSED_NODE_NAME);
  if (amR1ToLoadPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R1 ToLoad pressed";
    amR1ToLoadPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualR1ToLoadReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutoManual R1 ToLoad button pressed node
  QOpcUaNode* amR1ToLoadPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_AM_R1_TOLOAD_PRESSED_NODE_NAME);
  if (amR1ToLoadPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R1 ToLoad released";
    amR1ToLoadPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualR1ToNewCoordsPressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutomaticManual R1 NewCoords button pressed node
  QOpcUaNode* amR1NewCoordsPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_AM_R1_NEWCOORDS_PRESSED_NODE_NAME);
  if (amR1NewCoordsPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R1 NewCoord pressed";
    amR1NewCoordsPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualR1ToNewCoordsReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutomaticManual R1 NewCoords button pressed node
  QOpcUaNode* amR1NewCoordsPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_AM_R1_NEWCOORDS_PRESSED_NODE_NAME);
  if (amR1NewCoordsPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R1 NewCoord released";
    amR1NewCoordsPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualR2ToHomePressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutomaticManual R2 ToHome button pressed node
  QOpcUaNode* amR2ToHomePressedNode = d->FindNodeFromFullDisplayName(BUTTONS_AM_R2_TOHOME_PRESSED_NODE_NAME);
  if (amR2ToHomePressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R2 ToHome button pressed";
    amR2ToHomePressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualR2ToHomeReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutomaticManual R2 ToHome button pressed node
  QOpcUaNode* amR2ToHomePressedNode = d->FindNodeFromFullDisplayName(BUTTONS_AM_R2_TOHOME_PRESSED_NODE_NAME);
  if (amR2ToHomePressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R2 ToHome button released";
    amR2ToHomePressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualR2ToPlane1Pressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutomaticManual R2 ToPlane1 button pressed node
  QOpcUaNode* amR2ToPlane1PressedNode = d->FindNodeFromFullDisplayName(BUTTONS_AM_R2_TOPLANE1_PRESSED_NODE_NAME);
  if (amR2ToPlane1PressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R2 ToPlane1 button pressed";
    amR2ToPlane1PressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualR2ToPlane1Released()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutomaticManual R2 ToPlane1 button pressed node
  QOpcUaNode* amR2ToPlane1PressedNode = d->FindNodeFromFullDisplayName(BUTTONS_AM_R2_TOPLANE1_PRESSED_NODE_NAME);
  if (amR2ToPlane1PressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R2 ToPlane1 button released";
    amR2ToPlane1PressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualR2ToPlane2Pressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutomaticManual R2 ToPlane2 button pressed node
  QOpcUaNode* amR2ToPlane2PressedNode = d->FindNodeFromFullDisplayName(BUTTONS_AM_R2_TOPLANE2_PRESSED_NODE_NAME);
  if (amR2ToPlane2PressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R2 ToPlane2 button pressed";
    amR2ToPlane2PressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualR2ToPlane2Released()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutomaticManual R2 ToPlane2 button pressed node
  QOpcUaNode* amR2ToPlane2PressedNode = d->FindNodeFromFullDisplayName(BUTTONS_AM_R2_TOPLANE2_PRESSED_NODE_NAME);
  if (amR2ToPlane2PressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R2 ToPlane2 button released";
    amR2ToPlane2PressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualR1R2EmerEvacPressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutomaticManual R1R2 Emergency Evacuation button pressed node
  QOpcUaNode* amR1R2EmEvacPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_AM_R1R2_EMEVAC_PRESSED_NODE_NAME);
  if (amR1R2EmEvacPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R1R2 Emergency Evacuation button pressed";
    amR1R2EmEvacPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualR1R2EmerEvacReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutomaticManual R1R2 Emergency Evacuation button pressed node
  QOpcUaNode* amR1R2EmEvacPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_AM_R1R2_EMEVAC_PRESSED_NODE_NAME);
  if (amR1R2EmEvacPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R1R2 Emergency Evacuation button released";
    amR1R2EmEvacPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualApplyTablePositionPressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutomaticManual ApplyTableTopPosition button pressed node
  QOpcUaNode* amApplyTableTopPosPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_AM_APPLYTABLEPOSITION_PRESSED_NODE_NAME);
  if (amApplyTableTopPosPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": ApplyTableTopPosition button pressed";
    amApplyTableTopPosPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualApplyTablePositionReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutomaticManual ApplyTableTopPosition button pressed node
  QOpcUaNode* amApplyTableTopPosPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_AM_APPLYTABLEPOSITION_PRESSED_NODE_NAME);
  if (amApplyTableTopPosPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": ApplyTableTopPosition button pressed";
    amApplyTableTopPosPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualApplyNewCoordsPressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutomaticManual ApplyNewCoords button pressed node
  QOpcUaNode* amApplyNewCoordsPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_AM_APPLYNEWCOORDS_PRESSED_NODE_NAME);
  if (amApplyNewCoordsPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": ApplyNewCoords button pressed";
    amApplyNewCoordsPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualApplyNewCoordsReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutomaticManual ApplyNewCoords button pressed node
  QOpcUaNode* amApplyNewCoordsPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_AM_APPLYNEWCOORDS_PRESSED_NODE_NAME);
  if (amApplyNewCoordsPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": ApplyNewCoords button pressed";
    amApplyNewCoordsPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR1BreakTestPressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R1 BreakTest button pressed node
  QOpcUaNode* smR1BeakTestPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R1_BREAKTEST_PRESSED_NODE_NAME);
  if (smR1BeakTestPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R1 BreakTest button pressed";
    smR1BeakTestPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR1BreakTestReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R1 BreakTest button pressed node
  QOpcUaNode* smR1BeakTestPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R1_BREAKTEST_PRESSED_NODE_NAME);
  if (smR1BeakTestPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R1 BreakTest button released";
    smR1BeakTestPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR1MasterReferenceTestPressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R1 MasterReferenceTest button pressed node
  QOpcUaNode* smR1MasRefTestPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R1_MASREFTEST_PRESSED_NODE_NAME);
  if (smR1MasRefTestPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R1 MasterReferenceTest button pressed";
    smR1MasRefTestPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR1MasterReferenceTestReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R1 MasterReferenceTest button pressed node
  QOpcUaNode* smR1MasRefTestPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R1_MASREFTEST_PRESSED_NODE_NAME);
  if (smR1MasRefTestPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R1 MasterReferenceTest button released";
    smR1MasRefTestPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR1LoadPressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R1 Load button pressed node
  QOpcUaNode* smR1LoadPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R1_LOAD_PRESSED_NODE_NAME);
  if (smR1LoadPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R1 Load button pressed";
    smR1LoadPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR1LoadReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R1 Load button pressed node
  QOpcUaNode* smR1LoadPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R1_LOAD_PRESSED_NODE_NAME);
  if (smR1LoadPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R1 Load button released";
    smR1LoadPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR1ServicePos1Pressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R1 ServicePosition1 button pressed node
  QOpcUaNode* smR1ServicePos1PressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R1_SERVICEPOS1_PRESSED_NODE_NAME);
  if (smR1ServicePos1PressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R1 ServicePosition1 button pressed";
    smR1ServicePos1PressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR1ServicePos1Released()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R1 ServicePosition1 button pressed node
  QOpcUaNode* smR1ServicePos1PressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R1_SERVICEPOS1_PRESSED_NODE_NAME);
  if (smR1ServicePos1PressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R1 ServicePosition1 button released";
    smR1ServicePos1PressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR1ServicePos2Pressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R1 ServicePosition2 button pressed node
  QOpcUaNode* smR1ServicePos2PressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R1_SERVICEPOS2_PRESSED_NODE_NAME);
  if (smR1ServicePos2PressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R1 ServicePosition2 button pressed";
    smR1ServicePos2PressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR1ServicePos2Released()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R1 ServicePosition2 button pressed node
  QOpcUaNode* smR1ServicePos2PressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R1_SERVICEPOS2_PRESSED_NODE_NAME);
  if (smR1ServicePos2PressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R1 ServicePosition2 button released";
    smR1ServicePos2PressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR1ServicePos3Pressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R1 ServicePosition3 button pressed node
  QOpcUaNode* smR1ServicePos3PressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R1_SERVICEPOS3_PRESSED_NODE_NAME);
  if (smR1ServicePos3PressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R1 ServicePosition3 button pressed";
    smR1ServicePos3PressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR1ServicePos3Released()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R1 ServicePosition3 button pressed node
  QOpcUaNode* smR1ServicePos3PressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R1_SERVICEPOS3_PRESSED_NODE_NAME);
  if (smR1ServicePos3PressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R1 ServicePosition3 button released";
    smR1ServicePos3PressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR2BreakTestPressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R2 BreakTest button pressed node
  QOpcUaNode* smR2BeakTestPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R2_BREAKTEST_PRESSED_NODE_NAME);
  if (smR2BeakTestPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R2 BreakTest button pressed";
    smR2BeakTestPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR2BreakTestReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R2 BreakTest button pressed node
  QOpcUaNode* smR2BeakTestPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R2_BREAKTEST_PRESSED_NODE_NAME);
  if (smR2BeakTestPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R2 BreakTest button released";
    smR2BeakTestPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR2MasterReferenceTestPressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R2 MasterReferenceTest button pressed node
  QOpcUaNode* smR2MasRefTestPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R2_MASREFTEST_PRESSED_NODE_NAME);
  if (smR2MasRefTestPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R1 MasterReferenceTest button pressed";
    smR2MasRefTestPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR2MasterReferenceTestReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R2 MasterReferenceTest button pressed node
  QOpcUaNode* smR2MasRefTestPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R2_MASREFTEST_PRESSED_NODE_NAME);
  if (smR2MasRefTestPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R2 MasterReferenceTest button released";
    smR2MasRefTestPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR2HomePressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R2 Home button pressed node
  QOpcUaNode* smR2HomePressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R2_HOME_PRESSED_NODE_NAME);
  if (smR2HomePressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R2 Home button pressed";
    smR2HomePressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR2HomeReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R1 Load button pressed node
  QOpcUaNode* smR2HomePressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R2_HOME_PRESSED_NODE_NAME);
  if (smR2HomePressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R2 Home button released";
    smR2HomePressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR2ServicePos1Pressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R2 ServicePosition1 button pressed node
  QOpcUaNode* smR2ServicePos1PressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R2_SERVICEPOS1_PRESSED_NODE_NAME);
  if (smR2ServicePos1PressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R2 ServicePosition1 button pressed";
    smR2ServicePos1PressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR2ServicePos1Released()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R2 ServicePosition1 button pressed node
  QOpcUaNode* smR2ServicePos1PressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R2_SERVICEPOS1_PRESSED_NODE_NAME);
  if (smR2ServicePos1PressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R2 ServicePosition1 button released";
    smR2ServicePos1PressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR2ServicePos2Pressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R2 ServicePosition2 button pressed node
  QOpcUaNode* smR2ServicePos2PressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R2_SERVICEPOS2_PRESSED_NODE_NAME);
  if (smR2ServicePos2PressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R2 ServicePosition2 button pressed";
    smR2ServicePos2PressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR2ServicePos2Released()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R2 ServicePosition2 button pressed node
  QOpcUaNode* smR2ServicePos2PressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R2_SERVICEPOS2_PRESSED_NODE_NAME);
  if (smR2ServicePos2PressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R2 ServicePosition2 button released";
    smR2ServicePos2PressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR2ServicePos3Pressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R2 ServicePosition3 button pressed node
  QOpcUaNode* smR2ServicePos3PressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R2_SERVICEPOS3_PRESSED_NODE_NAME);
  if (smR2ServicePos3PressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R2 ServicePosition3 button pressed";
    smR2ServicePos3PressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR2ServicePos3Released()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R2 ServicePosition3 button pressed node
  QOpcUaNode* smR2ServicePos3PressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R2_SERVICEPOS3_PRESSED_NODE_NAME);
  if (smR2ServicePos3PressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": R2 ServicePosition3 button released";
    smR2ServicePos3PressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceRestartSmPressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service RestartServiceMode button pressed node
  QOpcUaNode* smRestartSmPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_RESTARTSM_PRESSED_NODE_NAME);
  if (smRestartSmPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": RestartServiceMode button pressed";
    smRestartSmPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceRestartSmReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service RestartServiceMode button pressed node
  QOpcUaNode* smRestartSmPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_RESTARTSM_PRESSED_NODE_NAME);
  if (smRestartSmPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": RestartServiceMode button released";
    smRestartSmPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onKukaAllowT1Pressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // KUKA Controllers AllowT1 button pressed node
  QOpcUaNode* kcmAllowT1PressedNode = d->FindNodeFromFullDisplayName(BUTTONS_KCM_ALLOWT1_PRESSED_NODE_NAME);
  if (kcmAllowT1PressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": AllowT1 button pressed";
    kcmAllowT1PressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onKukaAllowT1Released()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // KUKA Controllers AllowT1 button pressed node
  QOpcUaNode* kcmAllowT1PressedNode = d->FindNodeFromFullDisplayName(BUTTONS_KCM_ALLOWT1_PRESSED_NODE_NAME);
  if (kcmAllowT1PressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": AllowT1 button released";
    kcmAllowT1PressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onKukaAllowXrayPressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // KUKA Controllers AllowXRay button pressed node
  QOpcUaNode* kcmAllowXrayPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_KCM_ALLOWXRAY_PRESSED_NODE_NAME);
  if (kcmAllowXrayPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": AllowXRay button pressed";
    kcmAllowXrayPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onKukaAllowXrayReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // KUKA Controllers AllowXRay button pressed node
  QOpcUaNode* kcmAllowXrayPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_KCM_ALLOWXRAY_PRESSED_NODE_NAME);
  if (kcmAllowXrayPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": AllowXRay button released";
    kcmAllowXrayPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

void qSlicerSiemensPlcOpcUaWidget::onKukaAllowBeamPressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // KUKA Controllers AllowBeam button pressed node
  QOpcUaNode* kcmAllowBeamPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_KCM_ALLOWBEAM_PRESSED_NODE_NAME);
  if (kcmAllowBeamPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": AllowBeam button pressed";
    kcmAllowBeamPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onKukaAllowBeamReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // KUKA Controllers AllowXRay button pressed node
  QOpcUaNode* kcmAllowBeamPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_KCM_ALLOWBEAM_PRESSED_NODE_NAME);
  if (kcmAllowBeamPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": AllowBeam button released";
    kcmAllowBeamPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onResetErrorsPressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Reset errors button pressed node
  QOpcUaNode* resetErrorsPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_RESETERRORS_PRESSED_NODE_NAME);
  if (resetErrorsPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": ResetErrors pressed";
    resetErrorsPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onResetErrorsReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutoManual R1 ToLoad button pressed node
  QOpcUaNode* resetErrorsPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_RESETERRORS_PRESSED_NODE_NAME);
  if (resetErrorsPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": ResetErrors released";
    resetErrorsPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onMakeXrayPressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // MakeXRay button pressed node
  QOpcUaNode* makeXrayPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_MAKEXRAY_PRESSED_NODE_NAME);
  if (makeXrayPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": MakeXRay pressed";
    makeXrayPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onMakeXrayReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // MakeXRay button pressed node
  QOpcUaNode* makeXrayPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_MAKEXRAY_PRESSED_NODE_NAME);
  if (makeXrayPressedNode)
  {
    qDebug() << Q_FUNC_INFO << ": MakeXRay released";
    makeXrayPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServerInterfacesRead(QOpcUa::NodeAttributes attr)
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);

  if (attr & QOpcUa::NodeAttribute::DisplayName)
  { // Make sure the value attribute has been read
    QString nodeDisplayName = d->ServerInterfacesNode->attribute(QOpcUa::NodeAttribute::DisplayName).value<QOpcUaLocalizedText>().text();
    bool res = d->ParseServerParentNode(nodeDisplayName);
    if (res)
    {
      for (auto& nodeData : d->NodeNameDataMap)
      {
        if (nodeData.getNode())
        {
//          qDebug() << Q_FUNC_INFO << "Node ID:" << nodeData.getNodeId() \
            << ", Name:" << nodeData.getNodeName() \
            << ", Description:" << nodeData.getNodeDescription() << Qt::endl;
        }
      }
    }
    bool coords = d->ConnectSetCoordsNodes();
    if (coords)
    {
      qDebug() << Q_FUNC_INFO << "SetCoords nodes are OK";
    }

    bool resMsg = d->ConnectMessagesNodes();
    bool resStatus = d->ConnectModeAndStatusNodes();
    if (resMsg && resStatus)
    {
      qDebug() << Q_FUNC_INFO << "Messages and Status nodes are OK";
    }

    bool moveButtons = d->ConnectAutoManualMovementNodes();
    if (moveButtons)
    {
      qDebug() << Q_FUNC_INFO << "AutoManual move buttons nodes are OK";
    }

    moveButtons &= d->ConnectServiceMovementNodes();
    if (moveButtons)
    {
      qDebug() << Q_FUNC_INFO << "AutoManual & Service move buttons nodes are OK";
    }

    moveButtons &= d->ConnectKukaMovementNodes();
    if (moveButtons)
    {
      qDebug() << Q_FUNC_INFO << "Move buttons nodes are OK";
    }

    bool readynessFlags = d->ConnectFlagNodes();
    readynessFlags &= d->ConnectPatientOnTableTopNode();
    readynessFlags &= d->ConnectTablePositionNode();
    readynessFlags &= d->ConnectEmergencyEvacuationSignalNode();
    if (readynessFlags)
    {
      qDebug() << Q_FUNC_INFO << "Readyness flags nodes are OK";
    }

    bool resetErrors = d->ConnectResetErrorsNodes();
    bool makeXray = d->ConnectMakeXrayNodes();
    if (resetErrors && makeXray)
    {
      qDebug() << Q_FUNC_INFO << "Reset errors and Make X-Ray nodes are OK";
    }

    bool currentCoords = d->ConnectCurrentCoordsR1Nodes();
    currentCoords &= d->ConnectCurrentCoordsR2Nodes();
    if (currentCoords)
    {
      qDebug() << Q_FUNC_INFO << "R1 R2 Current coords nodes are OK";
    }

    bool reflTargetCoords = d->ConnectReflTargetCoordsR1Nodes();
    reflTargetCoords &= d->ConnectReflTargetCoordsR2Nodes();
    if (reflTargetCoords)
    {
      qDebug() << Q_FUNC_INFO << "R1 R2 Reflective target coords nodes are OK";
    }

  }
  d->PushButton_ReadStatusAndMessages->setEnabled(true);
  d->ParameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onReadStatusAndMessagesClicked()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  for (const QString& nodeName : d->ReadValueFullNodeNames)
  {
    QOpcUaNode* readValueNode = d->FindNodeFromFullDisplayName(nodeName);
    if (readValueNode)
    {
      readValueNode->readAttributes(QOpcUa::NodeAttribute::Value);
    }
  }
  d->ParameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onSetCoordXClicked()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Coord from ASU X node
  QOpcUaNode* coordXNode = d->FindNodeFromFullDisplayName(ASU_COORDS_X_NODE_NAME);
  double coord = d->DoubleSpinBox_CoordX->value();
  int32_t x = static_cast< int32_t >(coord * 1000.);
  if (coordXNode)
  {
    qDebug() << Q_FUNC_INFO << ": SetCoords X:" << x;
    coordXNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(x), QOpcUa::Types::Int32);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onSetCoordYClicked()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Coord from ASU Y node
  QOpcUaNode* coordYNode = d->FindNodeFromFullDisplayName(ASU_COORDS_Y_NODE_NAME);
  double coord = d->DoubleSpinBox_CoordY->value();
  int32_t y = static_cast< int32_t >(coord * 1000.);
  if (coordYNode)
  {
    qDebug() << Q_FUNC_INFO << ": SetCoords Y:" << y;
    coordYNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(y), QOpcUa::Types::Int32);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onSetCoordZClicked()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Coord from ASU Z node
  QOpcUaNode* coordZNode = d->FindNodeFromFullDisplayName(ASU_COORDS_Z_NODE_NAME);
  double coord = d->DoubleSpinBox_CoordZ->value();
  int32_t z = static_cast< int32_t >(coord * 1000.);
  if (coordZNode)
  {
    qDebug() << Q_FUNC_INFO << ": SetCoords Z:" << z;
    coordZNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(z), QOpcUa::Types::Int32);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onSetCoordAClicked()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Coord from ASU A node
  QOpcUaNode* angleANode = d->FindNodeFromFullDisplayName(ASU_COORDS_A_NODE_NAME);
  double angle = d->DoubleSpinBox_AngleA->value();
  int32_t a = static_cast< int32_t >(angle * 1000.);
  if (angleANode)
  {
    qDebug() << Q_FUNC_INFO << ": SetCoords A:" << a;
    angleANode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(a), QOpcUa::Types::Int32);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onSetCoordBClicked()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Coord from ASU B node
  QOpcUaNode* angleBNode = d->FindNodeFromFullDisplayName(ASU_COORDS_B_NODE_NAME);
  double angle = d->DoubleSpinBox_AngleB->value();
  int32_t b = static_cast< int32_t >(angle * 1000.);
  if (angleBNode)
  {
    qDebug() << Q_FUNC_INFO << ": SetCoords B:" << b;
    angleBNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(b), QOpcUa::Types::Int32);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onSetCoordCClicked()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Coord from ASU C node
  QOpcUaNode* angleCNode = d->FindNodeFromFullDisplayName(ASU_COORDS_C_NODE_NAME);
  double angle = d->DoubleSpinBox_AngleC->value();
  int32_t c = static_cast< int32_t >(angle * 1000.);
  if (angleCNode)
  {
    qDebug() << Q_FUNC_INFO << ": SetCoords C:" << c;
    angleCNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(c), QOpcUa::Types::Int32);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onSetCoordsClicked()
{
  this->onSetCoordXClicked();
  this->onSetCoordYClicked();
  this->onSetCoordZClicked();
  this->onSetCoordAClicked();
  this->onSetCoordBClicked();
  this->onSetCoordCClicked();
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onTablePositionChanged(QAbstractButton* aButton)
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  QRadioButton* rbutton = qobject_cast< QRadioButton* >(aButton);
  if (rbutton == d->RadioButton_PositionNotDefined)
  {
    qDebug() << Q_FUNC_INFO << "undef";
  }
  else if (rbutton == d->RadioButton_Position1)
  {
    qDebug() << Q_FUNC_INFO << "1";
  }
  else if (rbutton == d->RadioButton_Position2)
  {
    qDebug() << Q_FUNC_INFO << "2";
  }
  else if (rbutton == d->RadioButton_Position3)
  {
    qDebug() << Q_FUNC_INFO << "3";
  }
  else
  {
    qDebug() << Q_FUNC_INFO << "wrong";
  }
}
