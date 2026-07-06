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

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "redstonex_obj.h"
#include "redstonex_sim.h"
#include "redstonex_types.h"
#include "plugin.h"

typedef struct {
    RSXLineObject base;
    uint8_t gate_power;
} ANDGate;

void ANDGate_update(RSXSimulateEvent* event, RSXSimulator* sim);

void* create_and_gate(PluginArgs* args) {
    assert(args != NULL && "Args missing!");

    uint32_t id = (uint32_t)args->get_int(args, "id", 0);
    assert(id != 0 && "ID cannot be zero!");

    ANDGate* gate = (ANDGate*)malloc(sizeof(ANDGate));
    if (!gate) return NULL;

    // 插件与门无损弱可穿透
    rsx_init_line_object(&gate->base, id, "gate_plugin:and_gate", 15, true, true);
    gate->base.base.on_update_cb = ANDGate_update;
    gate->gate_power = args->get_int(args, "gate_power", 15);

    printf("[SUCCESS] ANDGate init success! \n");

    return (void*)gate;
}

void destroy_and_gate(void* self) {
    if (self == NULL) return;

    ANDGate* gate = (ANDGate*)self;
    rsx_clean_line_object(&gate->base);
    free(gate);
}

PropertyValue ANDGate_get_property(void* self, const char* name) {
    PropertyValue res;
    res.type = TYPE_NONE;

    if (!self || !name) return res;
    ANDGate* gate = (ANDGate*)self;

    if (strcmp(name, "gate_power") == 0) {
        res.type = TYPE_INT;
        res.value.v_int = gate->gate_power;
    } 

    return res;
}

void ANDGate_update(RSXSimulateEvent* event, RSXSimulator* sim) {
    assert(event != NULL && sim != NULL);

    ANDGate* gate = (ANDGate*)event->target_object;
    RSXConnectiveObject* source = event->source_object;
    uint8_t power = event->power;
    RSXPowerType type = event->type;

    RSXLineObject_update_map(&gate->base, source, power, type);
    
    uint8_t new_power = 0;
    if (gate->base.power_map_count > 0) {
        bool all_inputs_active = true;
        
        for (uint32_t i = 0; i < gate->base.power_map_count; i++) {
            if (gate->base.power_map[i].power == 0) {
                all_inputs_active = false;
                break;
            }
        }
        
        if (all_inputs_active) {
            new_power = gate->gate_power;
        }
    }

    if (new_power != gate->base.base.power) {
        gate->base.base.power = new_power;
        // 调用通用update（这个update会把自己能量传给其他人）
        RSX_SUPER_BROADCAST(gate, source, gate->base.base.power, RSX_POWER_WEAK, sim);
    }
}

typedef struct {
    RSXLineObject base;
    uint8_t gate_power;
} ORGate;

void ORGate_update(RSXSimulateEvent* event, RSXSimulator* sim);

void* create_or_gate(PluginArgs* args) {
    assert(args != NULL && "Args missing!");

    uint32_t id = (uint32_t)args->get_int(args, "id", 0);
    assert(id != 0 && "ID cannot be zero!");

    ORGate* gate = (ORGate*)malloc(sizeof(ORGate));
    if (!gate) return NULL;

    // 插件与门无损弱可穿透
    rsx_init_line_object(&gate->base, id, "gate_plugin:or_gate", 15, true, true);
    gate->gate_power = args->get_int(args, "gate_power", 15);
    gate->base.base.on_update_cb = ORGate_update;

    printf("[SUCCESS] ORGate init success! \n");

    return (void*)gate;
}

void destroy_or_gate(void* self) {
    if (self == NULL) return;

    ORGate* gate = (ORGate*)self;
    rsx_clean_line_object(&gate->base);
    free(gate);
}

PropertyValue ORGate_get_property(void* self, const char* name) {
    PropertyValue res;
    res.type = TYPE_NONE;

    if (!self || !name) return res;
    ORGate* gate = (ORGate*)self;

    if (strcmp(name, "gate_power") == 0) {
        res.type = TYPE_INT;
        res.value.v_int = gate->gate_power;
    } 

    return res;
}

void ORGate_update(RSXSimulateEvent* event, RSXSimulator* sim) {
    assert(event != NULL && sim != NULL);

    ORGate* gate = (ORGate*)event->target_object;
    RSXConnectiveObject* source = event->source_object;
    uint8_t power = event->power;
    RSXPowerType type = event->type;

    RSXLineObject_update_map(&gate->base, source, power, type);
    
    uint8_t new_power = 0;
    if (gate->base.power_map_count > 0) {
        for (uint32_t i = 0; i < gate->base.power_map_count; i++) {
            if (gate->base.power_map[i].power > 0) {
                new_power = gate->gate_power;
                break;
            }
        }
    }

    if (new_power != gate->base.base.power) {
        gate->base.base.power = new_power;
        // 调用通用update（这个update会把自己能量传给其他人）
        RSX_SUPER_BROADCAST(gate, source, gate->base.base.power, RSX_POWER_WEAK, sim);
    }
}
