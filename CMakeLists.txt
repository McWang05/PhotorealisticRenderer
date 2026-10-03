cmake_minimum_required(VERSION 3.15)
project(PhotorealisticCarRenderer)
set(CMAKE_POLICY_DEFAULT_CMP0175 OLD)
# Use modern C++ standard
set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

# 1. Fetch GLFW (Windowing and Inputs)
FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG 3.3.8)
FetchContent_MakeAvailable(glfw)

# 2. Fetch GLM (OpenGL Mathematics)
FetchContent_Declare(glm GIT_REPOSITORY https://github.com/g-truc/glm.git GIT_TAG master)
FetchContent_MakeAvailable(glm)

# 3. Fetch Assimp (3D Model Loader)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(assimp GIT_REPOSITORY https://github.com/assimp/assimp.git GIT_TAG v5.2.5)
FetchContent_MakeAvailable(assimp)

# 4. Define the Executable (Note: glad.c will be added in the next step)
add_executable(${PROJECT_NAME} src/main.cpp src/glad.c)

# 5. Include directories and link libraries
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw glm assimp)