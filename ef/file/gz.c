#include <ef/file.h>
#include <ef/memory.h>
#include <ef/str.h>
#include <ef/mth.h>
#include <ef/err.h>

#include <zlib.h>

#define ZGZ_CHUNK 0x4000
#define ZGZ_WB (15 | 32)

err_t fgz_extract(int fdout, int fdin){
    int ret;
    unsigned have;
    z_stream strm;
    unsigned char in[ZGZ_CHUNK];
    unsigned char out[ZGZ_CHUNK];

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit2(&strm, ZGZ_WB);
    if (ret != Z_OK){
        return ret;
	}

    /* decompress until deflate stream ends or end of file */
    do {
		ssize_t rd = fd_read(fdin, in, ZGZ_CHUNK);
		if( rd < 0 ){
            (void)inflateEnd(&strm);
            return Z_ERRNO;
		}
        strm.avail_in = rd;
        if (strm.avail_in == 0){
            break;
		}
        strm.next_in = in;

        /* run inflate() on input until output buffer not full */
        do {
            strm.avail_out = ZGZ_CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            iassert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            switch (ret) {
	            case Z_NEED_DICT:
		            ret = Z_DATA_ERROR;     /* and fall through */
					(void)inflateEnd(&strm);
				return ret;
			    case Z_DATA_ERROR:
				case Z_MEM_ERROR:
					(void)inflateEnd(&strm);
                return ret;
            }
            have = ZGZ_CHUNK - strm.avail_out;
			ssize_t wr;
            if( (wr=fd_write(fdout, out, have)) != have || wr < 0) {
                (void)inflateEnd(&strm);
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);
        /* done when inflate() says it's done */
    } while (ret != Z_STREAM_END);

    /* clean up and return */
    (void)inflateEnd(&strm);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}


