# CMakeList.txt: проект CMake для Boson; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#
cmake_minimum_required (VERSION 3.8)

project ("Boson")

include_directories(
    "src/template" 
    "src/document" 
    "src/index" 
    "src/storage" 
    "src/test")

# Добавьте источник в исполняемый файл этого проекта.
add_executable (Boson 
    "src/Boson.cpp" 
    "src/Boson.h" 
            
    "src/storage/RecordFileIO.h" 
    "src/storage/RecordFileIO.cpp"   
    "src/storage/CachedFileIO.h" 
    "src/storage/CachedFileIO.cpp" 

    
     
    
    "src/test/CachedFileIOTest.h" 
    "src/test/CachedFileIOTest.cpp"  
    "src/test/RecordFileIOTest.h"
    "src/test/RecordFileIOTest.cpp" 
        
    "src/index/BalancedIndex.h" 
    "src/index/BalancedIndex.cpp" 
    "src/index/Node.cpp"   
    "src/index/InnerNode.cpp" 
    "src/index/LeafNode.cpp"  
    "src/index/NodeData.cpp"

    "src/template/InMemoryTest.cpp"
    "src/template/InMemoryTest.h"     
    "src/template/InMemoryBPTree.h" 
)

set_property(TARGET Boson 
    PROPERTY 
        CXX_STANDARD 17
)

# TODO: Добавьте тесты и целевые объекты, если это необходимо.
