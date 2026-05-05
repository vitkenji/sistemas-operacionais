cmake_minimum_required(VERSION 3.14) # Atualizado para suportar FetchContent melhor
project(ProjetoImGui)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Procura o OpenGL (que já vem junto com o compilador/Windows)
find_package(OpenGL REQUIRED)

# ==============================================================================
# MÁGICA AQUI: Baixa e compila o GLFW automaticamente!
# ==============================================================================
include(FetchContent)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4 # Versão estável do GLFW
)
# Desativa os testes e exemplos do GLFW para compilar super rápido
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)
# ==============================================================================

# Lista de todos os arquivos .cpp do ImGui que precisam ser compilados
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
    src/gerenciadores/GerenciadorTarefa.cpp
    src/gerenciadores/gerenciadorGrafico.cpp
    src/escalonadores/PriopEscalonador.cpp
    src/algoritmos/PRIOp.cpp
    src/algoritmos/SRTF.cpp
    src/telas/TelaInicial.cpp
)

# Cria o seu executável
add_executable(${PROJECT_NAME} main.cpp ${IMGUI_SOURCES} ${PROJECT_SOURCES})

# Diz ao compilador onde achar os arquivos .h do ImGui e do projeto
target_include_directories(${PROJECT_NAME} PRIVATE
    headers
    imgui
)

# Linka o OpenGL e o glfw (que o CMake acabou de baixar) ao seu projeto
target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL glfw)
