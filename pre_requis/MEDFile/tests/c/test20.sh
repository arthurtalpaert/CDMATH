#!/bin/sh

file=test20
${CP} ${BUILDDIR}/test2.med ${BUILDDIR}/test20-0.med
${CP} ${SRCDIR}/dumps.ref/test10-med.hdf ${BUILDDIR}/test10-med.hdf
${CP} ${SRCDIR}/dumps.ref/test2-med.hdf ${BUILDDIR}/test2-med.hdf
test "x${CPYMED}" = "xyes" && ${CP} ${SRCDIR}/test.med.ref ${BUILDDIR}/${file}.med && chmod 644 ${BUILDDIR}/${file}.med
output=/dev/null && test "x${OUTPUT}" = "xyes" && output=${file}.out
${CHECKER} ${EXECDIR}/${file} > ${output} 2>&1 && ${H5DUMP} ${BUILDDIR}/${file}.med > ${BUILDDIR}/${file}.dump  && ( \
sed -e 's/H5T_STD_I32BE//g' -e 's/H5T_STD_I32LE//g'	 \
    -e 's/H5T_STD_I64BE//g' -e 's/H5T_STD_I64LE//g'  \
    -e 's/H5T_IEEE_F64BE//g' -e 's/H5T_IEEE_F64LE//g'  \
    -e 's/H5T_STD_I8LE//g' -e 's/H5T_STD_I8BE//g' -e 's/H5T_STD_U8BE//g'  \
    -e 's/HDF5.*{//g' \
	${BUILDDIR}/${file}.dump > ${BUILDDIR}/${file}.dump.tmp && \
sed -e 's/H5T_STD_I32BE//g' -e 's/H5T_STD_I32LE//g'	 \
    -e 's/H5T_STD_I64BE//g' -e 's/H5T_STD_I64LE//g'  \
    -e 's/H5T_IEEE_F64BE//g' -e 's/H5T_IEEE_F64LE//g'  \
    -e 's/H5T_STD_I8LE//g' -e 's/H5T_STD_I8BE//g' -e 's/H5T_STD_U8BE//g'  \
    -e 's/HDF5.*{//g' \
	 ${BUILDDIR}/dumps.ref/${file}.dump > ${BUILDDIR}/${file}.dump.ref && \
diff ${BUILDDIR}/${file}.dump.ref ${BUILDDIR}/${file}.dump.tmp && \
rm -f ${BUILDDIR}/${file}.dump.tmp && rm -f ${BUILDDIR}/${file}.dump.ref )

