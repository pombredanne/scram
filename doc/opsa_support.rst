.. _opsa_support:

########################################
Currently Supported OpenPSA MEF Features
########################################

- Label for elements

- Attributes list

- Fault Tree Description:

  * Non-nested gates (formula)

- Basic Event Description:

  * Float probability

- House Event Description:

  * Boolean probability description

- Model data


*****************************************
Deviations from OpenPSA MEF
*****************************************

- Names are not case-sensitive.
- House events do not default to false state implicitly. They must be defined.
- First gate for the fault tree must be a top event gate.
- Fault tree input structure must be top-down. Parents must appear before
  children.
- The correct number of a gate's children is required.
- Unused primary events are ignored but reported as warning.