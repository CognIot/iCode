#use the shared library
set (PROJECT_LINK_LIBS librfal_lib.so)
link_directories(../build/rfal)

#Bring headers into project
include_directories(../common/utils/Inc ../platform/Inc ../rfal/Inc ../rfal/Src/st25r3911 /usr/include iCodeDemo/Inc)

#sources
set (SOURCES iCodeDemo/Src/exampleNFC.c iCodeDemo/Src/logger.c)
add_executable(exampleNFC ${SOURCES})

target_link_libraries(exampleNFC ${PROJECT_LINK_LIBS})


