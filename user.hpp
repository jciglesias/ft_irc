// ************************************************************************** //
//                                                                            //
//                                                        :::      ::::::::   //
/*   user.hpp                                           :+:      :+:    :+:   */
//                                                    +:+ +:+         +:+     //
//   By: jiglesia <jiglesia@student.42.fr>          +#+  +:+       +#+        //
//                                                +#+#+#+#+#+   +#+           //
//   Created: 2022/06/08 16:59:36 by jiglesia          #+#    #+#             //
/*   Updated: 2022/06/24 17:15:52 by nayache          ###   ########.fr       */
//                                                                            //
// ************************************************************************** //

#ifndef USER_HPP
# define USER_HPP

# include <exception>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <stdlib.h>
# include <string>
# include <string.h>
# include <poll.h>

class User{
private:
	int					_sock;
	int					_port;
	struct sockaddr_in	_address;
	socklen_t			_addrlen;
	struct pollfd		_pfd;
	std::string			_ip;
	std::string			_nickName;
	std::string			_userName;
	bool				_firstMsg;
	bool				_ops;
	std::string			_channel;
public:
	User() : _sock(0), _port(0) {}
	User(int sock) {
		memset(&_address, 0, sizeof(_address));
		_addrlen = sizeof(_address);
		_sock = accept(sock, (struct sockaddr *)&_address, &_addrlen);
		if (_sock < 0)
			throw std::exception();
		_pfd.fd = _sock;
		_pfd.events = POLLIN;
		_ip = inet_ntoa(_address.sin_addr);
		_port = ntohs(_address.sin_port);
		_firstMsg = false;
		_ops = false;
		_channel = "";
	}
	~User(){}
	std::string getIP() {return _ip;}
	std::string getId()
	{
		return (getNickName() + "!~" + getNickName() + "@" + getIP());
	}
	std::string getIdd()
	{
		return (getNickName() + "!~" + getNickName() + "@localhost");
	}

	void		setName(std::string nick, std::string user)
	{
		this->_nickName = nick;
		this->_userName = user;
		this->_firstMsg = true;
	}
	void setOperator(bool ops) {this->_ops = ops;}
	bool getOperator() {return this->_ops;}
	void setChannel(std::string name) {this->_channel = name;}
	std::string getChannel() {return this->_channel;}
	int getfd(){return _pfd.fd;}
	short getRevents(){return _pfd.revents;}
	int getPort() {return _port;}
	std::string getNickName() {return this->_nickName;}
	std::string getUserName() {return this->_userName;}
	bool getFirstMsg() {return _firstMsg;}
	int u_poll(int time){ return poll(&_pfd, 1, time); }
};

#endif
