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

#ifndef __qSlicerOpcUaRobotsMessagesListModel_h
#define __qSlicerOpcUaRobotsMessagesListModel_h

#include "qSlicerU70RoomOpcUaModuleWidgetsExport.h"

// SlicerQt includes
#include "qSlicerObject.h"

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// SlicerRT ScadaOpcUa MRML includes
#include <vtkMRMLScadaOpcUaNode.h>

// Qt
#include <QAbstractListModel>
#include <QIcon>
#include <QString>
#include <QVector>

// FooBar Widgets includes
#include "qSlicerU70RoomOpcUaModuleWidgetsExport.h"

class Q_SLICER_MODULE_U70ROOMOPCUA_WIDGETS_EXPORT qSlicerOpcUaRobotsMessagesListModel : public QAbstractListModel
{
  Q_OBJECT
public:
  static constexpr size_t MESSAGES_SIZE = 64;
  struct ListItem
  {
    QIcon icon;
    QString text;
  };

  explicit qSlicerOpcUaRobotsMessagesListModel(QObject *parent = nullptr) : QAbstractListModel(parent) {}

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index, int role) const override;
  QHash<int, QByteArray> roleNames() const override;
  // Public API to fill/modify the model
  void addItem(const QIcon &icon, const QString &text);
  void updateErrorBits(quint64 errorMessages);
  void updateServiceBits(quint64 serviceMessages);
  void updateMiscBits(quint64 miscMessages);
  void updateMessagesBits(quint64 errorMessages, quint64 serviceMessages, quint64 miscMessages);
  void clear();

private:
  static constexpr const char* const ERROR_MESSAGES[MESSAGES_SIZE] = {
    QT_TR_NOOP("PROFISAFE error on robot R1 module (Tabletop)"),
    QT_TR_NOOP("Robot R1 error: USER_SAFE signal lost during program execution"),
    QT_TR_NOOP("Robot R2 error: USER_SAFE signal lost during program execution"),
    QT_TR_NOOP("Emergency trip of a power circuit breaker in the control cabinet"),
    QT_TR_NOOP("Power circuit breaker in the control cabinet is tripped/off"),
    QT_TR_NOOP("Control cabinet switch error"),
    QT_TR_NOOP("Digital input module DI1 error"),
    QT_TR_NOOP("Digital input module DI2 error"),
    QT_TR_NOOP("Digital output module DO1 error"),
    QT_TR_NOOP("Digital output module DO2 error"),
    QT_TR_NOOP("Digital output module DO3 error"),
    QT_TR_NOOP("Hardware safety circuit not closed (cable-pull switch triggered, emergency stop button pressed, or emergency stop signal received from the process control system)"),
    QT_TR_NOOP("KUKA R1: Emergency stop button on KUKA's smartPAD is pressed"),
    QT_TR_NOOP("KUKA R2: Emergency stop button on KUKA's smartPAD is pressed"),
    QT_TR_NOOP("KUKA R1: \"ALARM_STOP\" signal = FALSE"),
    QT_TR_NOOP("KUKA R2: \"ALARM_STOP\" signal = FALSE"),
    QT_TR_NOOP("KUKA R1: Robot error (details on KUKA's smartPAD)"),
    QT_TR_NOOP("KUKA R2: Robot error (details on KUKA's smartPAD)"),
    QT_TR_NOOP("KUKA R1: Resuming robot program execution is unsafe; robot is not on the path (ON_PATH signal missing)"),
    QT_TR_NOOP("KUKA R2: Resuming robot program execution is unsafe; robot is not on the path (ON_PATH signal missing)"),
    QT_TR_NOOP("KUKA R1: To start automatic operation, robot R1 (Tabletop) must be moved to the home position."),
    QT_TR_NOOP("KUKA R2: To start automatic operation, robot R2 (C-arm) must be moved to the home position."),
    QT_TR_NOOP("PROFISAFE error: R2 robot module (C-arm)"),
    QT_TR_NOOP("KUKA R2: Communication error with R2 robot (C-arm)"),
    QT_TR_NOOP("KUKA R1: Communication error with R1 robot (Tabletop)"),
    QT_TR_NOOP("Emergency stop"),
    QT_TR_NOOP("Emergency stop button pressed in the control room"),
    QT_TR_NOOP("Emergency stop button pressed in the robot room"),
    QT_TR_NOOP("Local emergency stop signal received from the Tabletop robot"),
    QT_TR_NOOP("Local emergency stop signal received from the C-arm robot"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)"),
    QT_TR_NOOP("error array (from RTC to APCS)")
  };

  static constexpr const char* const SERVICE_MESSAGES[MESSAGES_SIZE] = {
    QT_TR_NOOP("A brake test is required for robot R1 (Table top)"),
    QT_TR_NOOP("A brake test is required for robot R2 (C‑arm)"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages"),
    QT_TR_NOOP("array of service messages")
  };

  static constexpr const char* const MESSAGES[MESSAGES_SIZE] = {
    QT_TR_NOOP("To operate in manual control mode from the PC workstation, both robots must be switched to EXT mode."),
    QT_TR_NOOP("KUKA R1: To start this program, the robot with the table must be moved to the home position."),
    QT_TR_NOOP("KUKA R2: To start this program, the robot with the table must be moved to the home position."),
    QT_TR_NOOP("To operate in service mode, both robots must be switched to EXT mode."),
    QT_TR_NOOP("Robot R1 is not ready for service mode operation."),
    QT_TR_NOOP("Robot R2 is not ready for service mode operation."),
    QT_TR_NOOP("To continue program execution, press and hold the motion confirmation button."),
    QT_TR_NOOP("KUKA R1 (Diagnostic message): Permission to continue program execution is missing."),
    QT_TR_NOOP("KUKA R2 (Diagnostic message): Permission to continue program execution is missing."),
    QT_TR_NOOP("Robot movement is not possible due to an unsafe collimator position."),
    QT_TR_NOOP("To operate using KUKA handheld controllers, both robots must be switched to T1 mode."),
    QT_TR_NOOP("The program on ROBOT R1 (Tabletop) must be cancelled."),
    QT_TR_NOOP("The program on ROBOT R2 (C-arm) must be cancelled."),
    QT_TR_NOOP("Robot R1 (Tabletop) must be moved to the HOME position."),
    QT_TR_NOOP("Robot R2 (C-arm) must be moved to the HOME position."),
    QT_TR_NOOP("Emergency evacuation program successfully completed."),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages"),
    QT_TR_NOOP("array of messages")
  };

  void updateItemsList();
  QVector< ListItem > messages;
  quint64 errorMessages;
  quint64 serviceMessages;
  quint64 miscMessages;
};

#endif
