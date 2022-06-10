// ************************************************************************** //
//                                                                            //
//                                                        :::      ::::::::   //
/*   server.hpp                                         :+:      :+:    :+:   */
//                                                    +:+ +:+         +:+     //
//   By: jiglesia <jiglesia@student.42.fr>          +#+  +:+       +#+        //
//                                                +#+#+#+#+#+   +#+           //
//   Created: 2022/06/08 16:28:11 by jiglesia          #+#    #+#             //
/*   Updated: 2022/06/10 16:30:10 by nayache          ###   ########.fr       */
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
		if (bytes_sent < 0)
		{
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
		if (_bind_value < 0)
		{
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
	int run(){
		while (1) {
			poll(&_pfd, 1, 100);
			if (_pfd.revents == POLLIN){
				try {
					_users.push_back(User(_sock));
					this->_channel[0].addUser(this->_users.back());
				}
				catch (std::exception &e){
					std::cerr << "Could not accept user" << std::endl;
				}
				std::cout << "Accepted new client @ " << _users.back().getIP() << ":" << _users.back().getPort() << std::endl;
			}
			else{
				for (unsigned long i = 0; i < _users.size(); i++){
					_users[i].u_poll(100);
					if (_users[i].getRevents() == POLLIN){
						memset(_buffer, 0, BUFFERLEN);
						// 5 recv
						int bytes_received = recv(_users[i].getfd(), _buffer, BUFFERLEN-1, 0);
						if (bytes_received < 0) {
							std::cerr << "Could not receive" << std::endl;
							return 1;
						}
						if (_buffer[bytes_received-1] == '\n') {
							_buffer[bytes_received-1] = 0;
						}
						if (_users[i].getFirstMsg() == false)
						{
							getNames(_buffer, i);
						/*	if (_users[i].occurName() == true)
							{
								std::cout << _users[i].userName << " user name already used \n"; 
								_users[i].erase(_users.begin() + i);
							}
							*/
						}
						else {
						//std::cout << "Client message : " << _buffer << std::endl;

						std::stringstream stream;
						std::cout << GREEN << _users[i].getUserName() << "(" << _users[i].getNickName() << ") : " << RESET << _buffer << std::endl; // to server
						stream << _users[i].getUserName() << "(" << _users[i].getNickName() << ") : " << _buffer << std::endl; // to channel
						std::string msg = stream.str();
						this->_channel[0].print(msg);
						
						if (_buffer[0] == 'Q' && _buffer[1] == 'U' && _buffer[2] == 'I' && _buffer[3] == 'T'){
							std::cout << "Client at " << _users[i].getIP() << ":" << _users[i].getPort() << " has disconnected." << std::endl;
							_users.erase(_users.begin() + i);
							if (_users.size() == 0){
								std::cout << "Shutting down socket." << std::endl;
								return 0;
							}
							break ;
						}
						if (!strncmp(_buffer, "LIST", 4))
						{
							printChannels(i);
						}
						}
						// 6 send
					//	int bytes_sent = send(_users[i].getfd(), response.c_str(), response.length(), 0);
					//	if (bytes_sent < 0) {
					//		std::cerr << "Could not send" << std::endl;
					//		return 1;
				//		}
					}
				}
			}
		}
		// 7 goto 5
	}
};

#endif
