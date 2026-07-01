# Sistemas Operacionais

## Como rodar

Abra o projeto no Dev Container.

Dentro do container, compile:

```bash
cmake -S . -B build-devcontainer -DCMAKE_BUILD_TYPE=Debug
cmake --build build-devcontainer -j$(nproc)
```

Rode:

```bash
./build-devcontainer/ProjetoImGui
```

Se a janela não abrir no Linux, libere o X11 no host:

```bash
xhost +local:docker
```
