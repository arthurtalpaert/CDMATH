INCLUDE_DIRECTORIES(
  ${PROJECT_BINARY_DIR}/include
  ${PROJECT_SOURCE_DIR}/include
)

SET(testf_SOURCES test1.f test2.f test3.f test4.f test5.f90 test6.f test7.f90 test8.f test9.f90 test10.f test11.f90 test11b.f90 test12.f test13.f90 test14.f test15.f90 test16.f test17.f90 test18.f test20.f test21.f test22.f test23.f test24.f test25.f test26.f test27.f test28.f test29.f test30.f90 test31.f test32.f test33.f)

FOREACH(testf_SOURCE ${testf_SOURCES})
  GET_FILENAME_COMPONENT(testf_BASE ${testf_SOURCE} NAME_WE)
  # Shared lib tests
  IF(MEDFILE_BUILD_SHARED_LIBS)
    SET(testf_SHARED_BIN "${testf_BASE}f")
    ADD_EXECUTABLE(${testf_SHARED_BIN} ${testf_SOURCE})
    TARGET_LINK_LIBRARIES(${testf_SHARED_BIN} med)
    SET_TARGET_PROPERTIES(${testf_SHARED_BIN} PROPERTIES OUTPUT_NAME ${testf_BASE})
    GET_TARGET_PROPERTY(TMP ${testf_SHARED_BIN} OUTPUT_NAME)
    INSTALL(TARGETS ${testf_SHARED_BIN} DESTINATION bin/testf)
    ADD_TEST("Test${testf_SHARED_BIN}" ${TMP})
  ENDIF()
  
  # Static lib tests
  IF(MEDFILE_BUILD_STATIC_LIBS)
    SET(testf_STATIC_BIN "${testf_BASE}f_static")
    ADD_EXECUTABLE(${testf_STATIC_BIN} ${testf_SOURCE})
    TARGET_LINK_LIBRARIES(${testf_STATIC_BIN} med_static)
    SET_TARGET_PROPERTIES(${testf_STATIC_BIN} PROPERTIES OUTPUT_NAME "${testf_BASE}_static")
    GET_TARGET_PROPERTY(TMP ${testf_STATIC_BIN} OUTPUT_NAME)
    INSTALL(TARGETS ${testf_STATIC_BIN} DESTINATION bin/testf)
    ADD_TEST("Test${testf_STATIC_BIN}" ${TMP})
  ENDIF()
    
ENDFOREACH(testf_SOURCE ${testsf_BINARIES})
