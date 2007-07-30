/*
 *  Calculator -- OpenMoko simple Calculator
 *
 *  Authored by Rodolphe Ortalo <rodolphe.ortalo@free.fr>
 *
 *  Copyright (C) 2007 Rodolphe Ortalo
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2.1 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Public License for more details.
 *
 *  Current Version: $Rev$ ($Date$) [$Author$]
 */

#ifndef CALC_MAIN_H
#define CALC_MAIN_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#undef CALCULATOR_DEBUG

/*
 * calc_debug functions
 */
#ifdef CALCULATOR_DEBUG
#define calc_debug(...) g_debug(__VA_ARGS__)
#else
#define calc_debug(...)
#endif

#endif /* CALC_MAIN_H */
