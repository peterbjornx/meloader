//
// Created by pbx on 19/07/19.
//

#ifndef MELOADER_MISA_H
#define MELOADER_MISA_H

#include <stdint.h>

/**
 * Request Queue Limit (RQLIMIT): This field is used to throttle
 * against the theoretical case where the Bunit non-MIA request
 * queues fill up with transactions and the Bunit locks up because the
 * snooped transactions at the heads of the queues cannot snoop
 * without the MIA queue making progress and the MIA queue
 * cannot schedule a transaction because all the tags are occupied
 * with pending non-MIA transactions. This field cuts off all non-MIA
 * transactions if there are more than the specified entries in the
 * non-MIA queues.
 */
#define MISA_BCTRL_RQLIMIT(v)       ((v&0xf)<<16u)

/**
 * Snoop Everything (SNPE): This field forces snoops to be issued
 * for all incoming transactions even if marked as non-coherent.
 * 1: All incoming transactions are forced to snoop mFSB.
 * 0: Normal operation.
 * Note: Firmware must not set the SNPE to 1 when SNPDIS bit is
 * already 1. Setting both bits will result in an undefined hardware
 * behavior.
 */
#define MISA_BCTRL_SNPE             (1u<<11u)

/**
 * Miss Valid Entries (MVE): This mode, mostly present for test
 * purposes, causes reads to clean valid Bunit buffer entries with '0'
 * reference counts to look like 'misses' instead of 'hits'.
 */
#define MISA_BCTRL_MVE              (1u<<8u)

/**
 * Retry Dirty Entries (RDE): This mode, mostly present for test
 * purposes, causes reads or writes from the Hunit requestor
 * interface to dirty valid Bunit buffer entries with '0' reference
 * counts to stall on the appropriate requestor interface until the
 * entry has been flushed from the Bunit. As a side effect, the Bunit
 * will maintain a counter that forces a complete Bunit dirty buffer
 * flush once this mechanism has caused a transaction to be stalled
 * for more than around 4 microseconds.
 */
#define MISA_BCTRL_RDE              (1u<<7u)

/**
 * BE Clock Gate Enable (BECLKGTEN): When set, clock gating is
 * enabled on the BE flops and logic.
 */
#define MISA_BCTRL_BECLKGTEN        (1u<<2u)

/**
 * Miscellaneous Clock Gate Enable (MCLKGTEN): When set,
 * clock gating is enabled for the entire Bunit.
 */
#define MISA_BCTRL_MCLKGTEN         (1u<<1u)

/**
 * Scratchpad (SKPD): This field is a generic scratchpad register
 * with no hardware functional implementation behind it.
 */
#define MISA_BCTRL_SCRATCHPAD       (1u<<0u)

/**
 * Snoops Disabled (SNPDIS): This field controls whether snoops
 * are issued for incoming transactions even if marked as cache-
 * coherent.
 * Note: Firmware must not set the SNPDIS to 1 when SNPE bit is
 * already 1. Setting both bits will result in an undefined hardware
 * behavior.
 */
#define MISA_BWFLUSH_SNPDIS         (1<<24u)

/**
 * All Entries Idle (AEIDLE): This field indicates if there are
 * pending transactions in Bunit or not.
 */
#define MISA_BWFLUSH_AEIDLE         (1<<17u)

/**
 * All Entries Flushed (AEFLUSH): This field indicates if the Bunit
 * write buffer have been flushed or not.
 */
#define MISA_BWFLUSH_AEFLUSH        (1<<16u)

/**
 * Dirty Low Water Mark (DLWM): This field controls the low
 * water mark threshold for dirty entries retained by Bunit.
 */
#define MISA_BWFLUSH_DLWM(v)        ((v&0xffu)<<8u)

/**
 * Dirty High Water Mark (DHWM): This field controls the high
 * water mark threshold for dirty entries retained by Bunit.
 */
#define MISA_BWFLUSH_DHWM(v)        ((v&0xffu)<<0u)

typedef struct  __attribute__((packed)) {
    uint32_t reserved_0400;
    uint32_t BCTRL;                /* 0x404 Bunit Control */
    uint32_t BWFLUSH;              /* 0x408 BUnit Write Flush Policy */
    uint32_t reserved_040C[0x1B];
    uint32_t BNOCACHE;             /* 0x478 BUnit Non-Cached Region */
    uint32_t reserved_047C[0x11];
    uint32_t BDEBUG0;              /* 0x4C0 BUnit Debug Register 0 */
    uint32_t reserved_04C4[0x2f];
    uint32_t BARBCTRL;             /* 0x580 BUnit Incoming Arbiter Control */
    uint32_t reserved_0584[0x3];
    uint32_t BSTATUS;              /* 0x590 Bunit Status Override Policy */
    uint32_t reserved_0594[0x9b];
} misa_bunit_regs;

/**
 * WD Timer Enable (WDTEN): When 1, this enables the firmware
 * WDT. When 0, the firmware WDT functionality is disabled.
 * This bit is set or cleared by Firmware. Additionally, when a second
 * time out occurs, hardware clears this bit to 0 before initiating the
 * reset.
 * Note that the WatchDog timer stops counting down if the WDTEN
 * bit is set to 0 and gets reloaded to TO1_V programmed value
 * immediately. If the WDTEN bit is set to 0 after the first timeout
 * condition has set the FWDTIS bit, the FWDTIS bit remains set until
 * Firmware clears it in polling mode. If the WDTEN bit is set to 0
 * after the second timeout condition has initiated a reset, the reset
 * condition will occur.
 */
#define MISA_FWDCTL_WDTEN      (1u << 16u)

/**
 * WD Time Out 2 Value (TO2_V): This field specified the value
 * used as the starting point by the Firmware Watchdog Timer while
 * counting down towards its second time out. This value is what
 * reloads into the Watchdog Timer after first timeout occurs.
 * The firmware watchdog timer has a time resolution of approx.
 * 64ms with default as 5120ms. When Firmware reads this register,
 * hardware always returns the value last written by Firmware. When
 * the counter decrements from TO2_V to 0, the hardware will
 * initiate a reset.
 * Note: The timer has an error of 1 timer tick (~64ms). Hence
 * writing value of '01h' is prohibited.
 */
#define MISA_FWDCTL_TO2_V(v)  ((v & 0xffu) << 8u)

/**
 * WD Time Out 1 Value (TO1_V): This field specifies the value
 * used as the starting point by Firmware Watchdog Timer while
 * counting down towards its first time out. This value is what
 * reloads into the Watchdog Timer each time Firmware writes to the
 * Reload Port. The firmware Watchdog timer has a time resolution of
 * approx. 64ms with default as 5120ms. When Firmware reads this
 * register, hardware always return the value last written by
 * Firmware.
 * When the counter decrements from TO1_V to 0, ERRSTS.FWDTIS
 * is set and the counter reloads with the value TO2_V.
 * Note: The timer has an error of 1 timer tick (~64ms). Hence
 *  writing value of '01h' is prohibited.
 */
#define MISA_FWDCTL_TO1_V(v)   ((v & 0xffu) << 0u)

/**
 * WD Timer Read Value (WDTRV): When Firmware reads this
 * register, hardware returns the present value of the decrementing
 * Watchdog Timer.
 * Writes to this register have no effect.
 */
#define MISA_FWDTRP_WDTRV(v)   ((v & 0xffu) << 0u)

/**
 * WD Timer Read Value (WDTWP): When Firmware writes any
 * value to this register, hardware reloads the Watchdog Timer with
 * TO1_V. If the WDTEN bit is set to 1, the Watchdog Timer will
 * decrement by 1 every 64ms.
 * Reads of this register have no impact on Timer operation and
 * result in return value of all 0s.
 */
#define MISA_FWDTRP_WDTWP(v)   ((v & 0xffu) << 0u)

/**
 * Scratchpad (SKPD): This field is a generic scratchpad register
 * with no hardware functional implementation behind it
 */
#define MISA_CCTRL_SKPD(v)     ((v&0x3fu) << 2u)

/**
 * Munit Clock Gating Enable (MCLKGTEN): When set, Munits
 * internal clock gating logic is enabled.
 */
#define MISA_CCTRL_MCLKGTEN    (1u << 1u)

/**
 * Cunit Clock Gating Enable (CCLKGTEN): When set, Cunits
 * internal clock gating logic is enabled.
 */
#define MISA_CCTRL_CCLKGTEN    (1u << 0u)

typedef struct  __attribute__((packed)) {
    /**
     * @brief Firmware Watchdog Timer Control
     * This register contains basic control information used by the firmware watchdog timer.
     */
    uint32_t FWDCTL; /* 0x800 */
    /**
     * @brief Firmware Watchdog Timer Read Port
     * This register contains the read port for firmware watchdog timer.
     */
    uint32_t FWDTRP; /* 0x804 */
    /**
     * @brief Firmware Watchdog Timer Write Port
     * This register contains the write/reload port for firmware watchdog timer.
     */
    uint32_t FWDTWP; /* 0x808 */
    uint32_t reserved_080C[0xfc];
    /**
     * @brief Cunit Control
     * This register contains basic control information used by the Cunit.
     */
    uint32_t CCTRL;  /* 0xBFC */
} misa_cunit_regs;

/**
 * Command Tracker Size (CMDTRKSIZE): This field indicates
 * the maximum number of entries in the Hunit Command Tracker.
 */
#define MISA_HMISC0_CMDTRKSIZE(v)    (((v)&0xffu)<<24u)

/**
 * Performance Profiling Enable (PERFMEN): When set, HW
 * Logic for Performance Profiling is Enabled
 */
#define MISA_HMISC0_PERFMEN          (1u << 20u)

/**
 * PRI Status (PRISTS): PRI Status used on Request HB channel.
 * 0=Casual; 1=impending; 2=normal; 3=urgent.
 */
#define MISA_HMISC0_PRISTS(v)       (((v)&0x3u)<<18u)

/**
 * Command Tracker Allocation Limit for Snoop Request
 * (SNPALIMIT): This field is used to limit the number of command
 * tracker entries allocation for snoop request. The bits in this
 * register are 1s based. A value of 1h means a limit of 1, a value of
 * 2h means a value of 2, etc. However, a value of 0h means
 * unlimited allocation (no limit is set).
 */
#define MISA_HMISC0_SNPALIMIT(v)    (((v)&0x3u)<<16u)

/**
 * Hunit Cache Disable (HCDIS): When set, the caching behavior
 * of the Command Tracker is disabled, and all the mFSB memory
 * read/write transaction will be forwarded to either Aunit or Bunit.
 * Setting this bit will indirectly disable the mIA cache, as MISA will
 * not allow mIA to store any transaction into the mIA cache by
 * driving KEN# inactive.
 */
#define MISA_HMISC0_HCDIS           (1u<<15u)

/**
 * Scratchpad (SKPD): This field is a generic scratchpad register
 * with no hardware functional implementation behind it.
 */
#define MISA_HMISC0_SKPD            (1u<<14u)

/**
 * LRU Clock Gating Enable (HLRUCLKGTEN): When set, LRUs
 * internal clock gating logic is enabled.
 * Note: The same register bit will be used for both sa_clk and
 * mia_clk clock gating logic in Hunit.
 */
#define MISA_HMISC0_HLRUCLKGTEN     (1u<<13u)

/**
 * HCACHE Clock Gating Enable (HCACHECLKGTEN): When set,
 * HUnits internal SRAM (FLOP Based) clock gating logic is enabled.
 * Note: The same register bit will be used for both sa_clk and
 * mia_clk clock gating logic in Hunit.
 */
#define MISA_HMISC0_HCACHECLKGTEN   (1u<<12u)

/**
 * HXC Clock Gating Enable (HXCCLKGTEN): When set, HUnits
 * HXC internal clock gating logic is enabled.
 * Note: The same register bit will be used for both sa_clk and
 * mia_clk clock gating logic in Hunit.
 */
#define MISA_HMISC0_HXCCLKGTEN      (1u<<11u)

/**
 * HMSG Clock Gating Enable (HMSGCLKGTEN): When set,
 * HUnits MSG/CFG internal clock gating logic is enabled.
 * Note: The same register bit will be used for both sa_clk and
 * mia_clk clock gating logic in Hunit.
 */
#define MISA_HMISC0_HMSGCLKGTEN     (1u<<10u)

/**
 * HCMD Clock Gating Enable (HCMDCLKGTEN): When set,
 * HUnits CMD TRK internal clock gating logic is enabled.
 * Note: The same register bit will be used for both sa_clk and
 * mia_clk clock gating logic in Hunit.
 */
#define MISA_HMISC0_HCMDCLKGTEN     (1u<<9u)

/**
 * Prefetcher Hit Bypass Enable (PHBYPEN): When set, read
 * data from Hunit Prefetcher will bypass the mFSB Read Completion
 * Buffer. This will save 1 system agent clock on the MinuetIA L1$
 * miss latency for the Hunit Prefetcher Hit case.
 * Note: FW is not allowed to change the MinuteIA cache policy when
 * configuring the bypass enabling bits. Before changing the
 * MinuteIA cache policy, FW must poll the register to make sure that
 * the register has been updated accordingly.
 */
#define MISA_HMISC0_PHBYPEN         (1u<<8u)

/**
 * Data BH Bypass Enable (BHBYPEN): When set, read data from
 * Bunit could bypass the Bunit Completion Buffer, H Cache, and
 * mFSB Read Completion Buffer. This will save up to 3 system agent
 * clock on the MinuteIA L1$ miss latency for Hunit Prefetcher Miss
 * case, Bypass will be allowed only to WB/WT cache lines, that were
 * fully missed in the HUnit Cache. In case of a 'Partial' miss (Hunit
 * hold part of the requested data) the bypass wont be allowed. Only
 * the requested data (by mIA) will be bypassed, while rest of the
 * returning data will be using the 'Regular' path to be written into
 * Hunit Cache. In case the mIA core is not ready to receive the Data
 * (Snoop transaction in progress, using the mFSB), which is
 * indicated through the HXC FSM, Returning data will bypass Bunit
 * Completion Buffer and H Cache but will be stored in the mFSB
 * Read Completion Buffer until mIA can accept the RD Data.
 * Note: FW is not allowed to change the MinuteIA cache policy when
 * configuring the bypass enabling bits. Before changing the
 * MinuteIA cache policy, FW must poll the register to make sure that
 * the register has been updated accordingly.
 */
#define MISA_HMISC0_BHBYPEN         (1u<<7u)

/**
 * Request HB Bypass Enable (HBBYPEN): When set, the request
 * from mIA could bypass the Bunit Downstream Command queue.
 * This will save 1 system agent clock on the MiunteIA L1$ miss
 * latency for Hunit Prefetcher Miss case. Bypass will be allowed only
 * if no Previous request are already allocated in the Bunit
 * Downstream Command queue (Queue is empty). Bypass allowed
 * for all commands targeting BUnit (RD/WR,UC/WB/WT).
 * Note: FW is not allowed to change the MinuteIA cache policy when
 * configuring the bypass enabling bits. Before changing the
 * MinuteIA cache policy, FW must poll the register to make sure that
 * the register has been updated accordingly.
 */
#define MISA_HMISC0_HHBYPEN         (1u<<6u)

/**
 * IO Fabric Firewall Enable (IOFWEN): When set, Hunit will
 * discard any completion from Aunit which did not result in a prior
 * non-posted request by Hunit.
 */
#define MISA_HMISC0_IOFWEN          (1u<<5u)

/**
 * Cacheable Read Length (CACHERDLEN): When set to 1, reads
 * to cacheable locations in memory will be 64B reads. When set to
 * 0, reads to cacheable locations in memory will be 32B reads. This
 * field controls the amount of data that is stored in the Hunit SRAM
 * for cacheable reads.
 */
#define MISA_HMISC0_CACHERDLEN      (1u<<4u)

/**
 * MinuteIA Internal Timeout Counter Enable (MIATCEN):
 * When set, the MinuteIA Internal Timeout Counter is enabled.
 * Note: This register bit is output to MinuteIA.
 */
#define MISA_HMISC0_MIATCEN         (1u<<3u)

/**
 * MinuteIA Clock Gating Enable (MIACLKGTEN): When set, the
 * MinuteIA clock gating logic is enabled.
 * Note: The inverted version of this register bit will be routed to
 * MinuteIAs xstpclk_halt_clkgate_ovr pin.
 */
#define MISA_HMISC0_MIACLKGTEN      (1u<<2u)

/**
 * MISA STPCLK# Enable (MISASTPCLKEN): When set, MISA is
 * allowed to trigger STPCLK# to MinuteIA. When cleared, the MISA
 * STPCLK state machine will remain in C0 all the time, therefore will
 * not trigger STPCLK# to MinuteIA.
 * Note: FW should set this register bit by writing a 1 to this register.
 * Once set, this register bit can only be cleared by HW. MISA
 * STPCLK state machine will clear this register bit once it triggers
 * STPCLK# to MinuteIA.
 */
#define MISA_HMISC0_MISASTPCLKEN    (1u<<1u)

/**
 * MinuteIA Idle Mode (MIAIDLE): When cleared, MISA will use
 * MISA STPCLK state machine to generate the MinuteIA Idle
 * indication. When set, MISA will use the MinuteIA Clock Gate
 * Control indication (i.e. sideband wire from MinuteIA to MISA) to
 * generate the MinuteIA Idle indication.
 */
#define MISA_HMISC0_MIAIDLE         (1u<<0u)

/**
 * Hunit Memory-I/O Boundary (HMBOUND): This field is
 * compared with bits [31:12] of the incoming addresses of all
 * memory accesses below 4GB to understand whether the
 * associated transactions should be routed to memory space (Bunit)
 * or MMIO space (Aunit). If bits [31:12] of the address are greater
 * than or equal to this field, but smaller than ROMBASE, then the
 * transaction is routed to MMIO space. This allows the Memory-IO
 * boundary to be set to a 4KB aligned boundary.
 */
#define MISA_HMBOUND_HMBOUND_MASK   (0xFFFFF000u)

/**
 * Hunit Memory-I/O Boundary Lock (HMLOCK): When set, the
 * HMBOUND register is locked and can no longer be modified.
 */
#define MISA_HMBOUND_HMLOCK         (1u << 0u)

/**
 * Extended Configuration Base Address (ECBASE): This field
 * corresponds to bits 31 to 28 of the base address for PCI Express
 * Extended configuration space. BIOS will program this register
 * resulting in a base address for a contiguous memory address
 * space of 256MB.
 * The address used to access the PCI Express configuration space
 * for a specific device can be determined as follows:
 * PCI Express Base Address + (Bus Number * 1MB) + (Device
 * Number * 32KB) + (Function Number * 4KB)
 */
#define MISA_HEC_ECBASE_MASK        (0xF0000000u)

/**
 * Extended Configuration Enable (ECEN): This bit determines if
 * the PCI Express Extended Configuration space is enabled or not.
 * 1: PCI Express Extended Configuration space is enabled.
 * 0: PCI Express Extended Configuration space is disabled.
 */
#define MISA_HEC_ECEN               (1u << 0u)



/**
 * ROM Memory Base (ROMBASE): This field contains bits [31:12]
 * of the base address for ROM. The ROM code may read this field to
 * know the base address of valid ROM. The ROM is always valid from
 * ROMBASE to (4GB ROMBASE). This field always reflects the value
 * of the 'ROM Base Address Hardware Interface Strap'.
 */
#define MISA_HROMMB_ROMBASE_MASK    (0xFFFFF000u)

/**
 * NMI Pin Value (NMI): Reflects the value of the MISA NMI pin.
 * Pin value can also be set by writes to this register field.
 * This pin will be hooked up to MinuteIA LINT1 pin. Firmware is
 * expected to configure LINT1 as NMI.
 */
#define MISA_HMLSE_NMI              (1u<<14u)

/**
 * INTR Pin Value (INTR): Reflects the value of the MISA INTR pin.
 * Pin value can also be set by writes to this register field.
 * This pin will be hooked up to MinuteIA LINT0 pin. Firmware is
 * expected to configure LINT0 as Fixed Interrupt.
 * In CSE, fixed interrupt is delivered via LAPIC MSI interface and not
 * the INTR pin.
 */
#define MISA_HMLSE_INT              (1u<<10u)

/**
 * All Entries Flushed (ALLFLUSHED): This field indicates that all
 * dirty entries have been flushed from the Hunit SRAM.
 */
#define MISA_HWFLUSH_ALLFLUSHED     (1u<<16u)

/**
 * High Water Mark (HWM): This field controls the high water
 * mark for dirty entries in the Hunit SRAM. When this threshold is
 * exceeded, entries are flushed from the Hunit SRAM.
 */
#define MISA_HWFLUSH_HWM_MASK       (0x000000FFu)

/**
 * LRU Result (LRURES): This field holds the result which is LRU +
 * Page Number according to the FW request given in HLRUCR.
 */
#define MISA_HLRURES_LRURES_MASK    (0x000FFFFFu)

/**
 * Command (CMD): This field allows the firmware to indicate the
 * command that needs to be sent to the LRU hardware.
 * 11: Reserved.
 * 10: Read LRU, which indexed by PGNUM. PGNUM = 0 is the least
 * recently used page; PGNUM = 1 is the second least recently used
 * page, and PGNUM = N is the N least recently used page.
 * 01: Remove page indicated by PGNUM from the LRU list.
 * 00: Add page indicated by PGNUM into the LRU list.
 * Note: Firmware must write to this register to trigger the LRU
 * hardware to execute the command.
 */
#define MISA_HLRUCR_CMD(v)         (((v)&0x3) << 30u)

/**
 * Page Number (PGNUM): This field specifies the page number in
 * which the CMD is intended for.
 */
#define MISA_HLRUCR_PGNUM_MASK     (0x000FFFFFu)

/**
 * LRU List Error Status (LRULES): Hardware sets this bit to
 * indicate that the LRU programming error from FW. The specific
 * LRU programming errors and the associated Page Number are
 * captured in the RPNEE, APLFE, APPGNUMEE, and PGNUM register.
 * The RPNEE, APLFE, APPGNUMEE, and PGNUM register is only valid
 * when this bit is set.
 * When this bit is set, it will not log the subsequent error or report a
 * new LRU List error to ERRSTS. This bit will be clear when
 * Firmware/software write 1 to the ERRSTS.LLES.
 */
#define MISA_HLRULERR_LRULES       (1u<<31u)

/**
 * Remove Page Non Exist Error (RPNEE): This bit is set by HW
 * when FW issue a 'Remove Page' command but the Page Number is
 * not in any entry of the LRU list. HW will ignore the command. This
 * bit is locked until the LRULES bit is cleared.
 * This bit will be clear when LRULES bit is clear.
 */
#define MISA_HLRULERR_RPNEE        (1u<<30u)

/**
 * Add Page List Full Error (APLFE): This bit is set by HW when
 * FW issue an 'Add Page' command, but the LRU list already fully
 * filled. HW will ignore the command. This bit is locked until the
 * LRULES bit is cleared.
 * This bit will be clear when LRULES bit is clear.
 */
#define MISA_HLRULERR_APLFE        (1u<<29u)

/**
 * Add Page PGNUM Exist Error (APPGNUMEE): This bit is set by
 * HW when FW issue an 'Add Page' command, but the PGNUM of the
 * page to be added is already in an entry of the LRU list. HW will
 * ignore the command. This bit is locked until the LRULES bit is
 * cleared.
 * This bit will be clear when LRULES bit is clear.
 */
#define MISA_HLRULERR_APPGNUMEE    (1u<<28u)

/**
 * Page Number (PGNUM): This field specifies the page number in
 * the HLRUCR command when the command error occurs.
 * This field is locked until the LRULES bit is cleared.
 */
#define MISA_HLRUCR_PGNUM_MASK     (0x000FFFFFu)

/**
 * Lock (LOCK): Register Lock bit. When set, the HFWDBG register
 * is locked and can no longer be modified.
 * Note: When setting the Lock bit, FW must writes all 0s to SEND,
 * RECSIG, SKPD and FWMSG register field at the same time.
 */
#define MISA_HFWDBG_LOCK           (1u << 16u)

/**
 * Send (SEND): FW write 1 to this bit when it wants to send a byte
 * of trace message on VISA. When the SEND bit is set, HW will
 * assert a strobe signal (WRSTB) at 1 clock later. The WRSTB is a 1
 * clock pulse, which served as a qualifier for the RECSIG, SKPD, and
 * FWM-SG signals.
 * Reads of this register field have no impact on the WRST
 * generation. It would results in return value of 0.
 */
#define MISA_HFWDBG_SEND           (1u << 15u)

/**
 * Record Signal (RECSIG): FW will set this bit at the same time
 * when it sends the first byte of the trace message, and clear this
 * bit after it sends out the last bye of the trace message. In case the
 * external logic analyzer is unable to trigger on WRSTB, the RECSIG
 * could be used as an alternate/additional triggering condition for
 * the external logic analyzer.
 */
#define MISA_HFWDBG_RECSIG         (1u << 14u)

/**
 * Scratchpad (SKPD): This field is a generic scratchpad register
 * that could be viewed on VISA. FW could re-purpose this register to
 * send additional info on VISA.
 */
#define MISA_HFWDBG_SKPD(v)        (((v) & 0x3F) << 8u)

/**
 * FW Message (FWMSG): This field is a generic byte data register
 * for FW to send the trace message on VISA.
 */
#define MISA_HFWDBG_FWMSG_MASK     (0xFFu)

typedef struct __attribute__((packed)) {
    /**
     * @brief Hunit Miscellaneous Controls 0
     * This register contains basic control information used by the Hunit.
     */
    uint32_t HMISC0;  /* 0xC00 */
    uint32_t reserved_0C04[7];
    /**
     * @brief Hunit Memory-I/O Boundary Register
     * This register contains the delimiter that separates the memory interface bound
     * addresses from the I/O fabric interface bound addresses.
     */
    uint32_t HMBOUND; /* 0xC20 */
    /**
     * @brief Hunit Extended Configuration Space
     * This register contains the base address for the PCI Express configuration space. This
     * window of addresses contains the 4KB of configuration space for each PCI Express
     * device that can potentially be part of the PCI Express hierarchy associated with the
     * system agent. There is no physical memory within this window of 256MB that can be
     * addressed. Each PCI Express hierarchy requires a PCI Express Base Register. The
     * system agent supports one PCI Express hierarchy. The region reserved by this register
     * does not alias to any PCI 2.3 compliant memory mapped space.
     * If the ECAM-Only Hardware Interface Strap is strapped enabled, this register becomes
     * Read-Only and will always read a value of 0xE0000001 to hardwire the ECBASE to
     * 0xE0000000 and force ECEN to 1.
     * [SIP] If the ECAM-Only Hardware Interface Strap is strapped disabled, this register is
     * read-write. On reset, the HEC space is disabled and must be enabled by writing a value
     * of 1 to ECEN (i.e. bit [0] of this register). This base address shall be assigned on a
     * 256MB aligned boundary. All other bits not decoded are Read-Only and will always
     * return 0. The Extended Configuration Base Address cannot overlap any physical
     * memory. Firmware/software must guarantee that these ranges do not overlap with
     * known ranges located in PCI MMIO addressable space. This register should be
     * programmed to the same value as that of AEC (Aunit Extended Configuration Space).
     */
    uint32_t HEC;     /* 0xC24 */
    /**
     * @brief Hunit Miscellaneous Legacy Signal Enables
     * This register controls the MinuteIA. It allows Firmware to write to this register to
     * control the value of the respective pins.
     */
    uint32_t HMLSE;   /* 0xC28 */
    uint32_t reserved_0C2C;
    /**
     * @brief Hunit Write Flush
     */
    uint32_t HWFLUSH; /* 0xC30 */
    uint32_t reserved_0C34[0x23];
    /**
     * @brief MISA CHAP Control Register
     */
    uint32_t CHAPCTL; /* 0xCC0 */
    /**
     * @brief CHAP Counter 0 Command Register
     */
    uint32_t CNT0CMD; /* 0xCC4 */
    /**
     * @brief CHAP Counter 0 Data Register
     */
    uint32_t CNT0DAT; /* 0xCC8 */
    /**
     * @brief CHAP Counter 1 Command Register
     */
    uint32_t CNT1CMD; /* 0xCCC */
    /**
     * @brief CHAP Counter 1 Data Register
     */
    uint32_t CNT1DAT; /* 0xCD0 */
    /**
     * @brief CHAP Counter 2 Command Register
     */
    uint32_t CNT2CMD; /* 0xCD4 */
    /**
     * @brief CHAP Counter 2 Data Register
     */
    uint32_t CNT2DAT; /* 0xCD8 */
    uint32_t reserved_0CDC[0x49];
    /**
     * @brief Hunit LRU Result
     * This register holds the result which is LRU + Page Number according to the FW request
     * given in HLRUCR.
     */
    uint32_t HLRURES; /* 0xE00 */
    /**
     * @brief Hunit LRU Control Register
     * This register allows FW to provide command/control details to the hardware LRU.
     */
    uint32_t HLRUCR;  /* 0xE04 */
    /**
     * @brief Hunit LRU List Error Register
     * This register is used to capture status when an LRU List error is reported. Note that the
     * register only logs the first LRU List error. Firmware/software is required to write 1 to
     * clear the LLES field in the ERRSTS register before it can further log subsequent errors.
     */
    uint32_t HLRULERR;/* 0xE08 */
    uint32_t reserved_0E0C[0x5];
    /**
     * @brief Hunit ROM Memory Base
     * This register contains the base address for ROM.
     */
    uint32_t HROMMB;  /* 0xE20 */
    uint32_t reserved_0E24[0x7];
    /**
     * @brief Hunit FW Debug Register
     * This register provides the FW Trace Message support using VISA
     */
    uint32_t HFWDBG;  /* 0xE40 */
    uint32_t reserved_0E44[0x6f];
} misa_hunit_regs;

/**
 * CF8 Enable (CF8EN): This field enables the index/data pair
 * access mechanism for Standard PCI Configuration.
 */
#define MISA_ACF8_CF8EN      (1u<<31u)

/**
 * Bus Number (BUS): This field specifies the bus number for the
 * configuration access.
 */
#define MISA_ACF8_BUS(v)     ((v&0xffu) << 16u)

/**
 * Device Number (DEV): This field specifies the bus number for
 * the configuration access.
 */
#define MISA_ACF8_DEV(v)     ((v&0x1fu) << 11u)

/**
 * Function Number (FUNC): This field specifies the bus number
 * for the configuration access.
 */
#define MISA_ACF8_FUNC(v)    ((v&0x7u) << 8u)

/**
 * Bus Number (BUS): This field specifies the bus number for the
 * configuration access.
 */
#define MISA_ACF8_G_BUS(v)     ((v >> 16u)&0xffu)

/**
 * Device Number (DEV): This field specifies the bus number for
 * the configuration access.
 */
#define MISA_ACF8_G_DEV(v)     ((v >> 11u)&0x1fu)

/**
 * Function Number (FUNC): This field specifies the bus number
 * for the configuration access.
 */
#define MISA_ACF8_G_FUNC(v)    ((v >> 8u)&0x7u)


/**
 * Register Offset (REGOFF): This field specifies the register offset
 * for the configuration access.
 */
#define MISA_ACF8_REGOFF(v)   (v&0xfcu)
#define MISA_ACF8_G_REGOFF(v)   (v&0xfcu)

/**
 * PRI Status (PRISTS): PRI Status used on Request AB channel.
 * 0=Casual; 1=impending; 2=normal; 3=urgent.
 */
#define MISA_ACTRL_PRISTS(v)  (((v) & 0x3u) << 8u)

/**
 * Scratchpad (SKPD): This field is a generic scratchpad register
 * with no hardware functional implementation behind it
 */
#define MISA_ACTRL_SKPD(v)    (((v) & 0x3fu) << 2u)

/**
 * Aunit IOMMU Arrays Clock Gating Enable (AIOMCLKGTEN):
 * When set, all IOMMU arrays register (ATT, ACP, MSI/CPL) clock
 * enabled only during array register write/read. When clear, IOMMU
 * arrays clock is gated if ACLKGTEN is set (based on transactions
 * arrival and queues content).
 */
#define MISA_ACTRL_AIOMCLKGTEN (1u << 1u)

/**
 * Aunit Clock Gating Enable (ACLKGTEN): When set, Aunits
 * internal clock gating logic is enabled (when receiving upstream or
 * downstream transactions and there is valid transaction in any one
 * of the Aunit queues). When clear, Aunit clock is always enabled,
 * but internal aio mmu arrays clock is gated if AIOMCLKGTEN is set.
 */
#define MISA_ACTRL_ACLKGTEN    (1u << 0u)

/**
 * MISR Enable (MISREN): This field controls if the MISR
 * generators are enabled to updates its MISR signature when there
 * is a valid IOSF downstream/upstream command/data transaction.
 * 1: MISR Generator is enabled.
 * 0: MISR Generator is disabled. The MISR signatures would be
 * frozen to the last value, and remains unchanged until the DFx
 * Firmware enable the MISR or reset the MISR.
 */
#define MISA_AMISRCTL_MISREN   (1u << 8u)

/**
 * MISR Reset (MISRRST): When the DFx Firmware writes a 1 to
 * this register, all the MISR generators would synchronously reset
 * its MISR signature to FFFFFFFFh.
 * This is a write-only register. Reads of this register have no impact
 * on the MISR operation and result in return value of 0.
 */
#define MISA_AMISRCTL_MISRRST  (1u << 0u)

/**
 * DMA Access Control Error Log Enable (ENDMAAC): When set,
 * Aunit will capture the access violation status when there is an
 * access control violation occurred on DMA transaction targeting to
 * Memory.
 */
#define MISA_AAVCTL_ENDMAAC    (1u << 31u)

/**
 * MSI Access Control Error Log Enable (ENMSIAC): When set,
 * Aunit will capture the access violation status when there is an
 * access control violation occurred on MSI.
 */
#define MISA_AAVCTL_ENDMSIAC   (1u << 30u)

/**
 * Completion Access Control Error Log Enable (ENCPLAC):
 * When set, Aunit will capture the access violation status when
 * there is an access control violation occurred on the completion
 * received from IOSF primary fabric.
 */
#define MISA_AAVCTL_ENDCPLAC   (1u << 29u)

/**
 * Completion Access Control Error Log Enable (ENCPLAC):
 * When set, Aunit will capture the access violation status when
 * there is an access control violation occurred on the completion
 * received from IOSF primary fabric.
 */
#define MISA_AAVCTL_ENDCPLAC   (1u << 29u)

/**
 * DMA Address Translation Miss Error Log Enable
 * (ENDMATM): When set, Aunit will capture the access violation
 * status when there is an IOMMU address translation miss.
 */
#define MISA_AAVCTL_ENDMATM    (1u << 28u)

/**
 * Access Violation Error Status (AVES): Hardware sets this bit
 * to indicate that the access violation from I/O Fabric. The SAI, the
 * 3DW IOSF Header, and the First DW Data that triggered the
 * violation are captured in AVTYPE, AVSAI, AVHL0-AVHL2 and AVDL
 * register. The AVTYPE, AVSAI, AVHL0-AVHL2 and AVDL register is
 * only valid when this bit is set.
 * When this bit is set, it will not log the subsequent error. This bit
 * will be clear when Firmware/software write 1 to the
 * ERRSTS.DACVES, ERRSTS.MACVES, ERRSTS.CACVES or
 * ERRSTS.DATMES field, depending on the first error that being
 * reported in AVTYPE register.
 */
#define MISA_AAVCTL_AVES       (1u << 11u)

/**
 * Access Violation Type (AVTYPE): This 2-bit value indicates the
 * type of the request that caused the access violation.
 * 11: Completion Access Control Violation
 * 10: MSI Access Control Violation
 * 01: DMA Access Control Violation
 * 00: DMA Address Translation Miss
 */
#define MISA_AAVCTL_AVTYPE(v)   ((v & 3u) << 8u)

/**
 *Access Violation SAI (AVSAI): This 8-bit value indicates the
 * SAI of the requester that caused the access violation.
 */
#define MISA_AAVCTL_AVSAI(v)    ((v & 0xffu) << 0u)

/**
 * IOMMU Access Control Policy Enable (IACPEN): When set,
 * the IOMMU Access Control is enabled, and all the DMA access is
 * subjected to the IOMMU Access Control Policy.
 * When AIOMCTL.IACPENLK is set, this register bit is locked and can
 * no longer be modified.
 */
#define MISA_AIOMCTL_IACPEN     (1u << 0u)

/**
 * IOMMU Address Translation Enable (IATEN): When set, the
 * IOMMU Address Translation is enabled, and all the DMA access is
 * subjected to the IOMMU Address Translation Policy.
 */
#define MISA_AIOMCTL_IATEN     (1u << 1u)

/**
 * Unsupported MSI Check Enable (UMCEN): When set, MISA
 * will handle all the unsupported MSI type as MSI Access Control
 * Violation. When clear, MISA does not explicitly check or block
 * those MSIs.
 */
#define MISA_AIOMCTL_UMCEN     (1u << 2u)

/**
 * IOMMU Access Control Policy Enable Lock (IACPENLK):
 * When set, the AIOMCTL.IACPEN and the AIOMCTL.IACPENLK bits
 * are locked, and can no longer be modified.
 */
#define MISA_AIOMCTL_IACPENLK  (1u << 2u)

/**
 * IOMMU Error Status (IES): Hardware sets this bit to indicate
 * that the IOMMU programming error from FW. The specific IOMMU
 * programming errors, the MMIO First Byte Enable, MMIO offset
 * address, and the write data are captured in the ATTLPAE,
 * DMAACPSAIAE, MFBE, MOA, and MWD register.
 * The ATTLPAE, DMAACPSAIAE, MFBE, MOA, and MWD register is
 * only valid when this bit is set.
 * When this bit is set, it will not log the subsequent error or report a
 * new IOMMU programming error to ERRSTS. This bit will be clear
 * when Firmware/software write 1 to the ERRSTS.IOMES.
 */
#define MISA_AIOMERR_IES          (1u << 31u)

/**
 * Address Translation Table Linear Page Aliasing Error
 * (ATTLPAE): This bit is set to 1 by HW when FW write a new
 * Linear Page, but the page is already in an entry of the IOMMU
 * Address Translation table with valid bit set. HW will discard the
 * register write upon detection of this error. This bit is locked until
 * the IES bit is cleared.
 * This bit will be clear when IES bit is clear.
 */
#define MISA_AIOMERR_ATTLPAE      (1u << 30u)

/**
 * DMA Access Control Policy SAI Aliasing Error
 * (DMAACPSAIAE): This bit is set to 1 by HW when FW write a
 * new SAI, but the SAI is already in an entry of the IOMMU Access
 * Control Policy table with the policy enable bit set. HW will discard
 * the register write upon detection of this error. This bit is locked
 * until the IES bit is cleared.
 * This bit will be clear when IES bit is clear.
 */
#define MISA_AIOMERR_DMAACPSAIAE  (1u << 29u)

//TODO: Add MOA, MFBE

//TODO: RE DMA Access Control registers

/**
 * Linear Page Map (LINPGMAP): This field contains the linear
 * page address of this entry of IOMMU Translation Table.
 */
#define MISA_AIOMLIN_LINPGMAP_MASK (0xFFFFF000u)

/**
 * Entry Valid (EVLD): This field controls if this entry in the
 * IOMMU Translation Table contains a valid translation or not.
 */
#define MISA_AIOMLIN_EVLD          (1u << 0u)

typedef struct __attribute__((packed)) {
    uint32_t LIN;
    uint32_t PHY;
} misa_iommu_entry;

typedef struct __attribute__((packed)) {
    /**
     * @brief Aunit Extended Configuration Space
     * This register contains the base address for the PCI Express configuration space. This
     * window of addresses contains the 4KB of configuration space for each PCI Express
     * device that can potentially be part of the PCI Express hierarchy associated with the
     * system agent. There is no physical memory within this window of 256MB that can be
     * addressed. Each PCI Express hierarchy requires a PCI Express Base Register. The
     * system agent supports one PCI Express hierarchy. The region reserved by this register
     * does not alias to any PCI 2.3 compliant memory mapped space.
     * If the ECAM-Only Hardware Interface Strap is strapped enabled, this register becomes
     * Read-Only and will always read a value of 0xE0000001 to hardwire the ECBASE to
     * 0xE0000000 and force ECEN to 1.
     * [SIP] If the ECAM-Only Hardware Interface Strap is strapped disabled, this register is
     * read-write. On reset, the HEC space is disabled and must be enabled by writing a value
     * of 1 to ECEN (i.e. bit [0] of this register). This base address shall be assigned on a
     * 256MB aligned boundary. All other bits not decoded are Read-Only and will always
     * return 0. The Extended Configuration Base Address cannot overlap any physical
     * memory. Firmware/software must guarantee that these ranges do not overlap with
     * known ranges located in PCI MMIO addressable space. This register should be
     * programmed to the same value as that of HEC (Hunit Extended Configuration Space).
     */
    uint32_t AEC;         /* 0x1000 */
    /**
     * @brief Aunit Configuration CF8 Value
     * This register contains CONFIG_ADDRESS register value located at I/O offset 0xCF8
     * used in the Standard PCI Configuration mechanism.
     */
    uint32_t ACF8;        /* 0x1004 */
    /**
     * @brief Aunit Control
     * This register contains basic control information used by the Aunit.
     */
    uint32_t ACTRL;       /* 0x1008 */
    uint32_t reserved_100C[0x2d];
    /**
     * @brief Aunit MISR Control Register
     * This register controls the MISR MISR operation. During the typical operation, this
     * register should be set to 0x00000000.
     */
    uint32_t AMISRCTL;    /* 0x10C0 */
    /**
     * @brief Aunit Downstream Command MISR Output Register
     * This register displays the downstream command MISR signatures. Writes to this
     * register have no effect.
     */
    uint32_t ADNCMDMISR;  /* 0x10C4 */
    /**
     * @brief Aunit Downstream Data MISR Output Register
     * This register displays the downstream data MISR signatures. Writes to this register
     * have no effect.
     */
    uint32_t ADNDATMISR;  /* 0x10C8 */
    /**
     * @brief Aunit Upstream Command MISR Output Register
     * This register displays the upstream command MISR signatures. Writes to this
     * register have no effect.
     */
    uint32_t AUPCMDMISR;  /* 0x10CC */
    /**
     * @brief Aunit Upstream Data MISR Output Register
     * This register displays the upstream data MISR signatures. Writes to this register
     * have no effect.
     */
    uint32_t AUPDATMISR;  /* 0x10D0 */
    uint32_t reserved_10D4[0xB];
    /**
     * @brief Aunit Access Violation Control
     * This register is used to configure the access control reporting and to capture status
     * when an access violation is reported. Note that the register only logs the first access
     * violation. In the event of multiple violation happens at the same time (e.g. DMA Access
     * Control Violation and DMA Address Translation Miss), the MISA hardware will log either
     * one of the violation into all the logging registers. Due to the implementation, MISA
     * cannot guarantee the first error to be logged when errors are detected in multiple
     * transactions that MISA has yet to retire. The MISA hardware will log any of the
     * transactions into all the logging registers. Firmware/software is required to write 1 to
     * the respective ERRSTS field to clear it before it can further log subsequent errors.
     */
    uint32_t AAVCTL;      /* 0x1100 */
    uint32_t reserved_1104;
    /**
     * @brief Aunit Access Violation Data Log
     * This register is used to show the first DW of IOSF data from the transaction that
     * causing the access violation. Note that the register only logs the first access violation.
     * In the event of multiple violation happens at the same time (e.g. DMA Access Control
     * Violation and DMA Address Translation Miss), the MISA hardware will log either one of
     * the violation into all the logging registers. Due to the implementation, MISA cannot
     * guarantee the first error to be logged when errors are detected in multiple transactions
     * that MISA has yet to retire. The MISA hardware will log any of the transactions into all
     * the logging registers. Firmware/software is required to write 1 to the respective
     * ERRSTS field to clear it before it can further log subsequent errors.
     */
    uint32_t AAVDL;       /* 0x1108 */
    uint32_t reserved_110C;
    /**
     * @brief Aunit Access Violation Command Header Log 0
     * This register is used to show the first DW of IOSF Command Header from the
     * transaction that causing the access violation. Note that the register only logs the first
     * access violation. In the event of multiple violation happens at the same time (e.g. DMA
     * Access Control Violation and DMA Address Translation Miss), the MISA hardware will log
     * either one of the violation into all the logging registers. Due to the implementation,
     * MISA cannot guarantee the first error to be logged when errors are detected in multiple
     * transactions that MISA has yet to retire. The MISA hardware will log any of the
     * transactions into all the logging registers. Firmware/software is required to write 1 to
     * the respective ERRSTS field to clear it before it can further log subsequent errors.
     */
    uint32_t AAVHL0;       /* 0x1110 */
    /**
     * @brief Aunit Access Violation Command Header Log 1
     * This register is used to show the second DW of IOSF Command Header from the
     * transaction that causing the access violation. Note that the register only logs the first
     * access violation. In the event of multiple violation happens at the same time (e.g. DMA
     * Access Control Violation and DMA Address Translation Miss), the MISA hardware will log
     * either one of the violation into all the logging registers. Due to the implementation,
     * MISA cannot guarantee the first error to be logged when errors are detected in multiple
     * transactions that MISA has yet to retire. The MISA hardware will log any of the
     * transactions into all the logging registers. Firmware/software is required to write 1 to
     * the respective ERRSTS field to clear it before it can further log subsequent errors.
     */
    uint32_t AAVHL1;       /* 0x1114 */
    /**
     * @brief Aunit Access Violation Command Header Log 2
     * This register is used to show the third DW of IOSF Command Header from the
     * transaction that causing the access violation. Note that the register only logs the first
     * access violation. In the event of multiple violation happens at the same time (e.g. DMA
     * Access Control Violation and DMA Address Translation Miss), the MISA hardware will log
     * either one of the violation into all the logging registers. Due to the implementation,
     * MISA cannot guarantee the first error to be logged when errors are detected in multiple
     * transactions that MISA has yet to retire. The MISA hardware will log any of the
     * transactions into all the logging registers. Firmware/software is required to write 1 to
     * the respective ERRSTS field to clear it before it can further log subsequent errors.
     */
    uint32_t AAVHL2;       /* 0x1118 */
    uint32_t reserved_111C;
    /**
     * Aunit IOMMU Control Register
     * This register controls the IOMMU access control and address translation policy.
     */
    uint32_t AIOMCTL;      /* 0x1120 */
    /**
     * Aunit IOMMU Error Register
     * This register is used to capture status when an IOMMU programming error is reported.
     * Note that the register only logs the first IOMMU programming error. Firmware/software
     * is required to write 1 to clear the IOMES field in the ERRSTS register before it can
     * further log subsequent errors.
     */
    uint32_t AIOMERR;      /* 0x1124 */
    /**
     * Aunit IOMMU Error Data Register
     * This register is used to show the MMIO write data from the firmware that causing the
     * IOMMU programming error. Note that the register only logs the first IOMMU
     * programming error. Firmware/software is required to write 1 to clear the IOMES field in
     * the ERRSTS register before it can further log subsequent errors.
     */
    uint32_t AIOMERRD;      /* 0x1128 */
    uint32_t reserved_112C[0x5];
    /**
     * Aunit Completion Generation Matrix
     * This register defines the rights for a class of device (SAI) to send a Completion
     * upstream. It is programmed with a mask that indicates which agents in the system that
     * is allowed to send upstream Completion to MinuteIA.
     */
    uint32_t ACPLMTX[8];
    /**
     * Aunit MSI Generation Matrix
     * This register defines the rights for a class of device (SAI) to send an MSI upstream. It
     * is programmed with a mask that indicates which agents in the system that is allowed to
     * send upstream MSI to MinuteIA.
     */
    uint32_t AMSIMTX[8];
    uint32_t reserved_1180[0xa0];
    misa_iommu_entry AIOM[64];
    uint8_t reserved_1600[0x600];

} misa_aunit_regs;

typedef struct __attribute__((packed)) {
    uint32_t SRAM_CTL;
    uint32_t reserved_1C04[3];
    uint32_t SRAM_POWER_CTL;
    uint32_t reserved_1C14;
    uint32_t SRAM_POWER_STS;
    uint32_t reserved_1C1C;
    uint32_t SRAM_INIT_CTL;
    uint32_t reserved_1C24;
    uint32_t SRAM_INIT_STS;
    uint32_t reserved_1C2C;
    uint32_t SRAM_INIT_CFG;
    uint32_t reserved_1C34[3];
    uint32_t SRAM_INIT_LFSR_SEED[4];
    uint32_t SRAM_SM_CTL;
    uint32_t reserved_1C54;
    uint32_t SRAM_SM_STS;
    uint32_t reserved_1C5C;
    uint32_t SRAM_SMC_2;
    uint32_t reserved_1C64[3];
    uint32_t SRAM_ECC_CTL;
    uint32_t SRAM_ECC_POISONING;
    uint32_t SRAM_ECC_LOG;
    uint32_t SRAM_ECC_CNT;
    uint32_t SRAM_ERR_LOG;
    uint32_t SRAM_FS;
    uint32_t reserved_1C88[2];
    uint32_t SRAM_READ_CMD_BYPASS;
    uint32_t reserved_1C94;
    uint32_t SRAM_READ_CPL_BYPASS;
    uint32_t reserved_1C9C;
    uint32_t ROM_READ_CMD_CPL_BYPASS;
} misa_sram_regs;

typedef struct __attribute__((packed)) {
    uint8_t reserved_000[0x400];
    misa_bunit_regs bunit;
    misa_cunit_regs cunit;
    misa_hunit_regs hunit;
    misa_aunit_regs aunit;
    misa_sram_regs  sram;
} misa_regs;

#endif //MELOADER_MISA_H
