/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2017-01-21     Bernard      the first version
 */

#ifndef RTLIBC_H__
#define RTLIBC_H__

/* definitions for libc if toolchain has no these definitions */
#include <kernel/libc/libc_stat.h>
#include <kernel/libc/libc_errno.h>

#include <kernel/libc/libc_fcntl.h>
#include <kernel/libc/libc_ioctl.h>
#include <kernel/libc/libc_dirent.h>
#include <kernel/libc/libc_signal.h>
#include <kernel/libc/libc_fdset.h>

#if defined(__CC_ARM) || defined(__CLANG_ARM) || defined(__IAR_SYSTEMS_ICC__)
typedef signed long off_t;
typedef int mode_t;
#endif

#if defined(__MINGW32__) || defined(_WIN32)
typedef signed long off_t;
typedef int mode_t;
#endif

#endif

