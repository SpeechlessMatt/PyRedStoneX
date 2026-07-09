#include <redstonex_obj.h>
#include <stdint.h>

#define URI_NS "gate_plugin"
#define URI_GATE_LINE URI_NS ":gate_line"
#define URI_GATE_AND URI_NS ":and_gate"
#define URI_GATE_OR URI_NS ":or_gate"
#define URI_GATE_NOT URI_NS ":not_gate"
#define URI_GATE_NAND URI_NS ":nand_gate"
#define URI_GATE_XOR URI_NS ":xor_gate"
#define URI_GATE_NOR URI_NS ":nor_gate"
#define URI_GATE_XNOR URI_NS ":xnor_gate"
#define URI_GATE_TRI URI_NS ":tri_gate"

typedef struct LogicGate LogicGate;
typedef struct TRIGate TRIGate;

typedef enum {
    GATE_AND = 0,
    GATE_OR,
    GATE_NOT,
    GATE_NAND,
    GATE_XOR,
    GATE_NOR,
    GATE_XNOR,
    GATE_TRI
} GateType;

struct LogicGate {
    RSXLineObject base;
    RSXSlotObject output_slot;
    uint8_t gate_power;
    GateType gate_type;
};

struct TRIGate {
    RSXConnectiveObject base;
    RSXSlotObject input_slot;
    RSXSlotObject output_slot;
    RSXSlotObject en_slot;
};

void ANDGate_update(RSXSimulateEvent* event, RSXSimulator* sim);
void ORGate_update(RSXSimulateEvent* event, RSXSimulator* sim);
void TRIGate_update(RSXSimulateEvent* event, RSXSimulator* sim);
void NANDGate_update(RSXSimulateEvent* event, RSXSimulator* sim);
void XORGate_update(RSXSimulateEvent* event, RSXSimulator* sim);
void NORGate_update(RSXSimulateEvent* event, RSXSimulator* sim);
void XNORGate_update(RSXSimulateEvent* event, RSXSimulator* sim);
