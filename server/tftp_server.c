#include "../tftp.h"

int datasize = 512;
int port = 69;

int main(int argc, char **argv)
{
    int sock, n, client_len, pid, status, tid;
    char opcode, *bufindex, filenmae[196], mode[12];
    struct sockaddr_in server, client;

    // 클라이언트와 통신을 위해 socket 생성
    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        printf("\nServer Socket cannot be created!\n");
        return 0;
    }

    printf("\nServer socket created!\n");
    
    // 서버 구조체 생성 후 bind
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);

    // binding
    if (bind (sock, (struct sockaddr *) &server, sizeof(server)) < 0)
    {   
        perror("bind() error!");
        printf("\nServer bind failed... check if user has proper permission or server already running!\n");
        return (2);
    }

    // 클라이언트와 통신 시작 (지속된 연결을 위해 while 무한 루프문 사용)
    while (1)
    {
        client_len = sizeof (client);
        memset(buf, 0, BUFSIZ);
        n = 0;

        // 클라이언트로부터 요청 패킷을 받는다
        while (!n || errno == EAGAIN)
        {
            // waitpid 함수 사용 추후 결정 
            // waitpid (-1, &status, WNOHANG); 
            n = recvfrom(sock, buf, BUFSIZ, 0, (struct sockaddr *) &client, (socklen_t *) &client_len);

            if (n < 0 && errno != EAGAIN)
            {
                printf("\nServer could not receive request from client\n");
                return 0;
            }
            usleep(1000);
        }
        printf("\nServer received request from client!\n");
        printf("\nConnection from %s, port %d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));

        // 연결 성공 후 수신한 패킷 분류 (RRQ, WRQ)
        bufindex = buf;

        if(bufindex++[0] != 0x00)
        {
            printf("\nIt's not right packet!\n");
            return 0;
        }

        tid = ntohs(client.sin_port);
        opcode = *bufindex++;

        if (opcode == 1 || opcode == 2)
        {
            // 파일 이름 저장
            strncpy (filenmae, bufindex, sizeof(filenmae) - 1);
            bufindex += strlen(filenmae) + 1;
            // 모드 저장
            strncpy(mode, bufindex, sizeof(mode) - 1);
            bufindex += strlen(mode) + 1;

            printf("\nRight packet!\n");
            printf("opcode : %d\nfilename : %s\nmode : %s\n", opcode, filenmae, mode);
        }
        else
        {
            printf("\nWrong packet!\n");
            printf("opcode : %d", opcode);
            return 0;
        }
        // RRQ, WRQ 구분 (명령코드 구분)
        switch (opcode)
        {
            case 1: 
            printf("\nOpcode is 1, it means RRQ!\n");
            break;

            case 2:
            printf("\nOpcode is 2, it means WRQ!\n");
            break;

            default:
            printf("\nInvalid opcode! Ignoring packet\n");
            break;
        }
    }
}