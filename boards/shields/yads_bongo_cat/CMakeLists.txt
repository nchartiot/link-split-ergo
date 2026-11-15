# SPDX-License-Identifier: MIT
#
# Adds a bongo cat to the YADS (zmk-dongle-screen) status screen without
# modifying YADS: its zmk_display_status_screen() is renamed at compile time and
# wrapped by src/bongo_cat.c.

if(CONFIG_SHIELD_YADS_BONGO_CAT)
  if(NOT CONFIG_SHIELD_DONGLE_SCREEN OR NOT DEFINED SHIELD_DIR_dongle_screen)
    message(FATAL_ERROR "yads_bongo_cat requires the dongle_screen shield, listed before it")
  endif()

  # Library target name of the dongle_screen shield, as zephyr_library() derives it.
  file(RELATIVE_PATH yads_lib ${ZEPHYR_BASE} ${SHIELD_DIR_dongle_screen})
  string(REGEX REPLACE "[/:]" "__" yads_lib ${yads_lib})

  # Deferred so it runs once the dongle_screen shield (and its target) has been added.
  # EVAL bakes in the values now; DEFER would otherwise expand them out of scope.
  cmake_language(EVAL CODE "
    cmake_language(DEFER DIRECTORY [[${CMAKE_SOURCE_DIR}]] CALL set_source_files_properties
      [[${SHIELD_DIR_dongle_screen}/src/custom_status_screen.c]]
      TARGET_DIRECTORY [[${yads_lib}]]
      PROPERTIES COMPILE_DEFINITIONS zmk_display_status_screen=yads_display_status_screen)")

  zephyr_library()
  zephyr_library_include_directories(${ZEPHYR_LVGL_MODULE_DIR})
  zephyr_library_include_directories(${ZEPHYR_BASE}/lib/gui/lvgl/)
  zephyr_library_include_directories(${CMAKE_SOURCE_DIR}/include)
  zephyr_library_sources(src/bongo_cat.c)
  zephyr_library_sources(src/bongo_cat_images.c)
endif()
