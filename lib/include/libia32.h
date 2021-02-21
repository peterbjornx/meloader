//
// Created by pbx on 26/01/20.
//

#ifndef LIBIA32_LIBIA32_H
#define LIBIA32_LIBIA32_H
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>

#define LOG_TRACE (0)
#define LOG_DEBUG (1)
#define LOG_INFO  (2)
#define LOG_WARN  (3)
#define LOG_ERROR (4)
#define LOG_FATAL (5)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int ips;
} libia32_params;

typedef struct libia32_corecplx_s libia32_corecplx;

/**
 * Structure containing the callbacks used by the emulator to
 * interact with the application.
 * Fields filled with null pointers result in default behaviour:
 * If no mem_read/io_read is implemented, accesses will yield all 0xFF
 * bytes, which is the behaviour observed on real hardware for unhandled
 * address space.
 */
typedef struct {

    /**
     * Called when one of the emulated CPUs tries to write to memory
     * @param context The core complex the access originated from.
     * @param addr    The address being written to
     * @param buffer  The buffer containing the data to write
     * @param size    The size of the data being written
     */
    void (*mem_write)(
            libia32_corecplx *context,
            uint64_t addr,
            const void *buffer,
            size_t size);

    /**
     * Called when one of the emulated CPUs tries to read from memory
     * @param context The core complex the access originated from.
     * @param addr    The address being read from
     * @param buffer  The buffer the data should be read into
     * @param size    The size of the data being read
     */
    void (*mem_read )(
            libia32_corecplx *context,
            uint64_t addr,
            void *buffer,
            size_t size);

    /**
     * Called when one of the emulated CPUs tries to write to IO space
     * @param context The core complex the access originated from.
     * @param addr    The address being written to
     * @param buffer  The buffer containing the data to write
     * @param size    The size of the data being written
     */
    void (*io_write)(
            libia32_corecplx *context,
            uint32_t port,
            const void *buffer,
            size_t size);

    /**
     * Called when one of the emulated CPUs tries to write to IO space
     * @param context The core complex the access originated from.
     * @param addr    The address being read from
     * @param buffer  The buffer the data should be read into
     * @param size    The size of the data being read
     */
    void (*io_read )(
            libia32_corecplx *context,
            uint32_t port,
            void *buffer,
            size_t size);

    /**
     * Get host address for guest memory page.
     * This allows direct accesses to a 4K block of guest memory from
     * the emulator. If for whatever reason this is not possible, this
     * function should return NULL. Returning NULL will force the
     * emulator to use the mem_read/mem_write callbacks for this page.
     * @param context The core complex the access originated from.
     * @param paddr   The guest physical address to map
     * @param rw      The access bits for the page TODO: figure out bitfield
     */
    void *(*get_host_map)(
            libia32_corecplx *context,
            uint64_t paddr,
            unsigned rw);

    /**
     * Called by the emulator in response to an INTR generated interrupt.
     * Analogous to real HW, the device should provide an interrupt vector
     * in response to INTACK.
     * @param context The core complex the acknowledgement originated from.
     * @return        The interrupt vector number for the interrupt being
     *                acknowledged.
     */
    uint8_t (*legacy_int_ack)(libia32_corecplx *context);

    /**
     * Called by the emulator in response to a HRQ hold request.
     * When this function has been called, the emulator is guaranteed to not
     * issue bus transactions until HRQ is deasserted.
     * @param context The core complex the acknowledgement originated from.
     */
    void (*legacy_hold_ack)(libia32_corecplx *context);

    /**
     * Called by the emulator when a real mode FPU exception occurs.
     * The 8087 being emulated by a real mode CPU used to generate a
     * hardware interrupt through the PIC in the original PC. Because the
     * library does not implement any chipset hardware, the applications
     * should handle routing this to INTR or NMI somehow.
     * @param context
     */
    void (*legacy_raise_fpu_int)(libia32_corecplx *context);

    /**
     * Log a message from the emulator core
     */
    void (*log)( libia32_corecplx *context, int level, const char *format, va_list args );

} libia32_callbacks;

/**
 * This structure represent a core complex and wraps the internal
 * C++ state of the emulator core. It's members are not meant to be
 * accessed or modified by applications using the library.
 */
struct libia32_corecplx_s {
    libia32_callbacks callbacks;
    void *user_context;
    void *internal_complex;
    void *internal_system;
    void *internal_params;
};

/**
 * Create a core complex.
 * @param params    The emulation parameters, @see libia32_params
 * @param callbacks The callbacks from lib. to app., @see libia32_callbacks
 * @return          The core complex object that was created.
 */
libia32_corecplx *libia32_corecplx_create(
        libia32_params *params,
        libia32_callbacks *callbacks );

/**
 * Deallocates all resources associated with an emulated core complex
 * @param complex The core complex to destroy
 */
void libia32_corecplx_destroy( libia32_corecplx *complex );

/**
 * Sets the hold request signal (HRQ) for all of the emulated cores.
 * Once the cores are ready to relinquish bus control, the emulator will
 * call the legacy_hold_ack callback.
 * @param complex The core complex to operate on.
 * @param value   The value of the signal: 0->Operate normally, others->req HOLD
 */
void libia32_corecplx_set_hrq( libia32_corecplx *complex, int value );

/**
 * Asserts the INTR line on the bootstrap processor in the core complex.
 * This will effect a legacy x86 interrupt and will result in the emulator
 * calling legacy_int_ack to request the interrupt vector.
 * @param complex The core complex to operate on.
 */
void libia32_corecplx_raise_intr( libia32_corecplx *complex );

/**
 * Deasserts the INTR line on the bootstrap processor in the core complex.
 * This will prevent the processor from accepting an interrupt when its IF goes
 * high.
 * @param complex The core complex to operate on.
 */
void libia32_corecplx_clear_intr( libia32_corecplx *complex );

/**
 * Runs a quantum of instructions for each of the emulated cores.
 * @param complex The core complex to emulate
 */
void libia32_corecplx_run_slice( libia32_corecplx *complex );

/**
 * Causes a system management interrupt (SMI) to be delivered to the
 * bootstrap processor.
 * @param complex The core complex to emulate
 */
void libia32_corecplx_deliver_nmi( libia32_corecplx *complex );

/**
 * Causes a non-maskable interrupt (NMI) to be delivered to the
 * bootstrap processor.
 * @param complex The core complex to emulate
 */
void libia32_corecplx_deliver_smi( libia32_corecplx *complex );

/**
 * Resets the core complex.
 * @note Must be called with sw_only = 0 at least once before the first
 *       call to run_slice!
 * @param complex The core complex to reset.
 * @param sw_only Whether only the software-visible state is reset, or
 *                a virtual power cycle is effected.
 */
void libia32_corecplx_reset( libia32_corecplx *complex, int sw_only );

#ifdef __cplusplus
}
#endif


#endif //LIBIA32_LIBIA32_H
