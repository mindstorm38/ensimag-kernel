/// Segment selectors constants.

#ifndef __SEGMENT_H__
#define __SEGMENT_H__

/// Segment 1 in GDT (RPL = 0).
#define BASE_TSS	0x08
/// Segment 2 in GDT (RPL = 0), code.
#define KERNEL_CS	0x10
/// Segment 3 in GDT (RPL = 0), data.
#define KERNEL_DS	0x18
/// Segment 8 in GDT (RPL = 3), code.
#define USER_CS		0x43
/// Segment 9 in GDT (RPL = 3), data.
#define USER_DS		0x4b
/// Segment 10+ in GDT (RPL = 0), task.
#define TRAP_TSS_BASE	0x50

#endif
