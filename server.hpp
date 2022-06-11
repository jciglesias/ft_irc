// ************************************************************************** //
//                                                                            //
//                                                        :::      ::::::::   //
/*   server.hpp                                         :+:      :+:    :+:   */
//                                                    +:+ +:+         +:+     //
//   By: jiglesia <jiglesia@student.42.fr>          +#+  +:+       +#+        //
//                                                +#+#+#+#+#+   +#+           //
//   Created: 2022/06/08 16:28:11 by jiglesia          #+#    #+#             //
//   Updated: 2022/06/11 11:02:13 by jiglesia         ###   ########.fr       //
//                                                                            //
// ************************************************************************** //

#ifndef SERVER_HPP
# define SERVER_HPP

# include <vector>
# include <iostream>
//# include <stdlib.h>
//# include <sys/socket.h>
//# include <arpa/inet.h>
# include <cstring>
# include <sstream>
//# include <exception>
# include <unistd.h>
# include "user.hpp"
# include "channel.hpp"

# define GREEN "\e[32m"
# define RESET "\e[0m"

# define BUFFERLEN 1024

class Server{
private:
	std::vector<User>	_users;
	int					_port;
	int					_sock;
	struct pollfd		_pfd;
	int					_bind_value;
	struct sockaddr_in	_address;
	int 				_listen_value;
    char				_buffer[BUFFERLEN];
	std::string			client_ip;
	std::vector<Channel>	_channel;

public:

	void printChannels(int userIndex)
	{
		std::stringstream ss;

		for (std::vector<Channel>::iterator it = this->_channel.begin(); it != this->_channel.end(); it++)
			ss << "#" << it->getName() << std::endl;

		std::string response = ss.str();
		int bytes_sent = send(this->_users[userIndex].getfd(), response.c_str(), response.length(), 0);
		if (bytes_sent < 0){
			std::cerr << "could not send channels list\n";
			return;
		}
	}

	void getNames(std::string buffer, int index) {
		char*	str = const_cast<char *>(buffer.c_str());
		if (!strncmp(str, "CAP LS 302", 10))
		{
			std::string	nickName;
			std::string	userName;

			char* delimit = strchr(str + 17, '\n');
			delimit[-1] = '\0';
			nickName = str + 17;

			delimit = strstr(delimit + 1, "USER ");
			char* str = strstr(delimit + 6, " 0 * :");
			*str = '\0';
			userName = delimit + 5;

			this->_users[index].setName(nickName, userName);
		}
	}

	Server(int port, int sock) : _port(port), _sock(sock){
		_pfd.fd = _sock;
		_pfd.events = POLLIN;
		memset(&_address, 0, sizeof(_address));
		_address.sin_family = AF_INET;
		_address.sin_port = htons(_port);

		_bind_value = bind(_sock, (struct sockaddr *)&_address, sizeof(_address));
		if (_bind_value < 0){
			std::cout << "could not bind\n";
			throw std::exception();
		}

		_listen_value = listen(_sock, 1);
		if (_listen_value < 0)
			throw std::exception();

		Channel	Default("Default Channel", 0);
		this->_channel.push_back(Default);

	}
	~Server(){}
	void addUser(){
		try {
			_users.push_back(User(_sock));
			this->_channel[0].addUser(this->_users.back());
		}
		catch (std::exception &e){
			std::cerr << "Could not accept user" << std::endl;
		}
		std::cout << "Accepted new client @ " << _users.back().getIP() << ":" << _users.back().getPort() << std::endl;
	}

	std::string getline(int fd){
		std::stringstream stream;
		memset(_buffer, 0, BUFFERLEN);
		int bytes_read = read(fd, _buffer, BUFFERLEN - 1);
		if (_buffer[bytes_read - 1] == '\n')
			_buffer[bytes_read - 1] = 0;
		stream << _buffer;
		return stream.str();
	}

	int checkfd(int i){
		std::string str = this->getline(_users[i].getfd());
		// 5 recv
		if (_users[i].getFirstMsg() == false)
			getNames(str.c_str(), i);
		else {
			std::stringstream stream;
			stream << "[" << _users[i].getNickName() << "]: " << str << std::endl;
			this->_channel[0].print(stream.str());
			if (str[0] == 'Q' && str[1] == 'U' && str[2] == 'I' && str[3] == 'T'){
				std::cout << "Client at " << _users[i].getIP() << ":" << _users[i].getPort() << " has disconnected." << std::endl;
				_users.erase(_users.begin() + i);
				if (_users.size() == 0){
					return 0;
				}
			}
			if (!strncmp(str.c_str(), "LIST", 4)){
				printChannels(i);
			}
		}
		return 1;
	}

	int run(){
		while (1) {
			poll(&_pfd, 1, 100);
			if (_pfd.revents == POLLIN){
				this->addUser();
			}
			else{
				for (unsigned long i = 0; i < _users.size(); i++){
					_users[i].u_poll(100);
					if (_users[i].getRevents() == POLLIN){
						if (!this->checkfd(i))
							return 0;
					}
				}
			}
		}
	}
};

#endif
