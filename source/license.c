
/************************************************************************************
***
*** Copyright 2024 Dell Du(18588220928@163.com), All Rights Reserved.
***
*** File Author: Dell, Wed 15 May 2024 11:50:15 PM CST
***
************************************************************************************/

#include "license.h"

#include <sys/sysinfo.h>  // for int get_nprocs(void)
#include <cpuid.h> // for __get_cpuid_max, __get_cpuid
// #include <cuda_runtime_api.h> // for GPU

// Network interface
#include <ifaddrs.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <netinet/ether.h>

// openssl
#include <openssl/bn.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>

#define BEGIN_SIGNED_MESSAGE "-----BEGIN SIGNED MESSAGE-----"
#define END_SIGNED_MESSAGE "-----END SIGNED MESSAGE-----"
#define BEGIN_PUBLIC_KEY "-----BEGIN PUBLIC KEY-----"
#define END_PUBLIC_KEY "-----END PUBLIC KEY-----"
#define BEGIN_SIGNATURE "-----BEGIN SIGNATURE-----"
#define END_SIGNATURE "-----END SIGNATURE-----"


static void all_trim(char *str);
static RSA* load_public_key(char *pkey);
static int base64_decode(char *s, unsigned char* out);
static int rsa_sign_verify(char *publickey, char *message, char *signature);
static int load_hardware(char *message, Hardware *h);
// ------------------------------------------------------------------------------------------------------------------

static void all_trim(char *str)
{
    char *q, *p, *s;
    q = p = s = str;
    while (*p == ' ')
        p++;
    while (*p) {
        /* Note the last non-whitespace index */
        if (! isspace(*p))
            s = q;
        *q++ = *p++;
    }
    *(s + 1) = '\0';
}

int get_hardware(Hardware *h)
{
    int size;
    char* buffer;

    if (h == NULL)
        return RET_ERROR;

    memset(h, 0, sizeof(Hardware));

    // Running machine is docker ?
    if (file_exist("/proc/self/cgroup")) {
        buffer = file_load("/proc/self/cgroup", &size);
        if (buffer != NULL) {
            if (strstr(buffer, "docker") != NULL || strstr(buffer, "lxc") != NULL) 
                h->is_docker = 1;
            free(buffer);
        }
    }
    // Check more for host ...
    if (h->is_docker == 0 && file_exist("/var/run/systemd/container")) {
        buffer = file_load("/var/run/systemd/container", &size);
        if (buffer != NULL) {
            if (strstr(buffer, "docker") != NULL || strstr(buffer, "lxc") != NULL) 
                h->is_docker = 1;
            free(buffer);
        }
    }

    // CPU
    h->cpu_count = get_nprocs(); // 24 threads
    if (__get_cpuid_max(0x80000004, NULL)) {
        uint32_t brand[0x10];
        memset(brand, 0, sizeof(brand));
        __get_cpuid(0x80000002, brand + 0x0, brand + 0x1, brand + 0x2, brand + 0x3);
        __get_cpuid(0x80000003, brand + 0x4, brand + 0x5, brand + 0x6, brand + 0x7);
        __get_cpuid(0x80000004, brand + 0x8, brand + 0x9, brand + 0xa, brand + 0xb);

        all_trim((char *)brand);
        memcpy(h->cpu_name, brand, sizeof(brand));
    }

    // Board
    if (file_exist("/sys/class/dmi/id/board_name")) {
        buffer = file_load("/sys/class/dmi/id/board_name", &size);
    } else {
        buffer = file_load("/sys/class/dmi/id/sys_vendor", &size);
    }
    if (buffer != NULL) {
        size = MIN((int)sizeof(h->board_name), size);

        all_trim(buffer);
        memcpy(h->board_name, buffer, size);
        free(buffer);
    }

    // GPU 0 
    // cudaGetDeviceCount(&(h->gpu_count));
    // if (h->gpu_count >= 1) {
    //     int dev = 0;  // gpu -- 0
    //     cudaSetDevice(dev);
    //     cudaDriverGetVersion(&(h->cuda_version));

    //     struct cudaDeviceProp prop;
    //     cudaGetDeviceProperties(&prop, dev);

    //     all_trim((char *)prop.name);
    //     memcpy(h->gpu_name, prop.name, sizeof(h->gpu_name));
    //     h->gpu_memory_size = (int)(prop.totalGlobalMem / 1048576.0f);
    // }

    // $ nvidia-smi --query | grep Version
    //     Driver Version                            : 515.86.01
    //     CUDA Version                              : 11.7
    // $ nvidia-smi --query | grep Product
    //     Product Name                          : NVIDIA GeForce RTX 2080 Ti
    {
        size_t size;
        char buf[1024], *p, *e;
        FILE *fp;

        fp = popen("nvidia-smi --query | grep 'Product Name'", "r");
        if (fp != NULL) {
            while((size = fread(buf, sizeof(char), sizeof(buf), fp)) > 0) {
                p = strstr(buf, ": ");
                if (p) {
                    p += 2; // Skip ": "
                    e = strstr(p, "\n");
                    if (e) *e = '\0';
                }
                all_trim(p);
                if (h->gpu_count == 0) {
                    snprintf(h->gpu_name, sizeof(h->gpu_name), "%s", p);
                }
                h->gpu_count++;
            }
            fclose(fp);
        }
        
        fp = popen("nvidia-smi --query | grep 'CUDA Version'", "r");
        if (fp != NULL) {
            size = fread(buf, sizeof(char), sizeof(buf), fp);
            if (size > 0) {
                p = strstr(buf, ": ");
                if (p) 
                    p += 2; // Skip ": "
                e = strstr(p, "\n");
                if (e) *e = '\0';
                all_trim(p);
                snprintf(h->cuda_version, sizeof(h->cuda_version), "%s", p);
            }
            fclose(fp);
        }
    }

    // MAC
    struct ifaddrs *if_p, *if_buffer = NULL;
    if (getifaddrs(&if_buffer) < 0 || if_buffer == NULL)
        return RET_ERROR;

    struct ifreq ifr;
    for (if_p = if_buffer; if_p; if_p = if_p->ifa_next) {
        // find first ethernet interface ?
        if (if_p->ifa_addr && (AF_PACKET == if_p->ifa_addr->sa_family) && strcmp(if_p->ifa_name, "lo") != 0) {
            int sfd = socket(AF_INET, SOCK_STREAM, 0);
            if (sfd < 0)
                continue;

            strcpy(ifr.ifr_name, if_p->ifa_name);
            ioctl(sfd, SIOCGIFHWADDR, &ifr);
            close(sfd);

            // snprintf(h->mac_address, sizeof(h->mac_address), "%s", 
            //     ether_ntoa((struct ether_addr *) ifr.ifr_hwaddr.sa_data));
            // struct ether_addr {
            //     uint8_t ether_addr_octet[6];
            // }
            uint8_t *p = (uint8_t *)ifr.ifr_hwaddr.sa_data;
            snprintf(h->mac_address, sizeof(h->mac_address), "%02x:%02x:%02x:%02x:%02x:%02x", 
                p[0], p[1], p[2], p[3], p[4], p[5]);
            break;
        }
    }
    freeifaddrs(if_buffer);

    // Time
    h->date = time(NULL); // seconds from linux epoch
    h->expire = 365; // one year

    return RET_OK;
}


void dump_hardware(Hardware *h)
{
    printf("Machine: %s\n", h->is_docker?"Docker" : "Host");

    // CPU 0
    printf("CPU: %s, %d processors\n", h->cpu_name, h->cpu_count);
    printf("Board: %s\n", h->board_name);

    // GPU 0
    if (h->gpu_count > 0)
        printf("GPU: %s, CUDA version = %s, %d devices\n", h->gpu_name, h->cuda_version, h->gpu_count);

    // Ethernet 0
    printf("MAC: %s\n", h->mac_address);

    // Time
    char buffer[128];
    struct tm *t_tm;
    t_tm = localtime(&(h->date));
    if (t_tm != NULL && strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", t_tm)  > 0) {
        printf("Date: %s\n", buffer);
    }
    printf("Expire: %d days\n", h->expire);
}

// -----BEGIN SIGNED MESSAGE-----
// CPU: AMD Ryzen 9 3900X 12-Core Processor, 24 processors
// Board: X570-A PRO (MS-7C37)
// GPU: NVIDIA GeForce RTX 2080 Ti, driver version = 11.070, memory size = 10985 M, 1 devices
// MAC: 00:d8:61:79:85:57
// Date: 2024-05-01 01:02:03
// Expire: 365 days
// -----END SIGNED MESSAGE-----

// -----BEGIN PUBLIC KEY-----
// MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAv+6HyN8t22EIbsLbWL57
// +3dLDbU077ei0kC57iU6WHAePbavOqSCCYG6xxQXX1E6Ti0fJ1/Se3YC2QlL9Ksv
// P6sfVnoHkYUeXFye+trXrus/b40GcWbVtIK9/BuVZiCJUJzQNQ4eQVOTjs/tv0aZ
// /MtyhQ2jx/+UHCo49c3oDWCas1DaxkKoVf5P2saI7QYSk1gPuuBFd0lXXhcwsWMs
// Q66wLtZzUcmtL8gD/SDk+jJjl6bprAJYCILFqrVVsS6lxDjZleD9PRtJtjRRGFiG
// LMLj5vtUniF14O1EZbFv6ZnBXd3C0FhRHnzm5TLMGl4b38jiYb7uLH5qi+qKMJww
// fQIDAQAB
// -----END PUBLIC KEY-----

// -----BEGIN SIGNATURE-----
// HNNoNcYeYQifZZEF2PlP2LmwmFjlEWcom3dGZATDgP7UOupgIR4XdYjuAMDtgrwS5OC7HLGwVWrV
// 5940/sRfhOEm/7ZbCMttyrXYZZYh9CuRgKocJffoxlCA9sTk++3Gl+YqvyOB+dvJyrAIC3+DYgRD
// w4w8YjRsMtGXMnMAnbg5f2ZbzhHY/PK4K3hGx+/t1L6EM4tktgImCQNqNpD+y+Gau9Mv1nCE/dHD
// w3aKJ7eHvpsWcrLiXazidMlByY40RL6aS4EZ9dlNOm+Ner5mw5MHtAxNXNyT8J+hVZMFHaKdf2UK
// 0Jh/lBnhSgzR4E3Jgf2jtwZ0WlGK+W94J1oqag==
// -----END SIGNATURE-----

// RSA *PEM_read_bio_RSA_PUBKEY(BIO *bp, RSA **x, pem_password_cb *cb, void *u);
// RSA *PEM_read_RSAPublicKey(FILE *fp, RSA **x, pem_password_cb *cb, void *u);
static RSA* load_public_key(char *pkey)
{
    BIO *bio;
    RSA* public_key = NULL;

    bio = BIO_new(BIO_s_mem());
    if (bio == NULL)
        return NULL;
    BIO_puts(bio, pkey);

    public_key = PEM_read_bio_RSA_PUBKEY(bio, NULL, NULL, NULL); // PKCS#1 -----BEGIN PUBLIC KEY-----
    // PEM_read_bio_RSAPublicKey(bio, NULL, NULL, NULL); // PKCS#8 -----BEGIN RSA PUBLIC KEY-----
    BIO_free(bio);

    return public_key;
}

static int base64_decode(char *s, unsigned char* out)
{
    int outlen;

    BIO* b64 = BIO_new(BIO_f_base64());
    BIO* bmem = BIO_new_mem_buf(s, strlen(s));
    if (b64 == NULL || bmem == NULL) {
        syslog_error("Allocate memory");
        return -1;
    }
    BIO* bio_sign = BIO_push(b64, bmem);
    // BIO_set_flags(bio_sign, BIO_FLAGS_BASE64_NO_NL); // Do not use newlines to flush buffer
    outlen = BIO_read(bio_sign, out, strlen(s));
    BIO_free_all(bio_sign);

    return outlen;
}

static int rsa_sign_verify(char *publickey, char *message, char *signature)
{
    int ret = RET_ERROR;
    RSA *rsa_key = NULL;
    EVP_PKEY *pubkey = NULL;
    unsigned char *base64_decode_buff = NULL;
    int base64_decode_size = 0;

    // Decode signature ...
    base64_decode_buff = (unsigned char *)malloc(2 * strlen(signature));
    if (base64_decode_buff == NULL) {
        syslog_error("Allocate memory.");
        return RET_ERROR;
    }
    base64_decode_size = base64_decode(signature, base64_decode_buff);
    if (base64_decode_size <= 0)
        goto failure;

    // for (int i = 0; i < base64_decode_size; i++) {
    //     printf("%02x ", base64_decode_buff[i]);
    // }
    // printf("\n");
    // printf("------------------------------------------\n");

    // Loading public key
    rsa_key = load_public_key(publickey);
    if (rsa_key == NULL)
        goto failure;
    pubkey  = EVP_PKEY_new();
    if (pubkey == NULL)
        goto failure;
    EVP_PKEY_assign_RSA(pubkey, rsa_key);

    // Digest message and verify ...
    EVP_MD_CTX *mdctx = EVP_MD_CTX_create();
    if (mdctx != NULL) {
        ret = (EVP_DigestVerifyInit(mdctx, NULL, EVP_sha512(), NULL, pubkey) == 1)? RET_OK:RET_ERROR;
        if (ret == RET_OK) {
            EVP_DigestVerifyUpdate(mdctx, message, strlen(message));
            // Verify ...
            ret = (EVP_DigestVerifyFinal(mdctx, base64_decode_buff, base64_decode_size) == 1)?RET_OK:RET_ERROR;
        }
        EVP_MD_CTX_destroy(mdctx);
    }

failure:
    if (rsa_key)
        RSA_free(rsa_key);
    // if (pubkey)
    //     EVP_PKEY_free(pubkey); // ==> segment fault !!!
    if (base64_decode_buff)
        free(base64_decode_buff);

    return ret;
}

static int load_hardware(char *message, Hardware *h)
{
    char *p, *e;
    if (h == NULL)
        return RET_ERROR;

    // Suppose messge sequence as:
    // 1) Machine: ...
    // 2) CPU: ...
    // 3) Board: ...
    // 4) GPU ...
    // 5) MAC ...
    // 6) Time ...

    memset(h, 0, sizeof(Hardware));
    p = strstr(message, "Machine: Docker");
    if (p)
        h->is_docker = 1;

    // CPU
    p = strstr(message, "CPU:");
    if (p) {
        e = strstr(p, "\n");
        if (e) {
            *e = '\0';
            message = e + 1;
        }
        snprintf(h->cpu_name, sizeof(h->cpu_name), "%s", p + 5); // Skip "CPU:"
    }
    // Board
    p = strstr(message, "Board: ");
    if (p) {
        e = strstr(p, "\n");
        if (e) {
            *e = '\0';
            message = e + 1;
        }
        snprintf(h->board_name, sizeof(h->board_name), "%s", p + 7); // Skip "Board: "
    }
    // GPU 
    p = strstr(message, "GPU: ");
    if (p) {
        e = strstr(p, "\n");
        if (e) {
            *e = '\0';
            message = e + 1;
        }
        snprintf(h->gpu_name, sizeof(h->gpu_name), "%s", p + 5); // Skip "GPU: "
    }
    // MAC
    p = strstr(message, "MAC: ");
    if (p) {
        e = strstr(p, "\n");
        if (e) {
            *e = '\0';
            message = e + 1;
        }
        snprintf(h->mac_address, sizeof(h->mac_address), "%s", p + 5); // Skip "MAC: "
    }

    // Time
    struct tm t_tm;
    p = strstr(message, "Date: ");
    if (p) {
        e = strstr(p, "\n");
        if (e) {
            *e = '\0';
            message = e + 1;
        }
        p += 6; // Skip "Date: "
        strptime((const char *)p, "%Y-%m-%d %H:%M:%S", &t_tm);
        // t_tm.tm_year += 1900; t_tm.tm_mon += 1;
        t_tm.tm_isdst = 0; // t_tm.tm_isdst = 4096 ==> Fixed bug for mktime == -1
        h->date = mktime(&t_tm);
    }
    p = strstr(message, "Expire: ");
    if (p) {
        e = strstr(p, " days"); // 0 or 365 days
        if (e) {
            *e = '\0';
            message = e + 1;
        }
        h->expire = atoi(p + 8); // Skip "Expire: "
    }

    return RET_OK;
}

int check_license(char *fname)
{
    int ret = RET_ERROR;
    int buff_size;
    char *message, *publickey, *signature, *p;
    char *base64_data = NULL;
    char *buff_data = NULL;
    Hardware running_hw, sign_hw;

    base64_data = file_load(fname, &buff_size);
    if (base64_data == NULL) {
        syslog_error("Read license '%s'", fname);
        return RET_ERROR;
    }

    buff_data = (char *)malloc(2 * buff_size);
    if (buff_data == NULL) {
        syslog_error("Allocate memeory");
        goto failure;
    }
    base64_decode(base64_data, (unsigned char *)buff_data); // cat hw.lic | base64 -d

    message = strstr(buff_data, BEGIN_SIGNED_MESSAGE);
    publickey = strstr(buff_data, BEGIN_PUBLIC_KEY);
    signature = strstr(buff_data, BEGIN_SIGNATURE);

    if (message) {
        // Remove begin/end flags
        message += sizeof(BEGIN_SIGNED_MESSAGE);
        p = strstr(message, END_SIGNED_MESSAGE);
        if (p != NULL) {
            *p = '\0';
        } else {
            message = NULL;
        }
    }
    if (! message) {
        syslog_error("NO SIGNED MESSAGE in '%s'", fname);
        goto failure;
    }

    if (publickey) {
        // keep begin/end flags ...
        p = strstr(publickey, END_PUBLIC_KEY);
        if (p != NULL) {
            p += sizeof(END_PUBLIC_KEY);
            * p = '\0';
        } else {
            publickey = NULL;
        }
    }
    if (! publickey) {
        syslog_error("NO PUBLIC KEY in '%s'", fname);
        goto failure;
    }

    if (signature) {
        // Remove begin/end flags
        signature += sizeof(BEGIN_SIGNATURE);
        p = strstr(signature, END_SIGNATURE);
        if (p != NULL) {
            *p = '\0';
        } else {
            signature = NULL;
        }
    }
    if (! signature) {
        syslog_error("NO SIGNATURE in '%s'", fname);
        goto failure;
    }

    ret = rsa_sign_verify(publickey, message, signature);
    if (ret != RET_OK)
        goto failure;

    // Checking hardware ...
    get_hardware(&running_hw);
    load_hardware(message, &sign_hw);
    // dump_hardware(&running_hw);
    // dump_hardware(&sign_hw);

    if (strncmp(running_hw.cpu_name, sign_hw.cpu_name, strlen(running_hw.cpu_name)) != 0) {
        syslog_error("CPU");
        goto failure;
    }

    if (strncmp(running_hw.board_name, sign_hw.board_name, strlen(running_hw.board_name)) != 0) {
        syslog_error("Board");
        goto failure;
    }

    // ---------------------------------------------------------------------------------------------------
    // Docker 容器中的 MAC 地址是由 Docker 守护进程在启动容器时动态生成的。当 Docker 启动一个容器时，
    // 它会为该容器创建一个新的虚拟网络接口，并分配一个唯一的 MAC 地址。这个 MAC 地址遵循以下规则：
    //     MAC地址格式：Docker 容器的 MAC 地址遵循标准以太网地址格式，即 6 个字节（48 位）。
    //     前缀：Docker 容器的 MAC 地址前缀通常是 02:42:ac:11:00:00。这个前缀是 Docker 容器的默认 MAC 地址前缀。
    //     随机生成：在前缀之后，Docker 会随机生成 3 个字节（24 位），以确保每个容器的 MAC 地址都是唯一的。
    //     多播地址：Docker 容器的 MAC 地址也可以用于多播地址的计算。例如，如果容器的 MAC 地址是 02:42:ac:11:00:01，
    //         那么对应的多播地址将是 01:00:5e:00:00:01。
    //     容器重启：如果容器被重启，它将保留相同的 MAC 地址。但是，如果容器被删除并重新创建，它将获得一个新的 MAC 地址。
    //     自定义：虽然 Docker 默认会生成 MAC 地址，但用户也可以通过 Docker 的网络插件或自定义网络驱动来指定特定的 MAC 地址。
    //     虚拟化技术：Docker 容器的 MAC 地址与宿主机的 MAC 地址是独立的，这是通过虚拟化技术实现的，确保了容器的网络隔离。
    // Docker 容器的 MAC 地址生成机制确保了每个容器在网络层面上具有唯一性，并且可以与其他容器或网络设备进行通信。
    // 这种机制是 Docker 网络隔离策略的一部分，有助于容器化环境中的网络安全和隔离。
    // ---------------------------------------------------------------------------------------------------
    // ==> Skip docker, only check host(is_docker == 0) mac address ...
    if (running_hw.is_docker == 0 && strncmp(running_hw.mac_address, sign_hw.mac_address, strlen(running_hw.mac_address)) != 0) {
        syslog_error("MAC");
        goto failure;
    }

    // Checking date ...
    time_t now = time(NULL);
    if (now < sign_hw.date || now > sign_hw.date + (sign_hw.expire + 1)*24*60*60) { // one day threshold
        syslog_error("License Expired");
        goto failure;
    }

failure:
    if (base64_data)
        free(base64_data);

    if (buff_data)
        free(buff_data);
    return ret;
}