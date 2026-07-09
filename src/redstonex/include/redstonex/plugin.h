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
    TYPE_STRING,
    TYPE_PTR,
    TYPE_SLOT_PTR
} PropertyType;

/*
 * PropertyValue和PluginArgs均来自Gemini，为了保证C语言开发的兼容性
 * 至于为啥我没有想到这种写法，我不知道反正就是没想到
 * 但是因为写的确实很棒，所以保留，并且作为插件的标准接口使用
 */

typedef struct PropertyValue {
    PropertyType type;
    union {
        int64_t     v_int;
        double      v_double;
        int         v_bool;
        const char* v_string;
        void*       v_ptr;
    } value;
} PropertyValue;

typedef struct PluginArgs {
    void* py_kwargs_ptr; // 内部保留，指向 py::kwargs
    
    // 供作者调用的参数读取函数（找不到时返回默认值 default_val）
    int64_t     (*get_int)(struct PluginArgs* self, const char* key, int64_t default_val);
    double      (*get_double)(struct PluginArgs* self, const char* key, double default_val);
    int         (*get_bool)(struct PluginArgs* self, const char* key, int default_val);
    const char* (*get_string)(struct PluginArgs* self, const char* key, const char* default_val);
} PluginArgs;

// =============================================================================
// 【 插件回调函数签名定义 】
// =============================================================================
// 插件作者在实现函数时，应该确保函数签名与以下定义完全一致
// 如果是 C++ 开发者，请确保这些函数被包裹在 extern "C" 中
// =============================================================================

typedef struct ComponentOps {
    void* (*create)(PluginArgs* args);
    void (*destroy)(void* self);
    PropertyValue (*get_property)(void* self, const char* name);
} ComponentOps;

// =============================================================================
// 【插件开发者指南】
// =============================================================================
// 为了能让 PyRedstoneX 的 PluginRegistry 成功识别，请严格按照以下要求编写你的 C/C++ 插件：
// 
// 1. 如果使用 C++ 开发，**必须**在实现时加上 `extern "C"`，防止函数名被修饰（Mangled）
// 2. 你需要实现符合 `ComponentOps` 约定的三个函数，名字可以自定义，并在 Python 层注册时传入对应名字
//
// 示例实现模板：
// 
// // 插件作者自定义的结构体，必须把基类放在第一个成员位置！
// typedef struct MyComponent {
//     RSXConnectiveObject base; // 必须继承自"redstonex_obj.h"中的RSXConnectiveObject或者其他继承自RSXConnectiveObject的组件例如LineObject、SourceObject等，而且必须在最前面，确保指针强转安全
//     uint8_t max_power;         // 插件自己的属性
// } MyComponent;
//
// #ifdef __cplusplus
// extern "C" {
// #endif
//
// void* create_my_component(PluginArgs* args) {
//     // 示例：通过统一接口读取来自 Python 的参数
//     int64_t max_power = args->get_int(args, "max_power", 15); 
//     
//     MyState* state = (MyState*)malloc(sizeof(MyState));
//     state->power = max_power;
//     return state; // 返回的指针会被主程序托管
// }
//
// void destroy_my_component(void* self) {
//     free(self);
// }
//
// PropertyValue MyComponent_get_property(void* self, const char* name) {
//     MyState* state = (MyState*)self;
//     PropertyValue v;
//     if (strcmp(name, "power") == 0) {
//         v.type = TYPE_INT;
//         v.value.v_int = state->power;
//     } else {
//         v.type = TYPE_NONE;
//     }
//     return v;
// }
//
// #ifdef __cplusplus
// }
// #endif
//
// 接下来，便可以在 Python 侧使用注册工具：
// from redstonex import PluginRegistry
// with PluginRegistry("./plugins/libmy_redstone_items.so") as reg:
//    reg.add(
//        component_name="MyComponent",
//        create="create_my_component",
//        destroy="destroy_my_component",
//        get_property="MyComponent_get_property"
//    )
//
// 注册完成后，接下来便可以自由地使用这个元件了：
// import redstonex as rsx
// sim = rsx.Simulator()
// custom = rsx.Custom("MyComponent")
// sim.bind_object(custom)
// sim.run()
//
// 如果你希望为最终用户提供更友好的面向对象接口（避免硬编码字符串，同时提供 VSCode、Pycharm 等的自动补全能力），
// 可以参考 examples，通过继承 Custom 进行二次封装：
// class MyComponent(rsx.Custom):
//    def __init__(self, max_power: int = 15):
//       super().__init__("MyComponent", max_power=max_power)
//
// 这样，用户便可以像使用原生组件一样优雅地创建它：
// ..
// custom = MyComponent()
// sim.bind_object(custom)
// sim.run()

#endif
