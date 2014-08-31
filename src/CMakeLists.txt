################### File Configurations #######################
SET(scram_install_dir "${CMAKE_INSTALL_PREFIX}")
CONFIGURE_FILE(
    "${CMAKE_CURRENT_SOURCE_DIR}/env.cc.in"
    "${CMAKE_CURRENT_SOURCE_DIR}/env.cc"
    @ONLY
    )
################### End File Configurations ####################

set(SCRAM_CORE_SRC ${SCRAM_CORE_SRC}
  "${CMAKE_CURRENT_SOURCE_DIR}/risk_analysis.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/fault_tree.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/fault_tree_analysis.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/superset.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/error.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/event.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/gate.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/primary_event.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/xml_parser.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/relax_ng_validator.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/grapher.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/reporter.cc"
  "${CMAKE_CURRENT_SOURCE_DIR}/env.cc"
  )
add_library(scramcore ${SCRAM_CORE_SRC})

set_target_properties(scramcore
  PROPERTIES
  INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib"
  INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib"
  )

target_link_libraries(scramcore dl ${LIBS})

install(
  TARGETS scramcore
  LIBRARY DESTINATION lib
  COMPONENT scramcore
  )

add_executable(scram
  scram.cc
  )
target_link_libraries(scram scramcore ${Boost_LIBRARIES})

install(TARGETS scram
  RUNTIME DESTINATION bin
  COMPONENT scram
  )