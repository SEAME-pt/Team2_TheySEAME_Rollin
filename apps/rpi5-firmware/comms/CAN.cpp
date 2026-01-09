#include "CAN.hpp"
#include <linux/rtnetlink.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <iostream>

CAN::CAN(const std::string &interface) {
	struct sockaddr_can addr;

	_sock = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	if (_sock < 0) {
		std::perror("Error creating socket");
	}
	std::cout << "Created Socket" << std::endl;

	std::strcpy(_ifr.ifr_name, interface.c_str());
	if (ioctl(_sock, SIOCGIFINDEX, &_ifr) < 0) {
		std::perror("Error in ioctl");
	}
	addr.can_family = AF_CAN;
	addr.can_ifindex = _ifr.ifr_ifindex;
	if (bind(_sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_can)) < 0) {
		std::perror("Error in bind");
	}
	std::cout << "Binded Socket" << std::endl;
	if (netlinkSocket() < 0) {
		std::perror("Netlink");
	}
	getAttr();
}

CAN::~CAN() {
	close(_sock);
	close(_nlSock);
	std::cout << "Closed CAN socket" << std::endl;
}

int CAN::getSocketFd() const { return (_sock); }

std::string CAN::getInterface() const { return (_ifr.ifr_name); }

int CAN::getBitrate() const { return (10); };

bool CAN::isUp() const { return (_ifr.ifr_flags & IFF_UP); }

bool CAN::isRunning() const { return (_ifr.ifr_flags & IFF_RUNNING); }

/*
 * @brief Sends a CAN frame to the Bus
 *
 * This function fills the can_frame struct with the id, length and data and
 * sends the frame to the Bus
 *
 * ====================== Requirement Traceability ===========================
 * [impl->dsn~comms-can-rpi-sendMsg~1]
 * ==========================================================================
 *
 * @return void
 *
 */
int CAN::sendFrame(const canid_t id, const uint8_t *data, const uint8_t len) {
	struct can_frame frame;
	int nbytes;

	frame.can_id = id;
	frame.len = len;
	std::memcpy(frame.data, data, frame.len);

	nbytes = write(_sock, &frame, sizeof(struct can_frame));
	if (nbytes < 0) {
		std::perror("Error in write");
		return (-1);
	}
	return (0);
}

/*
 * @brief Read a CAN frame from the Bus
 *
 * This function reads a CAN frame from the Bus and prints the information
 * inside the can_frame struct
 *
 * ====================== Requirement Traceability ===========================
 * [impl->dsn~comms-can-rpi-receiveMsg~1]
 * ==========================================================================
 *
 * @return void
 *
 */
int CAN::readFrame(struct can_frame &frame) {
	int nbytes;

	nbytes = read(_sock, &frame, sizeof(struct can_frame));
	if (nbytes < 0) {
		std::perror("Error in read");
		return (-1);
	}
	return (0);
}

int CAN::sendDumpReq() {
	char buf[NLMSG_SPACE(sizeof(struct ifinfomsg))];
	struct nlmsghdr *nh = (struct nlmsghdr *)buf;
	struct ifinfomsg *ifi = (struct ifinfomsg *)NLMSG_DATA(nh);
	struct sockaddr_nl sa;
	struct iovec iov;
	struct msghdr msg;

	std::memset(&sa, 0, sizeof(sa));
	std::memset(&msg, 0, sizeof(msg));
	sa.nl_family = AF_NETLINK;
	*nh = {
		.nlmsg_len = NLMSG_LENGTH(sizeof(*ifi)),
		.nlmsg_type = RTM_GETLINK,
		.nlmsg_flags = NLM_F_REQUEST,
		.nlmsg_seq = 1,
	};
	nh->nlmsg_pid = getpid();
	*ifi = {
		.ifi_family = AF_PACKET,
		.ifi_index = _ifr.ifr_ifindex
	};
	iov = {
		.iov_base = &buf,
		.iov_len = nh->nlmsg_len,
	};
	msg = {
		.msg_name = &sa,
		.msg_namelen = sizeof(sa),
		.msg_iov = &iov,
		.msg_iovlen = 1,
	};

	return (sendmsg(_nlSock, &msg, 0));
}

int CAN::netlinkSocket() {
	struct sockaddr_nl sa;

	std::memset(&sa, 0, sizeof(sa));
	_nlSock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if (_nlSock < 0) {
		return (-1);
	}
	sa.nl_family = AF_NETLINK;
	sa.nl_pid = getpid();
	if (bind(_nlSock, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
		return (-1);
	}
	std::cout << "Netlink Socket Created" << std::endl;
	return (0);
}

int CAN::getAttr() {
	int ret;
	char nhbuf[8192];
	struct sockaddr_nl sa;
	struct rtattr *rta;

	sa.nl_family = AF_NETLINK;
	struct iovec iov = {
		.iov_base = (void *)nhbuf,
		.iov_len = sizeof(nhbuf),
	};
	struct msghdr msg = {
		.msg_name = (void *)&sa,
		.msg_namelen = sizeof(sa),
		.msg_iov = &iov,
		.msg_iovlen = 1,
		.msg_control = NULL,
		.msg_controllen = 0,
		.msg_flags = 0,
	};

	if (sendDumpReq() < 0) {
		std::perror("SendDump");
		return (-1);
	}
	std::cout << "Sended Msg" << std::endl;

	struct nlmsghdr *nh;

	while (true) {
		ret = recvmsg(_nlSock, &msg, 0);
		if (ret < 0) {
			std::perror("NetlinkRecv");
			return (-1);
		}
		std::cout << "Received Msg " << ret << std::endl;
		for (nh = (struct nlmsghdr *)nhbuf; NLMSG_OK(nh, ret); NLMSG_NEXT(nh, ret)) {
			int type = nh->nlmsg_type;

			printf("MSG_TYPE: %d\n", type);
			if (type == RTM_NEWLINK) {
				struct ifinfomsg *ifi = (struct ifinfomsg *)NLMSG_DATA(nh);
				printf("Ifi: %u\n", ifi->ifi_type);
				int len = nh->nlmsg_len - NLMSG_LENGTH(sizeof(*ifi));
				struct rtattr *canAttr;
				for (canAttr = (struct rtattr *)IFLA_RTA(ifi);
					RTA_OK(canAttr, len); RTA_NEXT(canAttr, len)) {
					printf("rtattr %u\n", canAttr->rta_type);
				}
			}
		}
		break;
	}

	return (0);
}

