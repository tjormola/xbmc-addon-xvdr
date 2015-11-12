/*
 *      xbmc-addon-xvdr - XVDR addon for XBMC
 *
 *      Copyright (C) 2010 Alwin Esch (Team XBMC)
 *      Copyright (C) 2012 Alexander Pipelka
 *
 *      https://github.com/pipelka/xbmc-addon-xvdr
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "os-config.h"

#include "xvdr/clientinterface.h"
#include "xvdr/session.h"
#include "xvdr/msgpacket.h"
#include "xvdr/thread.h"
#include "xvdr/command.h"

#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

using namespace XVDR;

Session::Session()
  : m_timeout(3000)
  , m_fd(INVALID_SOCKET)
  , m_connectionLost(false)
{
  m_port = 34891;
}

Session::~Session()
{
  Close();
}

void Session::Abort()
{
  shutdown(m_fd, SHUT_RDWR);
}

void Session::Close()
{
  if(!IsOpen())
    return;

  Abort();

  closesocket(m_fd);
  m_fd = INVALID_SOCKET;
}

int Session::OpenSocket(const std::string& hostname, int port) {
	int sock = INVALID_SOCKET;
	char service[10];
	snprintf(service, sizeof(service), "%i", port);

	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family   = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	struct addrinfo* result;

	if(getaddrinfo(hostname.c_str(), service, &hints, &result) != 0) {
		return INVALID_SOCKET;
	}

	// loop through results
	struct addrinfo* info = result;

	while(sock == INVALID_SOCKET && info != NULL) {
		sock = socket(info->ai_family, info->ai_socktype, info->ai_protocol);

		// try to connect
		if(sock != INVALID_SOCKET) {
			break;
		}

		info = info->ai_next;
	}

	if(sock == INVALID_SOCKET) {
		freeaddrinfo(result);
		return INVALID_SOCKET;
	}

	setsock_nonblock(sock);

	int rc = 0;

	if(connect(sock, info->ai_addr, info->ai_addrlen) == -1) {
		if(sockerror() == EINPROGRESS || sockerror() == SEWOULDBLOCK) {

      if(!pollfd(sock, m_timeout, false)) {
        freeaddrinfo(result);
        closesocket(sock);
        return INVALID_SOCKET;
      }

			socklen_t optlen = sizeof(int);
			getsockopt(sock, SOL_SOCKET, SO_ERROR, (sockval_t*)&rc, &optlen);
		}
		else {
			rc = sockerror();
		}
	}

  if(rc != 0) {
		freeaddrinfo(result);
    closesocket(sock);
		return INVALID_SOCKET;
  }

	setsock_nonblock(sock, false);

	int val = 1;
	setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (sockval_t*)&val, sizeof(val));

	setsock_keepalive(sock);

	freeaddrinfo(result);

	return sock;
}

bool Session::Open(const std::string& hostname)
{
  Close();

  m_fd = OpenSocket(hostname, m_port);

  if (m_fd == INVALID_SOCKET)
    return false;

  // store connection data
  m_hostname = hostname;

  return true;
}

bool Session::IsOpen()
{
  return m_fd != INVALID_SOCKET;
}

MsgPacket* Session::ReadMessage()
{
  bool bClosed = false;
  MsgPacket* p = MsgPacket::read(m_fd, bClosed, m_timeout);

  if(bClosed)
    SignalConnectionLost();

  return p;
}

bool Session::TransmitMessage(MsgPacket* vrp)
{
  return vrp->write(m_fd, m_timeout);
}

MsgPacket* Session::ReadResult(MsgPacket* vrp)
{
  if(!TransmitMessage(vrp))
    return NULL;

  return ReadMessage();
}

void Session::OnReconnect() {
}

void Session::OnDisconnect() {
}

void Session::SignalConnectionLost()
{
  if(m_connectionLost)
    return;

  Close();
  m_connectionLost = true;

  OnDisconnect();
}

bool Session::readData(uint8_t* buffer, int totalBytes)
{
	int read = 0;

	while(read < totalBytes) {
		if(pollfd(m_fd, m_timeout, true) == 0) {
			return false;
		}

		int rc = recv(m_fd, (char*)(buffer + read), totalBytes - read, MSG_DONTWAIT);

		if(rc == 0) {
			return false;
		}
		else if(rc == -1) {
			if(sockerror() == SEWOULDBLOCK) {
				continue;
			}

			return false;
		}

		read += rc;
	}

	return true;
}

bool Session::TryReconnect() {
	return false;
}

bool Session::ConnectionLost() {
  return m_connectionLost;
}
