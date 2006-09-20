#define PSR_MODE_MASK   0x1f
#define PSR_MODE_USER   0x10
#define PSR_MODE_FIQ    0x11
#define PSR_MODE_IRQ    0x12
#define PSR_MODE_SVC    0x13
#define PSR_MODE_DEBUG  0x15
#define PSR_MODE_ABORT  0x17
#define PSR_MODE_UNDEF  0x1b
#define PSR_MODE_SYSTEM 0x1f
#define PSR_NOINTS_MASK 0xc0

#define FUNC_REQ_GL 0
#define FUNC_REQ_DLW 1
#define FUNC_REQ_GAME 2 //not specifically referenced
#define FUNC_REQ_MAC 3
#define FUNC_REQ_WAKEUP 4
#define FUNC_REQ_PIC 5
