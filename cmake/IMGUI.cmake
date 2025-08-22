include(FetchContent)

FetchContent_Declare(
        IMGUI
        GIT_REPOSITORY          https://github.com/ocornut/imgui.git
        GIT_TAG                 docking
        GIT_SUBMODULES_RECURSE  OFF
)
FetchContent_MakeAvailable(IMGUI)