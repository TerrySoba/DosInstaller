#include "exomizer_compression.h"
#include "exo_helper.h"

std::vector<unsigned char> exomizerCompress(const std::vector<unsigned char>& data)
{
    struct crunch_options options = CRUNCH_OPTIONS_DEFAULT;
    options.direction_forward = 1;
    struct crunch_info info = STATIC_CRUNCH_INFO_INIT;
    struct buf inbuf;
    struct buf outbuf;

    buf_init(&inbuf);
    buf_init(&outbuf);

    inbuf.data = (unsigned char*)data.data();
    inbuf.size = data.size();
    inbuf.capacity = data.size();

    crunch(&inbuf, 0, NULL, &outbuf, &options, &info);
    print_crunch_info(LOG_NORMAL, &info);

    std::vector<unsigned char> output((unsigned char*)outbuf.data, (unsigned char*)outbuf.data + outbuf.size);

    buf_free(&outbuf);
    return output;
}
