
add_executable(${PROJECT_NAME} WIN32
    Game.cpp
    Game.h
    DeviceResources.cpp
    DeviceResources.h
    Main.cpp
    StepTimer.h
    pch.h
    SnakeGame.cpp
    SnakeGame.h
    Effects2D.cpp
    Effects2D.h
    InputRouter.cpp
    InputRouter.h
)

target_precompile_headers(${PROJECT_NAME} PRIVATE pch.h)

target_link_libraries(${PROJECT_NAME} PRIVATE
    d3d12.lib dxgi.lib dxguid.lib uuid.lib
    kernel32.lib user32.lib
    comdlg32.lib advapi32.lib shell32.lib
    ole32.lib oleaut32.lib
    runtimeobject.lib
)

# Set platform-specific compile definitions
if(VCPKG_TARGET_TRIPLET MATCHES "xbox")
    # Xbox platform
    target_compile_definitions(${PROJECT_NAME} PRIVATE _GAMING_XBOX WINAPI_FAMILY=WINAPI_FAMILY_GAMES __WRL_NO_DEFAULT_LIB__)
else()
    # Desktop platform
    target_compile_definitions(${PROJECT_NAME} PRIVATE _GAMING_DESKTOP WINAPI_FAMILY=WINAPI_FAMILY_DESKTOP_APP __WRL_NO_DEFAULT_LIB__)
endif()

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /Wall /EHsc /GR)
endif()

# Microsoft Config file
configure_file(MicrosoftGameConfig.mgc "${CMAKE_BINARY_DIR}/MicrosoftGame.config" @ONLY)

find_program(MAKEPKG_TOOL makepkg.exe REQUIRED)

add_custom_command(TARGET ${PROJECT_NAME} PRE_LINK
                   COMMAND ${MAKEPKG_TOOL} localize /d "${CMAKE_BINARY_DIR}" /pd "${CMAKE_BINARY_DIR}/bin"
                   COMMENT "makepkg localize MicrosoftGame.config"
                   WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                   VERBATIM)

# Packages
find_package(directxmath CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Microsoft::DirectXMath)

find_package(directx-headers CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Microsoft::DirectX-Headers Microsoft::DirectX-Guids)

find_package(directx12-agility CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Microsoft::DirectX12-Core Microsoft::DirectX12-Layers)

find_package(directx-dxc CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Microsoft::DirectXShaderCompiler)

find_package(winpixevent CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Microsoft::WinPixEventRuntime)

find_package(ms-gdk CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Xbox::GameRuntime)

if (DIRECTX_ARCH STREQUAL "x64")
    find_package(gameinput CONFIG REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE Microsoft::GameInput)

    # XSAPI is available for both Desktop and Xbox
    target_link_libraries(${PROJECT_NAME} PRIVATE Xbox::XSAPI)

    # Uncomment if using Xbox GameChat
    # target_link_libraries(${PROJECT_NAME} PRIVATE Xbox::GameChat2)

    # Uncomment if using Azure PlayFab Services
    # target_link_libraries(${PROJECT_NAME} PRIVATE Xbox::PlayFabServices)
endif()

# DirectX Tool Kit (integrated as subdirectory)
target_link_libraries(${PROJECT_NAME} PRIVATE DirectXTK12)

target_compile_definitions(${PROJECT_NAME} PRIVATE ${COMPILER_DEFINES})
target_compile_options(${PROJECT_NAME} PRIVATE ${COMPILER_SWITCHES})
target_link_options(${PROJECT_NAME} PRIVATE ${LINKER_SWITCHES})

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wpedantic -Wextra
        "-Wno-c++98-compat" "-Wno-c++98-compat-pedantic"
        "-Wno-exit-time-destructors" "-Wno-global-constructors" "-Wno-language-extension-token"
        "-Wno-missing-prototypes" "-Wno-missing-variable-declarations" "-Wno-reserved-id-macro"
        "-Wno-float-equal" "-Wno-switch-enum" "-Wno-tautological-type-limit-compare")

    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 16.0)
        target_compile_options(${PROJECT_NAME} PRIVATE "-Wno-unsafe-buffer-usage")
    endif()

    if(BUILD_TEST_TEMPLATE)
        target_compile_options(${PROJECT_NAME} PRIVATE "-Wno-unused-value")
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(${PROJECT_NAME} PRIVATE
         "/wd4061" "/wd4365" "/wd4514" "/wd4571" "/wd4668" "/wd4710" "/wd4820" "/wd5039" "/wd5045"
         "/wd4265" "/wd4625" "/wd4626" "/wd4986" "/wd5204" "/wd5220")

    if(ENABLE_CODE_ANALYSIS)
        target_compile_options(${PROJECT_NAME} PRIVATE /analyze)
    endif()

    if(BUILD_TEST_TEMPLATE)
        target_compile_options(${PROJECT_NAME} PRIVATE "/wd4555")
    endif()
endif()

if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _WIN32_WINNT=0x0A00)
endif()

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

# Copy config, images, and DLLs
add_custom_command(
  TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_BINARY_DIR}/MicrosoftGame.Config
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/Assets
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/Assets
    )

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E $<IF:$<BOOL:$<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>>,copy,true>
    $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}> $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMAND_EXPAND_LISTS
    )

if(TARGET Microsoft::DirectX12-Agility)
   file(MAKE_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/D3D12")
   add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_PROPERTY:Microsoft::DirectX12-Core,IMPORTED_LOCATION_RELEASE> $<TARGET_FILE_DIR:${PROJECT_NAME}>/D3D12
      COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_PROPERTY:Microsoft::DirectX12-Layers,IMPORTED_LOCATION_DEBUG> $<TARGET_FILE_DIR:${PROJECT_NAME}>/D3D12
      COMMAND ${CMAKE_COMMAND} -E rm -f $<TARGET_FILE_DIR:${PROJECT_NAME}>/D3D12Core.dll
      COMMAND ${CMAKE_COMMAND} -E rm -f $<TARGET_FILE_DIR:${PROJECT_NAME}>/d3d12SDKLayers.dll
      COMMAND_EXPAND_LISTS
      )
endif()
