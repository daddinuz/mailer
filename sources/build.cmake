add_library(mailer ${CMAKE_CURRENT_LIST_DIR}/mailer.h ${CMAKE_CURRENT_LIST_DIR}/mailer.c)
target_link_libraries(mailer PRIVATE curl error panic alligator)
