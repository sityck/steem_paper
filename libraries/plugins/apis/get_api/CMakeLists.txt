file(GLOB HEADERS "include/steem/plugins/get_api/*.hpp")
add_library( get_api_plugin
             get_api.cpp
           )

target_link_libraries( get_api_plugin follow_plugin database_api_plugin account_history_api_plugin condenser_api_plugin json_rpc_plugin steem_protocol appbase fc )
target_include_directories( get_api_plugin PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

if( CLANG_TIDY_EXE )
   set_target_properties(
      get_api_plugin PROPERTIES
      CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
   )
endif( CLANG_TIDY_EXE )

install( TARGETS
   get_api_plugin

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
