#include "filesystem.h"

// arbitrary offset into flash, hopefully doesn't overlap with the program space lol
#define FS_START 0xAC000
const uint8_t *flash_start = (const uint8_t *) (XIP_BASE + FS_START);

int lfs_flash_read(const struct lfs_config *c,
                                 lfs_block_t block, lfs_off_t off, void *dst, lfs_size_t size) {
    //    Serial.printf(" READ: %p, %d\n", me->_start + (block * me->_blockSize) + off, size);
    memcpy(dst, flash_start + (block * c->block_size) + off, size);
    return 0;
}

int lfs_flash_prog(const struct lfs_config *c,
                                 lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size) {
    uint8_t *addr = flash_start + (block * c->block_size) + off;
    multicore_lockout_start_timeout_us(500);
    uint32_t ints = save_and_disable_interrupts();
    flash_range_program((intptr_t)addr - (intptr_t)XIP_BASE, (const uint8_t *)buffer, size);
    restore_interrupts(ints);
    multicore_lockout_end_timeout_us(500);
    return 0;
}

int lfs_flash_erase(const struct lfs_config *c, lfs_block_t block) {
    uint8_t *addr = flash_start + (block * c->block_size);
    //printf("ERASE: %p, %d\n", (intptr_t)addr - (intptr_t)XIP_BASE, c->block_size);
    int res = 0;
    multicore_lockout_start_timeout_us(500);
    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase((intptr_t)addr - (intptr_t)XIP_BASE, c->block_size);
    restore_interrupts(ints);
    multicore_lockout_end_timeout_us(500);
    return 0;
}

int lfs_flash_sync(const struct lfs_config *c) {
    /* NOOP */
    (void) c;
    return 0;
}

// variables used by the filesystem
lfs_t lfs;
lfs_file_t file;
// configuration of the filesystem is provided by this struct
const struct lfs_config cfg = {
    // block device operations
    .read  = lfs_flash_read,
    .prog  = lfs_flash_prog,
    .erase = lfs_flash_erase,
    .sync  = lfs_flash_sync,

    // block device configuration
    .read_size = 256,
    .prog_size = 256,
    .block_size = 4096,
    .block_count = 256,
    .block_cycles = 16,
    .cache_size = 256,
    .lookahead_size = 16,
};

lfs_t* GetLFS()
{
    return &lfs;
}

int file_read(void *buffer, uint32_t offset, size_t size)
{
    memcpy(buffer, flash_start+offset, size);
    return 0;
}
int file_write(void *buffer, uint32_t offset, size_t size)
{
    uint32_t ints = save_and_disable_interrupts();
    multicore_lockout_start_timeout_us(500);
    flash_range_program(FS_START+offset, buffer, size);
    multicore_lockout_end_timeout_us(500);
    restore_interrupts(ints);
    return 0;
}

int file_erase() {
    //printf("ERASE: %p, %d\n", (intptr_t)addr - (intptr_t)XIP_BASE, c->block_size);
    multicore_lockout_start_timeout_us(500);
    uint32_t ints = save_and_disable_interrupts();
    // re-enable the audio related interrupts so we don't mess up the dac
    irq_set_enabled(DMA_IRQ_0, true);
    irq_set_enabled(DMA_IRQ_1, true);
    flash_range_erase(FS_START,4096*32);
    restore_interrupts(ints);
    multicore_lockout_end_timeout_us(500);
    return 0;
}
static s32_t my_spiffs_read(u32_t addr, u32_t size, u8_t *dst)
{
    memcpy(dst, flash_start+addr, size);
    return SPIFFS_OK;
}
static u8_t spiffs_write_buf[256];

static s32_t my_spiffs_write(u32_t addr, u32_t size, u8_t *src)
{
    //uint32_t ints = save_and_disable_interrupts();
    //multicore_lockout_start_timeout_us(500);
    // irritatingly, spiffs doesn't align writes to the logical page boundary, so we need to do that?
    u32_t addr_offset = addr%256;
    // lets assume the write size is always going to be less than 256?
    memset(spiffs_write_buf, 0xff, 256);
    // print out current data at target address
    // for (size_t i = 0; i < 256; i++)
    // {
    //     printf("%02x ", (*(flash_start+addr-addr_offset+i)) & 0xff);
    //     if(i%8==7)
    //         printf("\n");
    // }
    // printf("\nwrite:\n");
    // for (size_t i = 0; i < size; i++)
    // {
    //     printf("%02x ", (*(src+i)) & 0xff);
    //     if(i%8==7)
    //         printf("\n");
    // }
    // printf("\n");
    for (size_t i = 0; i < 256; i++)
    {
        if(i>=addr_offset && i-addr_offset < size)
        {
            spiffs_write_buf[i] = spiffs_write_buf[i] & src[i-addr_offset]; 
        }
        // todo: there is an early out we can do here, figure it out.
    }

    printf("write size %i addrbase %i addr_offset %i \n", size, addr, addr_offset);
    flash_range_program(FS_START+addr-addr_offset, spiffs_write_buf, 256);
    // for (size_t i = 0; i < 256; i++)
    // {
    //     printf("%02x ", (*(flash_start+addr-addr_offset+i)) & 0xff);
    //     if(i%8==7)
    //         printf("\n");
    // }
    // printf("\n");
    //multicore_lockout_end_timeout_us(500);
    //restore_interrupts(ints);
    return SPIFFS_OK;
}

static s32_t my_spiffs_erase(u32_t addr, u32_t size)
{
    //printf("ERASE: %p, %d\n", (intptr_t)addr - (intptr_t)XIP_BASE, c->block_size);
    //multicore_lockout_start_timeout_us(500);
    //uint32_t ints = save_and_disable_interrupts();
    // re-enable the audio related interrupts so we don't mess up the dac
    // irq_set_enabled(DMA_IRQ_0, true);
    // irq_set_enabled(DMA_IRQ_1, true);
    flash_range_erase(FS_START+addr,size);
    //restore_interrupts(ints);
    //multicore_lockout_end_timeout_us(500);
    return SPIFFS_OK;
}

static spiffs fs;
#define LOG_PAGE_SIZE       256

static u8_t spiffs_work_buf[LOG_PAGE_SIZE*32];
static u8_t spiffs_fds[32*4];
static u8_t spiffs_cache_buf[(LOG_PAGE_SIZE+32)*4];

void my_spiffs_mount() {
    spiffs_config cfg;
    cfg.phys_size = 1*512*1024; // use all spi flash
    cfg.phys_addr = 0; // start spiffs at start of spi flash
    cfg.phys_erase_block = 0xffff; // according to datasheet
    cfg.log_block_size = 0xffff; // let us not complicate things
    cfg.log_page_size = LOG_PAGE_SIZE; // as we said

    cfg.hal_read_f = my_spiffs_read;
    cfg.hal_write_f = my_spiffs_write;
    cfg.hal_erase_f = my_spiffs_erase;

    int res = SPIFFS_mount(&fs,
        &cfg,
        spiffs_work_buf,
        spiffs_fds,
        sizeof(spiffs_fds),
        spiffs_cache_buf,
        sizeof(spiffs_cache_buf),
        0);
    printf("mount res: %i\n", res);
    if(res == SPIFFS_ERR_NOT_A_FS)
    {
        int res = SPIFFS_format(&fs);
        printf("format res: %i\n", res);

        res = SPIFFS_mount(&fs,
            &cfg,
            spiffs_work_buf,
            spiffs_fds,
            sizeof(spiffs_fds),
            spiffs_cache_buf,
            sizeof(spiffs_cache_buf),
            0);
        printf("mount res: %i\n", res);
    }
}

void TestFS()
{
    // and again, lets just make my own file system, yay!
    // once again, an alternate approach, lets test out spiffs
        // instead of all this junk - lets just clear some space
    //multicore_lockout_start_timeout_us(500);
    // uint32_t ints = save_and_disable_interrupts();
    // flash_range_erase(FS_START,4096*32);
    // restore_interrupts(ints);
    //multicore_lockout_end_timeout_us(500);

    my_spiffs_mount();
    char buf[12];
    
    // Surely, I've mounted spiffs before entering here
  
    spiffs_file fd = SPIFFS_open(&fs, "my_file2", SPIFFS_CREAT | SPIFFS_TRUNC | SPIFFS_RDWR, 0);
    if (SPIFFS_write(&fs, fd, (u8_t *)"Hello world", 12) < 0) printf("errno %i\n", SPIFFS_errno(&fs));
    SPIFFS_close(&fs, fd); 
  
    fd = SPIFFS_open(&fs, "my_file2", SPIFFS_RDWR, 0);
    if (SPIFFS_read(&fs, fd, (u8_t *)buf, 12) < 0) printf("errno %i\n", SPIFFS_errno(&fs));
    SPIFFS_close(&fs, fd);
  
    printf("--> %s <--\n", buf);
    return;

    return;
    int err = lfs_mount(&lfs, &cfg);

    // // reformat if we can't mount the filesystem
    // // this should only happen on the first boot
    if (err)
    {
        lfs_format(&lfs, &cfg);
        lfs_mount(&lfs, &cfg);
    }

    // read current count
    uint32_t boot_count = 0;
    lfs_file_open(&lfs, &file, "boot_count", LFS_O_RDWR | LFS_O_CREAT);
    lfs_file_read(&lfs, &file, &boot_count, sizeof(boot_count));

    // update boot count
    boot_count += 1;
    lfs_file_rewind(&lfs, &file);
    lfs_file_write(&lfs, &file, &boot_count, sizeof(boot_count));

    // remember the storage is not updated until the file is closed successfully
    lfs_file_close(&lfs, &file);

    // write the sinewave into flash
    lfs_file_t sinefile;

    // fullSampleLength = AudioSampleSine440[0] & 0xffffff;
    // sample = (int16_t*)&AudioSampleSine440[1];
    // if the file already exists we can skip opening
    err = lfs_file_open(&lfs, &sinefile, "sine", LFS_O_RDWR | LFS_O_CREAT | LFS_O_TRUNC);
    if(!err)
    {
        lfs_file_write(&lfs, &sinefile, AudioSampleSine440, 4*65);
        // test to show the contents of the file
        if(0==1)
        {
            lfs_file_rewind(GetLFS(), &sinefile);
            uint32_t wavSize;
            lfs_file_read(GetLFS(), &sinefile, &wavSize, 4);
            int fullSampleLength = wavSize & 0xffffff; // low 24 bits are the length of the wav file https://www.pjrc.com/teensy/td_libs_AudioPlayMemory.html
            for(int i=0;i<fullSampleLength;i++)
            {
                int16_t buf;
                lfs_file_seek(GetLFS(), &sinefile, i*2+4, LFS_SEEK_SET);
                lfs_file_read(GetLFS(), &sinefile, &buf, 2);
                printf("%i, %i\n", i, buf);
            }
        }
        lfs_file_close(&lfs, &sinefile);
    }
    else
    {
        printf("failed to open sine file for write: %i", err);
    }



    // release any resources we were using
    // lfs_unmount(&lfs);

    // print the boot count
    printf("boot_count: %d\n", boot_count);
}
