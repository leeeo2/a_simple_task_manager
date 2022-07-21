cmake_minimum_required(VERSION 3.0)
project(a_simple_system_task_emulation)

set (RESOURCE_FILE 
    src/data.cpp 
    src/main.cpp 
)


add_executable(${PROJECT_NAME} ${RESOURCE_FILE})