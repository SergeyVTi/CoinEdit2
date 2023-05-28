cmake_minimum_required(VERSION 3.16)
project(CoinEdit2 LANGUAGES CXX)

find_package(Qt6
    REQUIRED COMPONENTS Core Gui Widgets PrintSupport StateMachine
)

qt_standard_project_setup()

qt_add_executable(CoinEdit2
    celldialog.cpp celldialog.h
    main.cpp
    coin.cpp coin.h
    coinedit.cpp coinedit.h
    graphics.cpp graphics.h
    texteditor.cpp texteditor.h
)

set_target_properties(CoinEdit2 PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(CoinEdit2 PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::PrintSupport
    Qt6::StateMachine
)

# Resources:
qt_add_resources(CoinEdit2 "icons"
    PREFIX
        "/"
    FILES
         "icons/checkbox-checked.png"
         "icons/checkbox.png"
         "icons/delete.png"
         "icons/editcopy.png"
         "icons/editcut.png"
         "icons/editpaste.png"
         "icons/editredo.png"
         "icons/editundo.png"
         "icons/exportpdf.png"
         "icons/filenew.png"
         "icons/fileopen.png"
         "icons/fileprint.png"
         "icons/filesave.png"
         "icons/format-indent-less.png"
         "icons/format-indent-more.png"
         "icons/minus-sign.png"
         "icons/new.png"
         "icons/open.png"
         "icons/paste.png"
         "icons/plus-sign.png"
         "icons/print.png"
         "icons/save.png"
         "icons/textbold.png"
         "icons/textcenter.png"
         "icons/textitalic.png"
         "icons/textjustify.png"
         "icons/textleft.png"
         "icons/textright.png"
         "icons/textunder.png"
         "icons/zoomin.png"
         "icons/zoomout.png"
)
