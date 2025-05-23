# Copyright (c) 2022 HPMicro
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.13)

set(SES_TOOLCHAIN_VARIANT "Andes")

set(CONFIG_COREMARK 1)
set(HPM_BUILD_TYPE "sec_core_img")
set(SEC_CORE_IMG_C_ARRAY_OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/../core0/src/sec_core_img.c)

find_package(hpm-sdk REQUIRED HINTS $ENV{HPM_SDK_BASE})

project(coremark_core1)

if("${TOOLCHAIN_VARIANT}" STREQUAL "zcc")
    set(opt_string "-O3 -flto -falign-functions=4 -flate-loop-unroll")
    sdk_zcc_ld_options(-Wl,-mllvm,--align-all-nofallthru-blocks=2)
    sdk_zcc_ld_options(-falign-functions=4)
    sdk_zcc_ld_options(-flate-loop-unroll)
    sdk_zcc_ld_options(-Wl,-mllvm,--jump-is-expensive=true)
    sdk_zcc_ld_options(-O3)
    sdk_zcc_ld_options("-fuse-size-lib")
else()
    set(opt_string "-O3 -funroll-all-loops -finline-limit=600 -fno-code-hoisting -fselective-scheduling -fno-forward-propagate -fno-tree-ter -fipa-pta -fno-tree-vrp")
endif()
separate_arguments(opt_list UNIX_COMMAND ${opt_string})

foreach(opt ${opt_list})
    sdk_compile_options(${opt})
endforeach()
sdk_get_compile_options(all_opts)
sdk_compile_definitions(COMPILER_FLAGS="${all_opts}")
sdk_nds_compile_options(-mcmov)

sdk_inc(src/port)
sdk_app_src(src/port/core_portme.c)

sdk_ses_compile_options(-mabi=ilp32d)
sdk_ses_compile_options(-march=rv32gc)
generate_ide_projects()
