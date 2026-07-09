#include <assert.h>
#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <redstonex_obj.h>
#include <redstonex_types.h>
#include <redstonex_sim.h>
#include <redstonex/plugin.h>

#include "gate_plugin.h"

#define GATE_OUTPUT_SLOT_ID 0xf

// LogicGate* create_not_gate(PluginArgs* args) { return create_gate(args, URI_GATE_NOT, GATE_NOT); }
// LogicGate* create_tri_gate(PluginArgs* args) { return create_gate(args, URI_GATE_TRI, GATE_TRI); }

bool init_gate_line(RSXLineObject* line, uint32_t id, const char* uri, uint32_t limit) {
    assert(line != NULL);

    if (!rsx_init_line_object(line, id, uri, limit, true, true)) {
        return false;
    }

    return true;
}

void* create_gate_line(PluginArgs* args) {
    assert(args != NULL && "Args missing!");

    uint32_t id = (uint32_t)args->get_int(args, "id", 0);
    assert(id != 0 && "ID cannot be zero!");

    RSXLineObject* line = (RSXLineObject*)malloc(sizeof(RSXLineObject));
    if (line == NULL) return NULL;

    uint32_t limit = (uint32_t)args->get_int(args, "limit", 3) + 1;
    uint8_t power = (uint8_t)args->get_int(args, "power", 15);

    if (!init_gate_line(line, id, URI_GATE_LINE, limit)) {
        free(line);
        return NULL;
    }

    return (void*)line;
}

bool init_tri_gate(TRIGate* gate, uint32_t id, const char* uri) {
    assert(gate != NULL);

    if (!rsx_init_object(&gate->base, id, RSX_ROLE_OBJECT, uri, 0, 3, true, true)) {
        return false;
    }

    if (!rsx_init_slot_object(&gate->input_slot, id, RSX_URI_SLOT, 2, (RSXConnectiveObject*)gate, RSX_POWER_NONE)) {
        rsx_clean_object(&gate->base);
        return false;
    }
    if (!rsx_init_slot_object(&gate->output_slot, id, RSX_URI_SLOT, 2, (RSXConnectiveObject*)gate, RSX_POWER_NONE)) {
        rsx_clean_object(&gate->base);
        rsx_clean_slot_object(&gate->input_slot);
        return false;
    }
    if (!rsx_init_slot_object(&gate->en_slot, id, RSX_URI_SLOT, 2, (RSXConnectiveObject*)gate, RSX_POWER_NONE)) {
        rsx_clean_object(&gate->base);
        rsx_clean_slot_object(&gate->input_slot);
        rsx_clean_slot_object(&gate->output_slot);
        return false;
    }
    //TODO
    gate->base.on_update_cb = TRIGate_update;

    return true;
}

void* create_tri_gate(PluginArgs* args) {
    assert(args != NULL && "Args missing!");

    uint32_t id = (uint32_t)args->get_int(args, "id", 0);
    assert(id != 0 && "ID cannot be zero!");

    TRIGate* gate = (TRIGate*)malloc(sizeof(TRIGate));
    if (gate == NULL) return NULL;

    if (!init_tri_gate(gate, id, URI_GATE_TRI)) {
        free(gate);
        return NULL;
    }

    return (void*)gate;
}

void TRIGate_update(RSXSimulateEvent* event, RSXSimulator* sim) {
    assert(event != NULL && sim != NULL);

    TRIGate* self = (TRIGate*)event->target_object;
    RSXConnectiveObject* source = event->source_object;
    // TODO
}

bool init_gate(LogicGate* gate, uint32_t id, const char* uri, uint32_t limit, uint8_t power, GateType type, RSXUpdateCallback on_update_cb) {
    assert(gate != NULL);

    if (!rsx_init_line_object(&gate->base, id, uri, limit, true, true)) {
        return false;
    }

    // RSX_POWER_NONE 视作高阻态哦
    if (!rsx_init_slot_object(&gate->output_slot, id, uri, 2, (RSXConnectiveObject*)gate, RSX_POWER_NONE)) {
        rsx_clean_line_object(&gate->base);
        return false;
    }

    gate->base.base.on_update_cb = on_update_cb;
    gate->gate_power = power;
    gate->gate_type = type;

    return true;
}

static inline void* create_gate(PluginArgs* args, const char* uri, GateType type, RSXUpdateCallback on_update_cb) {
    assert(args != NULL && "Args missing!");

    uint32_t id = (uint32_t)args->get_int(args, "id", 0);
    assert(id != 0 && "ID cannot be zero!");

    LogicGate* gate = (LogicGate*)malloc(sizeof(LogicGate));
    if (gate == NULL) return NULL;

    // 因为有一个连接了slot 所以额 要+1
    uint32_t limit = (uint32_t)args->get_int(args, "limit", 3) + 1;
    uint8_t power = (uint8_t)args->get_int(args, "power", 15);

    if (!init_gate(gate, id, uri, limit, power, type, on_update_cb)) {
        free(gate);
        return NULL;
    }

    return (void*)gate;
}

void* create_and_gate(PluginArgs* args) { return create_gate(args, URI_GATE_AND, GATE_AND, ANDGate_update); }
void* create_or_gate(PluginArgs* args) { return create_gate(args, URI_GATE_OR, GATE_OR, ORGate_update); }
void* create_nand_gate(PluginArgs* args) { return create_gate(args, URI_GATE_NAND, GATE_NAND, NANDGate_update); }
void* create_xor_gate(PluginArgs* args) { return create_gate(args, URI_GATE_XOR, GATE_XOR, XORGate_update); }
void* create_nor_gate(PluginArgs* args) { return create_gate(args, URI_GATE_NOR, GATE_NOR, NORGate_update); }
void* create_xnor_gate(PluginArgs* args) { return create_gate(args, URI_GATE_XNOR, GATE_XNOR, XNORGate_update); }

void destroy_logic_gate(void* self) {
    if (self == NULL) return;
    LogicGate* gate = (LogicGate*)self;

    rsx_clean_slot_object(&gate->output_slot);
    rsx_clean_line_object(&gate->base);

    free(gate);
}

PropertyValue get_logic_gate_property(void* self, const char* name) {
    PropertyValue res;
    res.type = TYPE_NONE;

    if (!self || !name) return res;
    LogicGate* gate = (LogicGate*)self;

    if (strcmp(name, "gate_power") == 0) {
        res.type = TYPE_INT;
        res.value.v_int = gate->gate_power;
    }
    else if (strcmp(name, "type") == 0) {
        res.type = TYPE_STRING;
        switch (gate->gate_type) {
            case GATE_AND:  res.value.v_string = "AND";  break;
            case GATE_OR:   res.value.v_string = "OR";   break;
            case GATE_NOT:  res.value.v_string = "NOT";  break;
            case GATE_NAND: res.value.v_string = "NAND"; break;
            case GATE_XOR:  res.value.v_string = "XOR";  break;
            case GATE_NOR:  res.value.v_string = "NOR";  break;
            case GATE_XNOR: res.value.v_string = "XNOR"; break;
            case GATE_TRI:  res.value.v_string = "TRI";  break;
            default:        res.value.v_string = "UNKNOWN"; break;
        }
    }
    else if (strcmp(name, "output_slot") == 0) {
        res.type = TYPE_SLOT_PTR;
        res.value.v_ptr = (void*)&gate->output_slot;
    }

    return res;
}

void ANDGate_update(RSXSimulateEvent* event, RSXSimulator* sim) {
    assert(event != NULL && sim != NULL);

    LogicGate* self = (LogicGate*)event->target_object;
    RSXConnectiveObject* source = event->source_object;
    uint8_t power = event->power;
    RSXPowerType type = event->type;

    assert(self != NULL);

    // 不接受输出接口的输入，反正目前来说就是这样
    if (source == (RSXConnectiveObject*)&self->output_slot) {
        return;
    }

    RSXLineObject_update_map(&self->base, source, power, type);

    uint8_t new_power = self->gate_power;
    RSXPowerType new_type = RSX_POWER_STRONG;
    uint32_t valid_input_count = 0;

    for (uint32_t i = 0; i < self->base.power_map_count; i++) {
        // 无视高阻态
        if (self->base.power_map[i].type == RSX_POWER_NONE) continue;
        valid_input_count++;
        if (self->base.power_map[i].power == 0) {
            new_power = 0;
            break;
        }
    }
    if (valid_input_count == 0) {
        new_power = 0;
        new_type = RSX_POWER_NONE;
    }

    if (new_power != self->base.base.power || new_type != self->output_slot.source_power_type) {
        self->base.base.power = new_power;
        rsx_simulator_append_deque(sim, (RSXConnectiveObject*)&self->output_slot, (RSXConnectiveObject*)self, new_power, new_type);
    }
}

void ORGate_update(RSXSimulateEvent* event, RSXSimulator* sim) {
    assert(event != NULL && sim != NULL);

    LogicGate* self = (LogicGate*)event->target_object;
    RSXConnectiveObject* source = event->source_object;
    uint8_t power = event->power;
    RSXPowerType type = event->type;

    assert(self != NULL);

    // 不接受输出接口的输入，反正目前来说就是这样
    if (source == (RSXConnectiveObject*)&self->output_slot) {
        return;
    }

    RSXLineObject_update_map(&self->base, source, power, type);

    uint8_t new_power = 0;
    RSXPowerType new_type = RSX_POWER_STRONG;
    uint32_t valid_input_count = 0;

    for (uint32_t i = 0; i < self->base.power_map_count; i++) {
        // 无视高阻态
        if (self->base.power_map[i].type == RSX_POWER_NONE) continue;
        valid_input_count++;
        if (self->base.power_map[i].power > 0) {
            new_power = self->gate_power;
            break;
        }
    }
    if (valid_input_count == 0) {
        new_power = 0;
        new_type = RSX_POWER_NONE;
    }

    if (new_power != self->base.base.power || new_type != self->output_slot.source_power_type) {
        self->base.base.power = new_power;
        rsx_simulator_append_deque(sim, (RSXConnectiveObject*)&self->output_slot, (RSXConnectiveObject*)self, new_power, new_type);
    }
}

void NANDGate_update(RSXSimulateEvent* event, RSXSimulator* sim) {
    assert(event != NULL && sim != NULL);

    LogicGate* self = (LogicGate*)event->target_object;
    RSXConnectiveObject* source = event->source_object;
    uint8_t power = event->power;
    RSXPowerType type = event->type;

    assert(self != NULL);

    // 不接受输出接口的输入，反正目前来说就是这样
    if (source == (RSXConnectiveObject*)&self->output_slot) {
        return;
    }

    RSXLineObject_update_map(&self->base, source, power, type);

    uint8_t new_power = self->gate_power;
    RSXPowerType new_type = RSX_POWER_STRONG;
    uint32_t valid_input_count = 0;

    for (uint32_t i = 0; i < self->base.power_map_count; i++) {
        // 无视高阻态
        if (self->base.power_map[i].type == RSX_POWER_NONE) continue;
        valid_input_count++;
        if (self->base.power_map[i].power == 0) {
            new_power = 0;
            break;
        }
    }
    if (valid_input_count == 0) {
        new_power = 0;
        new_type = RSX_POWER_NONE;
    }
    else {
        // NOT
        if (new_power == 0) {
            new_power = self->gate_power;
        } 
        else {
            new_power = 0;
        }
    }

    if (new_power != self->base.base.power || new_type != self->output_slot.source_power_type) {
        self->base.base.power = new_power;
        rsx_simulator_append_deque(sim, (RSXConnectiveObject*)&self->output_slot, (RSXConnectiveObject*)self, new_power, new_type);
    }
}

void NORGate_update(RSXSimulateEvent* event, RSXSimulator* sim) {
    assert(event != NULL && sim != NULL);

    LogicGate* self = (LogicGate*)event->target_object;
    RSXConnectiveObject* source = event->source_object;
    uint8_t power = event->power;
    RSXPowerType type = event->type;

    assert(self != NULL);

    // 不接受输出接口的输入，反正目前来说就是这样
    if (source == (RSXConnectiveObject*)&self->output_slot) {
        return;
    }

    RSXLineObject_update_map(&self->base, source, power, type);

    uint8_t new_power = 0;
    RSXPowerType new_type = RSX_POWER_STRONG;
    uint32_t valid_input_count = 0;

    for (uint32_t i = 0; i < self->base.power_map_count; i++) {
        // 无视高阻态
        if (self->base.power_map[i].type == RSX_POWER_NONE) continue;
        valid_input_count++;
        if (self->base.power_map[i].power > 0) {
            new_power = self->gate_power;
            break;
        }
    }
    if (valid_input_count == 0) {
        new_power = 0;
        new_type = RSX_POWER_NONE;
    }
    else {
        // NOT
        if (new_power == 0) {
            new_power = self->gate_power;
        } 
        else {
            new_power = 0;
        }
    }

    if (new_power != self->base.base.power || new_type != self->output_slot.source_power_type) {
        self->base.base.power = new_power;
        rsx_simulator_append_deque(sim, (RSXConnectiveObject*)&self->output_slot, (RSXConnectiveObject*)self, new_power, new_type);
    }
}

void XORGate_update(RSXSimulateEvent* event, RSXSimulator* sim) {
    assert(event != NULL && sim != NULL);

    LogicGate* self = (LogicGate*)event->target_object;
    RSXConnectiveObject* source = event->source_object;
    uint8_t power = event->power;
    RSXPowerType type = event->type;

    assert(self != NULL);

    // 不接受输出接口的输入，反正目前来说就是这样
    if (source == (RSXConnectiveObject*)&self->output_slot) {
        return;
    }

    RSXLineObject_update_map(&self->base, source, power, type);

    uint8_t new_power = 0;
    RSXPowerType new_type = RSX_POWER_STRONG;
    uint32_t valid_input_count = 0;

    bool xor_state = false;
    for (uint32_t i = 0; i < self->base.power_map_count; i++) {
        // 无视高阻态
        if (self->base.power_map[i].type == RSX_POWER_NONE) continue;
        valid_input_count++;
        if (self->base.power_map[i].power > 0) {
            xor_state = !xor_state;
        }
    }
    if (valid_input_count == 0) {
        new_power = 0;
        new_type = RSX_POWER_NONE;
    }
    else {
        new_power = xor_state ? self->gate_power : 0;
    }

    if (new_power != self->base.base.power || new_type != self->output_slot.source_power_type) {
        self->base.base.power = new_power;
        rsx_simulator_append_deque(sim, (RSXConnectiveObject*)&self->output_slot, (RSXConnectiveObject*)self, new_power, new_type);
    }
}

void XNORGate_update(RSXSimulateEvent* event, RSXSimulator* sim) {
    assert(event != NULL && sim != NULL);

    LogicGate* self = (LogicGate*)event->target_object;
    RSXConnectiveObject* source = event->source_object;
    uint8_t power = event->power;
    RSXPowerType type = event->type;

    assert(self != NULL);

    // 不接受输出接口的输入，反正目前来说就是这样
    if (source == (RSXConnectiveObject*)&self->output_slot) {
        return;
    }

    RSXLineObject_update_map(&self->base, source, power, type);

    uint8_t new_power = 0;
    RSXPowerType new_type = RSX_POWER_STRONG;
    uint32_t valid_input_count = 0;

    bool xor_state = true;
    for (uint32_t i = 0; i < self->base.power_map_count; i++) {
        // 无视高阻态
        if (self->base.power_map[i].type == RSX_POWER_NONE) continue;
        valid_input_count++;
        if (self->base.power_map[i].power > 0) {
            xor_state = !xor_state;
        }
    }
    if (valid_input_count == 0) {
        new_power = 0;
        new_type = RSX_POWER_NONE;
    }
    else {
        new_power = xor_state ? self->gate_power : 0;
    }

    if (new_power != self->base.base.power || new_type != self->output_slot.source_power_type) {
        self->base.base.power = new_power;
        rsx_simulator_append_deque(sim, (RSXConnectiveObject*)&self->output_slot, (RSXConnectiveObject*)self, new_power, new_type);
    }
}

