add_library(mltlv2 MODULE factory.c)

file(GLOB YML "*.yml")
add_custom_target(Other_lv2_Files SOURCES
  ${YML}
)

target_compile_options(mltlv2 PRIVATE ${MLT_COMPILE_OPTIONS})

target_link_libraries(mltlv2 PRIVATE mlt Threads::Threads)

if(GPL AND TARGET PkgConfig::xml AND TARGET PkgConfig::glib AND TARGET PkgConfig::lilv AND ladspa_h_FOUND)
  target_sources(mltlv2 PRIVATE
    lv2_rack.c lv2_rack.h
    lock_free_fifo.c lock_free_fifo.h
    plugin.c plugin.h
    plugin_desc.c plugin_desc.h
    plugin_mgr.c plugin_mgr.h
    plugin_settings.c plugin_settings.h
    process.c process.h
    producer_lv2.c
    filter_lv2.c
  )
  target_link_libraries(mltlv2 PRIVATE ${CMAKE_DL_LIBS} m PkgConfig::xml PkgConfig::glib PkgConfig::lilv)
  target_compile_definitions(mltlv2 PRIVATE GPL)
  if(RELOCATABLE)
    target_compile_definitions(mltlv2 PRIVATE RELOCATABLE)
  endif()

  install(FILES filter_lv2.yml producer_lv2.yml DESTINATION ${MLT_INSTALL_DATA_DIR}/lv2)

endif()

set_target_properties(mltlv2 PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${MLT_MODULE_OUTPUT_DIRECTORY}")

install(TARGETS mltlv2 LIBRARY DESTINATION ${MLT_INSTALL_MODULE_DIR})
