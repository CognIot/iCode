#use the shared library
set (PROJECT_LINK_LIBS librfal_lib.so)
link_directories(../build/rfal)

#Bring headers into project
include_directories(../common/utils/Inc ../platform/Inc ../rfal/Inc ../rfal/Src/st25r3911 /usr/include nfc_poller/Inc)

#sources
set (SOURCES nfc_poller/Src/main.c nfc_poller/Src/exampleRfalPoller.c nfc_poller/Src/logger.c)
add_executable(nfcPoller ${SOURCES})

target_link_libraries(nfcPoller ${PROJECT_LINK_LIBS})


