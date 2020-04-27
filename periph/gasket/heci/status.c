//
// Created by pbx on 01/09/19.
//
#include "log.h"
#include "gasket/heci/heci.h"

const char *gs1_icc_status[4] = {
        "Valid OEM data, ICC programmed",
        "No valid OEM data, ICC programmed with defaults",
        "Valid OEM data, ICC not programmed",
        "No valid OEM data, ICC not programmed",
};

const char *gs1_bup_status[] = {
    "BEGIN",
    "DISABLE_HOST_WAKE_EVENT",
    "ENABLE_CLOCK_GATING",
    "PM_ME_HANDSHAKING",
    "FLOW_DETERMINATION",
    "PMC_PATCHING",
    "GET_FLASH_VSCC",
    "SET_FLASH_VSCC",
    "VSCC_FAILURE",
    "EFFS_INITIALIZATION",
    "CHECK_ME_STRAP_DISABLED",
    "TIMEOUT_WAITING_T34",
    "CHECK_STRAP_DISABLED",
    "CHECK_BUP_OVERRIDE_STRAP",
    "CHECK_ME_POLICY_DISABLED",
    "PKTPM_INITIALIZATION",
    "BLOB_INITIALIZATION",
    "CM3",
    "CM0",
    "FLOW_ERROR",
    "CM3_CLOCK_SWITCH",
    "CM3_CLOCK_SWITCH_ERROR",
    "CM3_FLASH_PAGING",
    "CM3_ICV_RECOVERY",
    "CM3_LOAD_KERNEL",
    "CM0_HOST_PREP",
    "CM0_SKIP_HOST_PREP",
    "CM0_ICC_PROG",
    "CM0_T34_ERROR",
    "CM0_FLEX_SKU",
    "CM0_TPM_START",
    "CM0_DID_WAIT",
    "CM0_DID_ERROR",
    "CM0_DID_NOMEM",
    "CM0_UMA_ENABLE",
    "CM0_UMA_ENABLE_ERROR",
    "CM0_DID_ACK",
    "CM0_DID_ACK_ERROR",
    "CM0_CLOCK_SWITCH",
    "CM0_CLOCK_SWITCH_ERROR",
    "CM0_TEMP_DISABLE",
    "CM0_TEMP_DISABLE_ERROR",
    "CM0_TEMP_DISABLE_UMA_ERROR",
    "CM0_IPK_CHECK",
    "CM0_IPK_RECREATION",
    "CM0_IPK_RECREATION_ERROR",
    "CM0_UMA_VALIDATION",
    "CM0_UMA_VALIDATION_ERROR",
    "CM0_UMA_VALIDATION_IPK",
    "CM0_PKVENOM_START",
    "CM0_LOAD_KERNEL",
    "CM0_FTPM_INIT",
    "CM0_FTPM_ABORT",
    "HALT_UPON_FIPS_BUP_ERR",
    "HALT_UPON_FIPS_CRYPTO_DRV_ERR",
    "HALT_UPON_FIPS_TLS_ERR",
    "HALT_UPON_FIPS_DT_ERR",
    "HALT_UPON_FIPS_UNKNOWN_ERR",
    "CM0_MEMORY_ACCESS_RANGE_ERR",
    "CSE_RESET_LIMIT_ERR",
    "CSE_RESET_ENTER_RECOVERY",
    "HALT_UPON_UNKNOWN_ERROR",
    "VALIDATE_NFT",
    "READ_FIXED_DATA",
    "READ_ICC_DATA",
    "ZERO_UMA",
    "HALT_UPON_SKU_ERROR",
    "DERIVE_CHIPSET_KEY_ERROR",
    "HOST_ERROR",
    "69",
    "FTP_LOAD_ERROR",
    "MFG_CMRST",
    "MPR_VIOLATION_CMRST",
    "ICC_START_POLL_BEGIN",
    "ICC_START_POLL_END",
    "HOBIT_SET",
    "POLL_CPURST_DEASSERT_BEGIN"
};

const char *gs1_rom_status[] = {
        "BEGIN",//0
        "INIT_HARDWARE",//1
        "INIT_TPM_EST",//2
        "INIT_SUSRAM",//3
        "GET_FUSES",//4
        "DERIVE_UMCHID",//5
        "DISABLE",//6
        "INIT_HECI",//7
        "FIND_IMAGE",
        "MANIFEST_FOUND",
        "LOAD_MODULE",
        "CALL_NEXT_MODULE",
        "FIND_FPT",
        "FIND_CODE_PARTITION",
        "INIT_PAGING_LOGIC",
        "PATCH_FAILURE",
        "HW_PAGE_FAULT",
        "FPT_INVALID",
        "BRUTE_FORCE_SCAN",
        "IDLM_FOUND",
        "PG_EXIT_START",
        "PG_EXIT_KERNEL_LOAD"
};

const char *gs1_rom_status_high[] = {

        "PROTECTED_START",    //F1
        "PRE_PMC_HANDSHAKE",  //F2
        "POST_PMC_HANDSHAKE", //F3
        "FUSES_PULLED",       //F4
        "BEFORE_SRAM_INIT",   //F5
        "AFTER_SRAM_INIT",    //F6
        "ROM_EARLY_F7",
        "ROM_EARLY_F8",
        "ROM_EARLY_F9",
        "ROM_EARLY_FA",
        "ROM_EARLY_FB",
        "ROM_EARLY_FC",
        "ROM_EARLY_FD",
        "MIA_HALT",
        "DO_RESET"
};

const char *gs1_modules[16] = {
        "ROM/Preboot",
        "RBE",
        "Privilege Micro-Kernel",
        "BringUp",
        "Loading",
        "IDLM",
        "HOSTCOMM",
        "FWUpdate",
        "Maestro",
        "9",
        "10",
        "11",
        "12",
        "13",
        "14",
        "15"
};

const char *gs1_pmevent[16] = {
    "Clean Moff->Mx wake",
    "Moff->Mx wake after an error",
    "Clean global reset",
    "Global reset after an error",
    "Clean Intel ME reset",
    "Intel ME reset due to exception",
    "Pseudo-global reset",
    "CM0->CM3",
    "CM3->CM0",
    "Non-power cycle reset",
    "Power cycle reset through M3",
    "Power cycle reset through Moff",
    "Cx/Mx->Cx/Moff",
    "CM0->CM0PG",
    "CM3->CM3PG",
    "CM0PG->CM0"
};

void heci_handle_gs1_change( heci_inst *i, uint32_t newval ){
    uint32_t diff = i->cse_gs1 ^ newval;
    uint32_t field, mod;
    if ( diff & (0x3u << 1u) ) {
        log( LOG_INFO, i->self.name, "Set ICC status: %s", gs1_icc_status[(newval >> 1u) & 0x3u] );
    }
    if ( diff & (1u << 6u) ) {
        log( LOG_INFO, i->self.name, "Set ME filesystem corrupted: %i", (newval >> 6u) & 1u);
    }
    if ( diff & (0xffu << 16u) ) {
        field =  (newval >> 16u) & 0xffu;
        mod = (newval >> 28u) & 0xFu;
        if ( mod == 3 && field < (sizeof gs1_bup_status / sizeof(const char *)))
            log( LOG_INFO, i->self.name, "Set module progresss: %s", gs1_bup_status[field]);
        else if ( mod == 0 && field < (sizeof gs1_rom_status / sizeof(const char *)))
            log( LOG_INFO, i->self.name, "Set module progresss: %x %s",field, gs1_rom_status[field]);
        else if ( mod == 0 && field >= 256-(sizeof gs1_rom_status_high / sizeof(const char *)))
            log( LOG_INFO, i->self.name, "Set module progresss: %x %s",field, gs1_rom_status_high[field - (256 - (sizeof gs1_rom_status_high / sizeof(const char *)))]);
        else
            log( LOG_INFO, i->self.name, "Set module progresss: %x", field);
    }
    if ( diff & (0xfu << 24u) ) {
        log( LOG_INFO, i->self.name, "Set PM event: %s", gs1_pmevent[(newval >> 24u) & 0xfu]);
    }
    if ( diff & (0xfu << 28u) ) {
        log( LOG_INFO, i->self.name, "Set active module: %s", gs1_modules[(newval >> 28u) & 0xfu]);
    }
}

const char *fs_state[16] = {
        "Reset",
        "Init",
        "Recovery",
        "Test",
        "Disabled",
        "Normal",
        "Disabled and waiting to timeout",
        "State transition",
        "Invalid CPU and PCH combination",
        "Invalid PCH SKU emulation",
        "Boot Guard failure: ACM module failure",
        "Boot Guard failure: SM module failure",
        "Boot Guard failure: SM Timer zeroed out, infinite wait",
        "Boot Guard enabled: PCH is plugged in",
        "14",
        "15"
};

const char *fs_opstate[8] = {
        "Transitioning",
        "CM0 with UMA",
        "CM0 Power Gated",
        "Unknown",
        "CM3 with no UMA",
        "CM0 with no UMA",
        "Bring Up",
        "FW error when CM0 with no UMA",
};

const char *fs_error[16] = {
        "No Error",
        "Image Failure",
        "Disabled",
        "Unknown Failure",
        "PG exit Failure",
        "6",
        "7",
        "8",
        "9",
        "10",
        "11",
        "12",
        "13",
        "14",
        "15"
};

const char *fs_opmode[16] = {
        "Normal",
        "Ignition",
        "Alt Disable Mode",
        "Temporary Disable mode",
        "Unsecured mode by H/W jumper",
        "Unsecured mode by HECI message",
        "7",
        "8",
        "9",
        "10",
        "11",
        "12",
        "13",
        "14",
        "15"
};


void heci_handle_fs_change( heci_inst *i, uint32_t newval ){
    uint32_t diff = i->cse_fs ^ newval;
    if ( diff & (0xfu << 0u) ) {
        log( LOG_INFO, i->self.name, "Set current state: %s", fs_state[(newval >> 0u) & 0xfu] );
    }
    if ( diff & (1u << 4u) ) {
        log( LOG_INFO, i->self.name, "Manufacturing mode enabled: %i", (newval >> 4u) & 1u);
    }
    if ( diff & (1u << 5u) ) {
        log( LOG_INFO, i->self.name, "Flash partition valid: %i", (newval >> 5u) & 1u);
    }
    if ( diff & (0x7u << 6u) ) {
        log( LOG_INFO, i->self.name, "Set operational state: %s", fs_opstate[(newval >> 6u) & 0x7u] );
    }
    if ( diff & (1u << 9u) ) {
        log( LOG_INFO, i->self.name, "Set init complete: %i", (newval >> 9u) & 1u);
    }
    if ( diff & (1u << 10u) ) {
        log( LOG_INFO, i->self.name, "Set BUP Failure: %i", (newval >> 10u) & 1u);
    }
    if ( diff & (1u << 11u) ) {
        log( LOG_INFO, i->self.name, "Set bit 11: %i", (newval >> 11u) & 1u);
    }
    if ( diff & (0xfu << 12u) ) {
        log( LOG_INFO, i->self.name, "Set error code: %i", fs_error[(newval >> 12u) & 0xfu]);
    }
    if ( diff & (0xfu << 16u) ) {
        log( LOG_INFO, i->self.name, "Set operational mode: %i", fs_opmode[(newval >> 16u) & 0xfu]);
    }
}