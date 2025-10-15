set_project("RuLocaleFixes")
set_version("2.2")

set_languages("cxx20")
set_warnings("all")
set_optimize("faster")
set_symbols("debug")

set_policy("build.optimization.lto", true)

target("RuLocaleFixes")
    set_arch("x86")
    set_kind("shared")  
    add_files("*.cpp")
    add_files("../shared/NVSEManager/*.cpp")
    add_files("../shared/SafeWrite/*.cpp")
    add_includedirs("..")
    add_syslinks("user32", "dbghelp")

    add_linkdirs(".")
    add_links("libMinHook.x86")

    add_defines("WIN32", "_WINDOWS", "_USRDLL", "RUNTIME", "RUNTIME_VERSION=0x040020D0")

    if is_mode("debug") then
        add_defines("_DEBUG", "NVSE_PLUGIN_EXAMPLE_EXPORTS")
        set_runtimes("MTd") 
    else
        add_defines("NDEBUG")
        set_runtimes("MT") 
    end
