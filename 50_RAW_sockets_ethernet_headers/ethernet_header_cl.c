/* Демонстрация ethernet-заголовка для обмена пакетами по сети
клиента с сервером */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include <limits.h>
#include <errno.h>
#include <err.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <netinet/in.h>
#include <netinet/udp.h>	//Provides declarations for udp header
#include <netinet/ip.h>	//Provides declarations for ip header
#include <netpacket/packet.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <net/if.h>
// mac 08:00:27:41:b4:99

#define DEFAULT_SERVER_IP INADDR_LOOPBACK // IP адрес сервера по умолчанию (127.0.0.1)
#define DEFAULT_PORT_SERVER 8000 // номер порта сервера по умолчанию (8000)
#define DEFAULT_PORT_CLIENT 8001 // номер порта клиента по умолчанию (8001)
#define ETHER_ADDR_LEN 6 // Количество байт ethernet (MAC) адреса

// параметры сервера
#define SERVICE_ID_NOT_DEFINED USHRT_MAX

// параметры клиента
#define RECEIVE_MESSAGE_MAX_ATTEMPTS 5

// параметры UDP сообщения
#define UDP_HEADER_SIZE 8 // стандартный размер заголовка UPD-пакета в байтах
#define MAC_HEADER_SIZE 14 // стандартный размер заголовка mac-пакета в байтах
#define MESSAGE_DATA_LENGTH 20 // размер данных сообщения в байтах (sizeof char = 1 байт)
#define MESSAGE_FULL_LENGTH MESSAGE_DATA_LENGTH + 8 // полная длина сообщения в байтах (sizeof unsigned short = 2 байта)
#define UDP_MESSAGE_LENGTH UDP_HEADER_SIZE + MESSAGE_FULL_LENGTH // полная длина UDP-сообщения пересылаемого по сети
#define IP_PLUS_UDP_SIZE 20 + 8
#define RAW_MESSAGE_LEN MAC_HEADER_SIZE + IP_PLUS_UDP_SIZE + MESSAGE_FULL_LENGTH

// состояния сообщения
#define MESSAGE_PREPARED 0
#define MESSAGE_RECEIVED 1
#define MESSAGE_SENT 2
#define MESSAGE_ERROR 3

// типы сообщения
#define NO_MESSAGE 0
#define MESSAGE_CONNECT 1
#define MESSAGE_CLOSE 2
#define MESSAGE_DATA 3

// Подсчет checksum
unsigned short csum(unsigned short *ptr,int nbytes)
{
	register long sum;
	unsigned short oddbyte;
	register short answer;

	sum=0;
	while(nbytes>1) {
		sum+=*ptr++;
		nbytes-=2;
	}
	if(nbytes==1) {
		oddbyte=0;
		*((u_char*)&oddbyte)=*(u_char*)ptr;
		sum+=oddbyte;
	}

	sum = (sum>>16)+(sum & 0xffff);
	sum = sum + (sum>>16);
	answer=(short)~sum;

	return(answer);
}

// структура транспортного уровня
struct sockaddr_ll {
    unsigned short  sll_family;    /* Всегда AF_PACKET */
    unsigned short  sll_protocol;  /* Протокол физического уровня */
    int             sll_ifindex;   /* Hомер интерфейса */
    unsigned short  sll_hatype;    /* Тип заголовка */
    unsigned char   sll_pkttype;   /* Тип пакета */
    unsigned char   sll_halen;     /* Длина адреса */ 
    unsigned char   sll_addr[8];   /* Адрес физического уровня */
};

// структура MAC-заголовка
struct mac_header {
	unsigned short dest_mac;
	unsigned short source_mac;
	unsigned short type;
};

// структура IP-заголовка
struct ip_header  {
#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned int ihl:4;
    unsigned int version:4;
#elif __BYTE_ORDER == __BIG_ENDIAN
    unsigned int version:4;
    unsigned int ihl:4;
#else
# error        "Please fix <bits/endian.h>"
#endif
    u_int8_t tos;
    u_int16_t tot_len;
    u_int16_t id;
    u_int16_t frag_off;
    u_int8_t ttl;
    u_int8_t protocol;
    u_int16_t check;
    u_int32_t saddr;
    u_int32_t daddr;
    /*The options start here. */
};

// структура UDP-заголовка
struct udp_header {
	unsigned short source_port;  /* порт источника */
	unsigned short dest_port;    /* порт назначения */
	unsigned short packet_len;   /* длина UDP-пакета = 8 байт (заголовок) + MESSAGE_FULL_LENGTH (сообщение) */
	unsigned short check_sum;    /* чек-сумма для проверки целостности пакета = 0 (мы не проверяем)*/
};

// структура сообщения
struct message {
	unsigned short sid; // id привязанного сервиса
	unsigned short mid; // id сообщения
	unsigned short type; // тип сообщения (0 запрос подключения, 1 запрос отключения, 2 передача данных)
	unsigned short state; // состояние сообщения (0 подготовлено, 1 получено, 2 отправлено, 3 ошибка)
	char data[MESSAGE_DATA_LENGTH]; // данные сообщения
};

// структура mac-сообщения
struct ethernet_message {
	struct mac_header mach;
	struct ip_header iph;
	struct udp_header udph;
	struct message msg;
};

struct message_raw {
	char buf[IP_PLUS_UDP_SIZE];
	struct message msg;
};

// получение raw сообщения из сокета
struct message receive_message_reply(int socket_fd, struct sockaddr_in address) {
	struct message_raw message_raw;
	memset(&message_raw.msg.data, '\0', MESSAGE_DATA_LENGTH);
	socklen_t addr_len = sizeof(struct sockaddr_ll);
	
	ssize_t number_of_bytes = recvfrom(socket_fd, &message_raw, RAW_MESSAGE_LEN , 0, (struct sockaddr*) &address, &addr_len);
	
	if (number_of_bytes == -1) {
		message_raw.msg.state = MESSAGE_ERROR;
		perror("recvfrom");
	} else {
		message_raw.msg.state = MESSAGE_RECEIVED;
	}
	return message_raw.msg;
}

int main(int argc, char* argv[])
{
	//struct sockaddr_in svaddr; // claddr;
	struct sockaddr_ll svaddr;
	int sfd;
	int sid = SERVICE_ID_NOT_DEFINED;
	int mid = 0;
	int type = MESSAGE_CONNECT;
	char source_ip[32];
	strcpy(source_ip , "127.0.0.1");
	int index = (int)if_nametoindex("enp0s3");
	const unsigned char ether_broadcast_addr[]={08,00,27,41,b4,99};
	char ethernet_type[24];
	strcpy(ethernet_type , "0x.08.00"); // Правильно?
	
	/* Создаем клиентский сокет; привязываем его к уникальному пути
	(основанному на PID) */
	sfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (sfd == -1)
		perror("perror");

	//int flag = 1; // Включить опцию IP_HDRINCL
    //setsockopt(sfd, IPPROTO_IP, IP_HDRINCL, &flag, sizeof(flag));

	/* Формируем адрес сервера */
	memset(&svaddr, 0, sizeof(struct sockaddr_in));
	sll_family = htons(AF_PACKET);
    svaddr.sll_protocol = htons(ETH_P_ALL);  /* Протокол физического уровня */
    svaddr.sll_ifindex = index;   /* Hомер интерфейса */
    svaddr.sll_hatype = ARPHRD_ETHER;    /* Тип заголовка */
    svaddr.sll_pkttype = PACKET_HOST;   /* Тип пакета */
    svaddr.sll_halen = ETHER_ADDR_LEN;     /* Длина адреса */ 
    memcpy(&svaddr.sll_addr, ether_broadcast_addr, ETHER_ADDR_LEN);/* Адрес физического уровня */
	
	/* Ethernet-заголовок */
	struct ethernet_message ethernet_message; 

	// Формируем Ethernet заголовок сообщения
	memcpy(&ethernet_message.mach.dest_mac, ether_broadcast_addr, ETHER_ADDR_LEN);
	memcpy(&ethernet_message.mach.source_mac, ether_broadcast_addr, ETHER_ADDR_LEN);
	ethernet_message.mach.type = ethernet_type;
	
	// Формируем IP заголовок сообщения 
	ethernet_message.iph.ihl = 5;
	ethernet_message.iph.version = 4;
	ethernet_message.iph.tos = 0;
	ethernet_message.iph.tot_len = RAW_MESSAGE_LEN;
	ethernet_message.iph.id = htonl (54321);	//Id of this packet
	ethernet_message.iph.frag_off = 0;
	ethernet_message.iph.ttl = 255;
	ethernet_message.iph.protocol = IPPROTO_UDP;
	ethernet_message.iph.check = 0;
	ethernet_message.iph.saddr = inet_addr (source_ip);	//Spoof the source ip address
	ethernet_message.iph.daddr = htons(DEFAULT_PORT_SERVER);
	
	// Формируем IP checksum
	iph->check = csum ((unsigned short *) datagram, iph->tot_len);

	// Формируем UPD заголовок сообщения
	ethernet_message.udph.source_port=htons(DEFAULT_PORT_CLIENT); // Порт отправителя
	ethernet_message.udph.dest_port=htons(DEFAULT_PORT_SERVER); // Порт получателя
	ethernet_message.udph.packet_len=htons(UDP_MESSAGE_LENGTH); // Длина пакета
	ethernet_message.udph.check_sum=0; // Контрольная сумма (не проверяем)

	/* Подготавливаем данные к отправке */
	memset(&ethernet_message.msg.data, '\0', MESSAGE_DATA_LENGTH);
	strcpy(ethernet_message.msg.data, "Hello!");
	ethernet_message.msg.type = type;
	ethernet_message.msg.sid = sid;
	ethernet_message.msg.mid = mid;
	ethernet_message.msg.state = MESSAGE_PREPARED;
	
	if (sendto(sfd, &ethernet_message, RAW_MESSAGE_LEN, 0, (struct sockaddr*) &svaddr, sizeof(struct sockaddr_in)) == -1)
		perror("sendto");
	else
		printf("Packet Send. Length: %d \n", ip_message.iph.tot_len);

	/* Получение RAW UDP сообщения из сокета */
	int attempts = 0;
	struct message message;
	
	while(attempts < RECEIVE_MESSAGE_MAX_ATTEMPTS) {
		message = receive_message_reply(sfd, svaddr);
		if ((sid == SERVICE_ID_NOT_DEFINED || message.sid == sid) && message.mid == mid) {
			printf("%s\n", message.data);
    	}
    	attempts++;
    	usleep(5);
    }
    message.state = MESSAGE_ERROR;
    
	close(sfd);
	
	return 0;
}
