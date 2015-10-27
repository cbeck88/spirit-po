function(CreateFolderMarkerFile target marker_file_name folder)
    set(_SCRIPT_FILE "${CMAKE_CURRENT_BINARY_DIR}/${target}_marker_file.cmake")

    file(WRITE ${_SCRIPT_FILE}
    "# Generated Script file
    if(APPLE)
        include(BundleUtilities)
        get_bundle_and_executable(\"\${APP_PATH}\" bundle executable valid)
        if(valid)
          set(dest \"\${bundle}/Contents/Resources\")
        else()
          message(FATAL_ERROR \"App not found? \${APP_PATH}\")
        endif()
    else()
        get_filename_component(dest \"\${APP_PATH}\" DIRECTORY)
    endif()
    message(STATUS \"Dest: \${dest}\")
    message(STATUS \"Folder: ${folder}\")
    file(MAKE_DIRECTORY \"\${dest}\")
    file(RELATIVE_PATH relpath \"\${dest}\" \"${folder}\")
    file(WRITE \"\${dest}/${marker_file_name}\" \"\${relpath}\")
    "
    )

    ADD_CUSTOM_COMMAND(TARGET ${target}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -DAPP_PATH="$<TARGET_FILE:${target}>" -P "${_SCRIPT_FILE}"
    )
endfunction()