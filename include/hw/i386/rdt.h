#ifndef HW_RDT_H
#define HW_RDT_H

#include <stdbool.h>
#include <stdint.h>

typedef struct RDTState RDTState;
typedef struct RDTStateInstance RDTStateInstance;
typedef struct RDTMonitor RDTMonitor;
typedef struct RDTAllocation RDTAllocation;

uint32_t rdt_get_cpuid_10_1_edx_cos_max(void);

uint32_t rdt_get_cpuid_10_2_edx_cos_max(void);

uint32_t rdt_get_cpuid_10_3_edx_cos_max(void);

bool rdt_associate_rmid_cos(uint64_t msr_ia32_pqr_assoc);

void rdt_write_msr_l3_mask(uint32_t pos, uint16_t val);
void rdt_write_msr_l2_mask(uint32_t pos, uint16_t val);
void rdt_write_mba_thrtl(uint32_t pos, uint16_t val);

uint16_t rdt_read_l3_mask(uint32_t pos);
uint16_t rdt_read_l2_mask(uint32_t pos);
uint16_t rdt_read_mba_thrtl(uint32_t pos);

uint64_t rdt_read_event_count(RDTStateInstance *rdt, uint32_t rmid, uint32_t event_id);
uint32_t rdt_max_rmid(RDTStateInstance *rdt);

#endif
