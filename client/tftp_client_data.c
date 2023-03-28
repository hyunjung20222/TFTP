#include "../tftp.h"

void client_get(char *pFilename, struct sockaddr_in server, char *pMode, int sock)
{
    int len, server_len, opcode, n, i, tid = 0, ack_len = 0;
    unsigned short int bnumber = 0, recvcount = 0;
    unsigned char filebuf[MAXDATASIZE +1];
    unsigned char packetbuf[MAXDATASIZE + 12];
    char filename[128], mode[12], *bufindex, ackbuf[512];
    struct sockaddr_in data;
    FILE *fp;

    // 인자로 받은 파일 이름과 모드를 함수 내부에서 사용할 변수에 할당
    strcpy(filename, pFilename);
    strcpy(mode, pMode);

    // 수신 받을 파일 생성
    fp = fopen(filename, "w");
    if (fp == NULL)
    {
        printf("\nFile could not be opened!\n");
        return;
    }
    printf("\nGetting file... \n");

    // ACK 패킷 송신과 데이터 패킷 수신
    // 생성할 파일 메모리 초기화
    memset(filebuf, 0, sizeof (filebuf));
    n = DATASIZE + 4;

    // 반복을 위해 do while 구조 사용
    do 
    {
        // ACK 생성 패킷, 데이터 수신 패킷 초기화
        memset(packetbuf, 0, sizeof (packetbuf));
        memset(ackbuf, 0, sizeof (ackbuf));

        // 마지막 데이터 패킷 검사
        if (n != DATASIZE + 4)
        {
            printf("It's last data packet : size %d byte", n - 4);
            // ACK 패킷 생성 
            ack_len = ack_packet(bnumber, ackbuf);
            if (ack_len == 0)
                return;
            // ACK 패킷 전송
            if (sendto(sock, ackbuf, len, 0, (struct sockaddr *) &server, sizeof(server)) != len)
            {
                printf("\nCould not send ACK packet to Server!\n");
                return;
            }
            printf("\nClient successfully sent an ACK packet!");
            // 데이터 수신 완료
            goto done;
        }
        bnumber++;
        // 데이터 패킷 수신
        server_len = sizeof(data);
        n = -1;
        
        // 데이터 수신을 3 번만 확인
        for (i = 0; i < 3 && n < 0; i++)
            n = recvfrom(sock, packetbuf, sizeof (packetbuf) - 1, 0, (struct sockaddr *) &data, (socklen_t *) &server_len);
        
        // 클라이언트가 서버에 처음 접속할 때 (tid == 0)
        if (!tid)
        {
            tid = ntohs(data.sin_port);
            server.sin_port = htons (tid);
        }
        // 데이터 제대로 수신하지 못했을 때
        if (n < 0)
            printf("\nClient cannot get data packet from Server!\n");
        else
        {
            // IP 주소가 다를 때
            if (server.sin_addr.s_addr != data.sin_addr.s_addr)
            {
                printf("\nError while receiving Data packet : Wrong address\n");
                return;
            }
            // TID 와 포트 번호가 다를 때 
            if (tid != ntohs (server.sin_port))
            {
                printf("\nError while receiving ACK packet : Wrong tid\n");
                len = err_packet(4, err_msg[2], packetbuf);
                if (sendto (sock, packetbuf, len, 0, (struct sockaddr *) &server, sizeof (server)) != len)
                {
                    printf("\nError packet could not be transported!\n");
                }
                return;
            }
            // 데이터 성공적으로 수신 후
            bufindex = (char *) packetbuf;

            if (bufindex++[0] != 0x00)
            {
                printf("\nFirst byte is not NULL, Wrong packet!\n");
                return;
            }
            // 명령 코드, 블록 넘버 분리
            opcode = *bufindex++;
            recvcount = *bufindex++ << 8;
            recvcount &= 0xFF00;
            recvcount += (*bufindex++ & 0x00FF);
            // 데이터 복사
            memcpy((char *) filebuf, bufindex, n - 4);
            printf("\nClient get data packet # %d from server : opcode %d, packetsize %d\n", recvcount, opcode, n);

            // opcode 유효성 확인
            if (opcode != 3)
            {
                printf("\nIt's not right opcode\n");
                return;
            }
            else
            {
                ack_len = ack_packet(recvcount, ackbuf);
                if (sendto(sock, ackbuf, ack_len, 0, (struct sockaddr *) &server, sizeof (server)) != ack_len)
                {
                    printf("\nCould not send ACK packet to server!\n");
                    return;
                }
                printf("\nClient successfully sent an ACK packet to server!\n");
            }
        }
    }
    while (fwrite(filebuf, 1, n - 4, fp) == n - 4);

    // do while 문 END
    // 비정상적 종료이므로 에러 문구를 출력한다
    fclose (fp);
    sync();
    printf("\nClient could not get file from server... try agian!\n");
    return;

    // do while 문 내부 goto 라벨
    done:
        fclose(fp);
        sync();
        printf("\nClient successfully received file from server!\n");
        return;
}