#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/kmod.h>
#include <linux/slab.h>
#include <asm/io.h>

#define KB_IRQ 1

#define ALT_PRESSED 56    // Alt pressed
#define ALT_UNPRESSED 184 // Alt unpressed

#define LEVEL0_PRESSED 11     // Key 0 pressed 0%
#define LEVEL1_PRESSED 2      // Key 1 pressed 10%
#define LEVEL2_PRESSED 3      // Key 2 pressed 20%
#define LEVEL3_PRESSED 4      // Key 3 pressed 30%
#define LEVEL4_PRESSED 5      // Key 4 pressed 40%
#define LEVEL5_PRESSED 6      // Key 5 pressed 50%
#define LEVEL6_PRESSED 7      // Key 6 pressed 60%
#define LEVEL7_PRESSED 8      // Key 7 pressed 70%
#define LEVEL8_PRESSED 9      // Key 8 pressed 80%
#define LEVEL9_PRESSED 10     // Key 9 pressed 90%
#define LEVELMINUS_PRESSED 12 // Key - pressed decrease
#define LEVELPLUS_PRESSED 13  // Key = pressed increase

unsigned char scancode;
long current_audio = 0;

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Leonov Vladislav");

DEFINE_SPINLOCK(k_lock);

static struct workqueue_struct *wq;
static struct work_struct *work;

void audio_controll(struct work_struct *work)
{
    static int alt = 0;
    static long audio_flag = -1;

    spin_lock(&k_lock);
    switch (scancode)
    {
    case ALT_PRESSED:
        alt = 1;
        break;
    case ALT_UNPRESSED:
        alt = 0;
        break;
    case LEVEL0_PRESSED:
        if (alt)
            audio_flag = 0;
        break;
    case LEVEL1_PRESSED:
        if (alt)
            audio_flag = 10;
        break;
    case LEVEL2_PRESSED:
        if (alt)
            audio_flag = 20;
        break;
    case LEVEL3_PRESSED:
        if (alt)
            audio_flag = 30;
        break;
    case LEVEL4_PRESSED:
        if (alt)
            audio_flag = 40;
        break;
    case LEVEL5_PRESSED:
        if (alt)
            audio_flag = 50;
        break;
    case LEVEL6_PRESSED:
        if (alt)
            audio_flag = 60;
        break;
    case LEVEL7_PRESSED:
        if (alt)
            audio_flag = 70;
        break;
    case LEVEL8_PRESSED:
        if (alt)
            audio_flag = 80;
        break;
    case LEVEL9_PRESSED:
        if (alt)
            audio_flag = 90;
        break;
    case LEVELMINUS_PRESSED:
        if (alt && current_audio > 0)
            audio_flag = current_audio - 1;
        break;
    case LEVELPLUS_PRESSED:
        if (alt && current_audio < 100)
            audio_flag = current_audio + 1;
        break;
    default:
        break;
    }

    if (audio_flag >= 0)
    {
        printk(KERN_INFO "System Volume Changing Started.\n");
        current_audio = audio_flag;
        char buffer[10];
        sprintf(buffer, "%d", current_audio);
        char *argv[] = {"/home/leerycorsair/os_course/audio_controller", buffer, NULL};
        static char *envp[] = {
            "HOME=/",
            "TERM=linux",
            "PATH=/sbin:/bin:/usr/sbin:/usr/bin", NULL};

        call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC);
        printk(KERN_INFO "System Volume Changed to %d\%.\n", current_audio);
    }

    audio_flag = -1;
    spin_unlock(&k_lock);
}

irq_handler_t kb_irq_handler(int irq, void *dev_id, struct pt_regs *regs)
{
    spin_lock(&k_lock);
    scancode = inb(0x60);
    spin_unlock(&k_lock);

    queue_work(wq, work);

    return (irq_handler_t)IRQ_HANDLED;
}

static int __init md_init(void)
{
    int ret;
    printk(KERN_INFO "Module Initialization.\n");
    ret = request_irq(KB_IRQ, (irq_handler_t)kb_irq_handler, IRQF_SHARED, "Custom Handler", (void *)(kb_irq_handler));
    if (ret != 0)
        printk(KERN_INFO "Cannot Request IRQ for keyboard.\n");

    work = kmalloc(sizeof(struct work_struct), GFP_KERNEL);
    if (work)
    {
        INIT_WORK((struct work_struct *)work, audio_controll);
    }
    wq = create_workqueue("keyboard_queue");

    printk(KERN_INFO "Module Inited.\n");
    return 0;
}

static void __exit md_exit(void)
{
    free_irq(KB_IRQ, (void *)(kb_irq_handler));
    flush_workqueue(wq);
    destroy_workqueue(wq);

    if (work)
        kfree(work);

    printk(KERN_INFO "Module Exit.");
}

module_init(md_init);
module_exit(md_exit);