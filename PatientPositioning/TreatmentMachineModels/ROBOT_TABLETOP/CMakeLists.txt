#-----------------------------------------------------------------------------
# Add models
#-----------------------------------------------------------------------------

set(MACHINE_NAME IHEP_ROBOT_TABLETOP)

set(MACHINE_MODELS
  ROBOT_TABLETOP.json
  FixedReference.stl
  RobotBaseFixed.stl
  RobotBaseRotation.stl
  RobotShoulder.stl
  RobotElbow.stl
  RobotWrist.stl
  TableTop.stl
)


SlicerMacroConfigureTreatmentMachineComponentModels(
  NAME ${MACHINE_NAME}
  MODELS ${MACHINE_MODELS}
)
