# Add this to your CMakeLists.txt to fix architecture issues

# Force architecture for Apple platforms
if(APPLE)
    # For Apple Silicon Macs
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
        set(CMAKE_OSX_ARCHITECTURES "arm64")
    # For Intel Macs  
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
        set(CMAKE_OSX_ARCHITECTURES "x86_64")
    endif()
    
    # Ensure consistent architecture across all targets
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0")
endif()

# Clean any cached architecture settings
unset(CMAKE_OSX_ARCHITECTURES CACHE)