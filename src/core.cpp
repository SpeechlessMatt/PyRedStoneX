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

#include <cassert>
#include <cstdint>
#include <pybind11/detail/common.h>
#include <pybind11/pybind11.h>
#include <sys/types.h>
#include <unordered_map>

extern "C" {
#include "redstonex_sim.h"
#include "redstonex_obj.h"
#include "redstonex_components.h"
#include "redstonex_types.h"
}

#include <plugin.h>

#define ANSI_RESET   "\033[0m"
#define ANSI_DEBUG   "\033[36m"  // 青色
#define ANSI_INFO    "\033[32m"  // 绿色
#define ANSI_WARN    "\033[33m"  // 黄色
#define ANSI_ERROR   "\033[31m"  // 红色

namespace py = pybind11;

class ConnectiveObject {
    protected:
        RSXConnectiveObject* obj;
        bool owned_by_cpp = true;

    public:
        ConnectiveObject(RSXConnectiveObject* raw_obj) : obj(raw_obj) {}
        ConnectiveObject(uint32_t id, RSXObjectRole role, uint32_t limit, bool is_lossless, bool is_weak_transmissible) { obj = rsx_create_object(id, role, limit, is_lossless, is_weak_transmissible); }

        static ConnectiveObject create_view(uintptr_t raw_ptr) {
            ConnectiveObject view(nullptr);
            view.obj = reinterpret_cast<RSXConnectiveObject*>(raw_ptr);
            view.owned_by_cpp = false; // 不拥有内存
            return view;
        }
        
        virtual ~ConnectiveObject() { 
            if (obj && owned_by_cpp) {
                rsx_destroy_object(obj); 
            }
        }

        RSXConnectiveObject* get_raw() const { return obj; }

        bool connect(const ConnectiveObject& target) { return rsx_connect_objects(obj, target.get_raw()); }
        bool disconnect(const ConnectiveObject& target) { return rsx_disconnect_objects(obj, target.get_raw()); }
};

class LineObject : public ConnectiveObject {
    private:
        RSXLineObject* line;

    public:
        LineObject(RSXLineObject* raw_line) : ConnectiveObject(reinterpret_cast<RSXConnectiveObject*>(raw_line)), line(raw_line) {}
        LineObject(uint32_t id, uint32_t limit) : ConnectiveObject(nullptr) {
            line = rsx_create_line_object(id, limit); 
            obj = reinterpret_cast<RSXConnectiveObject*>(line);
        }
        ~LineObject() override {
            if (line) rsx_destroy_line_object(line);
            obj = nullptr;
        }
};

class SourceObject : public ConnectiveObject {
    private:
        RSXSourceObject* source;

    public:
        SourceObject(RSXSourceObject* raw_source) : ConnectiveObject(reinterpret_cast<RSXConnectiveObject*>(raw_source)), source(raw_source) {}
        SourceObject(uint32_t id, uint32_t limit, uint8_t power) : ConnectiveObject(nullptr) {
            source = rsx_create_source_object(id, limit, power); 
            obj = reinterpret_cast<RSXConnectiveObject*>(source);
        }
        ~SourceObject() override {
            if (source) rsx_destroy_source_object(source); 
            obj = nullptr;
        }
};

class SlotObject : public ConnectiveObject {
    private:
        RSXSlotObject* slot;

    public:
        SlotObject(RSXSlotObject* raw_slot, bool owner = false) : ConnectiveObject(reinterpret_cast<RSXConnectiveObject*>(raw_slot)), slot(raw_slot) {}
        SlotObject(uint32_t id, const ConnectiveObject& parent, RSXPowerType source_power_type) : ConnectiveObject(nullptr) { 
            slot = rsx_create_slot_object(id, parent.get_raw(), source_power_type); 
            obj = reinterpret_cast<RSXConnectiveObject*>(slot);
        }
        ~SlotObject() override { 
            if (slot) rsx_destroy_slot_object(slot); 
            obj = nullptr;
        }
};

class RelaySource : public SourceObject {
    private:
        RSXRelaySource* relay_source;

    public:
        RelaySource(uint32_t id, uint8_t power, uint32_t max_delay) : SourceObject(nullptr) {
            relay_source = rsx_create_relay_source(id, power, max_delay); 
            obj = reinterpret_cast<RSXConnectiveObject*>(relay_source);
        }
        ~RelaySource() override { 
            if (relay_source) rsx_destroy_relay_source(relay_source); 
            obj = nullptr;
        }
};

class ComparatorSource : public SourceObject {
    private:
        RSXComparatorSource* comparator_source;

    public:
        ComparatorSource(uint32_t id, uint32_t delay) : SourceObject(nullptr) { 
            comparator_source = rsx_create_comparator_source(id, delay); 
            obj = reinterpret_cast<RSXConnectiveObject*>(comparator_source);
        }
        ~ComparatorSource() override {
            if (comparator_source) rsx_destroy_comparator_source(comparator_source); 
            obj = nullptr;
        }
};

class TorchSource : public SourceObject {
    private:
        RSXTorchSource* torch_source;

    public:
        TorchSource(uint32_t id, uint8_t power, uint32_t delay) : SourceObject(nullptr) {
            torch_source = rsx_create_torch_source(id, power, delay); 
            obj = reinterpret_cast<RSXConnectiveObject*>(torch_source);
        }
        ~TorchSource() override {
            if (torch_source) rsx_destroy_torch_source(torch_source); 
            obj = nullptr;
        }
};

class Block : public LineObject {
    private:
        RSXBlock* block;

    public:
        Block(uint32_t id, uint32_t limit) : LineObject(nullptr) { 
            block = rsx_create_block(id, limit); 
            obj = reinterpret_cast<RSXConnectiveObject*>(block);
        }
        ~Block() override { 
            if (block) rsx_destroy_block(block); 
            obj = nullptr;
        }
};

extern "C" void c_log_callback_bridge(RSXLogLevel level, const char* msg, void* user_data);

class Simulator {
    private:
        RSXSimulator* sim;
        RSXLogLevel m_max_log_level = RSX_LOG_INFO;

    public:
        Simulator() { sim = rsx_create_simulator(); }
        ~Simulator() { rsx_destroy_simulator(sim); }
        void run() { rsx_simulator_run(sim); }
        bool step() { return rsx_simulator_step(sim); }
        void pause() { rsx_simulator_pause(sim); }
        void resume() { rsx_simulator_resume(sim); }

        RSXLogLevel get_max_log_level() const { return m_max_log_level; }
        void enable_logging(RSXLogLevel level = RSX_LOG_INFO) {
            if (!sim) return;
            m_max_log_level = level;
            rsx_simulator_set_log_callback(sim, c_log_callback_bridge, this);
        }

        void bind_object(const ConnectiveObject& obj){ rsx_simulator_bind_object(sim, obj.get_raw()); }
        void add_tick_breakpoint(uint32_t tick) { rsx_simulator_add_tick_breakpoint(sim, tick); }
        void remove_tick_breakpoint(uint32_t tick) { rsx_simulator_remove_tick_breakpoint(sim, tick); };
};

extern "C" void c_log_callback_bridge(RSXLogLevel level, const char* msg, void* user_data) {
    if (!user_data) return;

    auto* self = static_cast<Simulator*>(user_data);

    if (level < self->get_max_log_level()) {
        return;
    }

    std::string color_prefix = "";
    switch (level) {
        case RSX_LOG_DEBUG: color_prefix = ANSI_DEBUG; break;
        case RSX_LOG_INFO:  color_prefix = ANSI_INFO;  break;
        case RSX_LOG_WARN:  color_prefix = ANSI_WARN;  break;
        case RSX_LOG_ERROR: color_prefix = ANSI_ERROR; break;
        default:            color_prefix = ANSI_RESET; break;
    }

    std::string formatted_msg = color_prefix + "[PyRedStoneX Log] " + msg + ANSI_RESET;

    py::gil_scoped_acquire acquire;
    py::print(formatted_msg, py::arg("flush") = true);
}

std::unordered_map<std::string, ComponentOps> g_plugin_registry; 

/*
 * 这个自定义组件的创建基于插件注册而言，如果插件没有注册的话，是创建不了的
 * 哦对了，组件的定义一般来说还得是C/C++写吧，毕竟要接口什么的
 * 我其实不太会写cpp，写这个远远没有写c舒服，所以请原谅我代码质量没写C那么高
 */
class CustomObject : public ConnectiveObject {
    private:
        ComponentOps ops;

    public:
        CustomObject(const std::string& plugin_name, py::kwargs kwargs) : ConnectiveObject(nullptr) {
            auto it = g_plugin_registry.find(plugin_name);
            if (it == g_plugin_registry.end()) {
                throw std::runtime_error("Unregistry Plugin Name: " + plugin_name);
            }
            ops = it->second;

            // 这部分的动态args由Gemini生成，目前来看基本没有问题
            // 为了兼容C语言开发的插件，毕竟引擎本身就是纯C
            if (ops.create) {
                PluginArgs args_tool;
                args_tool.py_kwargs_ptr = &kwargs;

                args_tool.get_int = [](PluginArgs* self, const char* key, int64_t default_val) -> int64_t {
                    auto* dict = reinterpret_cast<py::kwargs*>(self->py_kwargs_ptr);
                    if (dict && dict->contains(key)) return (*dict)[key].cast<int64_t>();
                    return default_val;
                };

                args_tool.get_double = [](PluginArgs* self, const char* key, double default_val) -> double {
                    auto* dict = reinterpret_cast<py::kwargs*>(self->py_kwargs_ptr);
                    if (dict && dict->contains(key)) return (*dict)[key].cast<double>();
                    return default_val;
                };

                args_tool.get_bool = [](PluginArgs* self, const char* key, int default_val) -> int {
                    auto* dict = reinterpret_cast<py::kwargs*>(self->py_kwargs_ptr);
                    if (dict && dict->contains(key)) return (*dict)[key].cast<bool>() ? 1 : 0;
                    return default_val;
                };

                args_tool.get_string = [](PluginArgs* self, const char* key, const char* default_val) -> const char* {
                    auto* dict = reinterpret_cast<py::kwargs*>(self->py_kwargs_ptr);
                    if (dict && dict->contains(key)) {
                        PyObject* obj_ptr = (*dict)[key].ptr();
                        if (PyUnicode_Check(obj_ptr)) {
                            return PyUnicode_AsUTF8(obj_ptr); 
                        }
                    }
                    return default_val;
                };
                                
                obj = reinterpret_cast<RSXConnectiveObject*>(ops.create(&args_tool));
            }

            if (!obj) {
                throw std::runtime_error("Plugin create failed! ");
            }
        }

        ~CustomObject() override {
            if (obj) {
                if (ops.destroy) {
                    ops.destroy(obj);
                }
                obj = nullptr;
            }
        }

        // 来自Gemini C++胶水层代码有很多是由Gemini辅助完成的
        py::object get_dynamic_property(const std::string& name) const {
            if (!obj) throw py::attribute_error("Object is null");

            if (ops.get_property) {
                PropertyValue res = ops.get_property(obj, name.c_str());
                switch (res.type) {
                    case TYPE_INT:    return py::cast(res.value.v_int);
                    case TYPE_DOUBLE: return py::cast(res.value.v_double);
                    case TYPE_BOOL:   return py::cast(res.value.v_bool != 0);
                    case TYPE_STRING: return py::cast(res.value.v_string);
                    case TYPE_SLOT_PTR: {
                        if (!res.value.v_ptr) return py::none();
                
                        auto* raw_slot = reinterpret_cast<RSXSlotObject*>(res.value.v_ptr);
                        SlotObject wrapped_slot(raw_slot, false);
                        
                        return py::cast(wrapped_slot, py::return_value_policy::reference_internal);
                    }
                    default: break; 
                }
            }
            throw py::attribute_error("Attribute '" + name + "' not found.");
        }
};

void register_plugin_from_ptr(const std::string& name, size_t create_ptr, size_t destroy_ptr, size_t get_prop_ptr) {
    ComponentOps ops;
    ops.create  = reinterpret_cast<void* (*)(PluginArgs*)>(create_ptr);
    ops.destroy = reinterpret_cast<void (*)(void*)>(destroy_ptr);
    ops.get_property = reinterpret_cast<PropertyValue (*)(void*, const char*)>(get_prop_ptr);

    g_plugin_registry[name] = ops;
}

PYBIND11_MODULE(_core, m) {
    m.doc() = "RedStoneX 引擎核心";

    py::enum_<RSXPowerType>(m, "CorePowerType")
        .value("NONE", RSXPowerType::RSX_POWER_NONE)
        .value("WEAK", RSXPowerType::RSX_POWER_WEAK)
        .value("STRONG", RSXPowerType::RSX_POWER_STRONG)
        .export_values();

    py::class_<ConnectiveObject>(m, "CoreConnectiveObject")
        .def_static("create_view", &ConnectiveObject::create_view)
        .def("connect", &ConnectiveObject::connect)
        .def("disconnect", &ConnectiveObject::disconnect)
        .def_property_readonly("id", [](const ConnectiveObject &self) { return self.get_raw() ? self.get_raw()->id : 0; })
        .def_property_readonly("uri", [](const ConnectiveObject &self) { return self.get_raw() ? self.get_raw()->uri : ""; })
        .def_property_readonly("power", [](const ConnectiveObject &self) { return self.get_raw() ? self.get_raw()->power : 0; })
        .def_property_readonly("limit", [](const ConnectiveObject &self) { return self.get_raw() ? self.get_raw()->limit : 0; })
        .def_property_readonly("connect_count", [](const ConnectiveObject &self) { return self.get_raw() ? self.get_raw()->connect_count : 0; })
        .def_property_readonly("is_lossless", [](const ConnectiveObject &self) { return self.get_raw() ? self.get_raw()->is_lossless : false; })
        .def_property_readonly("is_weak_transmissible", [](const ConnectiveObject &self) { return self.get_raw() ? self.get_raw()->is_weak_transmissible : false; });

    py::class_<LineObject, ConnectiveObject>(m, "CoreLineObject")
        .def(py::init<uint32_t, uint32_t>());

    py::class_<SourceObject, ConnectiveObject>(m, "CoreSourceObject")
        .def(py::init<uint32_t, uint32_t, uint8_t>())
        .def_property_readonly("max_delay", [](const SourceObject &self) {
            auto* src = reinterpret_cast<RSXSourceObject*>(self.get_raw());
            return src ? src->max_delay : 0;
        });

    py::class_<SlotObject, ConnectiveObject>(m, "CoreSlotObject")
        .def(py::init<uint32_t, const ConnectiveObject&, RSXPowerType>());

    py::class_<RelaySource, SourceObject>(m, "CoreRelaySource")
        .def(py::init<uint32_t, uint8_t, uint32_t>())
        .def_property_readonly("delay", [](const RelaySource &self) {
            return reinterpret_cast<RSXRelaySource*>(self.get_raw())->delay;
        })
        .def_property_readonly("relay_power", [](const RelaySource &self) {
            return reinterpret_cast<RSXRelaySource*>(self.get_raw())->relay_power;
        })

        .def_property_readonly("_input_slot_ptr", [](const RelaySource &self) {
            auto* raw = reinterpret_cast<RSXRelaySource*>(self.get_raw());
            return reinterpret_cast<uintptr_t>(&raw->input_slot);
        }, py::return_value_policy::reference_internal)

        .def_property_readonly("_output_slot_ptr", [](const RelaySource &self) {
            auto* raw = reinterpret_cast<RSXRelaySource*>(self.get_raw());
            return reinterpret_cast<uintptr_t>(&raw->output_slot);
        }, py::return_value_policy::reference_internal);

    py::enum_<RSXComparatorSourceMode>(m, "CoreComparatorMode")
        .value("COMPARISON", RSXComparatorSourceMode::COMPARISON_MODE)
        .value("SUBTRACTION", RSXComparatorSourceMode::SUBTRACTION_MODE)
        .export_values();

    py::class_<ComparatorSource, SourceObject>(m, "CoreComparatorSource")
        .def(py::init<uint32_t, uint32_t>())
        .def_property_readonly("delay", [](const ComparatorSource &self) {
            return reinterpret_cast<RSXComparatorSource*>(self.get_raw())->delay;
        })
        .def_property_readonly("mode", [](const ComparatorSource &self) {
            return reinterpret_cast<RSXComparatorSource*>(self.get_raw())->mode;
        })

        .def_property_readonly("_input_slot_ptr", [](const ComparatorSource &self) {
            auto* raw = reinterpret_cast<RSXComparatorSource*>(self.get_raw());
            return reinterpret_cast<uintptr_t>(&raw->input_slot);
        }, py::return_value_policy::reference_internal)

        .def_property_readonly("_calculate_slot_a_ptr", [](const ComparatorSource &self) {
            auto* raw = reinterpret_cast<RSXComparatorSource*>(self.get_raw());
            return reinterpret_cast<uintptr_t>(&raw->calculate_slot_a);
        }, py::return_value_policy::reference_internal)

        .def_property_readonly("_calculate_slot_b_ptr", [](const ComparatorSource &self) {
            auto* raw = reinterpret_cast<RSXComparatorSource*>(self.get_raw());
            return reinterpret_cast<uintptr_t>(&raw->calculate_slot_b);
        }, py::return_value_policy::reference_internal)

        .def_property_readonly("_output_slot_ptr", [](const ComparatorSource &self) {
            auto* raw = reinterpret_cast<RSXComparatorSource*>(self.get_raw());
            return reinterpret_cast<uintptr_t>(&raw->output_slot);
        }, py::return_value_policy::reference_internal);

    py::class_<TorchSource, SourceObject>(m, "CoreTorchSource")
        .def(py::init<uint32_t, uint8_t, uint32_t>())
        .def_property_readonly("delay", [](const TorchSource &self) {
            return reinterpret_cast<RSXTorchSource*>(self.get_raw())->delay;
        })
        .def_property_readonly("torch_power", [](const TorchSource &self) {
            return reinterpret_cast<RSXTorchSource*>(self.get_raw())->torch_power;
        })

        .def_property_readonly("_bottom_slot_ptr", [](const TorchSource &self) {
            auto* raw = reinterpret_cast<RSXTorchSource*>(self.get_raw());
            return reinterpret_cast<uintptr_t>(&raw->bottom_slot);
        }, py::return_value_policy::reference_internal)

        .def_property_readonly("_power_slot_ptr", [](const TorchSource &self) {
            auto* raw = reinterpret_cast<RSXTorchSource*>(self.get_raw());
            return reinterpret_cast<uintptr_t>(&raw->power_slot);
        }, py::return_value_policy::reference_internal);

    py::class_<Block, LineObject>(m, "CoreBlock")
        .def(py::init<uint32_t, uint32_t>());

    py::class_<CustomObject, ConnectiveObject>(m, "CoreCustomObject")
        .def(py::init<const std::string&, py::kwargs>())
        .def("__getattr__", &CustomObject::get_dynamic_property);

    py::enum_<RSXLogLevel>(m, "CoreLogLevel")
        .value("DEBUG", RSX_LOG_DEBUG)
        .value("INFO", RSX_LOG_INFO)
        .value("WARN", RSX_LOG_WARN)
        .value("ERROR", RSX_LOG_ERROR)
        .export_values();

    py::class_<Simulator>(m, "CoreSimulator")
        .def(py::init<>())
        .def("bind_object", &Simulator::bind_object)
        .def("run", &Simulator::run)
        .def("pause", &Simulator::pause)
        .def("resume", &Simulator::resume)
        .def("step", &Simulator::step)
        .def("enable_logging", &Simulator::enable_logging)
        .def("add_tick_breakpoint", &Simulator::add_tick_breakpoint)
        .def("remove_tick_breakpoint", &Simulator::remove_tick_breakpoint);

    m.def("register_plugin_from_ptr", &register_plugin_from_ptr);
}
