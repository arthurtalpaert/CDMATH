INCLUDE_DIRECTORIES(
${BASE_DIR}/inc 
${MED_COUPLING_DIR}
${INTERP_KERNEL_DIR}
${INTERP_KERNEL_DIR}/Bases
)

FILE(GLOB src_base_CXX . *.cxx)
ADD_LIBRARY(base SHARED ${src_base_CXX})

INSTALL(TARGETS base DESTINATION lib)
