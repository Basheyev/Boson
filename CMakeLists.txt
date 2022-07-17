# CMakeList.txt: проект CMake для Boson; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#
cmake_minimum_required (VERSION 3.8)

project ("Boson")

include_directories("src/core" "src/io" "src/index" "src/storage" "src/test")

# Добавьте источник в исполняемый файл этого проекта.
add_executable (Boson 
    "src/Boson.cpp" 
    "src/Boson.h" 
     
    "src/core/BalancedTreeIndex.h" 
     
    "src/test/BalancedTreeTest.cpp"
    "src/test/BalancedTreeTest.h" 
     
     
     "src/index/DBIndex.h"   
     "src/storage/DatabaseStorage.h" 
     "src/storage/DatabaseStorage.cpp"   

     "src/io/CachedFile.h" 
     "src/io/CachedFile.cpp" "src/test/CachedFileTest.h" "src/test/CachedFileTest.cpp")

# TODO: Добавьте тесты и целевые объекты, если это необходимо.
