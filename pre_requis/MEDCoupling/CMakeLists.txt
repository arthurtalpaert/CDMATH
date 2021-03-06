# Copyright (C) 2012-2014  CEA/DEN, EDF R&D
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
# See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#
# Author : Anthony Geay (CEA/DEN)

INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_BINARY_DIR}/../..
  ${CMAKE_CURRENT_SOURCE_DIR}/../INTERP_KERNEL
  ${CMAKE_CURRENT_SOURCE_DIR}/../INTERP_KERNEL/Bases
  ${CMAKE_CURRENT_SOURCE_DIR}/../INTERP_KERNEL/Geometric2D
  ${CMAKE_CURRENT_SOURCE_DIR}/../INTERP_KERNEL/ExprEval
  ${CMAKE_CURRENT_SOURCE_DIR}/../INTERP_KERNEL/GaussPoints
  )

SET(medcoupling_SOURCES
  MEDCouplingField.cxx
  MEDCouplingFieldDouble.cxx
  MEDCouplingUMesh.cxx
  MEDCoupling1GTUMesh.cxx
  MEDCouplingMemArray.cxx
  MEDCouplingMemArrayChar.cxx
  MEDCouplingTimeLabel.cxx
  MEDCouplingCMesh.cxx
  MEDCouplingIMesh.cxx
  MEDCouplingCurveLinearMesh.cxx
  MEDCouplingStructuredMesh.cxx
  MEDCouplingTimeDiscretization.cxx
  MEDCouplingFieldDiscretization.cxx
  MEDCouplingRefCountObject.cxx
  MEDCouplingPointSet.cxx
  MEDCouplingFieldTemplate.cxx
  MEDCouplingExtrudedMesh.cxx
  MEDCouplingMesh.cxx
  MEDCouplingGaussLocalization.cxx
  MEDCouplingNatureOfField.cxx
  MEDCouplingMultiFields.cxx
  MEDCouplingDefinitionTime.cxx
  MEDCouplingFieldOverTime.cxx
  MEDCouplingCartesianAMRMesh.cxx
  MEDCouplingAMRAttribute.cxx
  MEDCouplingMatrix.cxx
  )

SET(medcouplingremapper_SOURCES
  MEDCouplingRemapper.cxx
  )

ADD_LIBRARY(medcoupling SHARED ${medcoupling_SOURCES})
TARGET_LINK_LIBRARIES(medcoupling interpkernel)
INSTALL(TARGETS medcoupling DESTINATION lib)

ADD_LIBRARY(medcouplingremapper SHARED ${medcouplingremapper_SOURCES})
TARGET_LINK_LIBRARIES(medcouplingremapper medcoupling)
INSTALL(TARGETS medcouplingremapper DESTINATION lib)

FILE(GLOB medcoupling_HEADERS_HXX "${CMAKE_CURRENT_SOURCE_DIR}/*.hxx")
FILE(GLOB medcoupling_HEADERS_TXX "${CMAKE_CURRENT_SOURCE_DIR}/*.txx")
INSTALL(FILES ${medcoupling_HEADERS_HXX} ${medcoupling_HEADERS_TXX} MEDCouplingNatureOfFieldEnum DESTINATION include)
