set(SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/cutil_string.c
    ${CMAKE_CURRENT_SOURCE_DIR}/cutil_math.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_string.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_math.c
    )
set(HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/types.h
    ${CMAKE_CURRENT_SOURCE_DIR}/cutil_string.h
    ${CMAKE_CURRENT_SOURCE_DIR}/cutil_math.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_string.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_math.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
