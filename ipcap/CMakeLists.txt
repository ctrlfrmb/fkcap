# CMakeList.txt: fksoft 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#

# 设置项目名称和版本
set(TARGET_NAME ipcap)

if (WIN32 OR MSVC)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")
elseif (UNIX)
    # 仅查找静态库，强制后缀为 .a
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")

    # 如果只是优先查找静态库，保证 .a 后缀在前面即可，把默认的后缀加上
    # set(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
endif()

# 添加库目录
link_directories(${CMAKE_SOURCE_DIR}/include/${NPCAP_NAME}/lib/${CMAKE_HOST_SYSTEM_PROCESSOR})
LIST(APPEND LINK_LIBS "wpcap")
LIST(APPEND LINK_LIBS "Packet")
LIST(APPEND LINK_LIBS "ws2_32")
set(IPCAP_LIB ${LINK_LIBS} PARENT_SCOPE)

file(GLOB_RECURSE COM_INCLUDE ${CMAKE_SOURCE_DIR}/include/*.h*)
file(GLOB_RECURSE ALL_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
file(GLOB_RECURSE ALL_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
set(ALL_FILE ${COM_INCLUDE} ${ALL_INCLUDE} ${ALL_SRC})
set(IPCAP_SRC ${ALL_FILE} PARENT_SCOPE)
message(STATUS "CMAKE_CURRENT_SOURCE_DIR === ${CMAKE_CURRENT_SOURCE_DIR}")
# 指定可执行文件和源文件
#add_executable(${TARGET_NAME} ${IPCAP_SRC})

# 链接WinPcap库
#target_link_libraries(${TARGET_NAME} ${LINK_LIBS})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  #set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD 20)
endif()

