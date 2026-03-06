#ifndef __FREEMASTER_CFG_H
#define __FREEMASTER_CFG_H

/* Target CPU family used by the FreeMASTER core. */
#define FMSTR_PLATFORM_CORTEX_M 1

/* Master on/off switch for the FreeMASTER integration. */
#define FMSTR_DISABLE           0

/* Interrupt-driven servicing is disabled; polling is used instead. */
#define FMSTR_LONG_INTR         0
#define FMSTR_SHORT_INTR        0
#define FMSTR_POLL_DRIVEN       1

/* Transport over the debugger probe instead of UART/CAN/etc. */
#define FMSTR_TRANSPORT         FMSTR_PDBDM

/* Communication buffers used by the selected transport. */
#define FMSTR_COMM_BUFFER_SIZE  0
#define FMSTR_COMM_RQUEUE_SIZE  16

/* TSA publishes typed variables and access permissions to the host. */
#define FMSTR_USE_TSA           1
#define FMSTR_USE_TSA_INROM     1
#define FMSTR_USE_TSA_SAFETY    1
#define FMSTR_USE_TSA_DYNAMIC   0

/* Optional FreeMASTER features kept off to minimize footprint/overhead. */
#define FMSTR_USE_SCOPE         1
#define FMSTR_USE_RECORDER      0
#define FMSTR_USE_PIPES         0
#define FMSTR_USE_APPCMD        0

/* Monitor-only setup: allow memory reads, deny all raw memory writes. */
#define FMSTR_USE_READMEM       1
#define FMSTR_USE_WRITEMEM      0
#define FMSTR_USE_WRITEMEMMASK  0

#endif /* __FREEMASTER_CFG_H */
