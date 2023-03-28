#include "../tftp.h"

void server_send(char *pFilename, struct sockaddr_in client, char *pMode, int tid)
{
    int sock, len, client_len, opcode, ssize = 0, n, i;
    unsigned short int bnumber = 0, recvcount = 0;
    unsigned char filebuf[MAXDATASIZE +1];
    unsigned char packetbuf[MAXPACFREQ][MAXDATASIZE + 1], recvbuf[MAXDATASIZE + 12];
    char filename[196], mode[12], *bufindex;
    struct sockaddr_in ack;
    FILE *fp;

    // 인자로 받은 파일 이름과 모드를 함수 내부에서 사용할 변수에 할당
    strcpy(filename, pFilename);
    strcpy(mode, pMode);

    // 서버 - 클라이언트 간 데이터 송수신을 위해 소켓을 생성한다
    // 이는 서버 - 클라이언트 연결 소켓과 다른 소켓이다 (UDP 기반 특징)
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        printf("\nServer Socket for data transport cannot be created!\n");
        return;
    }
    printf("\nServer socket for data transport created!\n");

    // 지원하는 모드 (octet, netascii) 처리
    if (!strncasecmp(mode, "octet", 5) && !strncasecmp(mode, "netascii", 8))
    {
        // 에러 패킷 생성 후 송신
        len = err_packet(4, err_msg[0], packetbuf[0]);
        // len = sprintf((char *) packetbuf[0], "%c%c%c%c Unrecognized mode (%s)%c", 
        //            0x00, 0x05, 0x00, 0x04, mode, 0x00);
        if (sendto (sock, packetbuf[0], len, 0, (struct sockaddr *) &client, sizeof(client)) != len)
        {
            printf("\nError packet could not be transported!\n");
        }
        return;
    }
    // 전송할 파일 이름이 제대로 열리는 지 확인한다
    fp = fopen(filename, "r");
    
    if (fp == NULL)
    {
        printf("\nFile could not be opened!\n");
        // 에러 패킷 생성 후 송신
        len = err_packet(4, err_msg[1], packetbuf[0]);
        // len = sprintf((char *) packetbuf[0], "%c%c%c%c File not found (%s)%c",
        //            0x00, 0x05, 0x00, 0x01, filename, 0x00);
        if (sendto(sock, packetbuf[0], len, 0, (struct sockaddr *) &client, sizeof (client)) != len)
        {
            printf("\nError packet could not be transported!\n");
        }
        printf("\nError packet transported!\n");
        return;
    }
    else
    {
        printf("\nServer sending file to client...\n");
    }
    // 전송할 파일을 filebuf 에 저장 후 전송
    // 전송 후 ACK 패킷 수신
    memset(filebuf, 0, sizeof (filebuf));
    // 데이터 송수신을 위해 무한 루프
    while (1)
    {
        ssize = fread(filebuf, 1, DATASIZE, fp);

        // 블록 넘버 증가
        bnumber++;

        // 데이터 패킷 생성
        sprintf((char *) packetbuf[bnumber], "%c%c%c%c", 0x00, 0x03, 0x00, 0x00);
        memcpy((char *) packetbuf[bnumber] + 4, filebuf, ssize);
        len = ssize + 4;
        packetbuf[bnumber][2] = (bnumber & 0xFF00) >> 8;
        packetbuf[bnumber][3] = (bnumber & 0x00FF);

        printf("\nCreated packet # %d : length %d, datasize %d byte\n", bnumber, len, ssize);

        // 데이터 패킷 전송
        if (sendto(sock, packetbuf[bnumber], len, 0, (struct sockaddr *) &client, sizeof(client)) != len)
        {
            printf("\nCould not send data packet to client!\n");
            return;
        }

        // 데이터 전송 후 ACK 패킷 수신
        // 추후 ACK 패킷 수신 주기를 둬서 조건문을 생성할 수 있다
        client_len = sizeof (ack);
        n = -1;
        n = recvfrom(sock, recvbuf, sizeof (recvbuf), 0, (struct sockaddr *) &ack, (socklen_t *) &client_len);

        // ACK 패킷 수신 유효성 확인 ()
        if (n < 0)
        {
            printf("\nServer could not get ACK packet from Client!\n");
            return;
        }
        else
        {
            if (client.sin_addr.s_addr != ack.sin_addr.s_addr)
            {
                printf("\nError while receiving ACK packet : Wrong address\n");
                return;
            }
            if (tid != ntohs(client.sin_port))
            {
                printf("\nError while receiving ACK packet : Wrong tid\n");
                len = err_packet(4, err_msg[2], recvbuf);
                // len = sprintf((char *) recvbuf, "%c%c%c%c Bad / Unknown TID %c", 0x00, 0x05, 0x00, 0x05, 0x00);
                if (sendto(sock, recvbuf, len, 0, (struct sockaddr *) &client, sizeof(client)) != len)
                {
                    printf("\nError packet could not be transported!\n");
                }
                return;
            }
        }
        // ACK 패킷 수신 완료 후 유효성 다시 확인
        bufindex = (char *) recvbuf;

        if (bufindex++[0] != 0x00)
        {
            printf("\nFirst byte is not NULL, Wrong packet!\n");
            return;
        }
        // opcode 와 블록 넘버 분리
        opcode = *bufindex++;
        recvcount = *bufindex++ << 8;
        recvcount &= 0xFF00;
        recvcount += (*bufindex++ & 0x00FF);

        if (opcode != 4 || bnumber != recvcount)
        {
            printf("\nIt's not right opcode or block number!\n");
            return;
        }
        else
            printf("\nServer successfully get ACK packet # %d", recvcount);
        
        // 마지막 데이터 송신 확인
        if (ssize != DATASIZE)
            break;
        // filebuf 비우고 while 문으로 데이터 송신 반복 
        memset(filebuf, 0, sizeof (filebuf));
    }
    // 파일 종료
    fclose (fp);
    printf("\nFile sent successfully!\n");

    return;
}