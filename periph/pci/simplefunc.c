/**
 * PCI Simple Function implements a basic PCI device
 */
#include <string.h>
#include <stdlib.h>
#include "pci/bus.h"
#include "pci/device.h"
#include "cfg_file.h"
#include "log.h"

static int pci_simple_cfg_read( pci_func *func, uint64_t addr,       void *out, int count )
{
    int off;
    pci_simplefunc *sfunc;

    /* Check if this is a type 0 transaction */
    if ( PCI_ADDR_TYPE(addr) != PCI_ADDR_TYPE0 )
        return -1;

    sfunc = (pci_simplefunc *)func;

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

    if ( sfunc->cfg_read )
        sfunc->cfg_read( func, off );

    /* Copy the configuration space contents to the buffer */
    memcpy( out, off + (void *) &func->config, count );

    /* Signal successful completion */
    return 0;

}

static int pci_simple_cfg_write( pci_func *func, uint64_t addr, const void *out, int count )
{
    int off;
    pci_simplefunc *sfunc;

    /* Check if this is a type 0 transaction */
    if ( PCI_ADDR_TYPE(addr) != PCI_ADDR_TYPE0 )
        return -1;

    sfunc = (pci_simplefunc *)func;

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

    log( LOG_TRACE,
         func->device->name,
         "Configuration write ( Offset: 0x%03x, Size: 0x%x )",
         off, count );

    /* Copy the configuration space contents from the buffer */
    memcpy( off + (void *) &func->config, out, count );

    if ( sfunc->cfg_write )
        sfunc->cfg_write( func, off );

    /* Signal successful completion */
    return 0;

}

static int pci_simple_mem_read( pci_func *func, uint64_t addr,       void *buf, int count, int sai, int lat )
{
    uint64_t off = 0, base;
    int bar;
    pci_simplefunc *sfunc;

    sfunc = (pci_simplefunc *)func;

    /* Match the address against our BARs */
    for ( bar = 0; bar < sfunc->max_bar; bar++ ) {
        if ( sfunc->flags & PCI_SIMPLEFUNC_64BIT )
            base = func->config.type0.bar64[bar] & PCI_BAR_ADDRESS64_MASK;
        else
            base = func->config.type0.bar[bar]   & PCI_BAR_ADDRESS_MASK;
        if ( addr < base)
            continue;
        off = addr - base;
        if ( off <= sfunc->bar_size[bar] )
            break;
    }

    /* Check if the access hit one of our BARs */
    if ( bar == sfunc->max_bar )
        return -1;

    /* Validate the target addresses */
    if ( off + count > sfunc->bar_size[bar] ) {
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

    /* Check if we implement memory reads */
    if ( !sfunc->bar_read ) {
        log( LOG_ERROR,
             func->device->name,
             "Memory read not implemented"
             "( BAR %i, Offset: 0x%03x, Size: 0x%x )",
             bar, off, count);
        return -1;
    }

    return sfunc->bar_read( func, bar, off, buf, count );

}

static int pci_simple_mem_write( pci_func *func, uint64_t addr, const void *buf, int count, int sai, int lat )
{
    uint64_t off = 0, base;
    int bar;
    pci_simplefunc *sfunc;

    sfunc = (pci_simplefunc *)func;

    /* Match the address against our BARs */
    for ( bar = 0; bar < sfunc->max_bar; bar++ ) {
        if ( sfunc->flags & PCI_SIMPLEFUNC_64BIT )
            base = func->config.type0.bar64[bar] & PCI_BAR_ADDRESS64_MASK;
        else
            base = func->config.type0.bar[bar]   & PCI_BAR_ADDRESS_MASK;
        if ( addr < base)
            continue;
        off = addr - base;
        if ( off <= sfunc->bar_size[bar] )
            break;
    }

    /* Check if the access hit one of our BARs */
    if ( bar == sfunc->max_bar )
        return -1;

    /* Validate the target addresses */
    if ( off + count > sfunc->bar_size[bar] ) {
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

    /* Check if we implement memory reads */
    if ( !sfunc->bar_read ) {
        log( LOG_ERROR,
             func->device->name,
             "Memory write not implemented"
             "( BAR %i, Offset: 0x%03x, Size: 0x%x )",
             bar, off, count);
        return -1;
    }

    return sfunc->bar_write( func, bar, off, buf, count );

}

void pci_simple_init( pci_simplefunc *func, const cfg_section *section ) {
    if ( !section ) {
        log(LOG_FATAL, "pci_simple_func", "Can't initialize PCI function without configuration");
        exit(EXIT_FAILURE);
    }
    pci_cfg_handle_type0( &func->func, section, func->flags & PCI_SIMPLEFUNC_64BIT );
    func->func.cfg_read = pci_simple_cfg_read;
    func->func.cfg_write = pci_simple_cfg_write;
    func->func.mem_read = pci_simple_mem_read;
    func->func.mem_write = pci_simple_mem_write;
    pci_cfg_handle_device( &func->func, section );
}