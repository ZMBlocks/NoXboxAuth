add_rules("mode.debug", "mode.release")

add_repositories("liteldev-repo https://github.com/LiteLDev/xmake-repo.git")
add_repositories("groupmountain-repo https://github.com/GroupMountain/xmake-repo.git")

add_requires("glaciehook 1.0.2", { configs = { static = true } })
add_requires("fmt 10.2.1")

if not has_config("vs_runtime") then
    set_runtimes("MD")
end

target("NoXboxAuth")
    add_cxflags(
        "/EHa",
        "/utf-8",
        "/W4",
        "/w44265",
        "/w44289",
        "/w44296",
        "/w45263",
        "/w44738",
        "/w45204"
    )
    add_files("src/**.cpp")
    add_includedirs("src")
    set_exceptions("none")
    set_kind("shared")
    set_languages("cxx20")
    set_symbols("debug")
    add_packages("glaciehook", "fmt")
    add_defines(
        "NOMINMAX",
        "UNICODE",
        "_HAS_CXX23=1"
    )
    after_build(function (target)
        local output_dir = path.join(os.projectdir(), "bin")
        os.rm(output_dir)
        os.mkdir(output_dir)
        os.cp(target:targetfile(), output_dir)
        if os.isfile(target:symbolfile()) then
            os.cp(target:symbolfile(), output_dir)
        end
    end)