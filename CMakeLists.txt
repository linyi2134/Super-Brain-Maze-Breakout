cmake_minimum_required(VERSION 3.19)
project(MazeBreakout LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

qt_standard_project_setup()

qt_add_executable(MazeBreakout
    WIN32 MACOSX_BUNDLE
    main.cpp          # 确保 main.cpp 存在
    widget.cpp
    widget.h
    Chess.h Chess.cpp
    board.h board.cpp
    player.h player.cpp
    gamecontroller.h gamecontroller.cpp
    # 以下全部注释掉，暂时不添加游戏类
    # chess.cpp
    # chess.h
    # board.cpp
    # board.h
    # player.cpp
    # player.h
    # gamecontroller.cpp
    # gamecontroller.h
    # filemanager.cpp
    # filemanager.h
)

target_link_libraries(MazeBreakout
    PRIVATE
        Qt::Core
        Qt::Widgets
)

install(TARGETS MazeBreakout
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET MazeBreakout
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})