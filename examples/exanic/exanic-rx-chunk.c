/* Basic RX example */
#include <stdio.h>
#include <exanic/exanic.h>
#include <exanic/fifo_rx.h>
#include <exanic/ioctl.h>


static void print_info(struct rx_chunk_info *info)
{
    printf("rx_chunk_info:\n");
    printf("\ttimestamp=%u\n", info->timestamp);
    printf("\tframe_status=%u\n", info->frame_status);
    printf("\tlength=%u\n", info->length);
    printf("\tmatched_filter=%u\n", info->matched_filter);
    printf("\tgeneration=%u\n", info->generation);
}

int main(void)
{
    char *device = "exanic0";
    int port = 0;

    exanic_t *exanic = exanic_acquire_handle(device);
    if (!exanic)
    {
        fprintf(stderr, "exanic_acquire_handle: %s\n", exanic_get_last_error());
        return 1;
    }

    printf("info_page=%p,hw_time=%lu\n", exanic->info_page, exanic->info_page->hw_time);

    exanic_rx_t *rx = exanic_acquire_rx_buffer(exanic, port, 0);
    if (!rx)
    {
        fprintf(stderr, "exanic_acquire_rx_buffer: %s\n", exanic_get_last_error());
        return 1;
    }

    struct rx_chunk_info info;
    char buf[2048];
    int more;
    //exanic_cycles32_t timestamp;
    ssize_t sz;

    while (1)
    {
        //sz = exanic_receive_frame(rx, buf, sizeof(buf), &timestamp);
        sz = exanic_receive_chunk_ex(rx, buf, &more, &info);
        if (sz > 0)
        {
            print_info(&info);
            break;
        }
    }

    exanic_release_rx_buffer(rx);
    exanic_release_handle(exanic);
    return 0;
}
