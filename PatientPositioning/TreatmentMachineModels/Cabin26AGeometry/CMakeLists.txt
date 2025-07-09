#-----------------------------------------------------------------------------
# Add models
#-----------------------------------------------------------------------------

# Geometry based on KUKA KR 210 r2700 extra document
set(MACHINE_NAME Cabin26AGeometry)

set(MACHINE_MODELS
  Cabin26AGeometry.json
  FixedReference.stl
  TableTop.stl
  TableFlange.stl
  TableRobotBaseFixed.stl
  TableRobotBaseRotation.stl
  TableRobotShoulder.stl
  TableRobotElbow.stl
  TableRobotWrist.stl
  CArmRobotBaseFixed.stl
  CArmRobotBaseRotation.stl
  CArmRobotShoulder.stl
  CArmRobotElbow.stl
  CArmRobotWrist.stl
  CArm.stl
  XrayImager.stl
  XrayImageReceptor.stl
  KR210_R3100-2_FixedReference_A1.stl
  KR210_R3100-2_A1_A2.stl
  KR210_R3100-2_A2_A3.stl
  KR210_R3100-2_A1_A2.stl
  KR210_R3100-2_A4_A5.stl
  KR210_R3100-2_A5_A6.stl
  KR210_R3100-2_A6_Flange.stl
)


SlicerMacroConfigureTreatmentMachineComponentModels(
  NAME ${MACHINE_NAME}
  MODELS ${MACHINE_MODELS}
)
