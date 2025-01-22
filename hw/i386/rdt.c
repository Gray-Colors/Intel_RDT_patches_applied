/*
 * Intel Resource Director Technology (RDT).
 *
 * Copyright 2024 Google LLC
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 */

#include "hw/i386/rdt.h"
#include "qemu/osdep.h" /* Needs to be included before isa.h */
#include "hw/isa/isa.h"
#include "hw/qdev-properties.h"
#include "qom/object.h"
#include "target/i386/cpu.h"

#define TYPE_RDT "rdt"
#define RDT_NUM_RMID_PROP "rmids"

OBJECT_DECLARE_TYPE(RDTState, RDTStateClass, RDT);

struct RDTMonitor {
    uint64_t count_local;
    uint64_t count_remote;
    uint64_t count_l3;
};

struct RDTAllocation {
    uint32_t active_cos;
};

struct RDTStatePerCore {
    uint32_t active_rmid;
    GArray *monitors;

    /*Parent RDTState*/
    RDTState *rdtstate;
};

/*One instance of RDT-internal state to be shared by all cores*/
struct RDTState {
    ISADevice parent;

    /*Max amount of RMIDs*/
    uint32_t rmids;

    /*Per core state*/
    RDTStatePerCore *rdtInstances;
    RDTAllocation *allocations;

    /*RDT Allocation bitmask MSRs*/
    uint32_t msr_L3_ia32_mask_n[RDT_MAX_L3_MASK_COUNT];
    uint32_t msr_L2_ia32_mask_n[RDT_MAX_L2_MASK_COUNT];
    uint32_t ia32_L2_qos_ext_bw_thrtl_n[RDT_MAX_MBA_THRTL_COUNT];
};

struct RDTStateClass {
};

OBJECT_DEFINE_TYPE(RDTState, rdt, RDT, ISA_DEVICE);

static Property rdt_properties[] = {
    DEFINE_PROP_UINT32(RDT_NUM_RMID_PROP, RDTState, rmids, 256),
};

static void rdt_init(Object *obj)
{
}

static void rdt_realize(DeviceState *dev, Error **errp)
{
    CPUState *cs = first_cpu;
    RDTState *rdtDev = RDT(dev);

    rdtDev->rdtInstances = g_malloc(sizeof(RDTStatePerCore) * cs->nr_cores);
    CPU_FOREACH(cs) {
        RDTStatePerCore *rdt = &rdtDev->rdtInstances[cs->cpu_index];
        X86CPU *cpu = X86_CPU(cs);

        rdt->rdtstate = rdtDev;
        cpu->rdt = rdt;

        rdt->monitors = g_malloc(sizeof(RDTMonitor) * rdtDev->rmids);
        rdt->rdtstate->allocations = g_malloc(sizeof(RDTAllocation) * rdtDev->rmids);
    }
}

static void rdt_finalize(Object *obj)
{
    CPUState *cs;
    RDTState *rdt = RDT(obj);

    CPU_FOREACH(cs) {
        RDTStatePerCore *rdtInstance = &rdt->rdtInstances[cs->cpu_index];
        g_free(rdtInstance->monitors);
        g_free(rdtInstance->rdtstate->allocations);
    }

    g_free(rdt->rdtInstances);
}

static void rdt_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->hotpluggable = false;
    dc->desc = "RDT";
    dc->user_creatable = true;
    dc->realize = rdt_realize;

    device_class_set_props(dc, rdt_properties);
}
