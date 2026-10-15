// usbdevfs, Linux's user-space USB, as plain C calls: its ioctls are macros
// Swift cannot import. What the server needs to be the USB host for the
// comma's gadget on a file descriptor, whether the Android app opened it
// (UsbDeviceConnection.getFileDescriptor) or the Linux server did: bulk URBs
// submitted, discarded and reaped, a wait for a completion that another
// thread can end, the speed the bus negotiated, and on Linux the descriptors
// and the interface claim that Android's UsbManager does for the app.
//
// Calls return 0 or an errno. The kernel writes a URB's status, length and
// any IN data only when the URB is reaped, so a URB's memory and its buffer
// must outlive it until then; see UsbfsPipes.swift.
#ifndef JL_USBFS_H
#define JL_USBFS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct jl_urb jl_urb;

// A bulk URB on `endpoint` (its address, 0x80 set for IN) over `buffer`.
// `context` comes back from jl_urb_context once reaped. NULL when out of memory.
jl_urb *jl_urb_create(uint8_t endpoint, void *buffer, int length, void *context);
// Points a reaped URB at `buffer` again and clears its last result, so one
// URB serves transfer after transfer without an allocation each.
void jl_urb_set(jl_urb *urb, void *buffer, int length);
void jl_urb_free(jl_urb *urb);
void *jl_urb_context(const jl_urb *urb);
// After the reap: 0, or a negative errno (-ENOENT or -ECONNRESET when discarded).
int jl_urb_status(const jl_urb *urb);
int jl_urb_actual(const jl_urb *urb);

int jl_usbfs_submit(int fd, jl_urb *urb);
int jl_usbfs_discard(int fd, jl_urb *urb);
// One finished URB without waiting: EAGAIN when there is none, ENODEV when
// the device is gone and nothing is left to reap.
int jl_usbfs_reap(int fd, jl_urb **out);

// An eventfd that ends a jl_usbfs_wait from another thread; -1 on failure.
int jl_usbfs_wake_create(void);
void jl_usbfs_wake(int wake_fd);
void jl_usbfs_wake_close(int wake_fd);
// Waits up to `timeout_ms` (-1: forever) for a URB to reap or a wake.
// 0 either way, ENODEV once the device is gone (reap what is left first).
int jl_usbfs_wait(int fd, int wake_fd, int timeout_ms);

// The bus speed: 1 low, 2 full, 3 high, 5 super, 6 super-plus; or -errno.
int jl_usbfs_speed(int fd);

// The descriptors a usbfs node reads back, from the start: the device's,
// then every configuration's in full. The bytes read, or -errno.
int jl_usbfs_descriptors(int fd, void *buffer, int capacity);
// The name of the kernel driver bound to `interface` into `name`
// (NUL-terminated): 0, ENODATA when none is, or an errno.
int jl_usbfs_driver(int fd, unsigned interface, char *name, int capacity);
// Unbinds that driver: USBDEVFS_DISCONNECT through USBDEVFS_IOCTL.
int jl_usbfs_disconnect(int fd, unsigned interface);
int jl_usbfs_claim(int fd, unsigned interface);
int jl_usbfs_release(int fd, unsigned interface);

#ifdef __cplusplus
}
#endif

#endif
