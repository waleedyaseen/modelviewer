include(FetchContent)

FetchContent_Declare(
        IMGUI
        GIT_REPOSITORY          https://github.com/ocornut/imgui.git
        GIT_TAG                 87f12e56fe37411068309db7d8f978035c60060d
        GIT_SUBMODULES_RECURSE  OFF
)
FetchContent_MakeAvailable(IMGUI)