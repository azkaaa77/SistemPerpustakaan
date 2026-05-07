@echo off
g++ main.c++ imgui/*.cpp imgui/backends/imgui_impl_glfw.cpp imgui/backends/imgui_impl_opengl3.cpp ^
    -o aplikasi.exe ^
    -I./include ^
    -I./imgui ^
    -I./imgui/backends ^
    -L./lib ^
    -lglfw3 ^
    -lopengl32 -lgdi32 -limm32 -lcomdlg32 -lshell32 -luser32 -lkernel32 ^
    -lmingw32 -lmingwex -lmsvcrt ^
    -Wl,--allow-multiple-definition ^
    -static-libgcc -static-libstdc++
pause