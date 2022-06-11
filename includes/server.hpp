// ************************************************************************** //
//                                                                            //
//                                                        :::      ::::::::   //
/*   server.hpp                                         :+:      :+:    :+:   */
//                                                    +:+ +:+         +:+     //
//   By: jiglesia <jiglesia@student.42.fr>          +#+  +:+       +#+        //
//                                                +#+#+#+#+#+   +#+           //
//   Created: 2022/06/08 16:28:11 by jiglesia          #+#    #+#             //
//   Updated: 2022/06/11 12:47:33 by jiglesia         ###   ########.fr       //
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
# include "color.hpp"

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
	void printChannels(int userIndex){
		std::stringstream ss;

		for (std::vector<Channel>::iterator it = this->_channel.begin(); it != this->_channel.end(); it++)
			ss << "#" << '[' << it->getName() << ']' << std::endl;

		std::string response = ss.str();
		int bytes_sent = send(this->_users[userIndex].getfd(), response.c_str(), response.length(), 0);
		if (bytes_sent < 0){
			std::cerr << "could not send channels list\n";
			return;
		}
	}

	bool allowChannelName(std::string name){
		if (name.size() == 1)
			return (name[0] >= '0' && name[0] <= '9');

		return ((name[0] >= 'a' && name[0] <= 'z') || (name[0] >= 'A' && name[0] <= 'Z'));
	}

	int findChannel(std::string name){
		int i = 0;
		for (std::vector<Channel>::iterator it = this->_channel.begin(); it != this->_channel.end(); it++){
			if (it->getName() == name)
				return (i);
			i++;
		}
		return (-1);
	}

	std::string getNameChannel(std::string buf){
		char*	str = const_cast<char *>(buf.c_str());
		char *delimit = strchr(str + 6, ' ');
		if (delimit != NULL)
			*delimit = '\0';
		else{
			char *delimit = strchr(str + 6, '\r');
			if (delimit != NULL)
				*delimit = '\0';
		}

		for (int i = 0; i < 6; i++)
			str++;

		std::string tmp(str);
		std::string name = tmp.c_str();

		return (name);
	}

	void getNames(std::string buffer, int index) {
		char*	str = const_cast<char *>(buffer.c_str());
		if (!strncmp(str, "CAP LS 302", 10)){
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

	int	getIndexChannel(std::string name){
		int i = 0;
		for (std::vector<Channel>::iterator it = this->_channel.begin(); it != this->_channel.end(); it++){
			if (name == it->getName())
				return (i);
			i++;
		}
		return (-1);
	}

	void leaveChannel(User* x){
		std::string nameChannel = x->getChannel();
		this->_channel[findChannel(nameChannel)].deleteUser(x);
		x->setChannel("");

		std::stringstream ss;
		ss << "You are leaved " << nameChannel << " !" << std::endl;
		std::string out = ss.str();
		int bytes_sent = send(x->getfd(), out.c_str(), out.length(), 0);
		if (bytes_sent < 0)
			std::cerr << "Could not send leavechannel msg" << std::endl;

	}

	void joinChannel(User* x, int indexChannel){
		std::string nameChannel = this->_channel[indexChannel].getName();
		if (nameChannel == x->getChannel())
		{
			std::stringstream ss;
			ss << "You are already in " << "#" << nameChannel << " channel!" << std::endl;
			std::string out = ss.str();
			int bytes_sent = send(x->getfd(), out.c_str(), out.length(), 0);
			if (bytes_sent < 0)
				std::cerr << "Could not send" << std::endl;
			return;
		}

		leaveChannel(x);
		this->_channel[indexChannel].addUser(x);
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

		Channel	Default("Default");
		this->_channel.push_back(Default);

	}

	bool	occurName(User& x){
		for (std::vector<User>::iterator it = this->_users.begin(); it != this->_users.end(); it++)
		{
			if (x.getUserName() == it->getUserName())
				if (x.getPort() != it->getPort())
					return (true);
		}
		return (false);
	}

	void	removeUser(int indexUser){
		std::string channel = this->_users[indexUser].getChannel();

		leaveChannel(&(this->_users[indexUser]));
		std::cout << "Client at " << this->_users[indexUser].getIP() << ":" << this->_users[indexUser].getPort() << " has disconnected." << std::endl;
		std::vector<User>::iterator pos(&(this->_users[indexUser]));
		this->_users.erase(pos);
	}

	~Server(){}
	void addUser(){
		try {
			_users.push_back(User(_sock));
			this->_channel[0].addUser(&this->_users.back());
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
		if (_users[i].getFirstMsg() == false){
			getNames(str.c_str(), i);
			if (occurName(this->_users[i]) == true)
			removeUser(i);
		  }
		else {
			if (!str.compare(0, 4, "QUIT")){
				std::cout << "Client at " << _users[i].getIP() << ":" << _users[i].getPort() << " has disconnected." << std::endl;
				_users.erase(_users.begin() + i);
				if (_users.size() == 0){
					std::cout << "Shutting down socket." << std::endl;
					return 0;
				}
			}
			else if (!str.compare(0, 4, "LIST")){
				printChannels(i);
			}
			else if (!str.compare(0, 6, "JOIN #")){
				std::string nameChannel(getNameChannel(_buffer));
				if (allowChannelName(nameChannel) == true){
					int nbChannel = findChannel(nameChannel);
					if (nbChannel < 0){
						this->_channel.push_back(nameChannel);
						joinChannel(&(this->_users[i]), this->_channel.size() - 1);
					}
					else
						joinChannel(&(this->_users[i]), nbChannel);
				}
			}
			else{
	  		std::stringstream stream;
		  	std::cout << GREEN << _users[i].getUserName() << "(" << _users[i].getNickName() << ") : " << RESET << _buffer << std::endl; // to server
			  if (this->_users[i].getOperator() == true)
				  stream << "(Ops)";
		  	stream << _users[i].getUserName() << "(" << _users[i].getNickName() << ") : " << _buffer << std::endl; // to channel
		  	std::string msg = stream.str();
				int idx = getIndexChannel(this->_users[i].getChannel());
				this->_channel[idx].print(msg);
				}
		}
		return 1;
	}

	int run(){
		while (1) {
			poll(&_pfd, 1, 100);
			if (_pfd.revents == POLLIN){
				try {
					_users.push_back(User(_sock));
					this->_channel[0].addUser(&(this->_users.back()));
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
						if (!this->checkfd(i))
							return 0;
					}
				}
			}
		}
	}
};

#endif
