add_executable(test_wicked_vault_cleanup)
init_target(test_wicked_vault_cleanup "(tests)")
target_include_directories(test_wicked_vault_cleanup PRIVATE ${src_loc})
nice_target_sources(test_wicked_vault_cleanup ${src_loc}
PRIVATE
    wicked/vault_cleanup.h
    tests/test_wicked_vault_cleanup.cpp
)
target_link_libraries(test_wicked_vault_cleanup PRIVATE desktop-app::external_qt)
set_target_properties(test_wicked_vault_cleanup PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
