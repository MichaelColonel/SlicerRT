#-----------------------------------------------------------------------------
# Add models
#-----------------------------------------------------------------------------

# Geometry based on KUKA KR 210 r2700 extra document
set(MACHINE_NAME Channel26Cabin3Geometry)

set(MACHINE_MODELS
  Channel26Cabin3Geometry.json
  TableTop.stl
  TableRobotFlange.stl
  TableFlange.stl
  TableRobotWrist.stl
  TableRobotElbowWrist.stl
  TableRobotElbowShoulder.stl
  KR210_R3100-2_A5_A6.stl
  KR210_R3100-2_A6_Flange.stl
  FixedReference.stl
)


SlicerMacroConfigureTreatmentMachineComponentModels(
  NAME ${MACHINE_NAME}
  MODELS ${MACHINE_MODELS}
)
