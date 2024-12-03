#-----------------------------------------------------------------------------
# Add models
#-----------------------------------------------------------------------------

# Geometry based on KUKA KR 210 r2700 extra document
set(MACHINE_NAME Cabin26AGeometry)

set(MACHINE_MODELS
  Cabin26AGeometry.json
  FixedReference.stl
  TableTop.stl
  Flange.stl
  RobotBaseFixed.stl
  RobotBaseRotation.stl
  RobotShoulder.stl
  RobotElbow.stl
  RobotWrist.stl
)


SlicerMacroConfigureTreatmentMachineComponentModels(
  NAME ${MACHINE_NAME}
  MODELS ${MACHINE_MODELS}
)
