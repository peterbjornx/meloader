/**
 * @defgroup PCIBus PCI bus emulation
 *
 * These definitions are related to PCI emulation, which takes a few shortcuts.
 * First of all, at present only memory and configuration cycles are supported,
 * special cycles and asynchronous signals such as interrupts are not. This
 * should be fine for the ME/TXE because it appears to exclusively use Message
 * Signalled Interrupts via memory cycles to the APIC.
 *
 * The emulator does not implement device/function hierarchy, meaning that
 * functions are directly attached to the bus and that the device number is
 * simply used as part of the address for a function.
 *
 * Subtractive and positive decoding are supported via repeated calls to
 * function mem_read, mem_write callbacks, passing the current cycle number as
 * a parameter. As soon as one of these callbacks returns a successful status
 * or the maximum latency number is hit, the cycle will complete.
 *
 * Instead of every device "mastering" the bus, there are centralized functions
 * offering access to bus resources, these can be called by any device on the
 * bus.
 *
 * @{
 */

#include <stdlib.h>
#include <string.h>
#include "pci/bus.h"
#include "log.h"

/**
 * The list of PCI buses
 */
static pci_bus *bus_list;

/**
 * Finds a PCI bus by its name
 * @param name The name of the PCI bus to look up
 * @return A pointer to the bus object
 */
pci_bus *pci_bus_find( const char *name ) {
    pci_bus *cur;
    for ( cur = bus_list; cur; cur = cur->next) {
        if ( strcmp( name, cur->name ) == 0 )
            break;
    }
    return cur;
}

/**
 * Register a new PCI bus.
 * PCI bus names must be unique, if this is not the case that is a fatal error
 * @param bus The PCI bus object to register
 */
void pci_bus_register( pci_bus *bus ) {
    if ( pci_bus_find( bus->name ) ) {
        log(LOG_FATAL, "pci", "Tried to redefine PCI bus %s", bus);
        exit(EXIT_FAILURE);
    }
    bus->next = bus_list;
    bus_list = bus;
}

/**
 * Looks a PCI function up by its device and function number.
 * This function does NOT traverse bus hierarchies.
 * @param bus The bus to query
 * @param bdf The BDF to search for, encoded using PCI_PACK_BDF
 *            where the bus ID is ignored.
 */
pci_func *pci_func_find( pci_bus *bus, uint32_t bdf ) {
    pci_func *cur;
    bdf &= PCI_BDF_DF_MASK;
    for ( cur = bus->first_func; cur; cur = cur->next) {
        if ( bdf == (cur->bdf & PCI_BDF_DF_MASK) && cur->cfg_read )
            break;
    }
    return cur;
}

/**
 * Register a new PCI function.
 * @param bus The bus to attach the function to
 * @param func The function to register
 */
void pci_func_register( pci_bus *bus, pci_func *func ) {
    if ( pci_func_find( bus, func->bdf ) && func->cfg_read ) {
        log(LOG_FATAL, "pci",
                "Tried to redefine PCI function 0x%x on bus %s", func->bdf,
                bus->name);
        exit(EXIT_FAILURE);
    }
    func->bdf &= PCI_BDF_DF_MASK;
    func->bdf |= PCI_PACK_BDF( bus->bus_num, 0u, 0u );
    func->bus = bus;
    func->next = bus->first_func;
    bus->first_func = func;
}

/**
 * Changes the bus number for a PCI bus.
 * Propagates the bus number to all functions on the bus.
 * @param bus     The bus to update
 * @param bus_num The new bus number.
 */
void pci_set_bus_num( pci_bus *bus, unsigned int bus_num ) {
    pci_func *cur;
    bus->bus_num = bus_num;
    for ( cur = bus->first_func; cur; cur = cur->next) {
        cur->bdf &= PCI_BDF_DF_MASK;
        cur->bdf |= PCI_PACK_BDF( bus_num, 0u, 0u );
    }
}

/**
 * Perform a configuration read cycle on a PCI bus.
 * Bus traversal is handled by bridges on the bus.
 * @param bus   The bus to initiate the cycle on.
 * @param addr  The configuration space address, encoded using PCI_PACK_ADDR
 * @param out   The buffer that will receive the read data
 * @param count The number of bytes to read
 * @return >= 0 when successful
 */
int pci_bus_config_read( pci_bus *bus, uint64_t addr, void *out, int count ) {
    pci_func *func;
    if ( PCI_ADDR_BUS(addr) != bus->bus_num ) {
        /* Foreign bus, issue type 1 transaction */
        addr &= PCI_ADDR_TYPE1_MASK;
        addr |= PCI_ADDR_TYPE1;
        for ( func = bus->first_func; func; func = func->next )
            if ( func->cfg_read && func->cfg_read( func, addr, out, count ) == 0 )
                return 0;
        return -1;
    } else {
        /* Local bus, issue type 0 transaction */
        func = pci_func_find( bus, addr & PCI_ADDR_BDF_MASK );
        if ( !func || !func->cfg_read )
            return -1;
        addr &= PCI_ADDR_TYPE0_MASK;
        addr |= PCI_ADDR_TYPE0;
        return func->cfg_read( func, addr & PCI_ADDR_TYPE0_MASK, out, count);
    }
}

/**
 * Perform a configuration write cycle on a PCI bus.
 * Bus traversal is handled by bridges on the bus.
 * @param bus   The bus to initiate the cycle on.
 * @param addr  The configuration space address, encoded using PCI_PACK_ADDR
 * @param out   The buffer that holds the data to write
 * @param count The number of bytes to write
 * @return >= 0 when successful
 */
int pci_bus_config_write( pci_bus *bus, uint64_t addr, const void *out, int count ) {
    pci_func *func;
    if ( PCI_ADDR_BUS(addr) != bus->bus_num ) {
        /* Foreign bus, issue type 1 transaction */
        addr &= PCI_ADDR_TYPE1_MASK;
        addr |= PCI_ADDR_TYPE1;
        for ( func = bus->first_func; func; func = func->next )
            if (  func->cfg_write && func->cfg_write( func, addr, out, count ) == 0 )
                return 0;
        return -1;
    } else {
        /* Local bus, issue type 0 transaction */
        func = pci_func_find( bus, addr & PCI_ADDR_BDF_MASK );
        if ( !func || !func->cfg_write  )
            return -1;
        addr &= PCI_ADDR_TYPE0_MASK;
        addr |= PCI_ADDR_TYPE0;
        return func->cfg_write( func, addr & PCI_ADDR_TYPE0_MASK, out, count);
    }
}

/**
 * Perform a memory read cycle on a PCI bus.
 * Bus traversal is handled by bridges on the bus.
 * @param bus     The bus to initiate the cycle on.
 * @param addr    The physical address to access
 * @param out     The buffer that will receive the read data
 * @param count   The number of bytes to read
 * @param sai     The security attribute of initiator, an ID used to identify masters
 * @param max_lat The maximum number of simulated clock cycles to wait for a
 *                completion.
 * @return        The SAI of the target when successful, <0 when not.
 */
int pci_bus_mem_read( pci_bus *bus, uint64_t addr, void *out, int count, int sai, int max_lat ) {
    int lat = 0, csai;
    pci_func *func;
    for ( lat = 0; lat < max_lat; lat++ ) {
        for ( func = bus->first_func; func; func = func->next ) {
            csai = func->mem_read(func, addr, out, count, sai, lat);
            if ( csai >= 0 || csai < -1 )
                return csai;
        }
    }
    return -1;
}

/**
 * Perform a memory write cycle on a PCI bus.
 * Bus traversal is handled by bridges on the bus.
 * @param bus     The bus to initiate the cycle on.
 * @param addr    The physical address to access
 * @param out     The buffer that holds the data to write
 * @param count   The number of bytes to write
 * @param sai     The security attribute of initiator, an ID used to identify masters
 * @param max_lat The maximum number of simulated clock cycles to wait for a
 *                completion.
 * @return >= 0 when successful
 */
int pci_bus_mem_write( pci_bus *bus, uint64_t addr, const void *out, int count, int sai, int max_lat ) {
    int lat = 0, s;
    pci_func *func;
    for ( lat = 0; lat < max_lat; lat++ ) {
        for ( func = bus->first_func; func; func = func->next ) {
            s = func->mem_write(func, addr, out, count, sai, lat);
            if ( s >= 0 || s < -1 )
                return s;
        }
    }
    return -1;
}

/**
 * Perform an IO read cycle on a PCI bus.
 * Bus traversal is handled by bridges on the bus.
 * @param bus     The bus to initiate the cycle on.
 * @param addr    The physical address to access
 * @param out     The buffer that will receive the read data
 * @param count   The number of bytes to read
 * @param sai     The security attribute of initiator, an ID used to identify masters
 * @param max_lat The maximum number of simulated clock cycles to wait for a
 *                completion.
 * @return        The SAI of the target when successful, <0 when not.
 */
int pci_bus_io_read( pci_bus *bus, uint64_t addr, void *out, int count, int sai, int max_lat ) {
    int lat = 0, csai;
    pci_func *func;
    /*for ( lat = 0; lat < max_lat; lat++ ) {
        for ( func = bus->first_func; func; func = func->next ) {
            csai = func->mem_read(func, addr, out, count, sai, lat);
            if ( csai >= 0 )
                return csai;
        }
    }*/
    return -1;
}

/**
 * Perform an IO write cycle on a PCI bus.
 * Bus traversal is handled by bridges on the bus.
 * @param bus     The bus to initiate the cycle on.
 * @param addr    The physical address to access
 * @param out     The buffer that holds the data to write
 * @param count   The number of bytes to write
 * @param sai     The security attribute of initiator, an ID used to identify masters
 * @param max_lat The maximum number of simulated clock cycles to wait for a
 *                completion.
 * @return >= 0 when successful
 */
int pci_bus_io_write( pci_bus *bus, uint64_t addr, const void *out, int count, int sai, int max_lat ) {
    int lat = 0;
    pci_func *func;
    /*for ( lat = 0; lat < max_lat; lat++ ) {
        for ( func = bus->first_func; func; func = func->next )
            if ( func->io_write && func->io_write( func, addr, out, count, sai, lat ) == 0 )
                return 0;
    }*/
    return -1;
}

/**
 * @}
 */