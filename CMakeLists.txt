# CMakeList.txt: проект CMake для Boson; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#
cmake_minimum_required (VERSION 3.8)

project ("Boson")

# Добавьте источник в исполняемый файл этого проекта.
add_executable (Boson 
    "src/Boson.cpp" 
    "src/Boson.h" 

    "src/registry/BalancedTree.cpp" 
    "src/registry/BalancedTree.h" 
    "src/registry/Node.cpp" 
    "src/registry/InnerNode.cpp" 
    "src/registry/LeafNode.cpp"
)

# TODO: Добавьте тесты и целевые объекты, если это необходимо.
