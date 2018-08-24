file(GLOB HEADERS "include/graphene/khc/*.hpp")

add_library( graphene_khc 
             ${HEADERS}
           )

# need to link graphene_debug_witness because plugins aren't sufficiently isolated #246
target_link_libraries( graphene_khc 
					   PUBLIC graphene_chain graphene_db fc)
target_include_directories( graphene_khc
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" 
						    PRIVATE "${CMAKE_SOURCE_DIR}/libraries/chain/include")


if(MSVC)
  set_source_files_properties( application.cpp api.cpp database_api.cpp PROPERTIES COMPILE_FLAGS "/bigobj" )
endif(MSVC)

INSTALL( TARGETS
   graphene_khc

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
INSTALL( FILES ${HEADERS} DESTINATION "include/graphene/khc" )
