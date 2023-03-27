#include "tftp.h"

// 요청 (WRQ, RRQ) 패킷 생성 함수 
int req_packet(int opcode, char *filename, char *mode, char buf[])
{
    int len;
    // int 형을 char 형에 할당 시 하위 1 바이트만 저장된다
    // 여기서 opcode 는 1, 2 만 전달되므로 하위 1 바이트를 저장해도 상관 없다 
    len = sprintf(buf, "%c%c%s%c%s%c", 0x00, opcode, filename, 0x00, mode, 0x00);
    printf("\nCreating Request packet...\n");

    if (!len)
    {
        printf("\nError in creating request packet!\n");
        exit (ERROR);
    }

    return len;
}

// 데이터 송신에 대한 응답 (ACK) 패킷 생성 함수
int ack_packet(int block, char buf[])
{
    int len;

    len = sprintf(buf, "%c%c%c%c", 0x00, ACK, 0x00, 0x00);
    buf[2] = (block & 0xFF00) >> 8;
    buf[3] = (block & 0x00FF);
    printf("\nCreating acknowledge packet...\n");

    if (!len)
    {
        printf("\nError in creating acknowledge packet!\n");
        exit(ERROR);
    }
    return len;
}

// 송수신 에러일 시 에러 (ERR) 패킷 생성 함수
int err_packet(int err_code, char *err_msg, char buf[])
{
    int len;
    memset (buf, 0, sizeof (&buf));

    len = sprintf(buf, "%c%c%c%c%s%c", 0x00, ERR, 0x00, err_code, err_msg, 0x00);
    printf("\nCreating error packet...\n");

    if (!len)
    {
        printf("\nError in creating error packet!\n");
        exit(ERROR);
    }
    return len;
}

// 사용법을 출력하는 함수
void usage(void)
{
    printf("\n========================================================\n");
    printf("Execute '.out' file where file exists.\n");
    printf("\n\n");
    printf("Execute Client : ");
    printf("./client [hostname] [option]\n");
    printf("Execute Server : ");
    printf("./server\n");
    printf("\n\n");
    printf("Client options below.\n");
    printf("-s [filename] : send file to server from client\n");
    printf("-g [filename] : get file from server to client\n");
    printf("-o -n : set octet mode / set netascii mode\n");
    printf("\n");
    printf("=========================================================\n");
}