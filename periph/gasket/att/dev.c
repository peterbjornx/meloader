//
// Created by pbx on 15/04/19.
//
#include <string.h>
#include <stdio.h>
#include "gasket/att/regs.h"
#include "gasket/att/dev.h"
#include "user/meloader.h"
#include "log.h"
#include "sideband.h"
#include "pci/device.h"

static int att_cfg_read( pci_func *func, uint64_t addr,       void *out, int count )
{
    int off;
    att_inst *i = func->device->impl;

    /* Check if this is a type 0 transaction */
    if ( PCI_ADDR_TYPE(addr) == PCI_ADDR_TYPE1 ) {
        return pci_bus_config_read(  &i->prim_bus, addr, out, count );
    }

    /* The bus only issues type 0 transactions to the targeted function,
     * so we do not need to verify the BDF part of the address */

    off = PCI_ADDR_REG(addr);

    /* Validate the target addresses */
    if ( off + count > PCI_CONFIG_SIZE ) {
        log( LOG_ERROR,
             func->device->name,
             "Configuration read beyond bounds ( Offset: 0x%03x, Size: 0x%x )",
             off, count);
        return -1;
    }

    /* Copy the configuration space contents to the buffer */
    memcpy( out, off + (void *) &func->config, count );

    /* Signal successful completion */
    return 0;

}

static int att_cfg_write( pci_func *func, uint64_t addr, const void *out, int count )
{
    int off;
    att_inst *i = func->device->impl;

    /* Check if this is a type 0 transaction */
    if ( PCI_ADDR_TYPE(addr) == PCI_ADDR_TYPE1 ) {
        return pci_bus_config_write(  &i->prim_bus, addr, out, count );
    }

    /* The bus only issues type 0 transactions to the targeted function,
     * so we do not need to verify the BDF part of the address */

    off = PCI_ADDR_REG(addr);

    /* Validate the target addresses */
    if ( off + count > PCI_CONFIG_SIZE ) {
        log( LOG_ERROR,
             func->device->name,
             "Configuration write beyond bounds ( Offset: 0x%03x, Size: 0x%x )",
             off, count);
        return -1;
    }

    /* Copy the configuration space contents to the buffer */
    memcpy( off + (void *) &func->config, out, count );

    /* Signal successful completion */
    return 0;

}

static int att_mem_write( pci_func *func, uint64_t addr, const void *buf, int count, int sai, int lat )
{
    uint64_t off = 0, base;
    int bar, s ;
    att_inst *i = func->device->impl;

    /* Match the address against our BARs */
    for ( bar = 0; bar < 2; bar++ ) {
        base = func->config.type0.bar[bar]   & PCI_BAR_ADDRESS_MASK;
        if ( addr < base)
            continue;
        off = addr - base;
        if ( off < 0x1000 )
            break;
    }

    /* Check if the access hit one of our BARs */
    if ( bar == 2 ) {
        s = att_prim_write( i, addr, buf, count, sai );
        if ( s >= 0 )
            return s;
        return att_sb_write( i, addr, buf, count, sai );
    }

    /* Validate the target addresses */
    if ( off + count > 0x1000 ) {
        log( LOG_ERROR,
             func->device->name,
             "Memory write beyond bounds"
             "( BAR %i, Offset: 0x%03x, Size: 0x%x )",
             bar, off, count);
        return -1;
    }

    /* Check if memory space is enabled */
    if ( ~func->config.type0.command & PCI_COMMAND_MSE ) {
        log( LOG_WARN,
             func->device->name,
             "Memory write while memory space disabled"
             "( BAR %i, Offset: 0x%03x, Size: 0x%x )",
             bar, off, count);
        return -1;
    }

    if ( bar == 0 )
        return att_prim_regs_write( i, off, buf, count );
    if ( bar == 1 )
        return att_sb_regs_write( i, off, buf, count );

    return 0;

}

static int att_mem_read( pci_func *func, uint64_t addr,       void *buf, int count, int sai, int lat )
{
    uint64_t off = 0, base;
    int bar, s ;
    att_inst *i = func->device->impl;

    /* Match the address against our BARs */
    for ( bar = 0; bar < 2; bar++ ) {
        base = func->config.type0.bar[bar]   & PCI_BAR_ADDRESS_MASK;
        if ( addr < base)
            continue;
        off = addr - base;
        if ( off < 0x1000 )
            break;
    }

    /* Check if the access hit one of our BARs */
    if ( bar == 2 ) {
        s = att_prim_read( i, addr, buf, count, sai );
        if ( s >= 0 )
            return s;
        return att_sb_read( i, addr, buf, count, sai );
    }

    /* Validate the target addresses */
    if ( off + count > 0x1000 ) {
        log( LOG_ERROR,
             func->device->name,
             "Memory read beyond bounds"
             "( BAR %i, Offset: 0x%03x, Size: 0x%x )",
             bar, off, count);
        return -1;
    }

    /* Check if memory space is enabled */
    if ( ~func->config.type0.command & PCI_COMMAND_MSE ) {
        log( LOG_WARN,
             func->device->name,
             "Memory read while memory space disabled"
             "( BAR %i, Offset: 0x%03x, Size: 0x%x )",
             bar, off, count);
        return -1;
    }

    if ( bar == 0 )
        return att_prim_regs_read( i, off, buf, count );
    if ( bar == 1 )
        return att_sb_regs_read( i, off, buf, count );

    return -1;

}

void att_prim_ret_init( att_inst *i );

device_instance * att_spawn(const cfg_file *file, const cfg_section *section) {
    int j;
    const char *name;
    char vn[40];
    uint32_t endpt, type, bar, rs, func;
    uint64_t ext;
    att_inst *i = malloc( sizeof(att_inst) );

    if ( !i ) {
        log( LOG_FATAL, section->name, "Could not allocate ATT instance structure" );
        exit(EXIT_FAILURE);
    }
    memset( i, 0, sizeof(att_inst) );

    i->self.impl      = i;
    i->self.name      = section->name;
    i->func.device    = &i->self;
    i->func.mem_read  = att_mem_read;
    i->func.mem_write = att_mem_write;
    i->func.cfg_read  = att_cfg_read;
    i->func.cfg_write = att_cfg_write;

    pci_cfg_handle_type0 ( &i->func, section, 0 );
    pci_cfg_handle_device( &i->func, section );

    name = cfg_find_string( section, "att_bus" );
    logassert( name != NULL, section->name, "No gasket bus name specified in config");

    i->prim_bus.name = name;
    i->prim_bus.bus_num = 0;
    pci_bus_register(&i->prim_bus);

    for ( j = 0; j < 128; j++ ) {
        snprintf( vn, 40, "att_win_%i_int_ba", j);
        cfg_find_int32( section, vn, &i->regs.WIN[j].INT_BA );
        snprintf( vn, 40, "att_win_%i_size", j);
        cfg_find_int32( section, vn, &i->regs.WIN[j].INT_SIZE );
        snprintf( vn, 40, "att_win_%i_ext_ba", j);
        cfg_find_int64( section, vn, &ext );
        i->regs.WIN[j].EXT_BA_HI = (ext >> 32ULL);
        i->regs.WIN[j].EXT_BA_LO = ext;
        snprintf( vn, 40, "att_win_%i_control", j);
        cfg_find_int32( section, vn, &i->regs.WIN[j].CONTROL );
    }

    for ( j = 0; j < 128; j++ ) {
        snprintf( vn, 40, "att_sbwin_%i_base", j);
        cfg_find_int32( section, vn, &i->regs.SB_WIN[j].window_base );
        snprintf( vn, 40, "att_sbwin_%i_size", j);
        cfg_find_int32( section, vn, &i->regs.SB_WIN[j].window_size );
        func = 0;
        rs = 0;
        bar = 0;
        type = 6;
        snprintf( vn, 40, "att_sbwin_%i_endpt", j);
        cfg_find_int32( section, vn, &endpt );
        snprintf( vn, 40, "att_sbwin_%i_type", j);
        cfg_find_int32( section, vn, &type );
        snprintf( vn, 40, "att_sbwin_%i_bar", j);
        cfg_find_int32( section, vn, &bar );
        snprintf( vn, 40, "att_sbwin_%i_rootspace", j);
        cfg_find_int32( section, vn, &rs );
        snprintf( vn, 40, "att_sbwin_%i_func", j);
        cfg_find_int32( section, vn, &func );
        i->regs.SB_WIN[j].sb_address  = endpt & 0xffu;
        i->regs.SB_WIN[j].sb_address |= (type << 8u) & 0x00ff00u;
        i->regs.SB_WIN[j].sb_address |= ((type + 1) << 16u) & 0xff0000u;
        i->regs.SB_WIN[j].sb_address |= (bar << 24u) & 0x0f000000u;
        i->regs.SB_WIN[j].reg_1C  = func & 0xffu;
        i->regs.SB_WIN[j].reg_1C |= (rs << 8u) & 0x000300u;
        snprintf( vn, 40, "att_sbwin_%i_control", j);
        cfg_find_int32( section, vn, &i->regs.SB_WIN[j].window_flags );
    }

    device_register( &i->self );

    att_prim_ret_init( i );

    return &i->self;
}

device_type att_type = {
        .name = "att",
        .spawn = att_spawn
};

static __attribute__((constructor)) void register_att() {
    device_type_register( &att_type );
}
