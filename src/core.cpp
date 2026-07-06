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

namespace py = pybind11;

class ConnectiveObject {
    protected:
        RSXConnectiveObject* obj;

    public:
        ConnectiveObject(RSXConnectiveObject* raw_obj) : obj(raw_obj) {}
        ConnectiveObject(uint32_t id, RSXObjectRole role, uint32_t limit, bool is_lossless, bool is_weak_transmissible) { obj = rsx_create_object(id, role, limit, is_lossless, is_weak_transmissible); }
        virtual ~ConnectiveObject() { if (obj) rsx_destroy_object(obj); }

        RSXConnectiveObject* get_raw() const { return obj; }

        void connect(const ConnectiveObject& target) { rsx_connect_objects(obj, target.get_raw()); }
        void disconnect(const ConnectiveObject& target) { rsx_disconnect_objects(obj, target.get_raw()); }
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
        SlotObject(RSXSlotObject* raw_slot) : ConnectiveObject(reinterpret_cast<RSXConnectiveObject*>(raw_slot)), slot(raw_slot) {}
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

class Simulator {
    private:
        RSXSimulator* sim;

    public:
        Simulator() { sim = rsx_create_simulator(); }
        ~Simulator() { rsx_destroy_simulator(sim); }
        void run() { rsx_simulator_run(sim); }
        void step() { rsx_simulator_step(sim); }
        void pause() { rsx_simulator_pause(sim); }
        void resume() { rsx_simulator_resume(sim); }
        void bind_object(const ConnectiveObject& obj){ rsx_simulator_bind_object(sim, obj.get_raw()); }
        void add_tick_breakpoint(uint32_t tick) { rsx_simulator_add_tick_breakpoint(sim, tick); }
        void remove_tick_breakpoint(uint32_t tick) { rsx_simulator_remove_tick_breakpoint(sim, tick); };
};

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

            // TODO:希望以后的我再来看一眼
            // 这部分的动态args由Gemini生成，不会写而且也没精力了...
            // 以后还是得看一眼的，AI生成的代码容易抽风...
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

                // 🚀 安全的 get_string 实现，无局部临时变量生命周期问题
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

        // 来自Gemini 这个动态属性我不会写 而且脑力已经耗尽了 肝了一整天了 交给ai了
        py::object get_dynamic_property(const std::string& name) const {
            if (!obj) throw py::attribute_error("Object is null");

            if (ops.get_property) {
                PropertyValue res = ops.get_property(obj, name.c_str());
                switch (res.type) {
                    case TYPE_INT:    return py::cast(res.value.v_int);
                    case TYPE_DOUBLE: return py::cast(res.value.v_double);
                    case TYPE_BOOL:   return py::cast(res.value.v_bool != 0);
                    case TYPE_STRING: return py::cast(res.value.v_string);
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
        .def("connect", &ConnectiveObject::connect)
        .def("disconnect", &ConnectiveObject::disconnect)
        .def_property_readonly("id", [](const ConnectiveObject &self) { return self.get_raw() ? self.get_raw()->id : 0; })
        .def_property_readonly("uri", [](const ConnectiveObject &self) { return self.get_raw() ? self.get_raw()->uri : ""; })
        .def_property_readonly("power", [](const ConnectiveObject &self) { return self.get_raw() ? self.get_raw()->power : 0; })
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
        });

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
        });

    py::class_<TorchSource, SourceObject>(m, "CoreTorchSource")
        .def(py::init<uint32_t, uint8_t, uint32_t>())
        .def_property_readonly("delay", [](const TorchSource &self) {
            return reinterpret_cast<RSXTorchSource*>(self.get_raw())->delay;
        })
        .def_property_readonly("torch_power", [](const TorchSource &self) {
            return reinterpret_cast<RSXTorchSource*>(self.get_raw())->torch_power;
        });

    py::class_<Block, LineObject>(m, "CoreBlock")
        .def(py::init<uint32_t, uint32_t>());

    py::class_<CustomObject, ConnectiveObject>(m, "CoreCustomObject")
        .def(py::init<const std::string&, py::kwargs>())
        .def("__getattr__", &CustomObject::get_dynamic_property);

    py::class_<Simulator>(m, "CoreSimulator")
        .def(py::init<>())
        .def("bind_object", &Simulator::bind_object)
        .def("run", &Simulator::run)
        .def("pause", &Simulator::pause)
        .def("resume", &Simulator::resume)
        .def("step", &Simulator::step)
        .def("add_tick_breakpoint", &Simulator::add_tick_breakpoint)
        .def("remove_tick_breakpoint", &Simulator::remove_tick_breakpoint);

    m.def("register_plugin_from_ptr", &register_plugin_from_ptr);
}
