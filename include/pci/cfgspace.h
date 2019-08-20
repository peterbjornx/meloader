//
// Created by pbx on 30/06/19.
//

#ifndef MELOADER_CFGSPACE_H
#define MELOADER_CFGSPACE_H

#include <stdint.h>

#define PCI_BAR_ADDRESS_MASK      (0xFFFFFFF0u)
#define PCI_BAR_ADDRESS64_MASK    (0xFFFFFFFFFFFFFFF0ull)
#define PCI_COMMAND_IOSE          (1u<<0u)
#define PCI_COMMAND_MSE           (1u<<1u)
#define PCI_COMMAND_BME           (1u<<2u)
#define PCI_COMMAND_SCE           (1u<<3u)
#define PCI_COMMAND_MWIE          (1u<<4u)

typedef struct pci_cfgspace_type0_s pci_cfgspace_type0;
typedef union pci_cfgspace_u pci_cfgspace;

struct pci_cfgspace_type0_s {
    uint16_t vid;
    uint16_t did;
    uint16_t command;
    uint16_t status;
    uint8_t revision_id;
    uint16_t class_low;
    uint8_t class_high;
    uint8_t cache_line_size;
    uint8_t latency_timer;
    uint8_t header_type;
    uint8_t bist;
    union {
        uint32_t bar[6];
        uint64_t bar64[3];
    };
    uint32_t cardbus_cis_pointer;
    uint16_t ss_vid;
    uint16_t ss_id;
    uint32_t rom_bar;
    uint8_t cap_pointer;
    uint8_t reserved[7];
    uint8_t int_line;
    uint8_t int_pin;
    uint8_t min_grant;
    uint8_t max_latency;

};

union pci_cfgspace_u {
    pci_cfgspace_type0 type0;
    uint8_t bytes[0x1000];
};

#endif //MELOADER_CFGSPACE_H
