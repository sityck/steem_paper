file(GLOB HEADERS "include/steem/plugins/demo_api/*.hpp")
add_library( demo_api_plugin
             demo_api.cpp
           )

target_link_libraries( demo_api_plugin json_rpc_plugin steem_protocol appbase fc )
target_include_directories( demo_api_plugin PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

if( CLANG_TIDY_EXE )
   set_target_properties(
      demo_api_plugin PROPERTIES
      CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
   )
endif( CLANG_TIDY_EXE )

install( TARGETS
   demo_api_plugin

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
