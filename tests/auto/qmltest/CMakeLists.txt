file(GLOB_RECURSE test_data_glob
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    *)
list(APPEND test_data ${test_data_glob})

qt_internal_add_test(tst_qmltest
    QMLTEST
    SOURCES
        tst_qmltest.cpp
    PUBLIC_LIBRARIES
        Qt::Gui
    TESTDATA ${test_data}
)

set(qmltest_resource_files
    "customitem.obj"
    "customtexture.jpg"
)

qt_internal_add_resource(tst_qmltest "qmltest"
    PREFIX
        "/"
    FILES
        ${qmltest_resource_files}
)
