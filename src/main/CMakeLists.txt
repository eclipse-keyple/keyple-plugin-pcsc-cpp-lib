#/*************************************************************************************************
# * Copyright (c) 2023 Calypso Networks Association https://calypsonet.org/                       *
# *                                                                                               *
# * See the NOTICE file(s) distributed with this work for additional information regarding        *
# * copyright ownership.                                                                          *
# *                                                                                               *
# * This program and the accompanying materials are made available under the terms of the Eclipse *
# * Public License 2.0 which is available at http://www.eclipse.org/legal/epl-2.0                 *
# *                                                                                               *
# * SPDX-License-Identifier: EPL-2.0                                                              *
# *************************************************************************************************/

SET(LIBRARY_NAME keyplepluginpcsccpplib)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DKEYPLEPLUGINPCSC_EXPORT")

ADD_LIBRARY(

    ${LIBRARY_NAME}

    ${LIBRARY_TYPE}

    ${CMAKE_CURRENT_SOURCE_DIR}/AbstractPcscPluginAdapter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/AbstractPcscReaderAdapter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PcscPluginAdapter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PcscPluginFactoryAdapter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PcscPluginFactoryBuilder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PcscReaderAdapter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PcscReader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PcscSupportedContactProtocol.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PcscSupportedContactlessProtocol.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cpp/CardTerminal.cpp
)

TARGET_INCLUDE_DIRECTORIES(

    ${LIBRARY_NAME}

    PUBLIC

    ${CMAKE_CURRENT_SOURCE_DIR}
    cpp
    cpp/exception
)

IF(APPLE)                                                                                           
        FIND_LIBRARY(PCSC PCSC)                                                                     
ELSEIF(UNIX)                                                                                            
        FIND_LIBRARY(PCSC pcsclite)                                                                 
ELSEIF(WIN32)                                                                                           
        SET(CMAKE_FIND_LIBRARY_PREFIXES "")                                                         
        SET(CMAKE_FIND_LIBRARY_SUFFIXES ".dll")                                                     
        SET(CMAKE_BUILD_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../../../../..")                     
        SET(PCSC winscard.lib)                                                                      
ENDIF(APPLE)                                                                                        
                                                                                                    
IF(NOT PCSC)                                                                                        
        MESSAGE(FATAL_ERROR "PC/SC framework/library not found")                                    
ENDIF() 

TARGET_LINK_LIBRARIES(
        
    ${LIBRARY_NAME} 
    
    PUBLIC
    
    ${PCSC}
    Keyple::CommonApi
    Keyple::PluginApi
    Keyple::Util
)

ADD_LIBRARY(Keyple::PluginPcsc ALIAS ${LIBRARY_NAME})
