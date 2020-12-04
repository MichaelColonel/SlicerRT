#-----------------------------------------------------------------------------
# Add models
#-----------------------------------------------------------------------------

set(MACHINE_NAME IHEPStand)

set(MACHINE_MODELS
  Canyon.stl
  PatientSupportRotation.stl
  TableTopStand.stl
  TableTop.stl
)

SlicerMacroConfigureTreatmentMachineComponentModels(
  NAME ${MACHINE_NAME}
  MODELS ${MACHINE_MODELS}
)
