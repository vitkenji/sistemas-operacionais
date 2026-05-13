cmake_minimum_required(VERSION 3.14)
project(ProjetoImGui)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# procura OpenGL
find_package(OpenGL REQUIRED)

# baixa e compila o GLFW
include(FetchContent)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4 # Versão estável do GLFW
)
# desativa os testes
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# lista de todos os .cpp que precisam ser compilados
set(IMGUI_SOURCES
    imgui/imgui.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_impl_glfw.cpp
    imgui/imgui_impl_opengl3.cpp
)

set(PROJECT_SOURCES
    src/tarefa/tarefa.cpp
    src/config/CarregadorConfig.cpp
    src/gerenciadores/GerenciadorTarefa.cpp
    src/gerenciadores/gerenciadorGrafico.cpp
    src/escalonadores/PriopEscalonador.cpp
    src/escalonadores/SRTFescalonador.cpp
    src/telas/TelaInicial.cpp
    src/telas/TelaSimulacao.cpp
    src/telas/GanttChart.cpp
)

# cria executável
add_executable(${PROJECT_NAME} main.cpp ${IMGUI_SOURCES} ${PROJECT_SOURCES})

# copia os exemplos para a pasta do executavel, mantendo os caminhos relativos
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/exemplos
            $<TARGET_FILE_DIR:${PROJECT_NAME}>/exemplos
)

# diz ao compilador onde achar os arquivos .h
target_include_directories(${PROJECT_NAME} PRIVATE
    headers
    imgui
)

# linka OpenGL e glfw ao projeto
target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL glfw)
