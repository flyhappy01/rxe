#include <stdint.h>
#include <stdio.h>
#include <string.h>
//#include "rxe_hdr.h"
#include <linux/types.h>
#include <asm-generic/types.h>
 #include <arpa/inet.h>
typedef __s8  s8;
typedef __u8  u8;
typedef __s16 s16;
typedef __u16 u16;
typedef __s32 s32;
typedef __u32 u32;
typedef __s64 s64;
typedef __u64 u64;

#define convert(data, type, offset)   (type *)((void*)data + offset)

/******************************************************************************
 * Base Transport Header
 ******************************************************************************/
typedef struct rxe_bth {
	u8			opcode;
	u8			flags;
	__be16			pkey;
	__be32			qpn;
	__be32			apsn;
} rxe_bth_t;

typedef struct ib_deth {
    uint32_t qkey;          // Queue Key，用于权限校验和访问控制
    uint32_t reserved:8;      // 保留字段（可能包含源QP信息或子网前缀）
    uint32_t source_queue_pair: 24;
} ib_deth_t;

typedef struct ib_mad_hdr {
	u8	base_version;
	u8	mgmt_class;
	u8	class_version;
	u8	method;
	__be16	status;
	__be16	class_specific;
	__be64	tid;
	__be16	attr_id;
	__be16	resv;
	__be32	attr_mod;
} ib_mad_hdr_t;

void cm_reponse(const uint8_t* data, int len)
{
    uint8_t buf[1500];
    rxe_bth_t *bth = (rxe_bth_t*)buf;
    memset(bth, 0, sizeof(*bth));
    bth->opcode =  0x64;
    bth->pkey = htons(0xffff);
    bth->qpn = htonl(1);
    bth->apsn = htonl(4) >> 8;
    ((uint8_t*)(&bth->apsn))[0] = 1;

    ib_deth_t *deth = (ib_deth_t*)(bth+1);
    deth->qkey = htonl(0x80010000);
    deth->source_queue_pair = htonl(1) >> 8;
    printf("deth->qkey:%x\n" ,deth->qkey);

    ib_mad_hdr_t *req_mad, *ans_mad;
    // = (ib_mad_hdr_t *)(( ((ib_deth_t*)(rxe_bth_t*)data + 1)+1));
    req_mad = convert(data, ib_mad_hdr_t, sizeof(ib_deth_t));
    ans_mad = convert(buf, ib_mad_hdr_t, sizeof(rxe_bth_t) + sizeof(ib_deth_t));
    printf("base ver %u %u\n", req_mad->base_version, ans_mad->base_version);

    memcpy(ans_mad, req_mad, sizeof(ib_mad_hdr_t));
    ans_mad.attr_id = ntohs(0x13);

}

// 解析DETH结构体的函数
void parse_deth(const uint8_t* data) {
    const struct ib_deth * deth = (const struct ib_deth*)data;

    printf("DETH Header:\n");
    printf("  qkey: %x\n", ntohl(deth->qkey));
    printf("  source queue pair: 0x%04x\n", ntohl(deth->source_queue_pair<<8));

    cm_reponse(data, 1500);
}

// 解析BTH头的函数
void parse_bth(const uint8_t* data, int len) {
    // 将字节数组映射到BTH结构体
    const rxe_bth_t* bth = (const rxe_bth_t*)data;

    // 打印BTH头的各个字段
    printf("BTH Header: %lu\n", sizeof(rxe_bth_t));
    printf("  Opcode: 0x%02x\n", bth->opcode);
    printf("  pkey: 0x%02x\n", ntohs(bth->pkey));
    printf("  qpn: 0x%04x\n", ntohl(bth->qpn));
    printf("  psn: 0x%04x\n", ntohl(bth->apsn));
    
    if (bth->qpn == htonl(0x0001)) {
        parse_deth((const uint8_t* )(bth + 1));
    }
}

int test_deth_main() {
    // 示例DETH数据（8字节）
    uint8_t deth_data[8] = {
        0x01, 0x23, 0x45, 0x67,  // Source QP
        0x00,                     // Reserved
        0x80,                     // Ack Request (bit 7) and Reserved
        0x00, 0x12                // PSN
    };

    // 解析DETH头
    parse_deth(deth_data);

    return 0;
}

int test_main() {
    // 示例BTH头数据（12字节）
    uint8_t bth_data[12] = {
        0x01, 0x23, 0x45, 0x67,  // 第一个字节包含Opcode等字段
        0x89, 0xAB,              // Partition Key
        0xCD, 0xEF, 0x01, 0x23,  // Destination QP
        0x45, 0x67               // Acknowledge Request和Packet Sequence Number
    };

    // 解析BTH头
    parse_bth(bth_data, 12);

    return 0;
}