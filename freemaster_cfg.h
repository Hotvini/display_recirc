#ifndef __FREEMASTER_CFG_H
#define __FREEMASTER_CFG_H

#define FMSTR_PLATFORM_CORTEX_M 1

#define FMSTR_DISABLE           0

#define FMSTR_LONG_INTR         0
#define FMSTR_SHORT_INTR        0
#define FMSTR_POLL_DRIVEN       1

#define FMSTR_TRANSPORT         FMSTR_PDBDM

#define FMSTR_COMM_BUFFER_SIZE  0
#define FMSTR_COMM_RQUEUE_SIZE  16

#define FMSTR_USE_TSA           1
#define FMSTR_USE_TSA_INROM     1
#define FMSTR_USE_TSA_SAFETY    1
#define FMSTR_USE_TSA_DYNAMIC   0

#define FMSTR_USE_SCOPE         0
#define FMSTR_USE_RECORDER      0
#define FMSTR_USE_PIPES         0
#define FMSTR_USE_APPCMD        0

#define FMSTR_USE_READMEM       1
#define FMSTR_USE_WRITEMEM      1
#define FMSTR_USE_WRITEMEMMASK  1

#endif /* __FREEMASTER_CFG_H */
