#include <msgpack.h>
#include <stdio.h>

void print(char const* buf,size_t len)
{
    size_t i = 0;
    for(; i < len ; ++i)
        printf("%02x ", 0xff & buf[i]);
    printf("\n");
}

/*
    Client:
    int RequestTensorSend(int socket, Tensor *tensor, int reqcode, float option);
    Server:
    Tensor *RequestTensorRecv(int socket, int *reqcode, float *option);
    ----------------------------------------------------------
    Server:
    int ResponseTensorSend(int socket, Tensor *tensor, int rescode);
    Client:
    Tensor *ResponseTensorRecv(int socket, int *rescode);
*/


int encode(msgpack_sbuffer *sbuf)
{
    msgpack_packer pk;

    // msgpack_sbuffer_init(sbuf);
    msgpack_packer_init(&pk, sbuf, msgpack_sbuffer_write);

    // First object
    msgpack_pack_array(&pk, 3);
    msgpack_pack_int(&pk, 1);
    msgpack_pack_true(&pk);
    msgpack_pack_str(&pk, 7);
    msgpack_pack_str_body(&pk, "example", 7);

    // Second object
    msgpack_pack_str_with_body(&pk, "second object", 13);

    return 0;
}

void decode(msgpack_sbuffer *sbuf)
{
    /* buf is allocated by client. */
    int i = 0;
    size_t off = 0;
    msgpack_unpacked result;
    msgpack_unpack_return ret;
    msgpack_unpacked_init(&result);

    ret = msgpack_unpack_next(&result, sbuf->data, sbuf->size, &off);
    while (ret == MSGPACK_UNPACK_SUCCESS) {
        msgpack_object obj = result.data;

        /* Use obj. */
        printf("Object no %d:\n", ++i);
        msgpack_object_print(stdout, obj);
        printf("\n");

        // o.type == MSGPACK_OBJECT_FLOAT32 ?
        // case MSGPACK_OBJECT_FLOAT32:
        // case MSGPACK_OBJECT_FLOAT64:
        //     fprintf(out, "%f", o.via.f64);
        //     break;

        ret = msgpack_unpack_next(&result, sbuf->data, sbuf->size, &off);
    }
    msgpack_unpacked_destroy(&result);

    if (ret == MSGPACK_UNPACK_CONTINUE) {
        printf("All msgpack_object in the buffer is consumed.\n");
    }
    else if (ret == MSGPACK_UNPACK_PARSE_ERROR) {
        printf("The data in the buf is invalid format.\n");
    }
}

int main(void)
{
    msgpack_sbuffer sbuf;

    msgpack_sbuffer_init(&sbuf);

    encode(&sbuf);
    decode(&sbuf);

    msgpack_sbuffer_destroy(&sbuf);

    return 0;
}
