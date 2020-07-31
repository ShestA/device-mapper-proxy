#include <linux/module.h>
#include <linux/init.h>
#include <linux/blkdev.h>
#include <linux/bio.h>
#include <linux/dax.h>
#include <linux/slab.h>
#include <linux/device-mapper.h>

#define DM_MSG_PREFIX "proxy"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Aleksei Shestakov <6akov98@gmail.com>");
MODULE_DESCRIPTION(DM_NAME" r/w statistics module");
MODULE_VERSION("1.0.0");

//Структура статистики запросов
static struct iostat {
    unsigned long rd_rq_cnt;   //Количество запросов на запись
    unsigned long wr_rq_cnt;   //Количество запросов на чтение
    unsigned long rq_cnt;      //Общее кол-во запросов
    unsigned long rd_sz_blk;   //Размер записанных блоков
    unsigned long wr_sz_blk;   //Размер прочитанных блоков
    unsigned long avg_rd_blk;  //Средний размер блока на запись
    unsigned long avg_wr_blk;  //Средний размер блока на чтение
    unsigned long avg_sz_blk;  //Средний размер блока
} dmp_stat;

/*
 * Proxy: сбор информации.
 */
struct proxy_c {
    struct dm_dev *dev;
    sector_t start;
};

/*
 * Создание proxy: <dev_path> <offset>
 */
static int proxy_ctr(struct dm_target *ti, unsigned int argc, char **argv)
{
    struct proxy_c *pc;
    unsigned long long tmp;
    char dummy;
    int ret;

    if (argc != 2) {
        ti->error = "Invalid argument count";
        return -EINVAL;
    }

    pc = kmalloc(sizeof(*pc), GFP_KERNEL);
    if (pc == NULL) {
        ti->error = "Cannot allocate proxy context";
        return -ENOMEM;
    }

    ret = -EINVAL;
    if (sscanf(argv[1], "%llu%c", &tmp, &dummy) != 1 || tmp != (sector_t)tmp) {
        ti->error = "Invalid device sector";
        goto bad;
    }
    pc->start = tmp;

    ret = dm_get_device(ti, argv[0], dm_table_get_mode(ti->table), &pc->dev);
    if (ret) {
        ti->error = "Device lookup failed";
        goto bad;
    }
    ti->num_flush_bios = 1;
    ti->num_discard_bios = 1;
    ti->num_secure_erase_bios = 1;
    ti->num_write_same_bios = 1;
    ti->num_write_zeroes_bios = 1;
    ti->private = pc;
    return 0;

  bad:
    kfree(pc);
    return ret;
}

static void proxy_dtr(struct dm_target *ti)
{
    struct proxy_c *pc = (struct proxy_c *) ti->private;

    dm_put_device(ti, pc->dev);
    kfree(pc);
}

static sector_t proxy_map_sector(struct dm_target *ti, sector_t bi_sector)
{
    struct proxy_c *pc = ti->private;

    return pc->start + dm_target_offset(ti, bi_sector);
}

static void proxy_map_bio(struct dm_target *ti, struct bio *bio)
{
    struct proxy_c *pc = ti->private;

    bio_set_dev(bio, pc->dev->bdev);
    if (bio_sectors(bio) || bio_op(bio) == REQ_OP_ZONE_RESET)
        bio->bi_iter.bi_sector =
            proxy_map_sector(ti, bio->bi_iter.bi_sector);
}

static int proxy_map(struct dm_target *ti, struct bio *bio)
{
    switch (bio_op(bio)) {
        case REQ_OP_READ:
            dmp_stat.rd_rq_cnt++;
            dmp_stat.rd_sz_blk += bio->bi_io_vec->bv_len;
            break;
        case REQ_OP_WRITE:
            dmp_stat.wr_rq_cnt++;
            dmp_stat.wr_sz_blk += bio->bi_io_vec->bv_len;
            break;
        default:
            break;
    }
    dmp_stat.rq_cnt++;
    proxy_map_bio(ti, bio);

    return DM_MAPIO_REMAPPED;
}

static struct target_type proxy_target = {
    .name   = "proxy",
    .version = {1, 0, 0},
    .features = DM_TARGET_PASSES_INTEGRITY,
    .module = THIS_MODULE,
    .ctr    = proxy_ctr,
    .dtr    = proxy_dtr,
    .map    = proxy_map
};

/*
Небезопасная функция копирования символов
*/
static unsigned short unsafe_add_string(char* dest, char* src, unsigned short offset){
    char* start = dest + offset;
    unsigned short count = strlen(src) + 1;
    int itr;
    for (itr = 0; itr < count; itr++)
        *(start + itr) = src[itr];
    return count - 1;
}

static char *temp;
static char *text;
/*
Функция передачи данных пользователю через kobject
*/
static ssize_t show_stat(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int prsz = 0;

    if (dmp_stat.rd_rq_cnt != 0)    dmp_stat.avg_rd_blk = dmp_stat.rd_sz_blk / dmp_stat.rd_rq_cnt;
    else                            dmp_stat.avg_rd_blk = 0;

    if (dmp_stat.wr_rq_cnt != 0)    dmp_stat.avg_wr_blk = dmp_stat.wr_sz_blk / dmp_stat.wr_rq_cnt;
    else                            dmp_stat.avg_wr_blk = 0;

    dmp_stat.avg_sz_blk = (dmp_stat.avg_wr_blk + dmp_stat.avg_rd_blk) / 2;
    prsz += unsafe_add_string(text, "read:\n", prsz);
    prsz += unsafe_add_string(text, "\treqs: ", prsz);
    sprintf(temp, "%lu\n", dmp_stat.rd_rq_cnt);
    prsz += unsafe_add_string(text, temp, prsz);
    prsz += unsafe_add_string(text, "\tavg size: ", prsz);
    sprintf(temp, "%lu\n", dmp_stat.avg_rd_blk );
    prsz += unsafe_add_string(text, temp, prsz);

    prsz += unsafe_add_string(text, "write:\n", prsz);
    prsz += unsafe_add_string(text, "\treqs: ", prsz);
    sprintf(temp, "%lu\n", dmp_stat.wr_rq_cnt);
    prsz += unsafe_add_string(text, temp, prsz);
    prsz += unsafe_add_string(text, "\tavg size: ", prsz);
    sprintf(temp, "%lu\n", dmp_stat.avg_wr_blk);
    prsz += unsafe_add_string(text, temp, prsz);

    prsz += unsafe_add_string(text, "total:\n", prsz);
    prsz += unsafe_add_string(text, "\treqs: ", prsz);
    sprintf(temp, "%lu\n", dmp_stat.rq_cnt);
    prsz += unsafe_add_string(text, temp, prsz);
    prsz += unsafe_add_string(text, "\tavg size: ", prsz);
    sprintf(temp, "%lu\n", dmp_stat.avg_sz_blk);
    prsz += unsafe_add_string(text, temp, prsz);

    return sprintf(buf, text);
}
/*
Функция-заглушка получения данных от пользователя
*/
static ssize_t store_stat(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    return count;
}

//sysfs
static struct kobj_attribute stat_attribute =__ATTR(volumes, 0660, show_stat, store_stat);
struct kobject * subdir;

int __init dm_proxy_init(void)
{
    int r = dm_register_target(&proxy_target);

    if (r < 0)
        DMERR("register failed %d", r);

    temp = kmalloc(10, GFP_KERNEL);
    text = kmalloc(128,GFP_KERNEL);
    subdir = kobject_create_and_add("stat", &(THIS_MODULE->mkobj.kobj));
    r = sysfs_create_file(subdir , &stat_attribute.attr);
    
    if (r < 0)
        DMERR("register failed %d", r);

    return r;
}

void __exit dm_proxy_exit(void)
{
    dm_unregister_target(&proxy_target);
    sysfs_remove_file(subdir , &stat_attribute.attr);
    kobject_put(subdir);
    kfree(temp);
    kfree(text);
}

module_init(dm_proxy_init)
module_exit(dm_proxy_exit)