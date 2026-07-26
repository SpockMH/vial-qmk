// Copyright 2023 sekigon-gonnoc
/* SPDX-License-Identifier: GPL-2.0-or-later */

#pragma once

void cli_init(void);
void cli_exec(void);

typedef struct {
    void  (*func)(void*);
    void* data;
} cli_app_t;

void set_cli_app(cli_app_t* const p_cli_app);
