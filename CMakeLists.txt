# CMakeList.txt: проект CMake для Boson; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#
cmake_minimum_required (VERSION 3.8)

project ("Boson")

include_directories("src/core" "src/index" "src/storage" "src/test")

# Добавьте источник в исполняемый файл этого проекта.
add_executable (Boson 
    "src/Boson.cpp" 
    "src/Boson.h" 
     
    "src/index/BalancedTreeIndex.h" 
    
    
    "src/storage/RecordFileIO.h" 
    "src/storage/RecordFileIO.cpp"   
    "src/storage/CachedFileIO.h" 
    "src/storage/CachedFileIO.cpp" 

    "src/test/CachedFileIOTest.h" 
    "src/test/CachedFileIOTest.cpp"  
    "src/test/RecordFileIOTest.cpp" 
    "src/test/RecordFileIOTest.h"
    "src/test/BalancedTreeTest.cpp"
    "src/test/BalancedTreeTest.h"     "src/index/BPTreeIndex.h")

set_property(TARGET Boson 
    PROPERTY 
        CXX_STANDARD 17
)

# TODO: Добавьте тесты и целевые объекты, если это необходимо.
