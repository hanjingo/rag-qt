function(win_deploy target)
	set(libs ${ARGN})
	include(InstallRequiredSystemLibraries)
	foreach(lib ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS})
		get_filename_component(filename "${lib}" NAME)
		set(dst "$<TARGET_FILE_DIR:${target}>/${filename}")
		add_custom_command(TARGET ${target} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E echo "copy file: ${lib} -> ${dst}"
			COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${lib}" "${dst}"
			COMMENT "Copying runtime: ${filename}..."
		)
	endforeach()
	foreach(lib ${libs})
		get_filename_component(filename "${lib}" NAME)
		if(filename MATCHES ".*\\.dll$")
			set(dst "$<TARGET_FILE_DIR:${target}>/${filename}")
			add_custom_command(TARGET ${target} POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E echo "copy file: ${lib} -> ${dst}"
				COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${lib}" "${dst}"
				COMMENT "Copying dep DLL: ${filename}..."
			)
		endif()
	endforeach()
endfunction()

function(mac_deploy target)
    set(libs ${ARGN})
    
    # 1. 定位标准的 Frameworks 目录与 PlugIns 目录
    set(FW_DIR "$<TARGET_FILE_DIR:${target}>/../Frameworks")
    set(PLUGINS_DIR "$<TARGET_FILE_DIR:${target}>/../PlugIns")
    
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${FW_DIR}"
        COMMENT "Creating Frameworks directory for ${target}..."
    )

    # 2. 显式遍历并拷贝通过 CMake 传进来的第三方 dylib
    foreach(lib ${libs})
        if(NOT IS_ABSOLUTE "${lib}")
            continue()
        endif()
        get_filename_component(filename "${lib}" NAME)
        if(filename MATCHES ".*\\.dylib$")
            set(dst "${FW_DIR}/${filename}")
            add_custom_command(TARGET ${target} POST_BUILD
                COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${lib}" "${dst}"
                COMMAND chmod 755 "${dst}"
                COMMENT "Initially packing explicitly passed library: ${filename}..."
            )
        endif()
    endforeach()

    # 3. 编写高级递归分析与目录净化 Shell 脚本
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/mac_deploy.sh"
"#!/bin/bash
set +e

exe=\"\$1\"
fw_dir=\"\$2\"
plugins_dir=\"\$3\"
macos_dir=\$(dirname \"\$exe\")

echo \"=== Starting Deep Rpath Deployment for Mac Bundle ===\"

# 核心：递归扫描并修正依赖函数
fix_dependencies() {
    local target_file=\"\$1\"
    local is_plugin=\"\$2\"
    if [ ! -f \"\$target_file\" ]; then return; fi
    
    local loader_path=\"@executable_path/../Frameworks\"
    if [ \"\$is_plugin\" = \"true\" ]; then
        loader_path=\"@loader_path/../../Frameworks\"
    fi

    otool -L \"\$target_file\" | grep ' /' | grep -v '/usr/lib' | grep -v '/System' | awk '{print \$1}' | while read dep; do
        if [[ \"\$dep\" == @* ]]; then continue; fi
        
        fname=\$(basename \"\$dep\")
        dst_lib=\"\$fw_dir/\$fname\"
        
        if [ ! -f \"\$dst_lib\" ]; then
            echo \"  -> Found deep dependency: \$dep\"
            if [ -f \"\$dep\" ]; then
                cp -f \"\$dep\" \"\$dst_lib\"
                chmod 755 \"\$dst_lib\"
                install_name_tool -id \"@executable_path/../Frameworks/\$fname\" \"\$dst_lib\" 2>/dev/null
                fix_dependencies \"\$dst_lib\" \"false\"
            else
                echo \"  [Warning] Physical file not found for: \$dep\"
            fi
        fi
        
        install_name_tool -change \"\$dep\" \"\$loader_path/\$fname\" \"\$target_file\" 2>/dev/null
    done
}

# 步骤 1：修复主程序 rag-qt 和副程序 rag-core
fix_dependencies \"\$exe\" \"false\"
if [ -f \"\$macos_dir/rag-core\" ]; then
    echo \"=== Fixing Helper Binary: rag-core ===\"
    fix_dependencies \"\$macos_dir/rag-core\" \"false\"
fi

# 步骤 2：地毯式扫描整个 PlugIns 里的所有 Qt 和自定义业务插件
if [ -d \"\$plugins_dir\" ]; then
    echo \"=== Fixing PlugIns Dependencies ===\"
    find \"\$plugins_dir\" -type f \\( -name \"*.dylib\" -o -name \"*.so\" \\) | while read plugin; do
        fix_dependencies \"\$plugin\" \"true\"
    done
fi

# 步骤 3：对 Frameworks 下的所有三方 dylib 进行自身 ID 和引用的最后闭环校对
echo \"=== Finalizing Frameworks Linkages ===\"
for dylib in \"\$fw_dir\"/*.dylib; do
    if [ -f \"\$dylib\" ]; then
        fname=\$(basename \"\$dylib\")
        install_name_tool -id \"@executable_path/../Frameworks/\$fname\" \"\$dylib\" 2>/dev/null
        fix_dependencies \"\$dylib\" \"false\"
    fi
done

# 💥【规范修正 2】：强制清理可能由旧缓存、子模块生成或构建残留引入 MacOS 下的杂质目录
echo \"=== Cleaning up invalid directories in MacOS folder ===\"
if [ -d \"\$macos_dir/configs\" ]; then
    echo \"  -> Clearing misplaced configs from MacOS...\"
    rm -rf \"\$macos_dir/configs\"
fi
if [ -d \"\$macos_dir/log\" ]; then
    echo \"  -> Clearing active logs directory from MacOS...\"
    rm -rf \"\$macos_dir/log\"
fi
if [ -d \"\$macos_dir/plugins\" ]; then
    echo \"  -> Clearing empty plugins placeholder from MacOS...\"
    rm -rf \"\$macos_dir/plugins\"
fi

echo \"=== Deep Rpath Deployment & Bundle Standardized Completed ===\"
")

    file(CHMOD "${CMAKE_CURRENT_BINARY_DIR}/mac_deploy.sh" PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

    # 4. 执行深度修复命令
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND bash "${CMAKE_CURRENT_BINARY_DIR}/mac_deploy.sh" "$<TARGET_FILE:${target}>" "${FW_DIR}" "${PLUGINS_DIR}"
        COMMENT "Executing recursively deep Rpath fixing and standardizing bundle folders..."
    )
endfunction()

function(linux_deploy target)
	set(libs ${ARGN})
	foreach(lib ${libs})
		get_filename_component(filename "${lib}" NAME)
		if(filename MATCHES ".*\\.so.*$")
			set(dst "$<TARGET_FILE_DIR:${target}>/${filename}")
			add_custom_command(TARGET ${target} POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E echo "copy file: ${lib} -> ${dst}"
				COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${lib}" "${dst}"
				COMMENT "Copying dep so: ${filename}..."
			)
		endif()
	endforeach()

	# ldd de | grep "=> /" | awk '{print $3}' | while read dep; do fname=$(basename "$dep"); dst="./$fname"; echo "copy file: $dep -> $dst"; cp -u "$dep" "$dst"; done
	file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/linux_deploy.sh"
"#!/bin/bash
set -e
exe=\"$1\"
dir=\"$2\"
ldd \"\$exe\" | grep '=> /' | awk '{print \$3}' | while read dep; do
    fname=\$(basename \"\$dep\")
    dst=\"\$dir/\$fname\"
    echo \"copy file: \$dep -> \$dst\"
    cp -u \"\$dep\" \"\$dst\"
done
")

	file(CHMOD "${CMAKE_CURRENT_BINARY_DIR}/linux_deploy.sh" PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

	add_custom_command(TARGET ${target} POST_BUILD
		COMMAND bash "${CMAKE_CURRENT_BINARY_DIR}/linux_deploy.sh" "$<TARGET_FILE:${target}>" "$<TARGET_FILE_DIR:${target}>"
		COMMENT "Copying shared library dependencies (ldd)..."
	)
endfunction()