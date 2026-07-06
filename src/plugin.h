/*
 * Copyright (C) 2026 Czy_4201b
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef REDSTONEX_PLUGIN_H
#define REDSTONEX_PLUGIN_H

#include <stdint.h>

typedef enum {
    TYPE_NONE = 0,
    TYPE_INT,
    TYPE_DOUBLE,
    TYPE_BOOL,
    TYPE_STRING
} PropertyType;

// 来自Gemini 我已累哭
// 但是这个写的挺好的
typedef struct PropertyValue {
    PropertyType type;
    union {
        int64_t     v_int;
        double      v_double;
        int         v_bool;
        const char* v_string;
    } value;
} PropertyValue;

// 来自Gemini 我已累哭
// 但是这个写的挺好的
typedef struct PluginArgs {
    void* py_kwargs_ptr; // 内部保留，指向 py::kwargs
    
    // 供 C 插件作者调用的参数读取函数（找不到时返回默认值 default_val）
    int64_t     (*get_int)(struct PluginArgs* self, const char* key, int64_t default_val);
    double      (*get_double)(struct PluginArgs* self, const char* key, double default_val);
    int         (*get_bool)(struct PluginArgs* self, const char* key, int default_val);
    const char* (*get_string)(struct PluginArgs* self, const char* key, const char* default_val);
} PluginArgs;

typedef struct ComponentOps {
    void* (*create)(PluginArgs* args);
    void (*destroy)(void* self);
    PropertyValue (*get_property)(void* self, const char* name);
} ComponentOps;

#endif
