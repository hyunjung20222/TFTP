#include "../tftp.h"

int w_size = 1;
int p_length = 512;

int main(int argc, char **argv)
{
    extern char *optarg;
    int sock, server_len, len, opt;
    char opcode, filename[196], mode[12] = "octet";
    struct hostent *host;
    struct sockaddr_in server;
    FILE *fp;

    // 명령행 인자 처리
    if (argc < 2)
    {
        usage();
        return 0;
    }
    
    // IP 주소 저장
    if (!(host = gethostbyname(argv[1])))
    {
        perror("\nClient could not get host address!");
        exit(2);
    }

    // 명령행 option 처리 
    while ((opt = getopt(argc, argv, "nos:g:")) != -1)
    {
        switch (opt)
        {
            // 서버에 파일 전송
            case 's':
            strncpy(filename, optarg, sizeof(filename) - 1);
            opcode = WRQ;
            fp = fopen(filename, "r");
            if (fp == NULL)
            {
                printf("\nFile could not be opened!\n");
                return 0;
            }
            printf("\nFile successfully opened!\n");
            fclose(fp);
            break;

            // 서버로부터 파일 수신 
            case 'g':
            strncpy(filename, optarg, sizeof(filename) - 1);
            opcode = RRQ;
            fp = fopen(filename, "w");
            if (fp == NULL)
            {
                printf("\nFile could not be opened!\n");
                return 0;
            }
            printf("\nFile successfully opened!\n");
            fclose(fp);         
            break;

            // netascii 모드
            case 'n':
            strncpy(mode, "netascii", sizeof(mode) - 1);
            printf("\nMode is set to netascii!\n");
            break;

            // octet 모드
            case 'o':
            strncpy(mode, "octet", sizeof(mode) - 1);
            printf("\nMode is set to octet!\n");            
            break;

            default:
            usage();
            return (0);
            break;
        }
        
    }
    // 인자의 개수에 따라 개수 오류를 출력한다
    if (argc < 5 || argc > 12)
    {
        printf("\ntoo few arguments or too many arguments! : % d\n", argc);
        usage();
        exit (ERROR);
    }

    // 서버와 통신 시작
    // 소켓 생성
    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        printf("\nClient Socket cannot be created!\n");
        return 0;
    }
    printf("\nClient socket created!\n");

    // 서버 구조체 생성
    memset(&server, 0, sizeof (server));
    server.sin_family = AF_INET;
    memcpy(&server.sin_addr, host -> h_addr_list[0], host -> h_length);
    server.sin_port = htons(PORT);
    server_len = sizeof(server);

    // 서버에 전송할 요청 패킷 (RRQ, WRQ) 생성 및 전송
    memset(buf, 0, BUFSIZ);
    len = req_packet(opcode, filename, mode, buf);

    if (sendto(sock, buf, len, 0, (struct sockaddr *) &server, server_len) != len)
    {
        perror("\nCould not send request to server!");
        exit(ERROR);
    }

    // opcode 분류 (RRQ, WRQ)
    switch (opcode)
    {
        case RRQ:
        printf("\nOpcode is RRQ, get file from server!\n");
        client_get(filename, server, mode, sock);
        break;

        case WRQ:
        printf("\nOpcode is WRQ, send file to server!\n");
        break;

        default:
        printf("\nInvalid opcode! Ignoring packet\n");
    }
    close (sock);
    return 1;
}