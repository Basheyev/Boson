# CMakeList.txt: проект CMake для Boson; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#
cmake_minimum_required (VERSION 3.8)

project ("Boson")

include_directories("src/index" "src/test")

# Добавьте источник в исполняемый файл этого проекта.
add_executable (Boson 
    "src/Boson.cpp" 
    "src/Boson.h" 

    "src/index/BalancedTreeIndex.cpp" 
    "src/index/BalancedTreeIndex.h" 
    "src/index/Node.cpp" 
    "src/index/InnerNode.cpp" 
    "src/index/LeafNode.cpp"

    "src/test/BalancedTreeTest.cpp"
    "src/test/BalancedTreeTest.h" "src/storage/PagerStorage.h")

# TODO: Добавьте тесты и целевые объекты, если это необходимо.
