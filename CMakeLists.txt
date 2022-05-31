# CMakeList.txt: проект CMake для Boson; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#
cmake_minimum_required (VERSION 3.8)

project ("Boson")

# Добавьте источник в исполняемый файл этого проекта.
add_executable (Boson 
    "src/Boson.cpp" 
    "src/Boson.h" 

    "src/table/BalancedTree.cpp" 
    "src/table/BalancedTree.h" 
    "src/table/Node.cpp" 
    "src/table/InnerNode.cpp" 
    "src/table/LeafNode.cpp"

    "src/test/BalancedTreeTest.cpp"
    "src/test/BalancedTreeTest.h")

# TODO: Добавьте тесты и целевые объекты, если это необходимо.
