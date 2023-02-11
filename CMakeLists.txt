include_directories(
  /usr/share/arduino/hardware/arduino/avr/cores/arduino/
  /usr/lib/avr/include/
  /usr/share/arduino/hardware/arduino/avr/variants/standard/
#   /home/obrol/.platformio/packages/framework-cmsis/CMSIS/RTOS/Template/CPP/
#   /home/obrol/.platformio/packages/framework-cmsis/CMSIS/RTOS/Template/
#   /home/obrol/.platformio/packages/framework-arduinoadafruitnrf52/variants/itsybitsy_nrf52840_express/
#   /home/obrol/study/firmware/src/
#   /home/obrol/study/firmware/.pio/libdeps/feather_nrf52832/Thread/
#   /home/obrol/study/firmware/.pio/libdeps/feather_nrf52832/Nanopb/
#   /home/obrol/study/firmware/src/mesh/generated/
#   /home/obrol/study/firmware/src/mesh/
#   /home/obrol/.platformio/packages/framework-arduinoadafruitnrf52/cores/nRF5/
#   /home/obrol/.platformio/packages/framework-arduinoadafruitnrf52/cores/nRF5/nordic/nrfx/mdk/
#   /home/obrol/.platformio/packages/framework-arduinoadafruitnrf52/cores/nRF5/nordic/softdevice/s140_nrf52_6.1.1_API/include/
#   /home/obrol/.platformio/packages/framework-arduinoadafruitnrf52/cores/nRF5/nordic/nrfx/hal/
#   /home/obrol/.platformio/packages/framework-arduinoadafruitnrf52/cores/nRF5/nordic/nrfx/
#   /home/obrol/.platformio/packages/framework-arduinoadafruitnrf52/cores/nRF5/nordic/
#   /home/obrol/study/firmware/src/platform/nrf52/
#   /home/obrol/.platformio/packages/framework-arduinoadafruitnrf52/cores/nRF5/sysview/SEGGER/
#   /home/obrol/.platformio/packages/framework-arduinoadafruitnrf52/cores/nRF5/sysview/Config/
#   /home/obrol/study/firmware/.pio/libdeps/feather_nrf52832/RadioLib/src/
)


build_lib(
  LIBNAME meshtastic
  SOURCE_FILES
    model/MeshtasticMac.cpp
    helper/MeshtasticHelper.cpp
  HEADER_FILES
    model/MeshtasticMac.h
    helper/MeshtasticHelper.h
  LIBRARIES_TO_LINK
    ${libnetwork}
    ${libpropagation}
    ${libenergy}
    ${libbuildings}
    ${libmobility}
    ${liblorawan}
)
