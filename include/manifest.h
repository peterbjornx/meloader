#ifndef __manifest__
#define __manifest__

#include <stdint.h>

/* TODO: ext 0,1,2,3,4 */

/**
 * Kernel exception policy: 0 - Reset System, 1 - Terminate Process
 */
#define PROC_ATTR_FLAG_FAULT_TOLERANT             (1<<0)
/**
 * @brief Permanent process Y/N.
 * A permanent process' code/rodata sections are not removed from RAM when it 
 * terminates normally in order to optimize its reload flow.
 */
#define PROC_ATTR_FLAG_PERMANENT_PROCESS          (1<<1)
/**
 * @brief Single Instance Y/N.
 * When the process is spawned if it is already running in the system the spawn will fail.
 */
#define PROC_ATTR_FLAG_SINGLE_INSTANCE            (1<<2)
/**
 * @brief Trusted SendReceive Sender Y/N. 
 * If set this process is allowed to send IPC_SendReceive messages to any process (not only public).
 */
#define PROC_ATTR_FLAG_TRUSTED_SND_REV_SENDER     (1<<3)
/**
 * @brief Trusted Notify Sender Y/N. 
 * If set this process is allowed to send IPC_Notify notifications to any process (not only public).
 */
#define PROC_ATTR_FLAG_TRUSTED_NOTIFY_SENDER      (1<<4)
/**
 * @brief Public SendReceive Receiver Y/N. 
 * If set any other process is allowed to send IPC_SendReceive messages to it (not only trusted).
 */
#define PROC_ATTR_FLAG_PUBLIC_SND_REV_RECEIVER    (1<<5)
/**
 * Public Notify Receiver Y/N.
 * If set any other process is allowed to IPC_Notify notifications messages to it (not only trusted).
 */
#define PROC_ATTR_FLAG_PUBLIC_NOTIFY_RECEIVER     (1<<6)

/**
 * Process attributes
 */
typedef struct __attribute__((packed)) {
    /* @brief 0x5 for process attribute extension */
    uint32_t    type;
    uint32_t    length;
    uint32_t    flags;
    /** 
     * @brief TID for main thread. 
     * Optional for IBL processes only. 
     * Must be 0 for other processes. 
     */
    uint32_t    main_thread_id;
    /** 
     * @brief Base address for code. 
     * Address is in LAS for Bringup/Kernel VAS for other processes. 
     * Must be 4KB aligned
     */
    uint32_t    priv_code_base_address;
    /**
     * @brief Size of uncompressed process code.
     * Does not include code for shared library.
     */
    uint32_t    uncompressed_priv_code_size;
    /**
     * @brief Size of Thread-Local-Storage for the process
     */
    uint32_t    cm0_heap_size;
    uint32_t    bss_size;
    uint32_t    default_heap_size;
    /**
     * @brief VAS of entry point function for the process main thread
     */
    uint32_t    main_thread_entry;
    /**
     * @brief Bitmask of allowed system calls by the process
     */
    uint8_t     allowed_sys_calls[12];
    /**
     * @brief Runtime User ID for process
     */
    uint16_t    user_id;
    /**
     * @brief Temporary placeholder for thread base
     */
    uint32_t    reserved_0;
    /**
     * @brief Must be 0
     */
    uint16_t    reserved_1;
    /** */
    uint64_t    reserved_2;
    /**
     * Group ID for process
     */
    uint16_t    group_ids[0];
} man_ext_process;

/**
 * Set to 0 for live thread 1 for CM0-U-only thread; 
 */
#define MAN_THREAD_FLAG0 (1<<0)

typedef struct __attribute__((packed)) {
    /** @brief Size of main thread stack in bytes 
     * (not including guard page including space reserved for TLS).
     * Must be divisible by 4K with the following exception: 
     *  if the default heap size is smaller than 4K the last thread's stack size may have any size.
     */
    uint32_t stack_size;
    /** Flags */
    uint32_t flags;
    /** @brief Scheduling policy
     * Bits 0-7: Scheduling Policy, 0 -> fixed priority; Bits 8-31: Scheduling attributes.
     * For a fixed priority policy this is the scheduling priority of the thread.
     */
    uint32_t scheduling_policy;
} man_thread;

/**
 * Thread info extension
 */
typedef struct __attribute__((packed)) {
    /** @brief 0x6 for thread info extension */
    uint32_t    type;
    /** @brief Length of this extension */
    uint32_t    length;
    /** */
    man_thread  threads[0];
} man_ext_threads;

typedef struct __attribute__((packed)) {
    /** Base address of the MMIO range */
    uint32_t    base;
    /** Limit in bytes of the MMIO range */
    uint32_t    limit;
    /** Read access Y/N */
    uint32_t    flags;
} man_mmio_range;

typedef struct __attribute__((packed)) {
    /** @brief 0x8 for MMIO ranges */
    uint32_t    type;
    /** @brief Length of this extension */
    uint32_t    length;
    /** */
    man_mmio_range mmio_range_defs[0];
} man_ext_mmio_ranges;

typedef struct __attribute__((packed)) {
    /** @brief 0x10 for module attributes */
    uint32_t    type;
    /** @brief Length of this extension */
    uint32_t    length;

    /** @brief 0 - Uncompressed; 1 - Huffman Compressed; 2 - LZMA Compressed */
    uint8_t     compression_type;
    uint8_t     reserved[3];

    /** @brief Uncompressed image size must be divisible by 4K */
    uint32_t    uncompressed_size;

    /** @brief Compressed image size.
     * This is applicable for LZMA compressed modules only. 
     * For other modules should be the same as uncompressed_size field.
     */
    uint32_t    compressed_size;

    /** Module number unique in the scope of the vendor. */
    uint16_t    module_number;

    /** Vendor ID (PCI style). For Intel modules must be 0x8086. */
    uint16_t    vendor_id;

    /** SHA2 Hash of uncompressed image */
    uint32_t    hash[32];

} man_ext_mod_attr;

typedef struct __attribute__((packed)) {
    /** @brief 0x4 for shared library extension */
    uint32_t    type;
    /** @brief Length of this extension */
    uint32_t    length;
    /** @brief Size in bytes of the shared library context */
    uint32_t    context_size;
    /**
     * Including padding pages for library growth.
     * This needs to be updated once the SHARED_CONTEXT_SIZE symbol is defined in the build process.
     * Currently set to a temporary value.
     */
    uint32_t    total_alloc_virtual_space;
    /**
     * Base address for the library private code in VAS.
     * Must be 4KB aligned.
     */
    uint32_t    code_base_address;
    /** @brief Size of Thread-Local-Storage used by the shared library. */
    uint32_t    tls_size;
    /**
     * reserved bytes set to 0xffffffff
     */
    uint32_t    reserved;

} man_ext_shared_lib;



typedef struct __attribute__((packed)) {
    /**
     * @brief Base address in VAS of range to be locked.
     * Must be divisible in 4KB.
     */
    uint32_t    base;
    /**
     * @brief Size of range to be locked.
     * Must be divisible in 4KB.
     */
    uint32_t    size;
} man_locked_range;

/**
 * Locked ranges extension
 */
typedef struct __attribute__((packed)) {
    /** @brief 0xB for locked range extension */
    uint32_t    type;
    /** @brief Length of this extension */
    uint32_t    length;
    /** */
    man_locked_range ranges[0];
} man_ext_locked_ranges;

typedef struct __attribute__((packed)) {
    uint32_t    type;
    uint32_t    length;
} man_ext;

void *man_ext_find( void *manifest, int size, int type );

#endif
